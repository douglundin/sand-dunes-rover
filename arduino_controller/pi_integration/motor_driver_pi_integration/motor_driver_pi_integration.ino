// Motor control pins

/** availabel arduino r3 pwm pins(six total): 3 5 6 9 10 11

/**
 * birds eye view Six-wheel Schematic Layout
 *  [front of robot]
 * +-----+    +-----+
 * |     |    |     |
 * |  L1 |    |  R1 |
 * |     |    |     |
 * +-----+    +-----+
 *
 * +-----+    +-----+
 * |     |    |     |
 * |  L2 |    |  R2 |
 * |     |    |     |
 * +-----+    +-----+
 *
 * +-----+    +-----+
 * |     |    |     |
 * |  L3 |    |  R3 |
 * |     |    |     |
 * +-----+    +-----+
 *
 * Legend:
 * L1-L3: Left wheels (top to bottom)
 * R1-R3: Right wheels (top to bottom)
 */

// IMPORTANT: need servos connected to system, until then, will implement skid steering
// all left wheels L1-L3 will be assigned L, and R1 - R3 will be R

const int MOTOR_L_IN1 = 5; // PWM pin
const int MOTOR_L_IN2 = 6; // PWM pin

const int MOTOR_R_IN1 = 9;  // PWM pin
const int MOTOR_R_IN2 = 10; // PWM pin

// Speed settings (0-255)
const int DEFAULT_SPEED = 160;
const int TURN_SPEED = 80;

void setup()
{
  // Start UART communication at 9600 baud
  Serial.begin(9600);
  
  // Set up motor control pins
  pinMode(MOTOR_L_IN1, OUTPUT);
  pinMode(MOTOR_L_IN2, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT);
  pinMode(MOTOR_R_IN2, OUTPUT);

  // Initially stop motors
  stopMotors();

  Serial.println("Arduino ready to receive UART commands");
}

void loop()
{
  // Check for incoming UART data
  if (Serial.available() > 0)
  {
    char command = Serial.read(); // Read one character
    executeCommand(command);
    Serial.print("Command executed: ");
    Serial.println(command);
  }
}

// Execute motor commands
void executeCommand(char command)
{
  switch (command)
  {
  case 'f':
    forward();
    break;
  case 'b':
    backward();
    break;
  case 'l':
    left();
    break;
  case 'r':
    right();
    break;
  case 's':
    stopMotors();
    break;
  case 't':
    brakeMotors();
    break;
  default:
    break; // Ignore unknown commands
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
// 1    0    |  FORWARD (may be reversed, it's based of current direction, will always spin opposite of REVERSE)
// 0    1    |  REVERSE (may be reversed, it's based of current direction, will always spin opposite of FORWARD)
// 1    1    |  BRAKE
// 0    0    |  OFF (Coast)

// 1 = HIGH/ON (3.3V or 5V), 0 = LOW/OFF (0V or ground)

// Motor control functions

void forward()
{
  // Motors recieve a (IN1=1, IN2=0) signal, which is a FORWARD output as per above comments

  // Right motors go FORWARD
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);

  // Left Motors go FORWARD
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void backward()
{
  // Motors recieve a (IN1=0, IN2=1) signal, which is a FORWARD output as per above comments

  // Right motors go REVERSE
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);

  // Left Motors go REVERSE
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

// right and left will be redefined when servos get integrated

void left()
{
  // Right motors go REVERSE
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);

  // Left Motors go FORWARD
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void right()
{
  // Right motors go FORWARD
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);

  // Left Motors go REVERSE
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

void stopMotors()
{
  // Right motors go OFF
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, 0);

  // Left Motors go OFF
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, 0);
}

void brakeMotors()
{
  // Right Motors go BRAKE
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  // Left Motors go BRAKE
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}