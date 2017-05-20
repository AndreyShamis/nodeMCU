#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>  // Include the Servo library


#define servoPin  D7  // Declare the Servo pin

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

ESP8266WebServer server(80);
Servo Servo1;  // Create a servo object

/**

*/
void stop(void) {
  analogWrite(5, 0);
  analogWrite(4, 0);
}

/**

*/
void forward(void) {
  analogWrite(5, 1023); analogWrite(4, 1023);
  digitalWrite(0, HIGH); digitalWrite(2, HIGH);
}

/**

*/
void backward(void) {
  analogWrite(5, 1023); analogWrite(4, 1023);
  digitalWrite(0, LOW); digitalWrite(2, LOW);
}

/**

*/
void left(void) {
  analogWrite(5, 1023); analogWrite(4, 900);
  //digitalWrite(0, HIGH);digitalWrite(2, HIGH);
}

/**

*/
void right(void) {
  analogWrite(5, 900); analogWrite(4, 1023);
  // digitalWrite(0, HIGH); digitalWrite(2, HIGH);
}

/**

*/
void servo_right(void) {
  Servo1.write(60); // Make servo go to 0 degrees
}

/**

*/
void servo_center(void) {
  Servo1.write(90); // Make servo go to 90 degrees
}

/**

*/
void servo_left(void) {
  Servo1.write(120); // Make servo go to 180 degrees
}

/**

*/

void handle_form() {
  if (server.arg("dir"))  {
    int direction = server.arg("dir").toInt();
    switch (direction)   {
      case 1:  left();
        break;
      case 2:  right();
        break;
      case 3:  backward();
        break;
      case 4: forward();
        break;
      case 5:  stop();
        break;
      case 6:  servo_right();
        break;
      case 7:  servo_left();
        break;
      case 8:  servo_center();
        break;
    }
  }
  server.send(200, "text/html", form);
}
void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  Serial.println("INFO: Starting SPIFFS...");
  WiFi.begin("RadiationG", "polkalol");
  // static ip, gateway, netmask
  //    WiFi.config(IPAddress(192,168,1,2),IPAddress(192,168,1,1),
  //                        IPAddress(255,255,255,0));
  while (WiFi.status() != WL_CONNECTED)  {
    delay(200);
  }

  Serial.println("");

  Serial.print("PASS:  IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("PASS: MDNS responder started");
  }
  Serial.println("-----------------------------------");

  // callback для http server
  server.on("/", handle_form);
  server.begin();
  pinMode(5, OUTPUT); // motor A speed
  pinMode(4, OUTPUT); // motor B speed
  pinMode(0, OUTPUT); //  motor A направление
  pinMode(2, OUTPUT); //  motor B направление
  Servo1.attach(servoPin); // We need to attach the servo to the used pin number
  servo_center();
}

/**

*/
void loop() {
  server.handleClient();
}
