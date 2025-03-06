
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#define WIFI_SSID "Cong"        
#define WIFI_PASSWORD "123456789"
#define FIREBASE_HOST "https://demo1-f568f-default-rtdb.firebaseio.com/"  
#define FIREBASE_AUTH "AIzaSyCfkM-c6TM_89dL3SdABMUi_0IYsFsH9D0"         

// Khai báo đối tượng Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Chân LED
#define LED_PIN D2  

void setup() {
    Serial.begin(115200);
    
    // Kết nối WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi");

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

        // Kiểm tra trạng thái và điều khiển LED
        if (ledState == 1) {
            digitalWrite(LED_PIN, HIGH);  // Bật đèn
        } else {
            digitalWrite(LED_PIN, LOW);   // Tắt đèn
        }
    } else {
        Serial.println("Firebase Read Error: " + firebaseData.errorReason());
    }

    delay(2000);  // Đọc dữ liệu mỗi 2 giây
}
