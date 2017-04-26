#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define ADC       A0  // select the input pin for the potentiometer

#define X_CTRL    D1  
#define Y_CTRL    D2  
int X = 0;
int Y = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(X_CTRL, OUTPUT);
  pinMode(Y_CTRL, OUTPUT);
  digitalWrite(X_CTRL, LOW);
  digitalWrite(Y_CTRL, LOW);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
}

void read_adc(){

  // put your main code here, to run repeatedly:
  digitalWrite(X_CTRL, HIGH);
  //digitalWrite(Y_CTRL, LOW);
  X = analogRead(ADC);
  digitalWrite(X_CTRL, LOW);
  digitalWrite(Y_CTRL, HIGH);
  Y = analogRead(ADC);
  digitalWrite(Y_CTRL, LOW);
}
void loop() {
  read_adc();
  Serial.println("INFO: sensor value X:" + String(X) + " Y:" + String(Y));
  delay(10);
}
