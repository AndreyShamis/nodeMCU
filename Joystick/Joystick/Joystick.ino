/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13.
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead().

 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground

 * Note: because most Arduinos have a built-in LED attached
 to pin 13 on the board, the LED is optional.

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/AnalogInput

 */
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
//#define BLYNK_PRINT Serial    // Prints to serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <BlynkSimpleEsp8266.h>//Calls Blynk ESP8266 Library

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = D7;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
#define ADC       A0  // select the input pin for the potentiometer

#define X_CTRL    D0  
#define Y_CTRL    D1  
int X = 0;
int Y = 0;
void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
//  // put your setup code here, to run once:
//  pinMode(X_CTRL, OUTPUT);
//  pinMode(Y_CTRL, OUTPUT);
//  digitalWrite(X_CTRL, LOW);
//  digitalWrite(Y_CTRL, LOW);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  calibrate();
  Serial.println("PASS: Callibration finished.");
}
//void read_adc(){
//
//  // put your main code here, to run repeatedly:
//  digitalWrite(X_CTRL, HIGH);
//  delay(10);
//  //digitalWrite(Y_CTRL, LOW);
//  X = analogRead(ADC);
//  digitalWrite(X_CTRL, LOW);
//  delay(10);
//  digitalWrite(Y_CTRL, HIGH);
//  delay(10);
//  Y = analogRead(ADC);
//  digitalWrite(Y_CTRL, LOW);
//  delay(10);
//}

int    joystick_xmax = 1023;
float  joystick_xmult = 1;
int    joystick_ymax = 1023;
float  joystick_ymult = 1;

 void calibrate()
{
  int cnt = 0;
  int xmax = 0;
  int xmin = 1023;
  int ymax = 0;
  int ymin = 1023;
  while(cnt < 4)
  {
    delay(100);
    Serial.println("CNT: " + String(cnt));
    int x = get_joystick_x();
    int y = get_joystick_y();
     int jx = joystick_xmax-x;
     int jy = joystick_ymax-y;
     Serial.println("x: " + String(x));
     Serial.println("y: " + String(y));
     if((cnt == 0) && jx < 100) // 450
       cnt++;
       xmax = MAX(xmax, jx);
     if((cnt == 1) && jx > 300) //800
       cnt++;
       xmin = MIN(xmin, jx);
      if((cnt == 2) && jy > 300) //800
       cnt++;
       ymin = MIN(ymin, jy);
     if((cnt == 3) && jy < 100) //800
       cnt++;
       ymax = MAX(ymax, jy);
     
     Serial.println("jx: " + String(jx));
     Serial.println("jy: " + String(jy));
     Serial.println("XMAX: " + String(xmax));
     Serial.println("YMAX: " + String(ymax));
     Serial.println("XMIN: " + String(xmin));
     Serial.println("YMIN: " + String(ymin));
  }
  joystick_xmax = xmax;
  joystick_xmult = 1023.0/(xmax - xmin);
  joystick_ymax = ymax;
  joystick_ymult = 1023.0/(ymax - ymin);
}
 unsigned int get_joystick_x()
{
  pinMode(X_CTRL,INPUT);
  pinMode(Y_CTRL,OUTPUT);
  digitalWrite(Y_CTRL,LOW);
  unsigned int x= (unsigned int)MAX(0, MIN(1023,((joystick_xmax-analogRead(ADC))*joystick_xmult)));
  if(x> 1023){
    x = 0;
  }
  return x;  
}
 unsigned int get_joystick_y()
{
  pinMode(Y_CTRL,INPUT);
  pinMode(X_CTRL,OUTPUT);
  digitalWrite(X_CTRL,LOW);
  unsigned int y = (unsigned int)MAX(0, MIN(1023,((joystick_ymax-analogRead(ADC))*joystick_ymult)));
  if(y> 1023){
    y = 0;
  }
  return y;
}
void loop() {
  digitalWrite(ledPin, HIGH);
  //read_adc();
  Serial.println("INFO: sensor value X:" + String(get_joystick_x()) + " Y:" + String(get_joystick_y()));
  //delay(50);
  digitalWrite(ledPin, LOW);
  delay(150);
}
