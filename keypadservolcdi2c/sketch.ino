#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD address is 0x27 (it may change depending on the screen)

// Khai báo Keypad
const byte ROW_NUM = 4; // Four rows
const byte COLUMN_NUM = 4; // Four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'A','3','2','1'},
  {'B','6','5','4'},
  {'C','9','8','7'},
  {'D','#','0','*'}
};

// Pinout for the rows and columns
byte pin_rows[ROW_NUM] = {2, 3, 4, 5};  //{R1, R2, R3, R4}
byte pin_column[COLUMN_NUM] = {9, 8, 7, 6}; //{L1, L2, L3, L4}

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Khai báo Servo Motor
Servo myServo;
int servoPin = 11;

// Mật khẩu
String correctPassword = "1234";  // Default password

int attemptCounter = 0;
int maxAttempts = 3;  // Max incorrect attempts

// Thời gian cửa mở
unsigned long doorOpenTime = 0;
bool doorIsOpen = false;

// Biến xác nhận mật khẩu
String enteredPassword = "";
bool passwordEntered = false;

unsigned long lastAttemptTime = 0;
const unsigned long lockTime = 10000;  // Lock time in ms (30 seconds)
bool isLocked = false;  // Trạng thái khóa
unsigned long lockStartTime = 0;  // Thời gian bắt đầu khóa
unsigned long lastMillis = 0;  // Biến theo dõi thời gian trôi qua

unsigned long lastPasswordChangeAttempt = 0;
const unsigned long PASSWORD_CHANGE_TIMEOUT = 2000; // Thời gian chờ thay đổi mật khẩu

void setup() {
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  
  // Khởi tạo Servo
  myServo.attach(servoPin);
  myServo.write(0);  // Make sure the door is closed initially
  
  // Hiển thị "Welcome"
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome <3!");
  delay(1000); // Use standard delay for initial welcome
  Serial.begin(9600);  // Debugging via Serial Monitor
}

void loop() {
  char key = keypad.getKey();
  
  if (key == 'A') {
    enterPassword();
  } else if (key == 'C') {
    if (doorIsOpen) {
      closeDoor();  // Nếu cửa mở, đóng cửa
    } else {
      // Nếu cửa đang đóng, hiển thị thông báo
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Door is closed!");  // Hiển thị thông báo
      delay(1000);  // Dừng trong 2 giây để người dùng nhìn thấy thông báo
      returnToMainScreen();  // Quay lại màn hình chính
    }
  } else if (key == 'B') {
    changePassword();  // Change password when B is pressed
  }

  // Kiểm tra xem có đang bị khóa hay không
  if (isLocked) {
    unsigned long elapsedLockTime = millis() - lockStartTime;
    if (elapsedLockTime < lockTime) {
      // Nếu đang bị khóa, hiển thị trạng thái khóa chỉ một lần
      if (lastMillis == 0) {  // Kiểm tra lần đầu tiên khi bắt đầu khóa
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Locked for 10s");
        lastMillis = millis();  // Lưu thời gian hiển thị khóa
      }
      return;  // Không làm gì thêm khi bị khóa
    } else {
      isLocked = false;  // Sau khi hết 30 giây, mở khóa
      returnToMainScreen();  // Quay về màn hình chính
    }
  }

  // Kiểm tra nếu cửa đang mở, và nếu quá thời gian thì tự động đóng cửa
  if (doorIsOpen && millis() - doorOpenTime >= 10000) {
    closeDoor();
  }
}

void enterPassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter password:");

  enteredPassword = "";
  passwordEntered = false;

  // Reset the attempt counter for each password entry
  attemptCounter = 0;

  while (attemptCounter < maxAttempts) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Password submission
        passwordEntered = true;
        if (enteredPassword == correctPassword) {
          openDoor();  // Door opens if password is correct
          return;  // Exit the function after opening the door
        } else {
          attemptCounter++;  // Increment the wrong attempt counter
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wrong pwd!");
          delay(1000);  // Show wrong password message for 1 second
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter password:");  // Ask for password again
          enteredPassword = "";  // Clear entered password
          passwordEntered = false;  // Reset passwordEntered flag
          lastAttemptTime = millis(); // Record time of failed attempt
        }
      } 
      else if (key == '*') {  // Delete character
        if (enteredPassword.length() > 0) {
          enteredPassword.remove(enteredPassword.length() - 1);
          lcd.setCursor(enteredPassword.length(), 1);
          lcd.print(" ");
        }
      } else {  // Enter character
        enteredPassword += key;
        lcd.setCursor(enteredPassword.length() - 1, 1);
        lcd.print('*');
      }
    }
  }

  // Nếu vượt quá số lần thử sai, khoá 30 giây
  if (attemptCounter >= maxAttempts) {
    isLocked = true;  // Đặt trạng thái là bị khóa
    lockStartTime = millis();  // Lưu thời gian bắt đầu khóa
  }
}

void openDoor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct!");  // Password correct

  // Open door (servo rotates 90 degrees)
  myServo.write(90);
  doorOpenTime = millis();  // Store door open time
  doorIsOpen = true;
  delay(1000); // Door open for 1 second

  // Show door status
  displayDoorStatus();
}

void displayDoorStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (doorIsOpen) {
    lcd.print("Door open!");
  } else {
    lcd.print("Door closed!");
  }
}

void closeDoor() {
  if (doorIsOpen) {
    myServo.write(0);  // Close door
    doorIsOpen = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door closed!");
    delay(2000); // Display "Door closed" for 2 seconds
    returnToMainScreen();
  }
}

void returnToMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome <3!");
  delay(1000);  // Display "Welcome" for 2 seconds
}

void changePassword() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter old pwd:");

  // Enter old password to confirm
  String oldPassword = "";
  bool oldPasswordEntered = false;
  
  while (!oldPasswordEntered) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        oldPasswordEntered = true;
        if (oldPassword == correctPassword) {
          // Enter new password
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter new pwd:");
          
          String newPassword = "";
          bool newPasswordEntered = false;
          
          while (!newPasswordEntered) {
            key = keypad.getKey();
            if (key) {
              if (key == '#') {
                newPasswordEntered = true;
                // Ask user to re-enter new password for confirmation
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Confirm new pwd:");

                String confirmPassword = "";
                bool confirmPasswordEntered = false;
                
                while (!confirmPasswordEntered) {
                  key = keypad.getKey();
                  if (key) {
                    if (key == '#') {
                      confirmPasswordEntered = true;
                      if (newPassword == confirmPassword) {
                        correctPassword = newPassword;  // Update password
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Password updated!");
                        delay(2000);
                        returnToMainScreen();
                      } else {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Pwd mismatch!");
                        delay(2000);
                        returnToMainScreen();
                      }
                    }
                    else if (key == '*') {  // Delete character
                      if (confirmPassword.length() > 0) {
                        confirmPassword.remove(confirmPassword.length() - 1);
                        lcd.setCursor(confirmPassword.length(), 1);
                        lcd.print(" ");
                      }
                    } else {  // Enter character
                      confirmPassword += key;
                      lcd.setCursor(confirmPassword.length() - 1, 1);
                      lcd.print('*');
                    }
                  }
                }
              }
              else if (key == '*') {  // Delete character
                if (newPassword.length() > 0) {
                  newPassword.remove(newPassword.length() - 1);
                  lcd.setCursor(newPassword.length(), 1);
                  lcd.print(" ");
                }
              } else {  // Enter character
                newPassword += key;
                lcd.setCursor(newPassword.length() - 1, 1);
                lcd.print('*');
              }
            }
          }
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Old pwd wrong!");
          delay(2000);
          returnToMainScreen();
        }
      }
      else if (key == '*') {  // Delete character
        if (oldPassword.length() > 0) {
          oldPassword.remove(oldPassword.length() - 1);
          lcd.setCursor(oldPassword.length(), 1);
          lcd.print(" ");
        }
      } else {  // Enter character
        oldPassword += key;
        lcd.setCursor(oldPassword.length() - 1, 1);
        lcd.print('*');
      }
    }
  }
}
