import cv2
import onnxruntime as ort
import numpy as np
import serial
import time

# Serial communication setup (e.g., Raspberry Pi to motor controller)
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 9600

# Initialize serial communication
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print("Serial connection established.")
except Exception as e:
    print(f"Serial connection failed: {e}")
    ser = None

# Load ONNX model
MODEL_PATH = "models/bottle_detection.onnx"
session = ort.InferenceSession(MODEL_PATH)

# Define COCO class names (only bottle ID is relevant for this project)
CLASS_NAMES = ["background", "bottle"]  # Add more if needed
CONFIDENCE_THRESHOLD = 0.3

# Initialize camera
camera = cv2.VideoCapture(0)
if not camera.isOpened():
    raise Exception("Could not open the camera.")

# Robot control functions
def send_command(command):
    if ser:
        ser.write((command + "\n").encode())
        print(f"Sent command: {command}")
    else:
        print(f"Command not sent (no serial): {command}")

def detect_objects(frame):
    # Preprocess the frame
    blob = cv2.dnn.blobFromImage(frame, 1/255.0, (640, 640), swapRB=True, crop=False)
    input_name = session.get_inputs()[0].name
    preds = session.run(None, {input_name: blob})[0]

    results = []
    for pred in preds:
        confidence = pred[4]
        if confidence > CONFIDENCE_THRESHOLD:
            class_id = int(pred[5:].argmax())
            if CLASS_NAMES[class_id] == "bottle":
                results.append({
                    "class_id": class_id,
                    "confidence": confidence,
                    "box": pred[:4]  # x, y, width, height
                })
    return results

def calculate_robot_action(detections, frame_width):
    if not detections:
        return "stop"

    # Take the first detection
    detection = detections[0]
    x, y, w, h = detection["box"]
    center_x = x + w / 2

    # Decide direction
    if center_x < frame_width / 3:
        return "left"
    elif center_x > 2 * frame_width / 3:
        return "right"
    else:
        return "forward"

# Main loop
try:
    while True:
        ret, frame = camera.read()
        if not ret:
            print("Failed to grab frame.")
            break

        frame_width = frame.shape[1]
        detections = detect_objects(frame)

        for det in detections:
            x, y, w, h = map(int, det["box"])
            confidence = det["confidence"]
            cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
            label = f"{CLASS_NAMES[det['class_id']]}: {confidence:.2f}"
            cv2.putText(frame, label, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # Decide action based on detections
        action = calculate_robot_action(detections, frame_width)
        send_command(action)

        # Display frame
        cv2.imshow("Bottle Detection", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("Interrupted by user.")
finally:
    camera.release()
    cv2.destroyAllWindows()
    if ser:
        ser.close()
