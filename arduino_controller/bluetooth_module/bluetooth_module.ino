#include <SoftwareSerial.h>

const int BT_RX_PIN = 4;  // HC-06 TX
const int BT_TX_PIN = 3;  // HC-06 RX
SoftwareSerial bluetoothSerial(BT_RX_PIN, BT_TX_PIN);

void setup() {
  Serial.begin(9600);
  bluetoothSerial.begin(9600);
  Serial.println("Ready");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (bluetoothSerial.available()) {
    char c = bluetoothSerial.read();
    Serial.write(c);  // Print to Serial Monitor
    digitalWrite(LED_BUILTIN, HIGH);  // Blink LED
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}