//deep sleep include
extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
#include <OneWire.h>
#include <DallasTemperature.h>

typedef enum{
  UNDEF   = 0,  //  UNKNOWN
  MANUAL  = 1,  //  Controlled by USER - manual diable, manual enable, secured by MAX_TMP
  AUTO    = 2,  //  Controlled by TIME, secured by MAX_TMP
  KEEP    = 3,  //  Controlled by BOARD, keep temperature between MAX_TMP <> TRASHHOLD_TMP, secured by MAX_TMP
} BoilerModeType;

#define               ONE_WIRE_BUS  D4 //D4 2
#define               BOILER_VCC    D7 //D7 13

//const int led         = D7; //13;
const char* ssid      = "RadiationG";
const char* password  = "polkalol";
const int sleepTimeS  = 10;  // Time to sleep (in seconds):
int counter           = 0;
bool boilerStatus     = 0;
float MAX_POSSIBLE_TMP = 40;
OneWire             oneWire(ONE_WIRE_BUS);
DallasTemperature   sensor(&oneWire);
ESP8266WebServer    server(80);
DeviceAddress       insideThermometer; // arrays to hold device address

BoilerModeType boilerMode = UNDEF;
//enum ADCMode {
//    ADC_TOUT = 33,
//    ADC_TOUT_3V3 = 33,
//    ADC_VCC = 255,
//    ADC_VDD = 255
//};
ADC_MODE(ADC_VCC);

float getTemperature() {
  sensor.requestTemperatures();
  return sensor.getTempC(insideThermometer);
}

String printTemperatureToSerial(){
  float tempC       = getTemperature();
  Serial.print("INFO: Temperature C: ");
  Serial.println(tempC);
  return String(tempC);
}

void handleRoot() {
  String tmp = printTemperatureToSerial();
  String message = "<h1> Temperature : " + tmp + "</h1><form action='/eb'><input style='font-size:82px' type='submit' value='Enable Load'></form><form action='/db'><input style='font-size:82px' type='submit' value='Disable Load'></form>";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += server.client();
  server.send(200, "text/html", message);
}

void enableBoiler(){
  float current_temp = getTemperature();
  if(current_temp > MAX_POSSIBLE_TMP){
    Serial.println("ERROR: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
  }
  else{
    boilerStatus = 1;
    digitalWrite(BOILER_VCC, 1);
  }
}

void disableBoiler(){
  boilerStatus = 0;
  digitalWrite(BOILER_VCC, 0);
}
void handleNotFound(){
  enableBoiler();
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args() + "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  disableBoiler();
}

/**
 * 
 */
void save_setting(const char* fname,String value){
  File f = SPIFFS.open(fname, "w");
  if (!f){
    Serial.print("Cannot open file:");
    Serial.println(fname);
    return;
  }
  f.println(value);
  Serial.print("Writed:" );
  Serial.println(value);
  f.close();
}

/**
 * 
 */
String read_setting(const char* fname){
  String s      = "";
  File f = SPIFFS.open(fname , "r");
  if (!f){
    Serial.print("file open failed:");
    Serial.println(fname);
  }
  else{
    s = f.readStringUntil('\n');
    f.close();
  }
  return s;
}

/**
 * 
 */
void setup(void){
//ADC_MODE(ADC_VCC);
  pinMode(BOILER_VCC, OUTPUT);
  disableBoiler();
  sensor.begin();
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  Serial.println("INFO: Starting SPIFFS...");
  SPIFFS.begin();
  Serial.println("PASS: SPIFFS startted.");
//  Serial.println("INFO: Compile SPIFFS");
//  SPIFFS.format();
  WiFi.begin(ssid, password);

  // Wait for connection
  Serial.print("INFO: Connecting to ");
  Serial.print(ssid);
  Serial.print(password);
  Serial.println("...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("PASS: Connected to ");
  Serial.println(ssid);
  Serial.print("PASS:  IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("PASS: MDNS responder started");
  }
  Serial.println("-----------------------------------");
  Serial.print("Found ");
  Serial.print(sensor.getDeviceCount(), DEC);
  Serial.println(" devices.");
  Serial.print("Parasite power is: ");
  if (sensor.isParasitePowerMode())
    Serial.println("ON");
  else
    Serial.println("OFF");
  if (!sensor.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensor.setResolution(insideThermometer, 9);
  server.on("/", handleRoot);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/eb", [](){
    enableBoiler();
    handleRoot();
  });
  server.on("/db", [](){
    disableBoiler();
    handleRoot();
  });
  server.onNotFound(handleNotFound);
  Serial.println("INFO: Staring HTTP server...");
  server.begin();
  Serial.println("PASS: HTTP server started");
//  save_setting("/ssid", ssid);
//  save_setting("/password", password);
  Serial.println(read_setting("/ssid"));
  Serial.println(read_setting("/password"));
      // Sleep
  Serial.println("ESP8266 in sleep mode");
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
}

/**
 * 
 */
void loop(void){
  
  server.handleClient();
  if(boilerStatus){
    float current_temp = getTemperature();
    if(current_temp > MAX_POSSIBLE_TMP){
      Serial.println("WARNING: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
      Serial.println("WARNING: Disabling Load");
      disableBoiler();
    }  
  }
  
  if(counter > 1000){
    counter = 0;
    printTemperatureToSerial();
  }
  if(counter == 5){
    Serial.print("Boiler MODE: ");
    Serial.println(boilerMode);
    
    Serial.print("Boiler status: ");
    Serial.println(boilerStatus);

    Serial.print("getFlashChipId: ");
    Serial.println(ESP.getFlashChipId());

    Serial.print("getFlashChipSize: ");
    Serial.println(ESP.getFlashChipSize());

    Serial.print("getFlashChipSpeed: ");
    Serial.println(ESP.getFlashChipSpeed());

    Serial.print("getFlashChipMode: ");
    Serial.println(ESP.getFlashChipMode());

    Serial.print("getSdkVersion: ");
    Serial.println(ESP.getSdkVersion());

    Serial.print("getCoreVersion: ");
    Serial.println(ESP.getCoreVersion());

    Serial.print("getBootVersion: ");
    Serial.println(ESP.getBootVersion());

    Serial.print("getBootMode: ");
    Serial.println(ESP.getBootMode());

    Serial.print("getCpuFreqMHz: ");
    Serial.println(ESP.getCpuFreqMHz());

    Serial.print("getResetReason: ");
    Serial.println(ESP.getResetReason());

    Serial.print("getResetInfo: ");
    Serial.println(ESP.getResetInfo());

    Serial.print("macAddress: ");
    Serial.println(WiFi.macAddress());

    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());

    Serial.print("channel: ");
    Serial.println(WiFi.channel());
//
//    Serial.print("magicFlashChipSize: ");
//    Serial.println(ESP.magicFlashChipSize());
//
//    Serial.print("magicFlashChipSpeed: ");
//    Serial.println(ESP.magicFlashChipSpeed());
//
//    Serial.print("magicFlashChipMode: ");
//    Serial.println(ESP.magicFlashChipMode());

    Serial.print("getSketchSize: ");
    Serial.println(ESP.getSketchSize());

    Serial.print("getFreeSketchSpace: ");
    Serial.println(ESP.getFreeSketchSpace());
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI DISCONNECTED");
    delay(500);
  }
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  delay(100);
  counter++;

}


