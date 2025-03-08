/*
 * QMC5883L Compass Sensor Demo
 *
 * Connections:
 * Arduino GND -> GY-271/QMC5883L GND
 * Arduino 3.3V -> GY-271/QMC5883L VCC
 * Arduino A4 (SDA) -> GY-271/QMC5883L SDA
 * Arduino A5 (SCL) -> GY-271/QMC5883L SCL
 */

#include <Wire.h>

#define QMC5883L_ADDR 0x0D  // I2C address of QMC5883L

bool haveQMC5883L = false;

// Function to detect QMC5883L
bool detectQMC5883L() {
  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x00);  // Select data output register to test communication
  if (Wire.endTransmission() == 0) {  // Successful communication
    return true;
  }
  Serial.println("I2C communication failed");
  return false;
}

void setup() {
  Serial.begin(9600);
  Serial.println("QMC5883L Sensor Test");
  Wire.begin();

  // Initial sensor detection
  if (detectQMC5883L()) {
    haveQMC5883L = true;
    Serial.println("QMC5883L detected, initializing...");

    // Configure QMC5883L
    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(0x09);  // Control Register 1
    Wire.write(0x1D);  // Mode: Continuous, ODR: 200Hz, Range: 8G, OSR: 512
    Wire.endTransmission();

    Wire.beginTransmission(QMC5883L_ADDR);
    Wire.write(0x0B);  // SET/RESET Period Register
    Wire.write(0x01);  // Recommended value for stable operation
    Wire.endTransmission();
  } else {
    Serial.println("No QMC5883L sensor detected! Check connections.");
  }
}

void loop() {
  if (!haveQMC5883L) {
    if (detectQMC5883L()) {
      haveQMC5883L = true;
      Serial.println("QMC5883L connected!");

      // Reconfigure the sensor
      Wire.beginTransmission(QMC5883L_ADDR);
      Wire.write(0x09);
      Wire.write(0x1D);  // Continuous mode, 200Hz, 8G, 512 OSR
      Wire.endTransmission();

      Wire.beginTransmission(QMC5883L_ADDR);
      Wire.write(0x0B);
      Wire.write(0x01);
      Wire.endTransmission();
    } else {
      Serial.println("No QMC5883L detected!");
      delay(1000);
      return;
    }
  } else {
    if (!detectQMC5883L()) {
      haveQMC5883L = false;
      Serial.println("Lost connection to QMC5883L!");
      delay(1000);
      return;
    }
  }

  // Read magnetic data
  int x, y, z;

  Wire.beginTransmission(QMC5883L_ADDR);
  Wire.write(0x00);  // Start reading from X LSB register
  Wire.endTransmission();

  Wire.requestFrom(QMC5883L_ADDR, 6);
  if (6 <= Wire.available()) {
    x = Wire.read();       // X LSB
    x |= Wire.read() << 8; // X MSB
    y = Wire.read();       // Y LSB
    y |= Wire.read() << 8; // Y MSB
    z = Wire.read();       // Z LSB
    z |= Wire.read() << 8; // Z MSB

    // Print values
    Serial.print("x: ");
    Serial.print(x);
    Serial.print("  y: ");
    Serial.print(y);
    Serial.print("  z: ");
    Serial.println(z);
  } else {
    Serial.println("Failed to read data!");
    haveQMC5883L = false;
  }

  delay(250);
}