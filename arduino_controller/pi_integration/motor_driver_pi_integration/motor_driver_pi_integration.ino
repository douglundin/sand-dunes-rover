// Motor control pins
const int MOTOR_A_IN1 = 3;  // PWM pin
const int MOTOR_A_IN2 = 9;  // PWM pin
const int MOTOR_B_IN1 = 10; // PWM pin
const int MOTOR_B_IN2 = 11; // PWM pin

// Speed settings (0-255)
const int DEFAULT_SPEED = 80;
const int TURN_SPEED = 80;

void setup() {
  // Start UART communication at 9600 baud
  Serial.begin(9600);
  
  // Set up motor control pins
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  
  // Initially stop motors
  stopMotors();
  
  Serial.println("Arduino ready to receive UART commands");
}

void loop() {
  // Check for incoming UART data
  if (Serial.available() > 0) {
    char command = Serial.read();  // Read one character
    executeCommand(command);
    Serial.print("Command executed: ");
    Serial.println(command);
  }
}

// Execute motor commands
void executeCommand(char command) {
  switch (command) {
    case 'f': forward(); break;
    case 'b': backward(); break;
    case 'l': left(); break;
    case 'r': right(); break;
    case 's': stopMotors(); break;
    case 't': brakeMotors(); break;
    default: break;  // Ignore unknown commands
  }
}

// Motor control functions
void forward() {
  analogWrite(MOTOR_A_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN2, 0);
}

void backward() {
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, DEFAULT_SPEED);
}

void left() {
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, TURN_SPEED);
  analogWrite(MOTOR_B_IN1, TURN_SPEED);
  analogWrite(MOTOR_B_IN2, 0);
}

void right() {
  analogWrite(MOTOR_A_IN1, TURN_SPEED);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, TURN_SPEED);
}

void stopMotors() {
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, 0);
}

void brakeMotors() {
  analogWrite(MOTOR_A_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_A_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_B_IN2, DEFAULT_SPEED);
}