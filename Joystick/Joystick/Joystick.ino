/*
  Analog Input
  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED)  connected to digital pin 13.
  The amount of time the LED will be on and off depends on
  the value obtained by analogRead().

  The circuit:
   Potentiometer attached to analog input 0
   center pin of the potentiometer to the analog pin
   one side pin (either one) to ground
   the other side pin to +5V
   LED anode (long leg) attached to digital output 13
   LED cathode (short leg) attached to ground

   Note: because most Arduinos have a built-in LED attached
  to pin 13 on the board, the LED is optional.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInput

*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define ADC       A0  // select the input pin for the potentiometer
#define X_CTRL    D0
#define Y_CTRL    D1
//------------------------------------------------------------------------------------
#define     MAXSC     6           // MAXIMUM NUMBER OF CLIENTS

// Define I/O Pins
#define     LED0      2           // WIFI Module LED
//#define     LED1      D0          // Connectivity With Client #1
//#define     LED2      D2          // Connectivity With Client #2
//#define     BUTTON    D1          // Connectivity ReInitiate Button

// Authentication Variables
char*       TKDssid;              // SERVER WIFI NAME
char*       TKDpassword;          // SERVER PASSWORD

int X = 0;
int Y = 0;

//int sensorPin = A0;    // select the input pin for the potentiometer
//int ledPin = D7;      // select the pin for the LED
//int sensorValue = 0;  // variable to store the value coming from the sensor

int bf_ard_middle = 0;
int lr_ht_middle = 0;
int prev_x = 0;
int prev_y = 0;

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
WiFiServer  TKDServer(9001);      // THE SERVER AND THE PORT NUMBER
WiFiClient  TKDClient[MAXSC];     // THE SERVER CLIENTS (Devices)

/**

*/
void setup() {
  // declare the ledPin as an OUTPUT:
  //  pinMode(ledPin, OUTPUT);
  //  // put your setup code here, to run once:
  //  pinMode(X_CTRL, OUTPUT);
  //  pinMode(Y_CTRL, OUTPUT);
  //  digitalWrite(X_CTRL, LOW);
  //  digitalWrite(Y_CTRL, LOW);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");

  // Setting The Mode Of Pins
  pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light
  //pinMode(LED1, OUTPUT);          // Indicator For Client #1 Connectivity
  //pinMode(LED2, OUTPUT);          // Indicator For Client #2 Connectivity
  //pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity

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
  ads.begin();
  // Setting Up A Wifi Access Point
  SetWifi("TAKEONE", "");

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



  int bf_ard = 0;
  int lr_ht = 0;
  int trashhold = 7;
  if (adc0 - trashhold > bf_ard_middle) {
    bf_ard = map(adc0, bf_ard_middle , 1100, 1, 100);
  }
  else if (adc0 + trashhold < bf_ard_middle) {
    bf_ard = map(adc0, 1 , bf_ard_middle, -100, -1);
  }

  if (adc1 - trashhold > lr_ht_middle) {
    lr_ht = map(adc1, lr_ht_middle , 1100, 1, 100);
  }
  else if (adc1 + trashhold < lr_ht_middle) {
    lr_ht = map(adc1, 1 , lr_ht_middle, -100, -1);
  }

  if ( prev_y == bf_ard ) {

  }
  if ( prev_y != bf_ard) {
    prev_y = bf_ard;
    Serial.print("AIN0: Forward/Bachward ");
    Serial.print(String(adc0) + " - " );
    Serial.println("|" + String(bf_ard) + "|% ");
  }
  if ( prev_x != lr_ht) {
    prev_x = lr_ht;

    Serial.print("AIN1: Left - Right  ");
    Serial.print(String(adc1) + " - " );
    Serial.println("|" + String(lr_ht) + "|% ");
  }
  // Checking For Available Clients
  AvailableClients();
  // Checking For Available Client Messages
  AvailableMessage();
  delay(1);
}

/**

*/
void AvailableMessage()
{
  //check clients for data
  for (uint8_t i = 0; i < MAXSC; i++)
  {
    if (TKDClient[i] && TKDClient[i].connected() && TKDClient[i].available())
    {
      while (TKDClient[i].available())
      {
        String Message = TKDClient[i].readStringUntil('\r');
        TKDClient[i].flush();
        Serial.println("Client No " + String(i) + " - " + Message);
      }
    }
  }
}

/**

*/
void AvailableClients()
{
  if (TKDServer.hasClient())
  {
    // Read LED0 Switch To Low If High.
    if (digitalRead(LED0) == HIGH) digitalWrite(LED0, LOW);

//    // Light Up LED1
//    digitalWrite(LED1, HIGH);

    for (uint8_t i = 0; i < MAXSC; i++)
    {
      //find free/disconnected spot
      if (!TKDClient[i] || !TKDClient[i].connected())
      {
        // Checks If Previously The Client Is Taken
        if (TKDClient[i])
        {
          TKDClient[i].stop();
        }

        // Checks If Clients Connected To The Server
        if (TKDClient[i] = TKDServer.available())
        {
          Serial.println("New Client: " + String(i));
        }

        // Continue Scanning
        continue;
      }
    }

    //no free/disconnected spot so reject
    WiFiClient TKDClient = TKDServer.available();
    TKDClient.stop();
  }
  else
  {
    // This LED Blinks If No Clients Where Available
    digitalWrite(LED0, HIGH);
    delay(100);
    digitalWrite(LED0, LOW);
    delay(100);
  }
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

  // Setting The AccessPoint Name & Password
  TKDssid      = Name;
  TKDpassword  = Password;

  // Starting The Access Point
  WiFi.softAP(TKDssid, TKDpassword);
  Serial.println("WIFI < " + String(TKDssid) + " > ... Started");

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
  TKDServer.begin();
  TKDServer.setNoDelay(true);
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
