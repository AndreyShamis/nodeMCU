
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


#include <MFRC522.h>

#define RST_PIN               D3 //0 D1    //5  // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN                D8    //4  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO4 

#define RFID_NODEMCU_MISO     D6    //12  // connect
#define RFID_NODEMCU_SCK      D5    //14  // connect
#define RFID_NODEMCU_MOSI     D7    //13  // connect

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
#define PC_SERIAL_BAUD_RATE       115200


/**

*/
void setup() {
  Serial.begin(PC_SERIAL_BAUD_RATE); // Initialize serial communications with the PC
  Serial.println("PC serial started");
  SPI.begin();   // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  Serial.println(F("*****************************"));
  Serial.println(F("MFRC522 Digital self test"));
  Serial.println(F("*****************************"));
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
  //  Serial.println(F("-----------------------------"));
  //  Serial.println(F("Only known versions supported"));
  //  Serial.println(F("-----------------------------"));
  //  Serial.println(F("Performing test..."));
  //  bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
  //  Serial.println(F("-----------------------------"));
  //  Serial.print(F("Result: "));
  //  if (result)
  //    Serial.println(F("OK"));
  //  else
  //    Serial.println(F("DEFECT or UNKNOWN"));
  //  Serial.println();

  Serial.println("Scan PICC to see UID and type...");
}


// Number of known default keys (hard-coded)
// NOTE: Synchronize the NR_KNOWN_KEYS define with the defaultKeys[] array
constexpr uint8_t NR_KNOWN_KEYS = 8;
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
  {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
  {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
  {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
  {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
  {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
  {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 00 00 00 00 00 00

};
/**

*/
void loop() {
  MFRC522::MIFARE_Key key;
  //for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;
  //  byte uidCard[4] = {0x93, 0x48, 0x67, 0x9A};

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
    return;
  }
  else {
    Serial.println("PICC_IsNewCardPresent");
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }  else {
    // Serial.println("PICC_ReadCardSerial");
  }
  Serial.println(F("**Card Detected:**"));

  Serial.println("INFO: ----------------------------------------");
  Serial.println("DEBUG: Starting search key");
  // Try the known default keys
  for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
    Serial.println("INFO: Search " + String(k) + "/" + String(NR_KNOWN_KEYS));
    Serial.println("Trying key ");
    // Copy the known key into the MIFARE_Key structure
    for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
      key.keyByte[i] = knownKeys[k][i];
    }
    
    dump_byte_array(key.keyByte,MFRC522::MF_KEY_SIZE);
    // Try the key
    if (try_key(&key)) {
      // Found and reported on the key and block,
      // no need to try other keys for this PICC
      Serial.println("DEBUG: Key Found");
      break;
    }
  }

  Serial.println("DEBUG: Stoping search key");
  Serial.println("INFO: ----------------------------------------");


  //  for (byte i = 0; i < 4; i++) {
  //    if (uidCard[i] != mfrc522.uid.uidByte[i])
  //      return;
  //  }
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  Serial.println("");



  //-------------------------------------------

  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("GET FIRST NAME Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
  }
  Serial.print(" ");

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;




  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("GET LAST NAME Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
    Serial.write(buffer2[i] );
  }


  //----------------------------------------

  Serial.println(F("\n**End Reading**\n"));

  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(500);
  // digitalWrite();
  //delay(5000);
  // digitalWrite();
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }


  Serial.println("");
}


/*
   Try using the PICC (the tag/card) with the given key to access block 0.
   On success, it will show the key details, and dump the block data on Serial.

   @return true when the given key worked, false otherwise.
*/
boolean try_key(MFRC522::MIFARE_Key *key)
{
  boolean result = false;
  byte buffer[18];
  byte block = 0;
  MFRC522::StatusCode status;

  // http://arduino.stackexchange.com/a/14316
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return false;
  if ( ! mfrc522.PICC_ReadCardSerial())
    return false;
  // Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Read block
  byte byteCount = sizeof(buffer);
  status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("MIFARE_Read() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
  }
  else {
    // Successful read
    result = true;
    Serial.print(F("Success with key:"));
    dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
    // Dump block data
    Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
    dump_byte_array(buffer, 16);
    Serial.println();
  }
  Serial.println();

  mfrc522.PICC_HaltA();       // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  return result;
}

