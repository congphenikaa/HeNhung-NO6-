#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "Cong"        
#define WIFI_PASSWORD "123456789"
#define FIREBASE_HOST "https://demo1-f568f-default-rtdb.firebaseio.com/"  
#define FIREBASE_AUTH "AIzaSyCfkM-c6TM_89dL3SdABMUi_0IYsFsH9D0"         

// Khai báo đối tượng Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Chân LED
#define LED_PIN D3 

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
    
    // Cấu hình LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Ban đầu tắt LED
}

void loop() {
    // Đọc giá trị từ Firebase
    if (Firebase.getInt(firebaseData, "/led")) {
        int ledState = firebaseData.intData();
        Serial.println("LED State: " + String(ledState));

        // Cập nhật trạng thái đèn
        if (ledState == 1) {
            digitalWrite(LED_PIN, HIGH);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("LED: ON");
        } else {
            digitalWrite(LED_PIN, LOW);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("LED: OFF");
        }
    } else {
        Serial.println("Firebase Read Error: " + firebaseData.errorReason());
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Firebase Error");
    }
    
    delay(2000);  // Đọc dữ liệu mỗi 2 giây
}
