#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

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
const int redLED = 13;   // Đèn đỏ
const int yellowLED = 12; // Đèn vàng
const int greenLED = 10;  // Đèn xanh

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
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  myServo.attach(servoPin);
  myServo.write(0);
  
  Serial.begin(9600);
  returnMainScreen();  // Hiển thị màn hình chính khi khởi động
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
  digitalWrite(redLED, LOW); 
  digitalWrite(yellowLED, LOW); 
  digitalWrite(greenLED, LOW); 
  lcd.print("A:Open C:Close");
  lcd.setCursor(0, 1);
  lcd.print("B:ChangePassword");
}

// Hàm nhập mật khẩu (dùng chung)
String getPasswordInput(const char* prompt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);

  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') return input; // Nhập xong
      if (key == '*' && input.length() > 0) {  // Kiểm tra nếu có ký tự thì mới xóa
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
  digitalWrite(greenLED, HIGH);  // Bật đèn xanh khi cửa mở
  showMessage("Door open!", 1000);
}

void closeDoor() {
  myServo.write(0);
  doorIsOpen = false; 
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
  digitalWrite(redLED, HIGH); 
  showMessage("Locked for 10s", 10000);
  returnMainScreen();
}

void changePassword() {
  digitalWrite(yellowLED, HIGH); // Bật đèn vàng khi thay đổi mật khẩu

  String oldPassword = getPasswordInput("Enter old pwd:");

  if (oldPassword != correctPassword) {
    showMessage("Wrong old pwd!", 1000);
    returnMainScreen();
    return;
  }

  String newPassword = getPasswordInput("Enter new pwd:");
  String confirmPassword = getPasswordInput("Confirm new pwd:");

  if (newPassword == confirmPassword) {
    correctPassword = newPassword;
    showMessage("Password updated!", 1000);
  } else {
    showMessage("Mismatch!", 1000);
  }

  digitalWrite(yellowLED, LOW); // Tắt đèn vàng
  returnMainScreen();
}

// Hàm hiển thị tin nhắn trên LCD
void showMessage(const char* message, int delayMs) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(delayMs);
}
