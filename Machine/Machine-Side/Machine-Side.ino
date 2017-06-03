#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>  // Include the Servo library

#define   LED0                2   // WIFI Module LED
#define   SERVO_PIN           D5  // Declare the Servo pin
#define   ADC                 A0  // select the input pin for the potentiometer
#define   SERVER_PORT         9001

int       LR_value_current        = 0;
int       FB_value_current        = 0;
bool      forward                 = true;
bool      forward_current         = true;

int       LR_value                = 0;
int       FB_value                = 0;
bool      connected_to_server     = false;

IPAddress             RC_Server(192, 168, 4, 1);
WiFiClient            RC_Client;

const char *form = "<center><form action='/'>"
                   "<button name='dir' style='font-size:72px' type='submit' value='4'>Forward</button><br/>&nbsp;&nbsp;"
                   "<button name='dir' style='font-size:62px' type='submit' value='1'>Left</button>&nbsp;&nbsp;"
                   "<button name='dir' style='font-size:62px' type='submit' value='2'>Right</button><br/>"

                   "<button name='dir' style='font-size:72px' type='submit' value='6' title='Left Servo'>&nbsp;&nbsp;&nbsp;<-&nbsp;&nbsp;&nbsp;</button>&nbsp;"
                   "<button name='dir' style='font-size:72px' type='submit' value='8' title='Servo Center'>&nbsp;&nbsp;&nbsp; | &nbsp;&nbsp;&nbsp;</button>&nbsp;"
                   "<button name='dir' style='font-size:72px' type='submit' value='7' title='Right Servo'>&nbsp;&nbsp;&nbsp;->&nbsp;&nbsp;&nbsp;</button><br/>"
                   "<button name='dir' style='font-size:72px' type='submit' value='3'>Reverse</button><br/>"
                   "<button name='dir' style='font-size:82px' type='submit' value='5'>Stop</button>"
                   "</form></center>";

//ESP8266WebServer server(80);
Servo Servo1;  // Create a servo object


/**

*/
void forward_acc(int acc) {
  digitalWrite(D3, HIGH); digitalWrite(D4, HIGH);
  analogWrite(D1, acc); analogWrite(D2, acc);
  
}

/**

*/
void backward_acc(int acc) {
  digitalWrite(D3, LOW); digitalWrite(D4, LOW);
  analogWrite(D1, acc); analogWrite(D2, acc);
  
}

/**

*/
void stop(void) {
  analogWrite(D1, 0);
  analogWrite(D2, 0);
}
/**

*/
void AvailableMessage()
{
  //check clients for data

  if (RC_Client && RC_Client.connected() && RC_Client.available())
  {
    while (RC_Client.available())
    {
      String Message = RC_Client.readStringUntil('!');
      Message.replace("\n", "");
      Message.replace("\r", "");
      RC_Client.flush();
      if (Message.length() < 3) {
        continue;
      }
      //        sendToClient(String("") + "AIN0: Forward/Bachward" + "|" + String(bf_ard) + "|% ");
      //Serial.println(String("") + "Client No " + " - " + Message);
      //Serial.println(String("") + "Client val 0 " + getSplitedValue(Message, ' ', 0));
      //Serial.println(String("") + "Client val 1 " + getSplitedValue(Message, ' ', 1));
      //Serial.println(String("") + "Client val 2 " + getSplitedValue(Message, ' ', 2));
      //Serial.println(String("") + "Client val 3 " + getSplitedValue(Message, ' ', 3));
      String fb_str = getSplitedValue(Message, ':', 0);
      String lr_str = getSplitedValue(Message, ':', 1);

      if (fb_str != "") {
        int bf_val_pult = fb_str.toInt();
        if (bf_val_pult > 0 ) {
          FB_value  = map(bf_val_pult, 0, 100 , 300, 1023);
          forward = false;
        }
        else if ( bf_val_pult < 0 ) {
          FB_value  = map(bf_val_pult * (-1), 0, 100 , 300, 1023);
          forward = true;
        }
        else {
          FB_value = 0;
          //stop();
          //Serial.println(String("") + "Client MSG" + " - |" + Message + "| " + String(Message.length()));
        }
      }

      if (lr_str != "") {
        LR_value = map(lr_str.toInt(), -100, 100 , 30, 160);
        //Serial.println(String("") + "LR_ new value is [" + String(LR_value) + "] from[" + lr_str + "]");

      }
      break;
    }
  }
}



void blink_led(int times, int interval) {
  digitalWrite(LED0, !LOW);       // Turn WiFi LED Off
  delay(interval);
  for (int i = 0 ; i < times; i++) {
    digitalWrite(LED0, !HIGH);
    delay(interval);
    digitalWrite(LED0, !LOW);       // Turn WiFi LED Off
    delay(interval);
  }
}
void TKDRequest()
{
  // First Make Sure You Got Disconnected
  RC_Client.stop();
  digitalWrite(LED0, !HIGH);
  Serial.println    ("Trying to connect");
  // If Sucessfully Connected Send Connection Message
  if (RC_Client.connect(RC_Server, SERVER_PORT))
  {
    connected_to_server = true;

    RC_Client.println ("Hello RCctrl Im CLIENT");
    RC_Client.flush();
    blink_led(3, 100);

    Serial.println    ("Connected to SERVER");

  }
  else {
    blink_led(10, 200);
    Serial.println    ("Cannot connect to " + String(RC_Server) + ":" + String(SERVER_PORT));
  }
}



void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname("MACHINE");
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  Serial.println("INFO: Starting SPIFFS...");
  WiFi.begin("RCctrl", "");
  // static ip, gateway, netmask
  //    WiFi.config(IPAddress(192,168,1,2),IPAddress(192,168,1,1),
  //                        IPAddress(255,255,255,0));

  // WiFi Connectivity ----------------------------------------------------
  CheckWiFiConnectivity();        // Checking For Connection
  Serial.println("");
  //  IPAddress gateway(192, 168, 4, 1);
  //  IPAddress subnet(255, 255, 255, 0);
  //  WiFi.config(IPAddress(192, 168, 4, 2), gateway, subnet);
  Serial.print("PASS:  IP address: ");
  Serial.println(WiFi.localIP());

  //  if (MDNS.begin("esp8266")) {
  //    Serial.println("PASS: MDNS responder started");
  //  }
  Serial.println("-----------------------------------");
  // Printing IP Address --------------------------------------------------
  Serial.println("Connected To      : " + String(WiFi.SSID()));
  Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
  Serial.print  ("Server IP Address : ");
  Serial.println(RC_Server);
  Serial.print  ("Server Port Num   : ");
  Serial.println(SERVER_PORT);
  // Printing MAC Address
  Serial.print  ("Device MC Address : ");
  Serial.println(String(WiFi.macAddress()));
  // Printing IP Address
  Serial.print  ("Device IP Address : ");
  Serial.println(WiFi.localIP());
  //  // callback для http server
  //  server.on("/", handle_form);
  //  server.begin();

  pinMode(5, OUTPUT); // motor A speed
  pinMode(4, OUTPUT); // motor B speed
  pinMode(0, OUTPUT); //  motor A направление
  pinMode(2, OUTPUT); //  motor B направление

  Servo1.attach(SERVO_PIN); // We need to attach the servo to the used pin number
  servo_center();
  TKDRequest();
}
int counter = 0;

/**

*/
void loop() {
  //  server.handleClient();
  //delay(5);

  if (WiFi.status() != WL_CONNECTED) {
    stop();
    connected_to_server = false;
    Serial.println("WiFi is disconnted");
    CheckWiFiConnectivity();
  }
  if (!connected_to_server) {

    TKDRequest();
    if (!connected_to_server) {
      delay(1000);
    }
  }

  AvailableMessage();
  if (LR_value != LR_value_current) {
    Serial.println("INFO: Cahnging RL OLD:New\t\t[" + String(LR_value_current) + " : " + String(LR_value) + "]");
    LR_value_current = LR_value;
    Servo1.write(LR_value_current);
  }

  if (FB_value != FB_value_current || forward != forward_current) {
    Serial.println("INFO: Cahnging FB OLD:New\t\t[" + String(FB_value_current) + " : " + String(FB_value) + "]");
    FB_value_current = FB_value;
    forward_current = forward;
    if (FB_value == 0) {
      stop();
    }
    else if (forward) {
      forward_acc(FB_value);
    }
    else {
      backward_acc(FB_value);
    }
  }
}

/**

*/
String getSplitedValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//====================================================================================

void CheckWiFiConnectivity()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    for (int i = 0; i < 50; i++)
    {
      digitalWrite(LED0, !HIGH);
      delay(50);
      digitalWrite(LED0, !LOW);
      delay(50);
      Serial.print(".");
    }
    Serial.println("");
  }
}

/**

*/
void servo_center(void) {
  Servo1.write(90); // Make servo go to 90 degrees
}




///**
// *
// */
//void read_adc() {
//
//  // put your main code here, to run repeatedly:
//  //  digitalWrite(X_CTRL, HIGH);
//  //  delay(1);
//  //  //digitalWrite(Y_CTRL, LOW);
//  X = analogRead(ADC);
//  //int x_servo =  map(X, 0, 1024, 0, 179);
//  int x_servo =  map(X, 0, 1024, 60, 130);
//  //  digitalWrite(X_CTRL, LOW);
//  //  delay(2);
//  //  digitalWrite(Y_CTRL, HIGH);
//  //  delay(1);
//  //  Y = analogRead(ADC);
//  //  digitalWrite(Y_CTRL, LOW);
//  //  //delay(1);
//  Serial.println("X value "  + String(X) + ":" + String(x_servo));
//  Servo1.write(x_servo); // Make servo go to 90 degrees
//}
//

//
///**
//
//*/
//void handle_form() {
//  //  if (server.arg("dir"))  {
//  //    int direction = server.arg("dir").toInt();
//  //    switch (direction)   {
//  //      case 1:  left();
//  //        break;
//  //      case 2:  right();
//  //        break;
//  //      case 3:  backward();
//  //        break;
//  //      case 4: forward();
//  //        break;
//  //      case 5:  stop();
//  //        break;
//  //      case 6:  servo_right();
//  //        break;
//  //      case 7:  servo_left();
//  //        break;
//  //      case 8:  servo_center();
//  //        break;
//  //    }
//  //  }
//  //  server.send(200, "text/html", form);
//}

///**
//
//*/
//void forward(void) {
//  analogWrite(5, 1023); analogWrite(4, 1023);
//  digitalWrite(0, HIGH); digitalWrite(2, HIGH);
//}
//
///**
//
//*/
//void backward(void) {
//  analogWrite(5, 1023); analogWrite(4, 1023);
//  digitalWrite(0, LOW); digitalWrite(2, LOW);
//}
//
///**
//
//*/
//void left(void) {
//  analogWrite(5, 1023); analogWrite(4, 900);
//  //digitalWrite(0, HIGH);digitalWrite(2, HIGH);
//}
//
///**
//
//*/
//void right(void) {
//  analogWrite(5, 900); analogWrite(4, 1023);
//  // digitalWrite(0, HIGH); digitalWrite(2, HIGH);
//}

///**
//
//*/
//void servo_right(void) {
//  Servo1.write(60); // Make servo go to 0 degrees
//}


//
///**
//
//*/
//void servo_left(void) {
//  Servo1.write(120); // Make servo go to 180 degrees
//}

