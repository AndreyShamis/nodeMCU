#include <SPI.h>
#include <max6675.h>

#define thermoDO    14  //P1.6 //D6 //4;SO
#define thermoCS    7   //P1.5//D7 //5; SC
#define thermoCLK   6   //P1.4 //D5 //6; SCK

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
//int vccPin = 3;
//int gndPin = 2;

void setup() {
  //
  //SPI.begin(); 
  Serial.begin(9600);
  
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  // use Arduino pins
  //  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);
  //  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize

  //delay(500);
}

void loop() {
  delay(1000);
  // basic readout test, just print the current temp
  Serial.print("C = Round ");
  Serial.print(round(thermocouple.readCelsius() - 0.8));
  Serial.print("   . ");
  Serial.println(thermocouple.readCelsius() - 0.8);
  
//  Serial.print("[-0.8]C = Round ");
//  Serial.print(round(thermocouple.readCelsius() - 0.8));
//  Serial.print("   . Not Round ");
//  Serial.println(thermocouple.readCelsius() - 0.8);
//  Serial.print("[-0.0]C = Round ");
//  Serial.print(round(thermocouple.readCelsius() ));
//  Serial.print("   . Not Round ");
//  Serial.println(thermocouple.readCelsius() );
  //  Serial.print("F = ");
  //  Serial.println(thermocouple.readFahrenheit());

  
}

