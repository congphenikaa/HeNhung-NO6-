#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ LCD là 0x27 (có thể thay đổi tùy vào màn hình)

// Khai báo Keypad
const byte ROW_NUM    = 4; // Four rows
const byte COLUMN_NUM = 4; // Four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Khai báo Servo Motor
Servo myServo;
int servoPin = 11;

// Mật khẩu
String correctPassword = "1234";  // Mật khẩu mặc định

int attemptCounter = 0;
int maxAttempts = 3;  // Số lần thử nhập sai

// Thời gian cửa mở
unsigned long doorOpenTime = 0;
bool doorIsOpen = false;

// Biến xác nhận mật khẩu
String enteredPassword = "";
bool passwordEntered = false;

void setup() {
  // Khởi tạo LCD
  lcd.begin(16, 2);
  lcd.backlight();
  
  // Khởi tạo Servo
  myServo.attach(servoPin);
  myServo.write(0);  // Đảm bảo cửa đóng lúc đầu
  
  // Hiển thị "Welcome"
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  delay(2000); // Hiển thị 2 giây
}

void loop() {
  char key = keypad.getKey();
  
  if (key == 'A') {
    enterPassword();
  }
  else if (key == 'C') {
    closeDoor();
  }
  else if (key == 'D') {
    resetSystem();
  }
  else if (key == 'B') {
    changePassword();  // Thực hiện đổi mật khẩu khi nhấn nút B
  }

  // Nếu cửa đang mở, kiểm tra thời gian để đóng cửa tự động
  if (doorIsOpen && millis() - doorOpenTime >= 10000) {
    closeDoor();
  }
}

void enterPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nhap mat khau:");

  enteredPassword = "";
  passwordEntered = false;

  while (!passwordEntered) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        passwordEntered = true;
        if (enteredPassword == correctPassword) {
          openDoor();
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Mat khau sai!");
          delay(2000);
          returnToMainScreen();
        }
      } 
      else if (key == '*') {  // Xóa ký tự
        if (enteredPassword.length() > 0) {
          enteredPassword.remove(enteredPassword.length() - 1);
          lcd.setCursor(enteredPassword.length(), 1);
          lcd.print(" ");
        }
      } else {  // Nhập ký tự
        enteredPassword += key;
        lcd.setCursor(enteredPassword.length() - 1, 1);
        lcd.print('*');
      }
    }
  }
}

void openDoor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nhap dung!");  // Mật khẩu đúng

  // Mở cửa (servo xoay 90 độ)
  myServo.write(90);
  doorOpenTime = millis();  // Lưu thời gian cửa mở
  doorIsOpen = true;
  delay(3000); // Cửa mở trong 3 giây

  // Hiển thị trạng thái cửa
  displayDoorStatus();
}

void displayDoorStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (doorIsOpen) {
    lcd.print("Cua dang mo");
  } else {
    lcd.print("Cua dang dong");
  }
}

void closeDoor() {
  if (doorIsOpen) {
    myServo.write(0);  // Đóng cửa lại
    doorIsOpen = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cua dong!");
    delay(2000);
    returnToMainScreen();
  }
}

void resetSystem() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Restarting...");
  delay(2000);
  asm volatile ("  jmp 0"); // Khởi động lại hệ thống
}

void returnToMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  delay(2000);  // Hiển thị "Welcome" trong 2 giây
}

void changePassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nhap mat khau cu:");

  // Nhập mật khẩu cũ để xác nhận
  String oldPassword = "";
  bool oldPasswordEntered = false;
  
  while (!oldPasswordEntered) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        oldPasswordEntered = true;
        if (oldPassword == correctPassword) {
          // Nhập mật khẩu mới
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Nhap mat khau moi:");
          
          String newPassword = "";
          bool newPasswordEntered = false;
          
          while (!newPasswordEntered) {
            key = keypad.getKey();
            if (key) {
              if (key == '#') {
                newPasswordEntered = true;
                // Yêu cầu người dùng nhập lại mật khẩu mới để xác nhận
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Xac nhan mat khau moi:");

                String confirmPassword = "";
                bool confirmPasswordEntered = false;
                
                while (!confirmPasswordEntered) {
                  key = keypad.getKey();
                  if (key) {
                    if (key == '#') {
                      confirmPasswordEntered = true;
                      if (newPassword == confirmPassword) {
                        correctPassword = newPassword;  // Cập nhật mật khẩu mới
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Mat khau moi da cap nhat!");
                        delay(2000);
                        returnToMainScreen();
                      } else {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Mat khau moi khong khop!");
                        delay(2000);
                        returnToMainScreen();
                      }
                    }
                    else if (key == '*') {  // Xóa ký tự
                      if (confirmPassword.length() > 0) {
                        confirmPassword.remove(confirmPassword.length() - 1);
                        lcd.setCursor(confirmPassword.length(), 1);
                        lcd.print(" ");
                      }
                    } else {  // Nhập ký tự
                      confirmPassword += key;
                      lcd.setCursor(confirmPassword.length() - 1, 1);
                      lcd.print('*');
                    }
                  }
                }
              }
              else if (key == '*') {  // Xóa ký tự
                if (newPassword.length() > 0) {
                  newPassword.remove(newPassword.length() - 1);
                  lcd.setCursor(newPassword.length(), 1);
                  lcd.print(" ");
                }
              } else {  // Nhập ký tự
                newPassword += key;
                lcd.setCursor(newPassword.length() - 1, 1);
                lcd.print('*');
              }
            }
          }
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Mat khau cu sai!");
          delay(2000);
          returnToMainScreen();
        }
      }
      else if (key == '*') {  // Xóa ký tự
        if (oldPassword.length() > 0) {
          oldPassword.remove(oldPassword.length() - 1);
          lcd.setCursor(oldPassword.length(), 1);
          lcd.print(" ");
        }
      } else {  // Nhập ký tự
        oldPassword += key;
        lcd.setCursor(oldPassword.length() - 1, 1);
        lcd.print('*');
      }
    }
  }
}
