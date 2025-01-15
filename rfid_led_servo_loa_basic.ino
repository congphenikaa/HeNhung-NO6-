#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo myServo;
int servoPin = 5;
int ledRed = 2;
int ledGreen = 3;
int buzzerPin = 4;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  myServo.attach(servoPin);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, LOW);
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      String cardID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        cardID += String(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println(cardID);
      
      // Kiểm tra ID thẻ hợp lệ (thay "your_card_id" bằng ID thẻ của bạn)
      if (cardID == "your_card_id") {
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, HIGH);
        myServo.write(90); // Mở cổng (servo quay 90 độ)
        tone(buzzerPin, 1000, 500); // Phát âm thanh thông báo
        delay(5000); // Giữ cổng mở 5 giây
        myServo.write(0); // Đóng cổng (servo quay về 0 độ)
        delay(1000);
        digitalWrite(ledGreen, LOW);
      } else {
        digitalWrite(ledRed, HIGH);
        tone(buzzerPin, 2000, 500); // Phát âm thanh cảnh báo
        delay(1000);
        digitalWrite(ledRed, LOW);
      }
    }
  }
}
