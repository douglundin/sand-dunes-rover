/*
 * Enhanced Servo Control
 * Controls:
 * 's' - start sweep
 * 'p' - pause/stop sweep
 * 'r' - reset to 0 degrees
 * number (0-180) - move to specific degree
 */

#include <Servo.h>

Servo myservo;        // create servo object to control a servo
int pos = 0;          // variable to store the servo position
bool sweeping = false; // flag to control sweep state
bool forward = true;  // direction of sweep
String inputString = ""; // string to hold incoming data

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600); // initialize serial communication
  Serial.println("Controls:");
  Serial.println("'s' - start sweep");
  Serial.println("'p' - pause sweep");
  Serial.println("'r' - reset to 0 degrees");
  Serial.println("0-180 - move to specific degree");
}

void loop() {
  // Read serial input
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    
    // If newline, process the complete input
    if (inChar == '\n') {
      processInput(inputString);
      inputString = ""; // Clear the string for new input
    } else {
      inputString += inChar; // Add character to string
    }
  }

  // Perform sweep if enabled
  if (sweeping) {
    if (forward) {
      pos++;
      if (pos >= 180) {
        forward = false;
      }
    } else {
      pos--;
      if (pos <= 0) {
        forward = true;
      }
    }
    myservo.write(pos);
    delay(15); // dont overload microcontroller with commands 
  }
}

void processInput(String input) {
  // Remove any whitespace
  input.trim();
  
  // Convert to lowercase
  input.toLowerCase();
  
  // Check for commands
  if (input == "s") {
    sweeping = true;
    Serial.println("Sweeping started");
  }
  else if (input == "p") {
    sweeping = false;
    Serial.println("Sweeping paused");
  }
  else if (input == "r") {
    sweeping = false;    // Stop sweep if running
    pos = 0;            // Reset position to 0
    myservo.write(pos);
    Serial.println("Reset to 0 degrees");
  }
  // Try to convert input to number
  else {
    int degree = input.toInt();
    
    // Check if conversion was successful and in valid range
    if (degree >= 0 && degree <= 180) {
      sweeping = false; // Stop sweep if running
      pos = degree;     // Update position
      myservo.write(pos);
      Serial.print("Moving to position: ");
      Serial.println(degree);
    }
    else {
      Serial.println("Invalid input. Please enter:");
      Serial.println("'s' to start sweep");
      Serial.println("'p' to pause sweep");
      Serial.println("'r' to reset position");
      Serial.println("0-180 for specific degree");
    }
  }
}