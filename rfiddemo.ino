/*
 * .........By Thich Che Tao TV.............
 * Link tai thu vien MFRC522: https://github.com/miguelbalboa/rfid/archive/master.zip
 * Video huong dan chi tiet: https://youtu.be/AHz6XerRwtE
 */
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10  
#define LED_G 4
#define LED_R 5       // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();
  pinMode(LED_G, OUTPUT);   // Init MFRC522
  pinMode(LED_R, OUTPUT);
  Serial.println("Dua the vao ...");
  Serial.println();
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "73 71 1F FD") {
    Serial.println("Dung the");
    Serial.println("");
    digitalWrite(LED_G, HIGH);
    delay(100);
    digitalWrite(LED_G, LOW);
  } else {
    Serial.println("Sai the");
    Serial.println();
    digitalWrite(LED_R, HIGH);
    delay(100);
    digitalWrite(LED_R, LOW);
  }
}
