#include <SoftPWM.h>
#include <Servo.h>

// four servos
// _____   _____
// | 1 |   | 2 |
// _____   _____
//
// _____   _____
// | 3 |   | 4 |
// _____   _____
//


// six motors
// _____ _____
// | 1 | | 2 |
// _____ _____
//
// _____ _____
// | 3 | | 4 |
// _____ _____
//
// _____ _____
// | 5 | | 6 |
// _____ _____
//

// servo channels to breadboard pin
// channel 1: servo 1 servo 4
// channel 2: servo 2 servo 3

// motor channels
// channel 1: motor 1 (front left)
// channel 2: motor 2 (front right)
// channel 3: motor 5 (back left)
// channel 4: motor 6 (back right)
// channel 5: motor 3, motor 4 (middle motors)

/*
 * BREADBOARD CONNECTION GRID
 * =========================
 *
 *           Arduino Pin Numbers
 *    | 13 | 12 | 11 | 10 | 9  | 8  | 7  | 6  | 5  | 4  | 3  | 2  |
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * S1 | X  |    |    |    |    |    |    |    |    |    |    |    | Front Left (servo 1)
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * S2 |    | X  |    |    |    |    |    |    |    |    |    |    | Front Right (servo 2)
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * S3 |    | X  |    |    |    |    |    |    |    |    |    |    | Back Left (servo 3)
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * S4 | X  |    |    |    |    |    |    |    |    |    |    |    | Back Right (servo 4)
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M1 |    |    | X  | X  |    |    |    |    |    |    |    |    | Front Left
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M2 |    |    |    |    | X  | X  |    |    |    |    |    |    | Front Right
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M5 |    |    |    |    |    |    | X  | X  |    |    |    |    | Back Left
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M6 |    |    |    |    |    |    |    |    | X  | X  |    |    | Back Right
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M3 |    |    |    |    |    |    |    |    |    |    | X  | X  | Middle Left
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 * M4 |    |    |    |    |    |    |    |    |    |    | X  | X  | Middle Right
 * ---+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * Legend:
 * - S1-S4: Servo motors 1-4
 * - M1-M6: DC motors 1-6
 * - X: Connection point
 * - For DC motors, each motor uses two pins (IN1, IN2) for direction control
 *
 * Notes:
 * - Servos S1 & S4 share pin 13 (Channel 1)
 * - Servos S2 & S3 share pin 12 (Channel 2)
 * - Motors M3 & M4 share pins 3 & 2 (Channel 5)
 * - Power and ground connections not shown
 */


// Define pins we want to use for PWM, 

const int SERVO_CHANNEL_1  = 13;
const int SERVO_CHANNEL_2 = 12;

// motor channel 1
const int MOTOR_CHANNEL_1_IN1 = 11;
const int MOTOR_CHANNEL_1_IN2 = 10;

// motor channel 2
const int MOTOR_CHANNEL_2_IN1 = 9;
const int MOTOR_CHANNEL_2_IN2 = 8;

// motor channel 3
const int MOTOR_CHANNEL_3_IN1 = 7;
const int MOTOR_CHANNEL_3_IN2 = 6;

// motor channel 4
const int MOTOR_CHANNEL_4_IN1 = 5;
const int MOTOR_CHANNEL_4_IN2 = 4;

// motor channel 5
const int MOTOR_CHANNEL_5_IN1 = 3;
const int MOTOR_CHANNEL_5_IN2 = 2;

// define servo class for using servos
Servo servo_channel_1;
Servo servo_channel_2;

// Speed settings (0-255)
const int DEFAULT_SPEED = 50;
const int TURN_SPEED = 40;

// servo baseline 'straight angle" value
// note: this will change based on how we attach physical motor housing to servo horn
const int SERVO_STRAIGHT = 45
;
// servo turn margin angle
const int SERVO_MARGIN = 30;

const byte MOTOR_FADE_RATE = 10;

void setup()
{
  // ==== Pin setup ====
  // Initialize the SoftPWM library for manual PWM assignment (don't need to include servo pins)
  SoftPWMBegin();
  // Set up all motor control pins with SoftPWM
  // Parameters: pin, initial value (0-255), fade rate (optional)
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, 0, MOTOR_FADE_RATE);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 0, MOTOR_FADE_RATE);
  
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, 0, MOTOR_FADE_RATE);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 0, MOTOR_FADE_RATE);
  
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, 0, MOTOR_FADE_RATE);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 0, MOTOR_FADE_RATE);
  
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, 0, MOTOR_FADE_RATE);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 0, MOTOR_FADE_RATE);
  
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 0, MOTOR_FADE_RATE);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 0, MOTOR_FADE_RATE);
  
  // Set fade time (optional, can be set to 0 for immediate response)
  SoftPWMSetFadeTime(ALL, 0, 0);
  // ===========================

  // ==== Serial communication setup ====
  // Start UART communication at 9600 baud, 9600 bits per second
  Serial.begin(9600);
  Serial.println("Arduino ready to receive UART commands");
  // ====================================

  // ==== Servo Setup ====
  servo_channel_1.attach(13); // attaches the servo on pin 13 to the servo object
  servo_channel_2.attach(12); // attaches the servo on pin 13 to the servo object
  // initially orient servos to 0 degrees
  servo_channel_1.write(SERVO_STRAIGHT);
  servo_channel_2.write(SERVO_STRAIGHT);
  // =====================

  // Initially stop motors
  stopMotors();
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
  // Set servos to straight position (0 degrees)
  servo_channel_1.write(SERVO_STRAIGHT);
  servo_channel_2.write(SERVO_STRAIGHT);
  
  delay(50); // Short delay to allow servos to reach position

  // Motor channel 1 (Front Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 0);
  
  // Motor channel 2 (Front Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 0);
  
  // Motor channel 3 (Back Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 0);
  
  // Motor channel 4 (Back Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 0);
  
  // Motor channel 5 (Middle motors) - Forward
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, DEFAULT_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 0);
}

void backward()
{
  // Set servos to straight position (0 degrees)
  servo_channel_1.write(SERVO_STRAIGHT);
  servo_channel_2.write(SERVO_STRAIGHT);
  
  delay(50); // Short delay to allow servos to reach position

  // Motor channel 1 (Front Left) - Reverse
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, DEFAULT_SPEED);
  
  // Motor channel 2 (Front Right) - Reverse
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, DEFAULT_SPEED);
  
  // Motor channel 3 (Back Left) - Reverse
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, DEFAULT_SPEED);
  
  // Motor channel 4 (Back Right) - Reverse
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, DEFAULT_SPEED);
  
  // Motor channel 5 (Middle motors) - Reverse
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, DEFAULT_SPEED);
}
void left()
{
  // Configure servos for left turn
  // Servo channel 1 controls servo 1 (front left) and servo 4 (back right)
  // Servo channel 2 controls servo 2 (front right) and servo 3 (back left)
  
  // Turn servos in appropriate directions using the margin constant
  // For a left turn:
  // - Front wheels point left
  // - Back wheels point left
  servo_channel_1.write(SERVO_STRAIGHT + SERVO_MARGIN); // Front left and back right servos
  servo_channel_2.write(SERVO_STRAIGHT - SERVO_MARGIN); // Front right and back left servos
  
  delay(50); // Short delay to allow servos to reach position

  // Set motors to turn mode (typically at a reduced speed for better control)
  // Motor channel 1 (Front Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 0);
  
  // Motor channel 2 (Front Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 0);
  
  // Motor channel 3 (Back Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 0);
  
  // Motor channel 4 (Back Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 0);
  
  // Motor channel 5 (Middle motors) - Coast
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 0);
}

void right()
{
  // Configure servos for right turn
  // Servo channel 1 controls servo 1 (front left) and servo 4 (back right)
  // Servo channel 2 controls servo 2 (front right) and servo 3 (back left)
  
  // Turn servos in appropriate directions using the margin constant
  // For a right turn:
  // - Front wheels point right
  // - Back wheels point right
  servo_channel_1.write(SERVO_STRAIGHT - SERVO_MARGIN); // Front left and back right servos
  servo_channel_2.write(SERVO_STRAIGHT + SERVO_MARGIN); // Front right and back left servos
  
  delay(50); // Short delay to allow servos to reach position

  // Set motors to turn mode (typically at a reduced speed for better control)
  // Motor channel 1 (Front Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 0);
  
  // Motor channel 2 (Front Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 0);
  
  // Motor channel 3 (Back Left) - Forward
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 0);
  
  // Motor channel 4 (Back Right) - Forward
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, TURN_SPEED);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 0);
  
    // Motor channel 5 (Middle motors) - Coast
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 0);
}

void stopMotors()
{
  // Reset servos to straight position
  servo_channel_1.write(SERVO_STRAIGHT);
  servo_channel_2.write(SERVO_STRAIGHT);
  
  // Coast stop all motors (set both pins to 0)
  // Motor channel 1 (Front Left)
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 0);
  
  // Motor channel 2 (Front Right)
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 0);
  
  // Motor channel 3 (Back Left)
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 0);
  
  // Motor channel 4 (Back Right)
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 0);
  
  // Motor channel 5 (Middle motors)
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 0);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 0);
}

void brakeMotors()
{
  // Reset servos to straight position
  servo_channel_1.write(SERVO_STRAIGHT);
  servo_channel_2.write(SERVO_STRAIGHT);
  
  // Active brake all motors (set both pins to 1)
  // Motor channel 1 (Front Left)
  SoftPWMSet(MOTOR_CHANNEL_1_IN1, 255);
  SoftPWMSet(MOTOR_CHANNEL_1_IN2, 255);
  
  // Motor channel 2 (Front Right)
  SoftPWMSet(MOTOR_CHANNEL_2_IN1, 255);
  SoftPWMSet(MOTOR_CHANNEL_2_IN2, 255);
  
  // Motor channel 3 (Back Left)
  SoftPWMSet(MOTOR_CHANNEL_3_IN1, 255);
  SoftPWMSet(MOTOR_CHANNEL_3_IN2, 255);
  
  // Motor channel 4 (Back Right)
  SoftPWMSet(MOTOR_CHANNEL_4_IN1, 255);
  SoftPWMSet(MOTOR_CHANNEL_4_IN2, 255);
  
  // Motor channel 5 (Middle motors)
  SoftPWMSet(MOTOR_CHANNEL_5_IN1, 255);
  SoftPWMSet(MOTOR_CHANNEL_5_IN2, 255);
  
  // Short delay for active braking
  delay(100);
  
  // Then set to coast mode (both pins to 0) to avoid overheating the motors
  stopMotors();
}