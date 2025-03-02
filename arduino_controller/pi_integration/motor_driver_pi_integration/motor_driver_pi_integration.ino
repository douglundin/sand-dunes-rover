// Motor control pins

// this can represent one motor, but will send this signal to all LEFT-side motors on robot
// thus, these pins represent the LEFT wheels
const int MOTOR_A_IN1 = 3;  // PWM pin
const int MOTOR_A_IN2 = 9;  // PWM pin


// this can represent one motor, but will send this signal to all RIGHT-side motors on robot
// thus, these pins represent the RIGHT wheels
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


// H-Bridge Motor Controller - Single Motor Control (HW 254 is our model that uses an h bridge)

// - You provide high-level commands (direction and speed)
// - The microcontroller converts these into just two binary outputs (IN1 and IN2)
// - The H-bridge driver circuit takes these two signals and internally handles the activation of all four switches
// - Each combination of these two signals creates a different current flow pattern through 
//   the four switches of the H-bridge, determining how current flows through the motor
//   or if it flows at all.
// - The IN recieves PWM signals over pins from arduino
// - Each HW 254 contains two motor control modules (thus IN1 IN2 for motor A, and IN3 IN4 for motor B)

// IN1  IN2  |  Motor State
// ---------------------
// 1    0    |  FORWARD
// 0    1    |  REVERSE
// 1    1    |  BRAKE
// 0    0    |  OFF (Coast)

// 1 = HIGH/ON (3.3V or 5V), 0 = LOW/OFF (0V or ground)


// Motor control functions
void forward() {
  // Motor A and B recieve a (1, 0) signal, which is a forward output
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