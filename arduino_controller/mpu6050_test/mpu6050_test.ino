/*
 * MPU6050 Basic Reading
 *
 * This sketch reads accelerometer and gyroscope values from an MPU6050 sensor
 * and outputs the data to the Serial Monitor.
 *
 * Connections:
 * MPU6050 VCC -> Arduino 5V
 * MPU6050 GND -> Arduino GND
 * MPU6050 SCL -> Arduino A5 (or SCL pin on boards with dedicated I2C pins)
 * MPU6050 SDA -> Arduino A4 (or SDA pin on boards with dedicated I2C pins)
 *
 * Library dependencies:
 * - Wire.h (built-in)
 */

#include <Wire.h>

// MPU6050 I2C address (0x68 is default, 0x69 if AD0 pin is high)
const int MPU_ADDR = 0x68;

// Variables to store sensor readings
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

/*
 * ===== BEGINNER'S GUIDE TO BITWISE OPERATIONS IN THIS CODE =====
 *
 * WHAT IS I2C COMMUNICATION?
 * The MPU6050 communicates with Arduino using I2C protocol, which is a serial communication method
 * where devices talk to each other using just two wires (SDA for data and SCL for clock).
 *
 * WHY BITWISE OPERATIONS?
 * The MPU6050 stores each sensor value as a 16-bit number, but sends it as two separate 8-bit bytes.
 * We need to combine these two bytes to get our original 16-bit value.
 *
 * WHAT IS A BYTE AND A REGISTER?
 * - A byte is 8 bits of data (like 10101010)
 * - A register is a storage location in the MPU6050 that holds a byte of data
 * - The MPU6050 stores each sensor value across two registers (HIGH byte and LOW byte)
 *
 * BITWISE OPERATIONS EXPLAINED:
 *
 * 1. Left shift (<<):
 *    Wire.read() << 8
 *    This takes the first byte we read (the HIGH byte) and shifts it 8 positions to the left.
 *
 *    Example:
 *    If Wire.read() returns 00101101 (HIGH byte)
 *    After shifting: 00101101 00000000
 *
 *    This makes room for the second byte.
 *
 * 2. Bitwise OR (|):
 *    (Wire.read() << 8) | Wire.read()
 *    This combines the shifted HIGH byte with the LOW byte using OR operation.
 *
 *    Example:
 *    Shifted HIGH byte: 00101101 00000000
 *    LOW byte:                   11001010
 *    Result after OR:   00101101 11001010
 *
 *    This gives us our complete 16-bit value that represents the sensor reading.
 *
 * WHY DO WE READ REGISTERS IN A SPECIFIC ORDER?
 * The MPU6050 stores data in specific memory addresses (registers):
 * - Accelerometer data starts at register 0x3B
 * - Temperature data follows the accelerometer data
 * - Gyroscope data follows the temperature data
 *
 * We tell the MPU6050 which register to start from, then read bytes in sequence.
 */

void setup()
{
  // Initialize serial communication between Arduino and your computer
  Serial.begin(9600);

  // Initialize I2C communication between Arduino and MPU6050
  Wire.begin();

  // Wake up the MPU6050 from sleep mode
  Wire.beginTransmission(MPU_ADDR); // Start communication with MPU6050
  Wire.write(0x6B);                 // Access the power management register (0x6B)
  Wire.write(0);                    // Set to 0 to wake up the MPU6050
  Wire.endTransmission(true);       // End the transmission

  Serial.println("MPU6050 Initialized");
  Serial.println("Format: ax, ay, az, gx, gy, gz, temp(°C)");
}

void loop()
{
  // Start communication with MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);            // Tell MPU6050 we want to read starting from register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false); // End transmission but keep the connection active

  // Request 14 registers (each value is stored in 2 registers for a total of 7 values)
  // (3 accelerometer values + 1 temperature value + 3 gyroscope values) * 2 bytes each = 14 bytes
  Wire.requestFrom(MPU_ADDR, 14, true);

  // Read X-axis acceleration data
  // The value is stored in two registers: HIGH byte first, then LOW byte
  accelerometer_x = Wire.read() << 8 | Wire.read();
  // Step by step:
  // 1. Wire.read() reads the HIGH byte (ACCEL_XOUT_H)
  // 2. << 8 shifts this byte 8 positions left (making it 0xXX00)
  // 3. Wire.read() reads the LOW byte (ACCEL_XOUT_L)
  // 4. | combines the two bytes with OR operation (making it 0xXXYY)

  // Repeat for Y and Z acceleration axes
  accelerometer_y = Wire.read() << 8 | Wire.read();
  accelerometer_z = Wire.read() << 8 | Wire.read();

  // Read temperature (stored in 2 bytes)
  temperature = Wire.read() << 8 | Wire.read();

  // Read X, Y, and Z gyroscope data (each stored in 2 bytes)
  gyro_x = Wire.read() << 8 | Wire.read();
  gyro_y = Wire.read() << 8 | Wire.read();
  gyro_z = Wire.read() << 8 | Wire.read();

  // Convert raw temperature data to degrees Celsius
  // The formula comes from the MPU6050 datasheet
  float temp_celsius = temperature / 340.0 + 36.53;

  // Print accelerometer values
  Serial.print("Accelerometer (raw): ");
  Serial.print("X = ");
  Serial.print(accelerometer_x);
  Serial.print(" | Y = ");
  Serial.print(accelerometer_y);
  Serial.print(" | Z = ");
  Serial.println(accelerometer_z);

  // Print gyroscope values
  Serial.print("Gyroscope (raw): ");
  Serial.print("X = ");
  Serial.print(gyro_x);
  Serial.print(" | Y = ");
  Serial.print(gyro_y);
  Serial.print(" | Z = ");
  Serial.println(gyro_z);

  // Print temperature
  Serial.print("Temperature: ");
  Serial.print(temp_celsius);
  Serial.println(" °C");

  Serial.println("----------------------------");

  // Wait before reading again
  delay(1000);
}