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
String  getAddressString(DeviceAddress deviceAddress);
void    disableBoiler();
void    enableBoiler();
float   getTemperature();
String  printTemperatureToSerial();
String  read_setting(const char* fname);
void    save_setting(const char* fname, String value);

String build_device_info(){
    String ret = "<pre>Boiler MODE: " + String(boilerMode) + "\t\t\t Boiler status: " + String(boilerStatus);
    ret += "\ngetFlashChipId: " + String(ESP.getFlashChipId()) + "\t\t getFlashChipSize: " + String(ESP.getFlashChipSize());
    ret += "\ngetFlashChipSpeed: " + String(ESP.getFlashChipSpeed()) + "\t getFlashChipMode: " + String(ESP.getFlashChipMode());
    ret += "\ngetSdkVersion: " + String(ESP.getSdkVersion()) + "\t getCoreVersion: " + ESP.getCoreVersion() + "\t\t getBootVersion: " + ESP.getBootVersion();
    ret += "\ngetBootMode: " + String(ESP.getBootMode());
    ret += "\ngetCpuFreqMHz: " + String(ESP.getCpuFreqMHz());
    ret += "\nmacAddress: " + WiFi.macAddress() + "\t Channel : " + String(WiFi.channel()) + "\t\t\t RSSI: " + WiFi.RSSI();
    ret += "\ngetSketchSize: " + String(ESP.getSketchSize()) + "\t\t getFreeSketchSpace: " + String(ESP.getFreeSketchSpace());
    ret += "\ngetResetReason: " + ESP.getResetReason();
    ret += "\ngetResetInfo: " + ESP.getResetInfo();
    ret += "\nAddress : " + getAddressString(insideThermometer) + "</pre>";
    return ret;
}
void handleRoot() {
  String tmp = printTemperatureToSerial();
  String message = build_device_info() + "<h1> Temperature : " + tmp + "</h1>" + 
  "<form action='/'><input style='font-size:82px' type='submit' value='Refresh'></form>" +
  "<form action='/eb'><input style='font-size:82px' type='submit' value='Enable Load'></form>" +
  "<form action='/db'><input style='font-size:82px' type='submit' value='Disable Load'></form>" +
  "<form method='POST' action='/save_boilermode'><input type='text' value='" + String(boilerMode)  + "' /><input style='font-size:82px' type='submit' value='Save Boiler mode'></form>";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += server.client();
  server.send(200, "text/html", message);
}

/***
 * 
 */
void saveBoilerMode(){
  String message = "Saving Boiler Mode\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args() + "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(200, "text/plain", message);
  
}

/**
 * 
 */
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args() + "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
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
  if (sensor.isParasitePowerMode()){
    Serial.println("ON");
  }
  else{
    Serial.println("OFF");
  }
    
  if (!sensor.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
  }

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensor.setResolution(insideThermometer, 12);
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
  server.on("/save_boilermode", [](){
    saveBoilerMode();
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
    Serial.println("INFO: -----------------------------------------------------------------------");
    Serial.println("Boiler MODE: " + String(boilerMode) + "\t\t\t Boiler status: " + String(boilerStatus));
    Serial.println("getFlashChipId: " + String(ESP.getFlashChipId()) + "\t\t getFlashChipSize: " + String(ESP.getFlashChipSize()));
    Serial.println("getFlashChipSpeed: " + String(ESP.getFlashChipSpeed()) + "\t getFlashChipMode: " + String(ESP.getFlashChipMode()));
    Serial.println("getSdkVersion: " + String(ESP.getSdkVersion()) + "\t getCoreVersion: " + ESP.getCoreVersion() + "\t\t getBootVersion: " + ESP.getBootVersion());
    Serial.println("getBootMode: " + String(ESP.getBootMode()));
    Serial.println("getCpuFreqMHz: " + String(ESP.getCpuFreqMHz()));
    Serial.println("macAddress: " + WiFi.macAddress() + "\t Channel : " + String(WiFi.channel()) + "\t\t\t RSSI: " + WiFi.RSSI());
    Serial.println("getSketchSize: " + String(ESP.getSketchSize()) + "\t\t getFreeSketchSpace: " + String(ESP.getFreeSketchSpace()));
    Serial.println("getResetReason: " + ESP.getResetReason());
    Serial.println("getResetInfo: " + ESP.getResetInfo());
    Serial.print("Address : " + getAddressString(insideThermometer));
//    Serial.print("magicFlashChipSize: " + String(ESP.magicFlashChipSize()));
//    Serial.print("magicFlashChipSpeed: " + String(ESP.magicFlashChipSpeed()));
//    Serial.print("magicFlashChipMode: " + String(ESP.magicFlashChipMode()));

  }

  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI DISCONNECTED");
    delay(500);
  }
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  delay(100);
  counter++;

}

/**
 * 
 */
String getAddressString(DeviceAddress deviceAddress){
  String ret = "";
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) {
      ret += "0";
    }
    ret += String(deviceAddress[i], HEX);
    if(i<7){
      ret += ":";
    }
  }
  return ret;
}

/**
 * Enable boiler
 */
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

/**
 * Disable Boiler
 */
void disableBoiler(){
  boilerStatus = 0;
  digitalWrite(BOILER_VCC, 0);
}


/**
 * Get Temperature
 */
float getTemperature() {
  sensor.requestTemperatures();
  return sensor.getTempC(insideThermometer);
}

/**
 * Get end print temperature to serial
 */
String printTemperatureToSerial(){
  float tempC       = getTemperature();
  Serial.print("INFO: Temperature C: ");
  Serial.println(tempC);
  return String(tempC);
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

