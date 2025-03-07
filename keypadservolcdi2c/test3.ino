#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad
const byte ROW_NUM = 4;
const byte COLUMN_NUM = 4;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'A','3','2','1'},
  {'B','6','5','4'},
  {'C','9','8','7'},
  {'D','#','0','*'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};  
byte pin_column[COLUMN_NUM] = {2, 3, 4, 5};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Servo Motor
Servo myServo;
const int servoPin = 11;

// Mật khẩu
String correctPassword = "1234";

// Giới hạn thử mật khẩu
const int maxAttempts = 3;
int attemptCounter = 0;

// Trạng thái cửa
unsigned long doorOpenTime = 0;
bool doorIsOpen = false;

// Trạng thái khóa
bool isLocked = false;
unsigned long lockStartTime = 0;
const unsigned long lockTime = 10000; // 10s khóa

void setup() {
  lcd.init();
  lcd.backlight();
  myServo.attach(servoPin);
  myServo.write(0);
  
  Serial.begin(9600);
  loadPasswordFromEEPROM();
  returnMainScreen();
}

void loop() {
  if (isLocked && millis() - lockStartTime < lockTime) {
    return;
  }
  isLocked = false;

  char key = keypad.getKey();
  
  if (key == 'A') {
    enterPassword();
  } else if (key == 'C') {
    doorIsOpen ? closeDoor() : showMessage("Door is closed!", 1000);
  } else if (key == 'B') {
    changePassword();
  }

  if (doorIsOpen && millis() - doorOpenTime >= 10000) {
    closeDoor();
  }
}

// Hàm quay về màn hình chính
void returnMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:Open C:Close");
  lcd.setCursor(0, 1);
  lcd.print("B:ChangePassword");
}

// Hàm nhập mật khẩu
String getPasswordInput(const char* prompt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);

  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') return input; // Nhập xong
      if (key == '*' && input.length() > 0) {  // Xóa ký tự cuối
        input.remove(input.length() - 1);
      } else if (key != '*') {
        input += key;
      }

      lcd.setCursor(0, 1);
      lcd.print("                ");  // Xóa dòng cũ
      lcd.setCursor(0, 1);
      for (int i = 0; i < input.length(); i++) {
        lcd.print('*');
      }
    }
  }
}

void openDoor() {
  showMessage("Correct!", 1000);
  myServo.write(90);
  doorOpenTime = millis();
  doorIsOpen = true;
  showMessage("Door open!", 1000);
}

void closeDoor() {
  doorIsOpen = false; 
  myServo.write(0);
  showMessage("Door closed!", 1000);
  returnMainScreen();
}

void enterPassword() {
  attemptCounter = 0;
  
  while (attemptCounter < maxAttempts) {
    String enteredPassword = getPasswordInput("Enter password:");

    if (enteredPassword == correctPassword) {
      openDoor();
      return;
    } else {
      showMessage("Wrong password!", 1000);
      attemptCounter++;
    }
  }

  // Khóa sau quá số lần thử
  isLocked = true;
  lockStartTime = millis();
  showMessage("Locked for 10s", 10000);
  returnMainScreen();
}

void changePassword() {
  String oldPassword = getPasswordInput("Enter old pwd:");

  if (oldPassword != correctPassword) {
    showMessage("Wrong old pwd!", 1000);
    returnMainScreen();
    return;
  }

  String newPassword = getPasswordInput("Enter new pwd:");
  if (newPassword == correctPassword) {
      showMessage("New pwd same as old!", 1000);  // Kiểm tra nếu mật khẩu mới trùng với mật khẩu cũ
      returnMainScreen();
      return;
  }
  String confirmPassword = getPasswordInput("Confirm new pwd:");

  if (newPassword == confirmPassword) {
    correctPassword = newPassword;
    savePasswordToEEPROM();
    showMessage("Password updated!", 1000);
  } else {
    showMessage("Mismatch!", 1000);
  }
  returnMainScreen();
}

// Hàm lưu mật khẩu vào EEPROM
void savePasswordToEEPROM() {
  int len = correctPassword.length();
  EEPROM.write(0, len);  // Lưu độ dài mật khẩu vào địa chỉ 0
  for (int i = 0; i < len; i++) {
    EEPROM.write(i + 1, correctPassword[i]);  // Lưu từng ký tự mật khẩu
  }
}

// Hàm đọc mật khẩu từ EEPROM
void loadPasswordFromEEPROM() {
  int len = EEPROM.read(0);
  if (len < 1 || len > 10) { 
    correctPassword = "1234";
    savePasswordToEEPROM();
  } else {
    correctPassword = "";
    for (int i = 0; i < len; i++) {
      correctPassword += char(EEPROM.read(i + 1));
    }
  }
}

void showMessage(const char* message, int delayMs) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(delayMs);
}
