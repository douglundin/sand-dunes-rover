#include <Servo.h>


// DC MOTOR - LEFT side
const int LEFT_MOTOR_B_PIN = 6;  // forward
const int LEFT_MOTOR_F_PIN = 9;  // back
// DC MOTOR - RIGHT side
const int RIGHT_MOTOR_B_PIN = 5; // forward
const int RIGHT_MOTOR_F_PIN = 3; // back
//SERVO pin assignment
const int LEFT_SERVO_PIN  = 11;  
const int RIGHT_SERVO_PIN = 10; 
//speed
const int DEFAULT_SPEED = 160;  //  (0–255)
const int TURN_SPEED    = 160;  //  skid drive speed
const int SLOW_SPEED    = 120;  //  differential drive speed
//servo shi
const int LEFT_SERVO_CENTER  = 90;  
const int RIGHT_SERVO_CENTER = 90;  
const int SERVO_TURN_ANGLE   = 30;   // turn angle
// objects for Servos (for cleaner code)
Servo leftServo;
Servo rightServo;

// •••••••••••••••••••••••••••••••••••••••••••••••••
//                    SETUP
// •••••••••••••••••••••••••••••••••••••••••••••••••
void setup() {
  Serial.begin(9600);
  pinMode(LEFT_MOTOR_F_PIN, OUTPUT);
  pinMode(LEFT_MOTOR_B_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_F_PIN, OUTPUT);
  pinMode(RIGHT_MOTOR_B_PIN, OUTPUT);
  leftServo.attach(LEFT_SERVO_PIN);
  rightServo.attach(RIGHT_SERVO_PIN);
  steerZ();
  stopMotors();
  // (gay) little startup message because why not
  Serial.println("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••");
  Serial.println("•                                       Oberon INITIALIZED                                          •");
  Serial.println("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••");
  Serial.println("•                                                                                                   •");
  Serial.println("•     skidL, skidR, driveF, driveB, driveL, driveR, DdriveL, DriveR, steerL, steerR, steerZ, S      •");
  Serial.println("•                                                                                                   •");
  Serial.println("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••");
 Serial.println(" 'C' to show these commands again ");
}

// •••••••••••••••••••••••••••••••••••••••••••••••••
//              MAIN PROGRAM LOOP                     - checks for input from console (may have other ways of doing this in future)
// •••••••••••••••••••••••••••••••••••••••••••••••••
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "skidL") {
      skidL();
    } else if (command == "skidR") {
      skidR();
    } else if (command == "driveF") {
      driveF();
    } else if (command == "driveB") {
      driveB();
    } else if (command == "driveL") {
      driveL();
    } else if (command == "driveR") {
      driveR();
    } else if (command == "DdriveL") {
      DdriveL();
    } else if (command == "DriveR") {
      DriveR();
    } else if (command == "steerL") {
      steerL();
    } else if (command == "steerR") {
      steerR();
    } else if (command == "steerZ") {
      steerZ();
    } else if (command == "S") {
      stopMotors();
    } else if (command == "C") {
      showCommands();
    } else {
      Serial.print("Unknown command: ");
      Serial.println(command);
    }
    
    Serial.print("Executed command: ");
    Serial.println(command);
  }
}
// •••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
//                                                          FUNCTIONS
// •••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••
void showCommands() {
  Serial.println("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••");
  Serial.println("•                                                                                                   •");
  Serial.println("•    skidL, skidR, driveF, driveB, driveL, driveR, DdriveL, DriveR, steerL, steerR, steerZ, S, C    •");
  Serial.println("•                                                                                                   •");
  Serial.println("•••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••••");
}
// •••••••••••••••••••••••••••••••••••••••••••••••••
//       MOTOR CONTROL FUNCTIONS (DC only)
// •••••••••••••••••••••••••••••••••••••••••••••••••
void stopMotors() {
  analogWrite(LEFT_MOTOR_F_PIN, 0);
  analogWrite(LEFT_MOTOR_B_PIN, 0);
  analogWrite(RIGHT_MOTOR_F_PIN, 0);
  analogWrite(RIGHT_MOTOR_B_PIN, 0);
}
void driveF() {
  analogWrite(LEFT_MOTOR_F_PIN, DEFAULT_SPEED);
  analogWrite(LEFT_MOTOR_B_PIN, 0);
  analogWrite(RIGHT_MOTOR_F_PIN, DEFAULT_SPEED);
  analogWrite(RIGHT_MOTOR_B_PIN, 0);
}
void driveB() {
  analogWrite(LEFT_MOTOR_F_PIN, 0);
  analogWrite(LEFT_MOTOR_B_PIN, DEFAULT_SPEED);
  analogWrite(RIGHT_MOTOR_F_PIN, 0);
  analogWrite(RIGHT_MOTOR_B_PIN, DEFAULT_SPEED);
}
void skidL() {
  analogWrite(LEFT_MOTOR_F_PIN, 0);
  analogWrite(LEFT_MOTOR_B_PIN, TURN_SPEED);
  analogWrite(RIGHT_MOTOR_F_PIN, TURN_SPEED);
  analogWrite(RIGHT_MOTOR_B_PIN, 0);
}
void skidR() {
  analogWrite(LEFT_MOTOR_F_PIN, TURN_SPEED);
  analogWrite(LEFT_MOTOR_B_PIN, 0);
  analogWrite(RIGHT_MOTOR_F_PIN, 0);
  analogWrite(RIGHT_MOTOR_B_PIN, TURN_SPEED);
}
// •••••••••••••••••••••••••••••••••••••••••••••••••
//              DC + SERVO FUNCTIONS
// •••••••••••••••••••••••••••••••••••••••••••••••••
void driveL() {
  steerL();
  driveF();
}
void driveR() {
  steerR();
  driveF();
}
void DdriveL() {
  steerL();
  analogWrite(LEFT_MOTOR_F_PIN, SLOW_SPEED);
  analogWrite(LEFT_MOTOR_B_PIN, 0);
  
  analogWrite(RIGHT_MOTOR_F_PIN, DEFAULT_SPEED);
  analogWrite(RIGHT_MOTOR_B_PIN, 0);
}
void DriveR() {
  steerR();
  analogWrite(LEFT_MOTOR_F_PIN, DEFAULT_SPEED);
  analogWrite(LEFT_MOTOR_B_PIN, 0);
  
  analogWrite(RIGHT_MOTOR_F_PIN, SLOW_SPEED);
  analogWrite(RIGHT_MOTOR_B_PIN, 0);
}

// •••••••••••••••••••••••••••••••••••••••••••••••••
//               SERVO FUNCTIONS
// •••••••••••••••••••••••••••••••••••••••••••••••••
void steerL() {
  leftServo.write(LEFT_SERVO_CENTER - SERVO_TURN_ANGLE);
  rightServo.write(RIGHT_SERVO_CENTER + SERVO_TURN_ANGLE);
}
void steerR() {
  leftServo.write(LEFT_SERVO_CENTER + SERVO_TURN_ANGLE);
  rightServo.write(RIGHT_SERVO_CENTER - SERVO_TURN_ANGLE);
}
void steerZ() {
  leftServo.write(LEFT_SERVO_CENTER);
  rightServo.write(RIGHT_SERVO_CENTER);
}


