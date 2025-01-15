//Mô tả hoạt động
//Khởi động hệ thống:
//Quét thẻ RFID:
//Nhập PIN:Menu sau khi mở két:Lựa chọn 1: Open Safe; Lựa chọn 2: Change PIN

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>  // Thêm thư viện EEPROM để lưu mật khẩu

// Pin definitions for RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Keypad setup
const byte ROW_NUM    = 4; // Four rows
const byte COLUMN_NUM = 4; // Four columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {5, 6, 7, 8};  // Arduino pins connected to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {A0, A1, A2, A3};  // Arduino pins connected to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2-line display

// Servo setup (for lock simulation)
Servo lockServo;
int servoPin = 4;

// Pin definitions for LEDs and Buzzer
int ledGreen = 3;  // Green LED for success
int ledRed = 2;    // Red LED for failure
int buzzerPin = 1; // Buzzer pin

// Predefined RFID card ID and PIN
String validCardID = "73711FFD";  // Thẻ của bạn
String validPin = "1234";          // Default PIN

// Max attempts
int maxCardAttempts = 3;           // Giới hạn số lần quẹt thẻ sai
int maxPinAttempts = 3;            // Giới hạn số lần nhập sai PIN
int cardAttemptCount = 0;          // Đếm số lần quẹt thẻ sai
int pinAttemptCount = 0;           // Đếm số lần nhập PIN sai

// Timeout settings
unsigned long lastTime = 0;
unsigned long timeout = 30000;     // 30 giây timeout cho nhập PIN hoặc thẻ

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  lcd.begin();
  lcd.print("Safe System");
  lcd.setCursor(0, 1);
  lcd.print("Scan your card");
  
  lockServo.attach(servoPin);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);

  // Load the saved PIN from EEPROM (if any)
  loadSavedPin();
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    lastTime = millis(); // Reset time when card is presented
    if (mfrc522.PICC_ReadCardSerial()) {
      String cardID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        cardID += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      Serial.println(cardID);
      lcd.clear();
      lcd.print("Card Detected");
      
      if (cardID == validCardID) {
        pinAttemptCount = 0;  // Reset pin attempts after successful card scan
        lcd.clear();
        lcd.print("Enter PIN:");
        
        String enteredPin = "";
        char key = keypad.getKey();
        
        while (key != '#') {
          if (key) {
            enteredPin += key;
            lcd.print('*');
            delay(300);
          }
          key = keypad.getKey();
          
          // Timeout after a period of inactivity
          if (millis() - lastTime > timeout) {
            lcd.clear();
            lcd.print("Timeout, try again");
            tone(buzzerPin, 1500, 500); // Timeout sound
            delay(2000);
            return;  // Exit and wait for new input
          }
        }
        
        if (enteredPin == validPin) {
          lcd.clear();
          lcd.print("Access Granted");
          digitalWrite(ledGreen, HIGH);
          lockServo.write(90);  // Open the safe (rotate servo)
          tone(buzzerPin, 1000, 500); // Success tone
          delay(3000); // Keep the safe open for 3 seconds
          lockServo.write(0);  // Close the safe
          digitalWrite(ledGreen, LOW);
          
          // Show menu after opening safe
          showMenu();
        } else {
          pinAttemptCount++;  // Increment PIN attempt count
          lcd.clear();
          lcd.print("Invalid PIN");
          digitalWrite(ledRed, HIGH);
          tone(buzzerPin, 2000, 500); // Error tone
          delay(2000); // Wait before resetting
          digitalWrite(ledRed, LOW);
          
          if (pinAttemptCount >= maxPinAttempts) {
            lcd.clear();
            lcd.print("Too many PIN errors");
            tone(buzzerPin, 3000, 500); // Error sound
            delay(5000); // Lock out for 5 seconds
            pinAttemptCount = 0;  // Reset the PIN attempt counter
          }
        }
      } else {
        cardAttemptCount++;  // Increment card attempt count
        lcd.clear();
        lcd.print("Invalid Card");
        digitalWrite(ledRed, HIGH);
        tone(buzzerPin, 2000, 500); // Error tone
        delay(2000);
        digitalWrite(ledRed, LOW);
        
        if (cardAttemptCount >= maxCardAttempts) {
          lcd.clear();
          lcd.print("Too many card errors");
          tone(buzzerPin, 3000, 500); // Error sound
          delay(5000); // Lock out for 5 seconds
          cardAttemptCount = 0;  // Reset the card attempt counter
        }
      }
    }
  }
}

void showMenu() {
  lcd.clear();
  lcd.print("1: Open Safe");
  lcd.setCursor(0, 1);
  lcd.print("2: Change PIN");

  char key = keypad.getKey();
  if (key == '1') {
    lcd.clear();
    lcd.print("Safe Opened");
    lockServo.write(90);  // Open the safe (rotate servo)
    tone(buzzerPin, 1000, 500); // Success tone
    delay(3000); // Keep the safe open for 3 seconds
    lockServo.write(0);  // Close the safe
  } 
  else if (key == '2') {
    changePin();  // Call function to change PIN
  }
}

void changePin() {
  lcd.clear();
  lcd.print("Enter Old PIN:");
  String oldPin = "";
  char key = keypad.getKey();
  
  while (key != '#') {
    if (key) {
      oldPin += key;
      lcd.print('*');
      delay(300);
    }
    key = keypad.getKey();
  }

  // Check if the old PIN is correct
  if (oldPin == validPin) {
    lcd.clear();
    lcd.print("Enter New PIN:");
    String newPin = "";
    key = keypad.getKey();
    
    while (key != '#') {
      if (key) {
        newPin += key;
        lcd.print('*');
        delay(300);
      }
      key = keypad.getKey();
    }

    // Save the new PIN to EEPROM
    validPin = newPin;
    savePinToEEPROM(validPin);
    lcd.clear();
    lcd.print("PIN Changed");
    tone(buzzerPin, 1000, 500); // Success tone
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Invalid Old PIN");
    tone(buzzerPin, 2000, 500); // Error tone
    delay(2000);
  }
}

// Function to save PIN to EEPROM
void savePinToEEPROM(String pin) {
  for (int i = 0; i < pin.length(); i++) {
    EEPROM.write(i, pin[i]);
  }
  EEPROM.write(pin.length(), '\0');  // Add null terminator to mark the end of the string
}

// Function to load saved PIN from EEPROM
void loadSavedPin() {
  String savedPin = "";
  for (int i = 0; i < 4; i++) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    savedPin += c;
  }
  if (savedPin.length() > 0) {
    validPin = savedPin;  // Update the valid PIN with the saved one
  }
}
