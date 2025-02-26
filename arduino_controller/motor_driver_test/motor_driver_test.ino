// Motor A
const int MOTOR_A_IN1 = 3;  // PWM pin
const int MOTOR_A_IN2 = 9;  // PWM pin
// Motor B
const int MOTOR_B_IN1 = 10;  // PWM pin
const int MOTOR_B_IN2 = 11; // PWM pin

// Speed settings (0-255), 255 is 100% speed
const int DEFAULT_SPEED = 80;
const int TURN_SPEED = 80;     

void setup() {
  Serial.begin(9600);
  // Set all motor control pins as outputs
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  // Initially stop both motors
  stopMotors();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch(command) {
      case 'f': // Move Forward
        forward();
        break;
      case 'b': // Move Backward
        backward();
        break;
      case 'l': // Turn Left
        left();
        break;
      case 'r': // Turn Right
        right();
        break;
      case 's': // Stop
        stopMotors();
        break;
      case 't': // Tension (Brake)
        brakeMotors();
        break;
    }
    Serial.print("Executed command: ");
    Serial.println(command);
  }
}

// Modified motor control functions using analogWrite
void forward() {
  // Motor A forward
  analogWrite(MOTOR_A_IN1, DEFAULT_SPEED); // Writes an analog value (PWM wave) to a pin; 3, 9, 10, 11 are 490 Hz pins 5, 6 are 980 Hz
  analogWrite(MOTOR_A_IN2, 0);
  // Motor B forward
  analogWrite(MOTOR_B_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN2, 0);
}

void backward() {
  // Motor A backward
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, DEFAULT_SPEED);
  // Motor B backward
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, DEFAULT_SPEED);
}

void left() {
  // Motor A backward at turn speed
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, TURN_SPEED);
  // Motor B forward at turn speed
  analogWrite(MOTOR_B_IN1, TURN_SPEED);
  analogWrite(MOTOR_B_IN2, 0);
}

void right() {
  // Motor A forward at turn speed
  analogWrite(MOTOR_A_IN1, TURN_SPEED);
  analogWrite(MOTOR_A_IN2, 0);
  // Motor B backward at turn speed
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, TURN_SPEED);
}

void stopMotors() {
  // Stop both motors
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, 0);
}

void brakeMotors() {
  // Brake both motors with tension
  analogWrite(MOTOR_A_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_A_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN2, DEFAULT_SPEED);
}