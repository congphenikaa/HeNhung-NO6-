#include <MFRC522.h>                                                                          // Thư viện cho RFID
#include <LiquidCrystal_I2C.h>                                                                // Thư viện cho LCD I2C
#include <Keypad.h>                                                                           // Thư viện cho keypad
#include <SPI.h>                                                                              // Thư viện cho SPI communication

LiquidCrystal_I2C lcd(0x27, 16, 2);                                                           // Khởi tạo đối tượng LCD I2C với địa chỉ 0x27 và kích thước 16x2
MFRC522 mfrc522(10, 9);                                                                       // Khởi tạo đối tượng RFID với chân SDA 10 và RST 9
constexpr uint8_t buzzerPin = 8;                                                              // Chân nối buzzer

String correctPassword = "1111";                                                              // Mật khẩu mới
String tagUID = "73 71 1F FD";                                                                // UID của thẻ RFID được chấp nhận
String enteredPassword = "";                                                                  // Mật khẩu nhập từ keypad
boolean RFIDMode = true;                                                                      // Chế độ RFID (true) hoặc chế độ mật khẩu (false)
char key_pressed = 0;                                                                         // Biến lưu giữ nút được nhấn trên keypad
uint8_t i = 0;                                                                                // Biến đếm số ký tự mật khẩu đã nhập

const byte rows = 4;                                                                          // Số hàng trên keypad
const byte columns = 4;                                                                       // Số cột trên keypad

char hexaKeys[rows][columns] = {                                                              // Ma trận ký tự trên keypad
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte row_pins[rows] = {0, 1, 2, 3};                                                       // Chân kết nối các hàng trên keypad
byte column_pins[columns] = {7, 6, 5, 4};                                                    // Chân kết nối các cột trên keypad

Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);      // Khởi tạo đối tượng keypad

void setup() {
  pinMode(buzzerPin, OUTPUT);                                                                 // Đặt chân buzzer là OUTPUT
  lcd.init();                                                                                 // Khởi tạo LCD
  lcd.backlight();                                                                            // Bật đèn nền LCD
  SPI.begin();                                                                                // Khởi tạo SPI communication
  mfrc522.PCD_Init();                                                                         // Khởi tạo module RFID
  lcd.clear();                                                                                // Xóa màn hình LCD
}

void loop() {
  // Chế độ RFID: Quét thẻ
  if (RFIDMode == true) {
    lcd.setCursor(1, 0);
    lcd.print("Cua Dang Khoa");
    lcd.setCursor(1, 1);
    lcd.print("Quet The RFID");
    
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++) {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();
    
    if (tag.substring(1) == tagUID) {  // Nếu thẻ đúng
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("The Da Khop");
      delay(3000);
      lcd.clear();
      lcd.print(" Nhap Mat Khau:");
      lcd.setCursor(0, 1);
      RFIDMode = false; // Chuyển sang chế độ nhập mật khẩu
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" The Khong Dung");
      lcd.setCursor(0, 1);
      lcd.print("Quet Lai The");
      digitalWrite(buzzerPin, HIGH);
      delay(3000);
      digitalWrite(buzzerPin, LOW);
      lcd.clear();
    }
  }

  // Chế độ mật khẩu: Nhập mật khẩu
  if (RFIDMode == false) {
    key_pressed = keypad_key.getKey();  // Đọc giá trị nút được nhấn trên keypad
    if (key_pressed) {
      enteredPassword += key_pressed;  // Lưu giá trị nút vào mảng password
      lcd.setCursor(i++, 1);  // Đảm bảo con trỏ LCD di chuyển đúng
      lcd.print("*");  // Hiển thị dấu "*" trên LCD
    }

    // Nếu đã nhập đủ 4 ký tự mật khẩu
    if (enteredPassword.length() == 4) {
      delay(200);  // Đợi một chút trước khi kiểm tra

      // Kiểm tra mật khẩu nhập vào so với mật khẩu đã xác định
      if (enteredPassword == correctPassword) {
        lcd.clear();
        lcd.print("Mo Cua");
        // sg90.write(90);  // Mở khóa (giả sử có servo)
        delay(3000);  // Hiển thị "Mở cửa" trong 3 giây
        // sg90.write(0);   // Đưa servo về vị trí ban đầu (nếu có)

        // Xóa mật khẩu nhập vào sau khi kiểm tra
        enteredPassword = "";

        // Quay lại chế độ quét thẻ RFID
        RFIDMode = true;
        i = 0;  // Reset biến đếm mật khẩu
        lcd.clear();
      } else {
        lcd.clear();
        lcd.print("  Sai Mat Khau");
        digitalWrite(buzzerPin, HIGH);  // Kích hoạt buzzer báo lỗi
        lcd.clear();
        lcd.print(" Nhap lai Mat Khau");
        delay(1000);  // Đợi 3 giây trước khi quay lại chế độ nhập mật khẩu
        digitalWrite(buzzerPin, LOW);  // Tắt buzzer

        // Xóa mật khẩu nhập vào sau khi nhập sai
        enteredPassword = "";

        // Reset lại biến đếm i, tiếp tục cho phép nhập lại mật khẩu nếu cần
        i = 0;
      }
    }
  }
}
