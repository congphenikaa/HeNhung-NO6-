#include <Keypad.h>
// Define the number of rows and columns in the keypad
const byte ROW_NUM    = 4;   // Four rows
const byte COLUMN_NUM = 4;   // Four columns
// Define the keymap for the keypad (optional, this is for a 4x4 keypad)
char keys[ROW_NUM][COLUMN_NUM] = {
  {'A','3','2','1'},
  {'B','6','5','4'},
  {'C','9','8','7'},
  {'D','#','0','*'}
};

// Pinout for the rows and columns
byte pin_rows[ROW_NUM] = {2, 3, 4, 5};  //{R1, R2, R3, R4}
byte pin_column[COLUMN_NUM] = {9, 8, 7, 6}; //{L1, L2, L3, L4}
// Create an instance of the Keypad library
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 baud rate
}
void loop() {
  // Check if a key is pressed
  char key = keypad.getKey();
  
  if (key) {  // If a key is pressed
    Serial.print("Key pressed: ");
    Serial.println(key);
  }
}
