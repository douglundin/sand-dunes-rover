/*
 * Simple Serial Echo Test
 * 
 * This sketch will echo back any characters received over serial.
 * The built-in LED will also blink on each character received.
 */

void setup() {
  // Initialize serial communication at 9600 baud
  Serial.begin(9600);
  
  // Initialize the built-in LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Send a startup message
  Serial.println("Serial Echo Test Ready");
}

void loop() {
  // Check if data is available to read
  if (Serial.available() > 0) {
    // Read the incoming byte
    char incomingChar = Serial.read();
    
    // Echo the character back
    Serial.print("Received: ");
    Serial.println(incomingChar);
    
    // Blink the LED to visually confirm reception
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}