#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>  // Include the Servo library


#define servoPin  D7  // Declare the Servo pin
#define ADC       A0  // select the input pin for the potentiometer
int X = 0;
int Y = 0;
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

void read_adc() {

  // put your main code here, to run repeatedly:
  //  digitalWrite(X_CTRL, HIGH);
  //  delay(1);
  //  //digitalWrite(Y_CTRL, LOW);
  X = analogRead(ADC);
  //int x_servo =  map(X, 0, 1024, 0, 179);
  int x_servo =  map(X, 0, 1024, 60, 130);
  //  digitalWrite(X_CTRL, LOW);
  //  delay(2);
  //  digitalWrite(Y_CTRL, HIGH);
  //  delay(1);
  //  Y = analogRead(ADC);
  //  digitalWrite(Y_CTRL, LOW);
  //  //delay(1);
  Serial.println("X value "  + String(X) + ":" + String(x_servo));
  Servo1.write(x_servo); // Make servo go to 90 degrees
}

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
int counter = 0;
/**

*/
void loop() {
  counter++;
  server.handleClient();
  if (counter % 2 == 0) {
    read_adc();
    delay(10);
  }
  if (counter > 1000) {
    counter = 0;
  }
  delay(5);

}
