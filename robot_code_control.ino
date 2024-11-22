// Motor Pins
const int LEFT_MOTOR_FORWARD = 5;
const int LEFT_MOTOR_BACKWARD = 6;
const int RIGHT_MOTOR_FORWARD = 9;
const int RIGHT_MOTOR_BACKWARD = 10;

// Ultrasonic Sensor Pins
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;

// Servo Pin
#include <Servo.h>
Servo grabberServo;
const int SERVO_PIN = 3;

// Constants
const int OBSTACLE_DISTANCE = 20; // in cm
const int SERVO_GRAB_POSITION = 45; // Adjust for grabbing
const int SERVO_RELEASE_POSITION = 0; // Adjust for release

void setup() {
  // Motor pins
  pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
  pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);
  pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
  pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);

  // Ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo setup
  grabberServo.attach(SERVO_PIN);
  grabberServo.write(SERVO_RELEASE_POSITION);

  // Serial communication
  Serial.begin(9600);
  Serial.println("Robot is ready.");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (isObstacleDetected()) {
      stopMotors();
      Serial.println("Obstacle detected! Stopping.");
      grabBottle(); // Grabbing sequence
    } else {
      if (command == "forward") {
        moveForward();
      } else if (command == "left") {
        turnLeft();
      } else if (command == "right") {
        turnRight();
      } else if (command == "stop") {
        stopMotors();
      }
    }
  }
}

// Functions for robot movement
void moveForward() {
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
  Serial.println("Moving forward.");
}

void turnLeft() {
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
  Serial.println("Turning left.");
}

void turnRight() {
  digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
  Serial.println("Turning right.");
}

void stopMotors() {
  digitalWrite(LEFT_MOTOR_FORWARD, LOW);
  digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
  digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
  digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
  Serial.println("Motors stopped.");
}

// Ultrasonic sensor function
bool isObstacleDetected() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2; // Convert to cm
  return distance < OBSTACLE_DISTANCE && distance > 0;
}

// Bottle grabbing sequence
void grabBottle() {
  Serial.println("Grabbing bottle...");
  grabberServo.write(SERVO_GRAB_POSITION);
  delay(2000); // Allow time to grab
  grabberServo.write(SERVO_RELEASE_POSITION);
  Serial.println("Bottle grabbed and released.");
}
