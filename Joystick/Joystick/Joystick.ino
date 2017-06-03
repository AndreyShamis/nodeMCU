/*
Andrey Shamis lolnik@gmail.com
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

//#define ADC                     A0  // select the input pin for the potentiometer
//#define X_CTRL                  D0
//#define Y_CTRL                  D1
//------------------------------------------------------------------------------------
#define     MAXSC               2           // MAXIMUM NUMBER OF CLIENTS
#define     ADC_TRASH_HOLD      7
#define     SERVER_PORT         9001
#define     LED0                2           // WIFI Module LED


//int X = 0;
//int Y = 0;

//int sensorPin = A0;    // select the input pin for the potentiometer
//int ledPin = D7;      // select the pin for the LED
//int sensorValue = 0;  // variable to store the value coming from the sensor

int bf_ard_middle = 0;
int lr_ht_middle  = 0;
int prev_x        = 0;
int prev_y        = 0;
int bf_ard        = 0;
int lr_ht         = 0;

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
WiFiServer  RC_Server(SERVER_PORT);       // THE SERVER AND THE PORT NUMBER
WiFiClient  RC_Client[MAXSC];             // THE SERVER CLIENTS (Devices)

/**

*/
void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");

  // Setting The Mode Of Pins
  pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light

  // SET GAIN
  ads.begin();
  // Setting Up A Wifi Access Point
  SetWifi("RCctrl", "");
  // Printing IP Address --------------------------------------------------
  Serial.println("Created SSID      : " + String(WiFi.SSID()));
  Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");

  Serial.print  ("Server Port Num   : ");
  Serial.println(SERVER_PORT);
  // Printing MAC Address
  Serial.print  ("Device MC Address : ");
  Serial.println(String(WiFi.macAddress()));
  // Printing IP Address
  Serial.print  ("Device IP Address : ");
  Serial.println(WiFi.localIP());
}


void calculate_middle() {
  int counter = 0;
  bf_ard_middle = 0;
  lr_ht_middle = 0;
  for (counter = 0 ; counter < 200; counter++) {
    delay((counter % 2) + 1);
    bf_ard_middle += ads.readADC_SingleEnded(0);
    delay((counter % 2) + 1);
    lr_ht_middle += ads.readADC_SingleEnded(1);
  }
  bf_ard_middle = bf_ard_middle / counter;
  lr_ht_middle = lr_ht_middle / counter;
  lr_ht_middle += 3;
  bf_ard_middle += 3;
  Serial.println("|Middle for FB" + String(bf_ard_middle) + "|");
  Serial.println("|Middle for LR" + String(lr_ht_middle) + "|");

}

/**

*/
int16_t shure_read(int port) {
  int counter = 0;
  int16_t ret = 0;
  for (counter = 0; counter < 3; counter++) {
    int tmp = ads.readADC_SingleEnded(port);
    if (tmp > 1200) {
      tmp = 1;
    }
    if (tmp > 1090) {
      tmp = 1100;
    }
    ret += tmp;
    delay(1);
  }
  ret = ret / counter;
  return ret;
}


void loop() {
  if (lr_ht_middle == 0 || bf_ard_middle == 0) {
    calculate_middle();
  }
  //  //digitalWrite(ledPin, HIGH);
  //  read_adc();
  //  Serial.println("INFO: sensor value X:" + String(X) + " Y:" + String(Y));
  //  delay(50);
  //  //digitalWrite(ledPin, LOW);
  //  delay(50);

  int16_t adc0, adc1;

  adc0 = shure_read(0);
  adc1 = shure_read(1);

  if (adc0 - ADC_TRASH_HOLD > bf_ard_middle) {
    bf_ard = map(adc0, bf_ard_middle , 1100, 1, 100);
  }
  else if (adc0 + ADC_TRASH_HOLD < bf_ard_middle) {
    bf_ard = map(adc0, 1 , bf_ard_middle, -100, -1);
  }
  else if (adc0 - ADC_TRASH_HOLD <= bf_ard_middle && adc0 + ADC_TRASH_HOLD >= bf_ard_middle) {
    bf_ard = 0;
  }

  if (adc1 - ADC_TRASH_HOLD > lr_ht_middle) {
    lr_ht = map(adc1, lr_ht_middle , 1100, 1, 100);
  }
  else if (adc1 + ADC_TRASH_HOLD < lr_ht_middle) {
    lr_ht = map(adc1, 1 , lr_ht_middle, -100, -1);
  }
  else if (adc1 - ADC_TRASH_HOLD <= lr_ht_middle && adc1 + ADC_TRASH_HOLD >= lr_ht_middle) {
    lr_ht = 0;
  }

  bool send_changes = false;
  if ( prev_y != bf_ard) {
    prev_y = bf_ard;
    Serial.print("AIN0: Forward/Bachward ");
    Serial.print(String(adc0) + " - " );
    Serial.println("|" + String(bf_ard) + "|% ");
    //sendToClient(String("") + "AIN0: Forward/Bachward" + "|" + String(bf_ard) + "|% ");
    send_changes = true;

  }
  if ( prev_x != lr_ht) {
    prev_x = lr_ht;

    Serial.print("AIN1: Left - Right  ");
    Serial.print(String(adc1) + " - " );
    Serial.println("|" + String(lr_ht) + "|% ");
    //sendToClient(String("") + "AIN1: Left - Right" + "|" + String(lr_ht) + "|% ");
    send_changes = true;
  }
  if(send_changes){
   // sendToClient(String("") + "F " + String(bf_ard) + "" + " L " + String(lr_ht) + "");
    sendToClient(String("") + String(bf_ard) + ":" + String(lr_ht) + "!");
  }
  // Checking For Available Clients
  AvailableClients();
  // Checking For Available Client Messages
  AvailableMessage();
  //delay(1);
}

void sendToClient(String data) {
  int i = 0;
  if (RC_Client[i] && RC_Client[i].connected()) {
    RC_Client[i].println(String("") + data);
    RC_Client[i].flush();
  }
}
/**

*/
void AvailableMessage()
{
  //check clients for data
  for (uint8_t i = 0; i < MAXSC; i++)
  {
    if (RC_Client[i] && RC_Client[i].connected() && RC_Client[i].available())
    {
      while (RC_Client[i].available())
      {
        String Message = RC_Client[i].readStringUntil('\r');
        RC_Client[i].flush();
        
//        sendToClient(String("") + "AIN0: Forward/Bachward" + "|" + String(bf_ard) + "|% ");
//        sendToClient(String("") + "AIN1: Left - Right" + "|" + String(lr_ht) + "|% ");
        Serial.println("Client No " + String(i) + " - " + Message);
        Serial.println("Client val 0 " + getSplitedValue(Message, ' ', 0));
        Serial.println("Client val 1 " + getSplitedValue(Message, ' ', 1));
        Serial.println("Client val 2 " + getSplitedValue(Message, ' ', 2));
        Serial.println("Client val 3 " + getSplitedValue(Message, ' ', 3));
      }
    }
  }
}

/**
 * 
 */
String getSplitedValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/**

*/
void AvailableClients()
{
  if (RC_Server.hasClient())
  {
    // Read LED0 Switch To Low If High.
    if (digitalRead(LED0) == HIGH) digitalWrite(LED0, LOW);

    //    // Light Up LED1
    //    digitalWrite(LED1, HIGH);

    for (uint8_t i = 0; i < MAXSC; i++)
    {
      //find free/disconnected spot
      if (!RC_Client[i] || !RC_Client[i].connected())
      {
        // Checks If Previously The Client Is Taken
        if (RC_Client[i])
        {
          RC_Client[i].stop();
        }

        // Checks If Clients Connected To The Server
        if (RC_Client[i] = RC_Server.available())
        {
          Serial.println("New Client: " + String(i));
          sendToClient(String("") + "F 0" + " L 0");
          RC_Server.setNoDelay(true);
        }

        // Continue Scanning
        continue;
      }
    }

    //no free/disconnected spot so reject
    WiFiClient RC_Client = RC_Server.available();
    RC_Client.stop();
  }
//  else
//  {
//    // This LED Blinks If No Clients Where Available
//    digitalWrite(LED0, HIGH);
//    delay(10);
//    digitalWrite(LED0, LOW);
//  }
}


/**

*/
void SetWifi(char* Name, char* Password)
{
  // Stop Any Previous WIFI
  WiFi.disconnect();

  // Setting The Wifi Mode
  WiFi.mode(WIFI_AP_STA);
  Serial.println("WIFI Mode : AccessPoint Station");

  // Starting The Access Point
  WiFi.softAP(Name, Password);
  Serial.println("WIFI <" + String(Name) + "> ... Started");

  // Wait For Few Seconds
  delay(1000);

  // Getting Server IP
  IPAddress IP = WiFi.softAPIP();

  // Printing The Server IP Address
  Serial.print("AccessPoint IP : ");
  Serial.println(IP);

  // Printing MAC Address
  Serial.print("AccessPoint MC : ");
  Serial.println(String(WiFi.softAPmacAddress()));

  // Starting Server
  RC_Server.begin();
  RC_Server.setNoDelay(true);
  Serial.println("Server Started");
}


//void read_adc() {
//
//  // put your main code here, to run repeatedly:
//  digitalWrite(X_CTRL, HIGH);
//  delay(1);
//  //digitalWrite(Y_CTRL, LOW);
//  X = analogRead(ADC);
//  digitalWrite(X_CTRL, LOW);
//  delay(2);
//  digitalWrite(Y_CTRL, HIGH);
//  delay(1);
//  Y = analogRead(ADC);
//  digitalWrite(Y_CTRL, LOW);
//  //delay(1);
//
//
//}


  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
