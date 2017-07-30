
/*
   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN                    10
#define RST_PIN                   9
#define PC_SERIAL_BAUD_RATE       921600
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

/**

*/
void setup() {
  Serial.begin(PC_SERIAL_BAUD_RATE); // Initialize serial communications with the PC
  SPI.begin();   // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Scan PICC to see UID and type...");
}

/**

*/
void loop() {
  byte uidCard[4] = {0x93, 0x48, 0x67, 0x9A};

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  for (byte i = 0; i < 4; i++) {
    if (uidCard[i] != mfrc522.uid.uidByte[i])
      return;
  }

  Serial.println("OPEN");
  // digitalWrite();
  delay(5000);
  // digitalWrite();
}
