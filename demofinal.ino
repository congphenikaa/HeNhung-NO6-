#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>
#include <EEPROM.h>

// Khai báo keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {D0, D1, D2, D3};
byte colPins[COLS] = {D4, D5, D6, D7};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khai báo RTC
RTC_DS3231 rtc;

// Khai báo Servo
Servo myservo;

// Lưu trữ thời gian cho ăn (giờ và phút)
int feedHour = 8, feedMinute = 0;
bool fedThisMinute = false; // Biến cờ để tránh lặp lại việc cho ăn trong cùng phút

void setup() {
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Pet Feeder");

  // Khởi tạo RTC
  while (!rtc.begin()) {
    lcd.clear();
    lcd.print("RTC Error");
    delay(1000);
  }

  // Đọc lịch trình từ EEPROM (nếu có)
  feedHour = EEPROM.read(0);
  feedMinute = EEPROM.read(1);

  // Khởi tạo Servo
  myservo.attach(D8);

  delay(2000);
  lcd.clear();
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (key == 'A') {
      feedPet(); // Cho thú cưng ăn ngay lập tức
    } else if (key == 'B') {
      setSchedule(); // Thiết lập lịch cho ăn
    } else if (key == 'C') {
      resetSchedule(); // Reset lịch về mặc định
    }
  }

  checkFeedingTime(); // Kiểm tra thời gian cho ăn
  displayCurrentTime(); // Hiển thị thời gian hiện tại
}

// Hàm cho thú cưng ăn
void feedPet() {
  lcd.clear();
  lcd.print("Feeding...");
  myservo.write(90); // Mở servo
  delay(1500); // Đợi 1.5 giây
  myservo.write(0); // Đóng servo
  delay(1000); // Đợi 1 giây
  lcd.clear();
}

// Hàm thiết lập lịch cho ăn
void setSchedule() {
  lcd.clear();
  lcd.print("Set HHMM:");

  int tempHour = 0, tempMinute = 0;
  String input = "";
  while (input.length() < 4) {
    char key = keypad.getKey();
    if (key && isDigit(key)) {
      input += key;
      lcd.setCursor(input.length(), 1);
      lcd.print(key);
    }
  }

  tempHour = (input[0] - '0') * 10 + (input[1] - '0');
  tempMinute = (input[2] - '0') * 10 + (input[3] - '0');

  if (tempHour >= 0 && tempHour < 24 && tempMinute >= 0 && tempMinute < 60) {
    feedHour = tempHour;
    feedMinute = tempMinute;
    EEPROM.write(0, feedHour); // Lưu giờ vào EEPROM
    EEPROM.write(1, feedMinute); // Lưu phút vào EEPROM
    lcd.clear();
    lcd.print("Saved: ");
    lcd.print(feedHour);
    lcd.print(":");
    lcd.print(feedMinute);
  } else {
    lcd.clear();
    lcd.print("Invalid Time!");
  }

  delay(2000);
  lcd.clear();
}

// Hàm reset lịch về mặc định
void resetSchedule() {
  feedHour = 8;
  feedMinute = 0;
  EEPROM.write(0, feedHour); // Lưu giờ mặc định vào EEPROM
  EEPROM.write(1, feedMinute); // Lưu phút mặc định vào EEPROM
  lcd.clear();
  lcd.print("Reset to 08:00");
  delay(2000);
  lcd.clear();
}

// Hàm kiểm tra thời gian cho ăn
void checkFeedingTime() {
  DateTime now = rtc.now();
  if (now.hour() == feedHour && now.minute() == feedMinute && !fedThisMinute) {
    feedPet(); // Cho ăn nếu đúng thời gian
    fedThisMinute = true; // Đánh dấu đã cho ăn trong phút này
  } else if (now.minute() != feedMinute) {
    fedThisMinute = false; // Reset cờ nếu sang phút mới
  }
}

// Hàm hiển thị thời gian hiện tại
void displayCurrentTime() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0"); // Thêm số 0 nếu phút < 10
  lcd.print(now.minute());
}
