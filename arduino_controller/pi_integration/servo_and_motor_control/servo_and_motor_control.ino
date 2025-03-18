#include <SoftPWM.h>
#include <Servo.h>

// four motors
// _____   _____
// | 1 |   | 2 |
// _____   _____
//
// _____   _____
// | 3 |   | 4 |
// _____   _____
//

// Define which pins we want to use for PWM
// note: non default PWM pins (anything not 3 5 6 9 10 11) will need to be manually assigned as PWM pins

const int SERVO_1 = 13;     // servo 1 represents both servos
const int MOTOR_1_IN1 = 11; // front left wheel
const int MOTOR_1_IN2 = 10;
const int MOTOR_2_IN1 = 9; // front right wheel
const int MOTOR_2_IN2 = 8;
// motor 3 represents motors 3 & 4, as they will share the same rotation direction data
const int MOTOR_3_IN1 = 6;
const int MOTOR_3_IN2 = 5;

// define servo class for using servos
// note: our servos have a limited degree turning range and i believe it is 180
Servo servo_1;

// Speed settings (0-255)
const int DEFAULT_SPEED = 160;
const int TURN_SPEED = 210;

// servo baseline 'straight angle" value
// note: this will change based on how we attach physical motor housing to servo horn
const int SERVO_STRAIGHT = 140;
// servo turn margin angle
const int SERVO_MARGIN = 30;

void setup()
{
  // ==== Pin setup ====
  // Initialize the SoftPWM library for manual PWM assignment (don't need to include servo pins)
  SoftPWMBegin();

  // Parameters: pin, initial value (0-255), fade rate (optional)
  // Set up all pins with SoftPWM
  SoftPWMSet(MOTOR_1_IN1, 0); // pin 11
  SoftPWMSet(MOTOR_1_IN2, 0); // pin 10
  SoftPWMSet(MOTOR_2_IN1, 0); // pin 9
  SoftPWMSet(MOTOR_2_IN2, 0); // pin 8
  SoftPWMSet(MOTOR_3_IN1, 0); // pin 6
  SoftPWMSet(MOTOR_3_IN2, 0); // pin 5

  // Set fade time (optional, can be set to 0 for immediate response)
  SoftPWMSetFadeTime(ALL, 0, 0);

  // ==== Serial communication setup ====
  // Start UART communication at 9600 baud, 9600 bits per second
  Serial.begin(9600);

  // Initially stop motors
  stopMotors();

  Serial.println("Arduino ready to receive UART commands");

  // ==== Servo Setup ====
  servo_1.attach(13); // attaches the servo on pin 13 to the servo object
  // initially orient servos to 0 degrees
  servo_1.write(SERVO_STRAIGHT);
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
  // set servos to 0 degrees
  servo_1.write(SERVO_STRAIGHT);
  delay(50);

  // Motors receive a (IN1=1, IN2=0) signal, which is a FORWARD output as per above comments

  // Left front motor go FORWARD
  SoftPWMSet(MOTOR_1_IN1, DEFAULT_SPEED); // 1 signal for IN1
  SoftPWMSet(MOTOR_1_IN2, 0);             // 0 signal for IN2

  // Right front motor go FORWARD
  SoftPWMSet(MOTOR_2_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_2_IN2, 0);

  // Both back motors go FORWARD
  SoftPWMSet(MOTOR_3_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_3_IN2, 0);
}

void backward()
{
  // set servos to 0 degrees
  servo_1.write(SERVO_STRAIGHT);
  delay(50);

  // Left front motor go REVERSE
  SoftPWMSet(MOTOR_1_IN1, 0);             // 0 signal for IN1
  SoftPWMSet(MOTOR_1_IN2, DEFAULT_SPEED); // 1 signal for IN2

  // Right front motor go REVERSE
  SoftPWMSet(MOTOR_2_IN1, 0);
  SoftPWMSet(MOTOR_2_IN2, DEFAULT_SPEED);

  // Both back motors go REVERSE
  SoftPWMSet(MOTOR_3_IN1, 0);
  SoftPWMSet(MOTOR_3_IN2, DEFAULT_SPEED);
}

void left()
{
  // Note: we are only using two servos, on front wheels so we'll have to contend
  // with differential wheel spinning to execute proper turns
  // to do this, we use DEFAULT_SPEED and TURN_SPEED, where DEFAULT_SPEED < TURN_SPEED

  // initiate servos
  // move servos to position
  servo_1.write(SERVO_STRAIGHT - SERVO_MARGIN);
  delay(50);

  // front right motors go FORWARD, but slower
  SoftPWMSet(MOTOR_2_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_2_IN2, 0);

  // front left Motors go FORWARD
  SoftPWMSet(MOTOR_1_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_1_IN2, 0);

  // back motors go FORWARD
  SoftPWMSet(MOTOR_3_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_3_IN2, 0);
}

void right()
{
  // move servos to position
  servo_1.write(SERVO_STRAIGHT + SERVO_MARGIN);
  delay(50);

  // front left Motors go FORWARD
  SoftPWMSet(MOTOR_1_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_1_IN2, 0);

  // front right motors go FORWARD, but slower
  SoftPWMSet(MOTOR_2_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_2_IN2, 0);

  // back motors go FORWARD
  SoftPWMSet(MOTOR_3_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_3_IN2, 0);
}

void stopMotors()
{
  // front left motor go OFF
  SoftPWMSet(MOTOR_1_IN1, 0);
  SoftPWMSet(MOTOR_1_IN2, 0);

  // front right motor go OFF
  SoftPWMSet(MOTOR_2_IN1, 0);
  SoftPWMSet(MOTOR_2_IN2, 0);

  // back Motors go OFF
  SoftPWMSet(MOTOR_3_IN1, 0);
  SoftPWMSet(MOTOR_3_IN2, 0);
}

void brakeMotors()
{
  // front Left Motor go BRAKE
  SoftPWMSet(MOTOR_1_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_1_IN2, DEFAULT_SPEED);

  // front Right Motor go BRAKE
  SoftPWMSet(MOTOR_2_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_2_IN2, DEFAULT_SPEED);

  // back Motors go BRAKE
  SoftPWMSet(MOTOR_3_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_3_IN2, DEFAULT_SPEED);
}