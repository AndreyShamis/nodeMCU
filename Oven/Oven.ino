
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <math.h>
// #define _delay_ms(ms) delayMicroseconds((ms) * 1000)
#define ESP8266
#include <max6675.h>

#define thermoDO    D6 //4;
#define thermoCS    D7 //5;
#define thermoCLK   D5 //6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
//int vccPin = 3;
//int gndPin = 2;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  // use Arduino pins
//  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);
//  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
  
  delay(500);
}

void loop() {
  // basic readout test, just print the current temp

  Serial.print("[-0.8]C = Round ");
  Serial.print(round(thermocouple.readCelsius() - 0.8));
  Serial.print("   . Not Round ");
  Serial.println(thermocouple.readCelsius() - 0.8);
    Serial.print("[-0.0]C = Round ");
  Serial.print(round(thermocouple.readCelsius() ));
  Serial.print("   . Not Round ");
  Serial.println(thermocouple.readCelsius() );
//  Serial.print("F = ");
//  Serial.println(thermocouple.readFahrenheit());

  delay(1000);
}

