/*
 * MPU6050 CALIBRATION ROUTINE
 *
 * This code calculates the sensor's offset values by taking multiple readings
 * while the device is stationary and determining the average deviation from
 * expected values. These offsets can then be applied to future readings.
 *
 * IMPORTANT: During calibration, the MPU6050 must remain completely still
 * on a flat, level surface!
 *
 * Connections:
 * MPU6050 VCC -> Arduino 5V
 * MPU6050 GND -> Arduino GND
 * MPU6050 SCL -> Arduino A5 (or SCL pin on boards with dedicated I2C pins)
 * MPU6050 SDA -> Arduino A4 (or SDA pin on boards with dedicated I2C pins)
 */

#include <Wire.h>

// MPU6050 I2C address (0x68 is default, 0x69 if AD0 pin is high)
const int MPU_ADDR = 0x68;

// Variables for calibration
long gyro_x_cal = 0;
long gyro_y_cal = 0;
long gyro_z_cal = 0;
long accel_x_cal = 0;
long accel_y_cal = 0;
long accel_z_cal = 0;
int cal_iterations = 1000;

// Variables to store sensor readings
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for Serial to be ready - particularly for Leonardo/Micro

  Serial.println("MPU6050 Calibration");

  // Initialize I2C communication
  Wire.begin();

  // Wake up the MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // Set to 0 to wake up
  Wire.endTransmission(true);

  // Configure gyroscope sensitivity (optional)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B); // GYRO_CONFIG register
  Wire.write(0x00); // Set to ±250 deg/s
  Wire.endTransmission(true);

  // Configure accelerometer sensitivity (optional)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); // ACCEL_CONFIG register
  Wire.write(0x00); // Set to ±2g
  Wire.endTransmission(true);

  Serial.println("MPU6050 initialized");
  delay(1000); // Give sensor time to stabilize

  // Run calibration
  calibrateMPU6050();

  // After calibration, start reading and showing calibrated values
  Serial.println("Now showing calibrated readings...");
  Serial.println("Format: ax, ay, az, gx, gy, gz");
}

void loop()
{
  // Read raw values
  readMPU6050();

  // Print raw values
  Serial.println("Raw readings:");
  Serial.print("Accel (raw): X = ");
  Serial.print(accelerometer_x);
  Serial.print(" | Y = ");
  Serial.print(accelerometer_y);
  Serial.print(" | Z = ");
  Serial.println(accelerometer_z);

  Serial.print("Gyro (raw): X = ");
  Serial.print(gyro_x);
  Serial.print(" | Y = ");
  Serial.print(gyro_y);
  Serial.print(" | Z = ");
  Serial.println(gyro_z);

  // Apply calibration and print calibrated values
  Serial.println("Calibrated readings:");
  Serial.print("Accel (cal): X = ");
  Serial.print(accelerometer_x - accel_x_cal);
  Serial.print(" | Y = ");
  Serial.print(accelerometer_y - accel_y_cal);
  Serial.print(" | Z = ");
  Serial.println(accelerometer_z - (accel_z_cal - 16384)); // Subtract offset but keep gravity

  Serial.print("Gyro (cal): X = ");
  Serial.print(gyro_x - gyro_x_cal);
  Serial.print(" | Y = ");
  Serial.print(gyro_y - gyro_y_cal);
  Serial.print(" | Z = ");
  Serial.println(gyro_z - gyro_z_cal);

  Serial.println("----------------------------");
  delay(1000);
}

void calibrateMPU6050()
{
  Serial.println("Calibrating MPU6050...");
  Serial.println("DO NOT MOVE THE SENSOR!");
  Serial.println("Calibration will start in 3 seconds...");
  delay(3000);

  // Take multiple readings for better accuracy
  for (int cal_count = 0; cal_count < cal_iterations; cal_count++)
  {
    // Read data
    readMPU6050();

    // Accumulate gyroscope readings
    gyro_x_cal += gyro_x;
    gyro_y_cal += gyro_y;
    gyro_z_cal += gyro_z;

    // Accumulate accelerometer readings
    accel_x_cal += accelerometer_x;
    accel_y_cal += accelerometer_y;
    accel_z_cal += accelerometer_z;

    // Show progress
    if (cal_count % 100 == 0)
    {
      Serial.print("Progress: ");
      Serial.print((cal_count * 100) / cal_iterations);
      Serial.println("%");
    }

    // Wait a bit between readings
    delay(3);
  }

  // Calculate average offsets
  gyro_x_cal /= cal_iterations;
  gyro_y_cal /= cal_iterations;
  gyro_z_cal /= cal_iterations;

  accel_x_cal /= cal_iterations;
  accel_y_cal /= cal_iterations;
  accel_z_cal /= cal_iterations;

  Serial.println("Calibration complete!");
  Serial.println("Gyroscope offsets:");
  Serial.print("X = ");
  Serial.println(gyro_x_cal);
  Serial.print("Y = ");
  Serial.println(gyro_y_cal);
  Serial.print("Z = ");
  Serial.println(gyro_z_cal);

  Serial.println("Accelerometer offsets:");
  Serial.print("X = ");
  Serial.println(accel_x_cal);
  Serial.print("Y = ");
  Serial.println(accel_y_cal);
  Serial.print("Z = ");
  Serial.println(accel_z_cal);
  Serial.println("(Z should be around 16384 if flat)");

  Serial.println("----------------------------");
}

void readMPU6050()
{
  // Start communication with MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);

  // Request 14 registers (each value is stored in 2 registers)
  Wire.requestFrom(MPU_ADDR, 14, true);

  // Read acceleration data
  accelerometer_x = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  // Read temperature
  temperature = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)

  // Read gyroscope data
  gyro_x = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}