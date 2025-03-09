/* Consolidated Arduino code for motor control and MPU6050 sensor data transmission
 * Features:
 * - Controls a six-wheel skid-steering robot
 * - Reads MPU6050 sensor data (accelerometer, gyroscope, temperature)
 * - Transmits data over USB to Raspberry Pi
 *
 * Hardware Connections:
 * - Motor control: PWM pins 5, 6, 9, 10 to H-bridge(s)
 * - MPU6050: VCC->5V, GND->GND, SCL->A5, SDA->A4
 * - USB connection to Raspberry Pi for data transmission
 */

/* Six-wheel Schematic Layout (Bird's eye view):
 * [front]  +-----+    +-----+
 *          |  L1 |    |  R1 |
 *          +-----+    +-----+
 *          |  L2 |    |  R2 |
 *          +-----+    +-----+
 *          |  L3 |    |  R3 |
 *          +-----+    +-----+
 * Legend: L1-L3: Left wheels, R1-R3: Right wheels
 * Note: Using skid steering (all left wheels as one group, all right as another)
 * Available PWM pins on Arduino R3: 3, 5, 6, 9, 10, 11
 */

#include <Wire.h>  // Library for I2C communication with MPU6050

// Motor control pins (using PWM-capable pins)
const int MOTOR_L_IN1 = 5;   // Left motors forward
const int MOTOR_L_IN2 = 6;   // Left motors reverse
const int MOTOR_R_IN1 = 9;   // Right motors forward
const int MOTOR_R_IN2 = 10;  // Right motors reverse

// Speed settings (0-255 for PWM)
const int DEFAULT_SPEED = 160;  // Normal driving speed
const int TURN_SPEED = 80;      // Reduced speed for turning (not currently used separately)

// MPU6050 I2C address (default 0x68, use 0x69 if AD0 pin is high)
const int MPU_ADDR = 0x68;

// Sensor variables (16-bit integers for raw MPU6050 data)
int16_t accelerometer_x, accelerometer_y, accelerometer_z;  // Acceleration in 3 axes
int16_t gyro_x, gyro_y, gyro_z;                            // Angular velocity in 3 axes
int16_t temperature;                                       // Raw temperature reading

void setup() {
  Serial.begin(9600);  // Start serial communication at 9600 baud for USB transmission
  
  // Initialize I2C for MPU6050 communication
  Wire.begin();

  // Configure motor control pins as outputs
  pinMode(MOTOR_L_IN1, OUTPUT);
  pinMode(MOTOR_L_IN2, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT);
  pinMode(MOTOR_R_IN2, OUTPUT);

  // Wake up MPU6050 from sleep mode
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Power management register
  Wire.write(0);     // Set to 0 to wake up
  Wire.endTransmission(true);

  stopMotors();  // Ensure motors start in stopped state
  
  // Initial status messages
  Serial.println("Arduino ready to receive UART commands");
  Serial.println("MPU6050 Initialized");
  Serial.println("Sending data format: ax,ay,az,gx,gy,gz,temp");
}

void loop() {
  // Check for incoming UART commands to control motors
  if (Serial.available() > 0) {
    char command = Serial.read();  // Read single character command
    executeCommand(command);       // Execute the corresponding motor action
    Serial.print("Command executed: ");
    Serial.println(command);
  }

  // Read MPU6050 sensor data
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // Start at ACCEL_XOUT_H register
  Wire.endTransmission(false);
  
  // Request 14 bytes (7 registers × 2 bytes each)
  Wire.requestFrom(MPU_ADDR, 14, true);

  /* Read sensor data using bitwise operations:
   * - Each value is 16-bit, sent as two 8-bit bytes
   * - << 8 shifts high byte left, | combines with low byte
   */
  accelerometer_x = Wire.read() << 8 | Wire.read();  // X-axis acceleration
  accelerometer_y = Wire.read() << 8 | Wire.read();  // Y-axis acceleration
  accelerometer_z = Wire.read() << 8 | Wire.read();  // Z-axis acceleration
  temperature = Wire.read() << 8 | Wire.read();      // Temperature
  gyro_x = Wire.read() << 8 | Wire.read();          // X-axis rotation
  gyro_y = Wire.read() << 8 | Wire.read();          // Y-axis rotation
  gyro_z = Wire.read() << 8 | Wire.read();          // Z-axis rotation

  // Convert raw temperature to Celsius using MPU6050 formula
  float temp_celsius = temperature / 340.0 + 36.53;

  // Create a comma-separated string for transmission
  String data = String(accelerometer_x) + "," +
                String(accelerometer_y) + "," +
                String(accelerometer_z) + "," +
                String(gyro_x) + "," +
                String(gyro_y) + "," +
                String(gyro_z) + "," +
                String(temp_celsius, 2);  // 2 decimal places for temperature

  // Send data over serial (USB) to Raspberry Pi
  Serial.println(data);

  delay(1000);  // Update and transmit every second
}

/* Motor Command Execution
 * Accepts single-character commands via UART:
 * 'f' - forward, 'b' - backward, 'l' - left, 'r' - right
 * 's' - stop, 't' - brake
 */
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

/* H-Bridge Motor Control Logic (e.g., HW-254 model)
 * IN1  IN2  |  Motor State
 * ---------------------
 * 1    0    |  FORWARD
 * 0    1    |  REVERSE
 * 1    1    |  BRAKE
 * 0    0    |  OFF (Coast)
 * 1 = HIGH (PWM value), 0 = LOW (0)
 */

// Motor control functions
void forward() {
  // All motors forward at default speed
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void backward() {
  // All motors reverse at default speed
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

void left() {
  // Right motors forward, left motors reverse (skid turn)
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

void right() {
  // Left motors forward, right motors reverse (skid turn)
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void stopMotors() {
  // All motors off (coast to stop)
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, 0);
}

void brakeMotors() {
  // All motors brake (active stop)
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}