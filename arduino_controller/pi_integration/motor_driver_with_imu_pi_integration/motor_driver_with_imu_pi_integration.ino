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
 *
 * Uses non-blocking timing instead of delay() to ensure commands are processed promptly
 */

#include <Wire.h> // Library for I2C communication with MPU6050

// Motor control pins (using PWM-capable pins)
const int MOTOR_L_IN1 = 5;  // Left motors forward
const int MOTOR_L_IN2 = 6;  // Left motors reverse
const int MOTOR_R_IN1 = 9;  // Right motors forward
const int MOTOR_R_IN2 = 10; // Right motors reverse

// Speed settings (0-255 for PWM)
const int DEFAULT_SPEED = 160; // Normal driving speed
const int TURN_SPEED = 80;     // Reduced speed for turning

// MPU6050 I2C address
const int MPU_ADDR = 0x68;

// Sensor variables
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

// Timing variables for non-blocking operation
unsigned long previousMillis = 0;
const long sensorInterval = 1000; // Interval for sensor readings (1 second)

void setup()
{
  Serial.begin(9600);

  Wire.begin();

  // Configure motor control pins as outputs
  pinMode(MOTOR_L_IN1, OUTPUT);
  pinMode(MOTOR_L_IN2, OUTPUT);
  pinMode(MOTOR_R_IN1, OUTPUT);
  pinMode(MOTOR_R_IN2, OUTPUT);

  // Wake up MPU6050 from sleep mode
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // Power management register
  Wire.write(0);    // Set to 0 to wake up
  Wire.endTransmission(true);

  stopMotors();

  Serial.println("Arduino ready to receive UART commands");
  Serial.println("MPU6050 Initialized");
  Serial.println("Sending data format: ax,ay,az,gx,gy,gz,temp");
}

void loop()
{
  // Check for incoming UART commands to control motors
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    executeCommand(command);
    Serial.print("Command executed: ");
    Serial.println(command);
  }

  // Non-blocking sensor reading based on time interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= sensorInterval)
  {
    previousMillis = currentMillis; // Save the last time we read the sensor

    // Read MPU6050 sensor data
    readMPU6050Data();

    // Convert raw temperature to Celsius
    float temp_celsius = temperature / 340.0 + 36.53;

    // Create a comma-separated string for transmission
    String data = String(accelerometer_x) + "," +
                  String(accelerometer_y) + "," +
                  String(accelerometer_z) + "," +
                  String(gyro_x) + "," +
                  String(gyro_y) + "," +
                  String(gyro_z) + "," +
                  String(temp_celsius, 2);

    // Send data over serial
    Serial.println(data);
  }
}

void readMPU6050Data()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Start at ACCEL_XOUT_H register
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14, true);

  accelerometer_x = Wire.read() << 8 | Wire.read();
  accelerometer_y = Wire.read() << 8 | Wire.read();
  accelerometer_z = Wire.read() << 8 | Wire.read();
  temperature = Wire.read() << 8 | Wire.read();
  gyro_x = Wire.read() << 8 | Wire.read();
  gyro_y = Wire.read() << 8 | Wire.read();
  gyro_z = Wire.read() << 8 | Wire.read();
}

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
    break;
  }
}

// Motor control functions
void forward()
{
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void backward()
{
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

void left()
{
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}

void right()
{
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, 0);
}

void stopMotors()
{
  analogWrite(MOTOR_R_IN1, 0);
  analogWrite(MOTOR_R_IN2, 0);
  analogWrite(MOTOR_L_IN1, 0);
  analogWrite(MOTOR_L_IN2, 0);
}

void brakeMotors()
{
  analogWrite(MOTOR_R_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_R_IN2, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN1, DEFAULT_SPEED);
  analogWrite(MOTOR_L_IN2, DEFAULT_SPEED);
}