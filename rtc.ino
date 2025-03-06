#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "Cong"
#define WIFI_PASSWORD "123456789"
#define FIREBASE_HOST "https://demo1-f568f-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyCfkM-c6TM_89dL3SdABMUi_0IYsFsH9D0"

// Khai báo Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Khai báo RTC DS3231
RTC_DS3231 rtc;

// Khai báo LCD I2C (Địa chỉ 0x27 hoặc 0x3F tùy module)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Serial.begin(115200);
    
    // Khởi động LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    // Kết nối WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    delay(1000);

    // Cấu hình Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Kiểm tra RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    lcd.clear();
}

void loop() {
    // Đọc thời gian từ Firebase
    if (Firebase.getString(firebaseData, "/time")) {
        String timeString = firebaseData.stringData();
        Serial.println("Time from Firebase: " + timeString);

        int hour, minute;
        sscanf(timeString.c_str(), "%d:%d", &hour, &minute);
        rtc.adjust(DateTime(2025, 3, 6, hour, minute, 0)); // Cập nhật giờ & phút, ngày mặc định

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time Updated:");
        lcd.setCursor(0, 1);
        lcd.print(timeString);
        delay(2000);
    } else {
        Serial.println("Failed to get time from Firebase");
    }

    // Hiển thị thời gian hiện tại từ RTC
    DateTime now = rtc.now();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time:");
    lcd.setCursor(0, 1);
    lcd.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    delay(2000); // Cập nhật mỗi 2 giây
}
