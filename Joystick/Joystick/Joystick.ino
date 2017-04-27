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

#define X_CTRL    D1  
#define Y_CTRL    D2  
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

static void calibrate()
{
  int cnt = 0;
  int xmax = 0;
  int xmin = 1023;
  int ymax = 0;
  int ymin = 1023;
  while(cnt < 4)
  {
     int jx = joystick_xmax-get_joystick_x();
     int jy = joystick_ymax-get_joystick_y();
     if((cnt % 2 == 0) && jx < 450) 
       cnt++;
     if((cnt % 2 == 1) && jx > 800) 
       cnt++;
     xmax = MAX(xmax, jx);
     xmin = MIN(xmin, jx);
     ymax = MAX(ymax, jy);
     ymin = MIN(ymin, jy);
  }
  joystick_xmax = xmax;
  joystick_xmult = 1023.0/(xmax - xmin);
  joystick_ymax = ymax;
  joystick_ymult = 1023.0/(ymax - ymin);
}
static unsigned int get_joystick_x()
{
  pinMode(X_CTRL,INPUT);
  pinMode(Y_CTRL,OUTPUT);
  digitalWrite(Y_CTRL,LOW);
  return (unsigned int)MAX(0, MIN(1023,((joystick_xmax-analogRead(ADC))*joystick_xmult)));
}
static unsigned int get_joystick_y()
{
  pinMode(Y_CTRL,INPUT);
  pinMode(X_CTRL,OUTPUT);
  digitalWrite(X_CTRL,LOW);
  return (unsigned int)MAX(0, MIN(1023,((joystick_ymax-analogRead(ADC))*joystick_ymult)));
}
void loop() {
  digitalWrite(ledPin, HIGH);
  //read_adc();
  Serial.println("INFO: sensor value X:" + String(get_joystick_x()) + " Y:" + String(get_joystick_y()));
  //delay(50);
  digitalWrite(ledPin, LOW);
  delay(150);
}
