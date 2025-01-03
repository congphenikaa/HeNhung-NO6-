#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <RTClib.h>

// Khai báo LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ I2C của LCD

// Khai báo RTC DS3231
RTC_DS3231 rtc;

// Khai báo keypad 4x4
const byte ROW_NUM    = 4; // Four rows
const byte COL_NUM    = 4; // Four columns
char keys[ROW_NUM][COL_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6};    // Các chân row kết nối Arduino
byte pin_column[COL_NUM] = {5, 4, 3, 2}; // Các chân column kết nối Arduino

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COL_NUM);

// Khai báo chân điều khiển bóng đèn (relay)
const int lampPin = 10;

// Thời gian để bật bóng đèn
int setHour = 18;  // Giờ bật bóng đèn (ví dụ: 18h)
int setMinute = 0; // Phút bật bóng đèn (ví dụ: 00 phút)

// Thời gian để tắt bóng đèn
int turnOffHour = 0;
int turnOffMinute = 0;

bool lampStatus = false;

void setup() {
  // Khởi tạo LCD và bật đèn nền
  lcd.begin(16, 2);
  lcd.backlight();   // Bật đèn nền LCD
  lcd.clear();       // Xóa màn hình trước khi hiển thị thông tin

  // Khởi tạo RTC
  if (!rtc.begin()) {
    lcd.print("Không tìm thấy RTC");
    while (1);  // Dừng lại nếu không tìm thấy RTC
  }
  
  // Cài đặt chân điều khiển relay
  pinMode(lampPin, OUTPUT);
  digitalWrite(lampPin, LOW);  // Đảm bảo bóng đèn tắt ban đầu

  // Cài đặt giờ hiển thị mặc định (theo thời gian biên dịch)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // Lấy thời gian từ RTC
  DateTime now = rtc.now();

  // Hiển thị giờ hiện tại trên LCD (Hàng 1)
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  // Hiển thị trạng thái bóng đèn (Hàng 2)
  lcd.setCursor(0, 1);
  lcd.print("LED: ");
  if (lampStatus) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }

  // Kiểm tra xem giờ hiện tại có đến giờ bật bóng đèn không
  if (now.hour() == setHour && now.minute() == setMinute) {
    lampStatus = true;   // Bật bóng đèn
    digitalWrite(lampPin, HIGH);
  } 
  // Kiểm tra giờ hiện tại có đến giờ tắt bóng đèn không
  else if (now.hour() == turnOffHour && now.minute() == turnOffMinute) {
    lampStatus = false;  // Tắt bóng đèn
    digitalWrite(lampPin, LOW);
  }
  // Nếu không phải giờ bật đèn và không phải giờ tắt đèn, đảm bảo bóng đèn tắt
  else if (now.hour() != setHour || now.minute() != setMinute) {
    lampStatus = false;  // Tắt bóng đèn
    digitalWrite(lampPin, LOW);
  }

  // Kiểm tra nếu có phím được nhấn để chỉnh giờ
  char key = keypad.getKey();
  if (key) {
    if (key == 'A') {
      // Chế độ chỉnh giờ bật
      setTime();
    } else if (key == 'B') {
      // Chế độ hẹn giờ tắt bóng đèn
      setTurnOffTime();
    }
  }

  delay(100);  // Cập nhật mỗi giây
}

void setTime() {
  // Hiển thị màn hình nhập giờ
  lcd.clear();
  lcd.print("Nhap gio: ");
  
  // Nhập giờ
  int newHour = 0;
  int newMinute = 0;
  bool hourInputComplete = false;
  bool minuteInputComplete = false;
  
  // Đặt con trỏ bắt đầu nhập giờ
  lcd.setCursor(10, 0);  

  // Chế độ nhập giờ và phút
  while (!hourInputComplete || !minuteInputComplete) {
    char key = keypad.getKey();
    
    if (key >= '0' && key <= '9') {
      if (!hourInputComplete) {
        newHour = newHour * 10 + (key - '0');
        lcd.print(key);  // Hiển thị phím số nhấn
        if (newHour >= 10) {
          lcd.print(":");  // Hiển thị dấu ":" sau khi nhập đủ 2 chữ số
          hourInputComplete = true;  // Đánh dấu hoàn thành việc nhập giờ
        }
      }
      else if (!minuteInputComplete) {
        newMinute = newMinute * 10 + (key - '0');
        lcd.print(key);  // Hiển thị phím số nhấn
        if (newMinute >= 10) {
          minuteInputComplete = true;  // Đánh dấu hoàn thành việc nhập phút
        }
      }
    } 
    // Khi nhấn phím # (xác nhận) và giá trị hợp lệ
    else if (key == '#') {
      if (hourInputComplete && newHour >= 0 && newHour <= 23) {
        if (minuteInputComplete && newMinute >= 0 && newMinute <= 59) {
          hourInputComplete = true;
          minuteInputComplete = true;
        }
        else if (!minuteInputComplete) {
          // Nếu phút chưa nhập, đảm bảo nhập hợp lệ (0-59)
          if (newMinute >= 0 && newMinute <= 59) {
            minuteInputComplete = true;
          } else {
            lcd.clear();
            lcd.print("Phut khong hop le");
            delay(1000);  // Hiển thị thông báo lỗi trong 1 giây
            lcd.clear();
            lcd.print("Nhap phut: ");
            newMinute = 0;  // Reset lại giá trị phút
            lcd.setCursor(10, 1);  // Đặt lại con trỏ
          }
        }
      } else {
        lcd.clear();
        lcd.print("Gio khong hop le");
        delay(1000);  // Hiển thị thông báo lỗi trong 1 giây
        lcd.clear();
        lcd.print("Nhap gio: ");
        newHour = 0;  // Reset lại giá trị giờ
        newMinute = 0; // Reset lại giá trị phút
        lcd.setCursor(10, 0);  // Đặt lại con trỏ
      }
    }
    // Khi nhấn phím * (xóa một ký tự)
    else if (key == '*') {
      if (!minuteInputComplete) {
        newHour = newHour / 10;
        lcd.setCursor(10, 0);
        lcd.print("  ");  // Xóa ký tự vừa nhập
        lcd.setCursor(10, 0);
        lcd.print(newHour); // Hiển thị lại giờ đã nhập
      } else {
        newMinute = newMinute / 10;
        lcd.setCursor(13, 0);
        lcd.print("  ");  // Xóa ký tự vừa nhập
        lcd.setCursor(13, 0);
        lcd.print(newMinute); // Hiển thị lại phút đã nhập
      }
    }

    delay(100);  // Giảm độ trễ để cải thiện phản hồi
  }

  // Lưu lại giờ và phút bật
  setHour = newHour;
  setMinute = newMinute;

  // Hiển thị giờ đã thiết lập (Hàng 1)
  lcd.clear();
  lcd.print("Gio bat: ");
  lcd.print(setHour);
  lcd.print(":");
  if (setMinute < 10) lcd.print("0");
  lcd.print(setMinute);
  delay(2000); // Hiển thị trong 2 giây
}


void setTurnOffTime() {
  // Hiển thị màn hình nhập giờ tắt
  lcd.clear();
  lcd.print("Nhap gio: ");
  
  // Nhập giờ tắt
  int newOffHour = 0;
  lcd.setCursor(10, 0);  // Đặt con trỏ ở vị trí hiển thị giờ
  bool offHourInputComplete = false;

  while (!offHourInputComplete) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      newOffHour = newOffHour * 10 + (key - '0');
      lcd.print(key);  // Hiển thị phím số nhấn
    } else if (key == '#') {
      if (newOffHour >= 0 && newOffHour <= 23) {
        offHourInputComplete = true;
      } else {
        lcd.clear();
        lcd.print("Gio khong hop le");
        delay(1000);  // Hiển thị thông báo lỗi trong 1 giây
        lcd.clear();
        lcd.print("Nhap gio: ");
        newOffHour = 0;  // Reset lại giá trị
        lcd.setCursor(10, 0);  // Đặt lại con trỏ
      }
    } else if (key == '*') {
      newOffHour = newOffHour / 10;
      lcd.setCursor(10, 0);
      lcd.print("  ");
      lcd.setCursor(10, 0);
      lcd.print(newOffHour);
    }
  }

  // Sau khi nhập giờ hợp lệ, nhập phút tắt
  lcd.clear();
  lcd.print("Nhap phut: ");
  
  int newOffMinute = 0;
  lcd.setCursor(10, 1);  // Đặt con trỏ ở vị trí hiển thị phút
  bool offMinuteInputComplete = false;

  while (!offMinuteInputComplete) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      newOffMinute = newOffMinute * 10 + (key - '0');
      lcd.print(key);  // Hiển thị phím số nhấn
    } else if (key == '#') {
      if (newOffMinute >= 0 && newOffMinute <= 59) {
        offMinuteInputComplete = true;
      } else {
        lcd.clear();
        lcd.print("Phut khong hop le");
        delay(1000);  // Hiển thị thông báo lỗi trong 1 giây
        lcd.clear();
        lcd.print("Nhap phut: ");
        newOffMinute = 0;  // Reset lại giá trị
        lcd.setCursor(10, 1);  // Đặt lại con trỏ
      }
    } else if (key == '*') {
      newOffMinute = newOffMinute / 10;
      lcd.setCursor(10, 1);
      lcd.print("  ");
      lcd.setCursor(10, 1);
      lcd.print(newOffMinute);
    }
  }

  // Lưu lại giờ và phút tắt
  turnOffHour = newOffHour;
  turnOffMinute = newOffMinute;

  // Hiển thị giờ tắt đã thiết lập (Hàng 1)
  lcd.clear();
  lcd.print("Gio tat: ");
  lcd.print(turnOffHour);
  lcd.print(":");
  if (turnOffMinute < 10) lcd.print("0");
  lcd.print(turnOffMinute);
  delay(2000); // Hiển thị trong 2 giây
}
