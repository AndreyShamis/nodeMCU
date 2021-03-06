//deep sleep include
extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
}
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
#include <OneWire.h>
#include <DallasTemperature.h>

typedef enum {
  UNDEF   = 0,  //  UNKNOWN
  MANUAL  = 1,  //  Controlled by USER - manual diable, manual enable, secured by MAX_TMP
  AUTO    = 2,  //  Controlled by TIME, secured by MAX_TMP
  KEEP    = 3,  //  Controlled by BOARD, keep temperature between MAX_TMP <> TRASHHOLD_TMP, secured by MAX_TMP
} BoilerModeType;

#define   UART_BAUD_RATE            921600
#define   ONE_WIRE_BUS              D4 //D4 2
#define   LOAD_VCC                  D7 //D7 13
#define   NUMBER_OF_DEVICES         1
#define   CS_PIN                    D3

const char  *ssid                   = "RadiationG";
const char  *password               = "polkalol";

const int   sleepTimeS              = 10;  // Time to sleep (in seconds):
int         counter                 = 0;
bool        boilerStatus            = 0;
float       MAX_POSSIBLE_TMP        = 65;
bool        secure_disabled         = false;
float       temperatureKeep         = 40;
float       current_temp            = -10;
OneWire             oneWire(ONE_WIRE_BUS);
DallasTemperature   sensor(&oneWire);
ESP8266WebServer    server(80);
DeviceAddress       insideThermometer; // arrays to hold device address
WiFiUDP ntpUDP;


// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800, 60000);

BoilerModeType boilerMode = MANUAL;
//enum ADCMode {
//    ADC_TOUT = 33,
//    ADC_TOUT_3V3 = 33,
//    ADC_VCC = 255,
//    ADC_VDD = 255
//};
ADC_MODE(ADC_VCC);
String  getAddressString(DeviceAddress deviceAddress);
void    disableLoad();
void    enableLoad();
float   getTemperature(int dev = 0);
String  printTemperatureToSerial();
String  read_setting(const char* fname);
void    save_setting(const char* fname, String value);


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/**

*/
void setup(void) {
  //ADC_MODE(ADC_VCC);
  pinMode(LOAD_VCC, OUTPUT);
  disableLoad();
  sensor.begin();
  Serial.begin(UART_BAUD_RATE);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
  Serial.println("INFO: Starting SPIFFS...");
  SPIFFS.begin();
  Serial.println("PASS: SPIFFS startted.");
  //  Serial.println("INFO: Compile SPIFFS");
  //  SPIFFS.format();
  WiFi.mode(WIFI_STA);       //  Disable AP Mode - set mode to WIFI_AP, WIFI_STA, or WIFI_AP_STA.
  WiFi.begin(ssid, password);

  // Wait for connection
  Serial.println("INFO: Connecting to [" + String(ssid) + "][" + String(password) + "]...");
  int con_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    con_counter++;
    if (con_counter % 20 == 0) {
      Serial.println("");
      Serial.println("WARNING: Still connecting...");
    }
  }
  Serial.println("");
  Serial.print("PASS: Connected to [" + String(ssid) + "]  IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266")) {
    Serial.println("PASS: MDNS responder started");
  }
  Serial.println("-----------------------------------");
  timeClient.begin();

  Serial.print("INFO: Found ");
  Serial.print(sensor.getDeviceCount(), DEC);
  Serial.println(" Thermometer Dallas devices.");

  Serial.print("INFO: Parasite power is: ");
  if (sensor.isParasitePowerMode()) {
    Serial.println("isParasitePowerMode ON");
  }
  else {
    Serial.println("isParasitePowerMode OFF");
  }
  
  if (!sensor.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
  }

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensor.setResolution(insideThermometer, 11);
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/eb", []() {
    enableLoad();
    boilerMode = MANUAL;
    handleRoot();
  });
  server.on("/db", []() {
    disableLoad();
    boilerMode = MANUAL;
    handleRoot();
  });
  server.on("/keep", []() {
    saveBoilerMode();
    handleRoot();
  });

  server.onNotFound(handleNotFound);
  Serial.println("INFO: Staring HTTP server...");
  server.begin();
  Serial.println("PASS: HTTP server started");

  //  save_setting("/ssid", ssid);
  //  save_setting("/password", password);
  //Serial.println(read_setting("/ssid"));
  //Serial.println(read_setting("/password"));
  // Sleep
  //Serial.println("ESP8266 in sleep mode");
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  timeClient.update();

}

/**
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
*/
void loop(void) {

  server.handleClient();
  if (counter % 50 == 0) {
    current_temp = getTemperature();
  }
  if (!boilerStatus && boilerMode == KEEP) {
    if (current_temp < _min(temperatureKeep,MAX_POSSIBLE_TMP) && current_temp > 0) {
      Serial.println("WARNING: Keep enabled, enable boiler");
      enableLoad();
    }
  }
  if (boilerStatus) {
    if (current_temp > MAX_POSSIBLE_TMP || (boilerMode == KEEP && current_temp > temperatureKeep)) {
      Serial.println("WARNING: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
      Serial.println("WARNING: Disabling Load");
      disableLoad();
      secure_disabled = true;
    }
  }

  if (current_temp < 1) {
    Serial.println("WARNING: Very LOW temperatute. " + String(current_temp));
    Serial.println("WARNING: Disabling Load");
    disableLoad();
    secure_disabled = true;
  }

  if (counter > 1000) {
    counter = 0;
    printTemperatureToSerial();
  }
  if (counter == 25) {
    timeClient.update();
    Serial.println("INFO: -----------------------------------------------------------------------");
    Serial.println("Boiler MODE: " + String(boilerMode) + "\t\t\t Boiler status: " + String(boilerStatus));
    Serial.println("getFlashChipId: " + String(ESP.getFlashChipId()) + "\t\t getFlashChipSize: " + String(ESP.getFlashChipSize()));
    Serial.println("getFlashChipSpeed: " + String(ESP.getFlashChipSpeed()) + "\t getFlashChipMode: " + String(ESP.getFlashChipMode()));
    Serial.println("getSdkVersion: " + String(ESP.getSdkVersion()) + "\t getCoreVersion: " + ESP.getCoreVersion() + "\t\t getBootVersion: " + ESP.getBootVersion());
    Serial.println("getBootMode: " + String(ESP.getBootMode()));
    Serial.println("getCpuFreqMHz: " + String(ESP.getCpuFreqMHz()));
    Serial.println("HostName :" + WiFi.hostname() + "\tmacAddress: " + WiFi.macAddress() + "\t Channel : " + String(WiFi.channel()) + "\t\t\t RSSI: " + WiFi.RSSI());
    Serial.println("getSketchSize: " + String(ESP.getSketchSize()) + "\t\t getFreeSketchSpace: " + String(ESP.getFreeSketchSpace()));
    //Serial.println("getResetReason: " + ESP.getResetReason());
    //Serial.println("getResetInfo: " + ESP.getResetInfo());
    Serial.println("Address : " + getAddressString(insideThermometer));
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI DISCONNECTED");
    delay(500);
  }

  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  delay(100);
  counter++;
//  if (counter % 200 == 0) {
//    Serial.println(timeClient.getFormattedTime());
//    //Serial.println(timeClient.getEpochTime());
//  }

}

String build_index() {
  String ret_js = String("") + "boiler = {" +
                  "'boiler_mode': '" + String(boilerMode) + "'," +
                  "'boiler_status': '" + String(boilerStatus) + "'," +
                  "'disbaled_by_watch': '" + String(secure_disabled) + "'," +
                  "'max_temperature': '" + String(MAX_POSSIBLE_TMP) + "'," +
                  "'keep_temperature': '" + String(temperatureKeep) + "'," +
                  "'current_temperature': '" + String(printTemperatureToSerial()) + "'," +
                  "'flash_chip_id': '" + String(ESP.getFlashChipId()) + "'," +
                  "'flash_chip_size': '" + String(ESP.getFlashChipSize()) + "'," +
                  "'flash_chip_speed': '" + String(ESP.getFlashChipSpeed()) + "'," +
                  "'flash_chip_mode': '" + String(ESP.getFlashChipMode()) + "'," +
                  "'core_version': '" + ESP.getCoreVersion() + "'," +
                  "'sdk_version': '" + String(ESP.getSdkVersion()) + "'," +
                  "'boot_version': '" + ESP.getBootVersion() + "'," +
                  "'boot_mode': '" + String(ESP.getBootMode()) + "'," +
                  "'cpu_freq': '" + String(ESP.getCpuFreqMHz()) + "'," +
                  "'mac_addr': '" + WiFi.macAddress() + "'," +
                  "'wifi_channel': '" + String(WiFi.channel()) + "'," +
                  "'rssi': '" + WiFi.RSSI() + "'," +
                  "'sketch_size': '" + String(ESP.getSketchSize()) + "'," +
                  "'free_sketch_size': '" + String(ESP.getFreeSketchSpace()) + "'," +
                  "'dallas_addr': '" + getAddressString(insideThermometer) + "'," +
                  "'time_str': '" + timeClient.getFormattedTime() + "'," +
                  "'time_epoch': '" + timeClient.getEpochTime() + "'," +
                  "'hostname': '" + WiFi.hostname() + "'" +
                  "};";
  String ret = String("") + "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>Boiler Info</title></head>" +
               " <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.0/jquery.min.js'></script>\n" +
               " <script src='http://tm.anshamis.com/js/boiler.js'></script>\n" +
               " <link rel='stylesheet' type='text/css' href='http://tm.anshamis.com/css/boiler.css'>\n" +
               "<body><script>" + ret_js + "</script>\n" +
               "<div id='content'></div>" +
               "<script>\n " +               "$(document).ready(function(){ onBoilerPageLoad(); });</script>\n" +
               "</body></html>";
  return ret;
}
String build_device_info() {
  String ret = "<pre>Boiler MODE: " + String(boilerMode) + "\t\t\t Boiler status: " + String(boilerStatus);
  ret += "\ngetFlashChipId: " + String(ESP.getFlashChipId()) + "\t\t getFlashChipSize: " + String(ESP.getFlashChipSize());
  ret += "\ngetFlashChipSpeed: " + String(ESP.getFlashChipSpeed()) + "\t getFlashChipMode: " + String(ESP.getFlashChipMode());
  ret += "\ngetSdkVersion: " + String(ESP.getSdkVersion()) + "\t getCoreVersion: " + ESP.getCoreVersion() + "\t\t getBootVersion: " + ESP.getBootVersion();
  ret += "\ngetBootMode: " + String(ESP.getBootMode());
  ret += "\ngetCpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  ret += "\nmacAddress: " + WiFi.macAddress() + "\t Channel : " + String(WiFi.channel()) + "\t\t\t RSSI: " + WiFi.RSSI();
  ret += "\ngetSketchSize: " + String(ESP.getSketchSize()) + "\t\t getFreeSketchSpace: " + String(ESP.getFreeSketchSpace());
  //ret += "\ngetResetReason: " + ESP.getResetReason();
  //ret += "\ngetResetInfo: " + ESP.getResetInfo();
  ret += "\nAddress : " + getAddressString(insideThermometer) + "</pre>";
  return ret;
}
void handleRoot() {

  //  for (uint8_t i=0; i<server.args(); i++){
  //    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  //  }
  //  message += server.client();
  String message = build_index();
  server.send(200, "text/html", message);
}

/***

*/
void saveBoilerMode() {
  String message = "Saving Boiler Mode\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args() + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    if (server.argName(i) == "temperatureKeep") {
      temperatureKeep = server.arg(i).toFloat();
      boilerMode = KEEP;
      Serial.println("Keep temperature " + String(temperatureKeep));
      if (temperatureKeep > MAX_POSSIBLE_TMP) {
        temperatureKeep = MAX_POSSIBLE_TMP;
        Serial.println("Override Keep temperature " + String(temperatureKeep));
      }
    }
  }
}

/**

*/
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args() + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/**
   Enable Load
*/
void enableLoad() {
  float current_temp = getTemperature(0);
  if (current_temp > MAX_POSSIBLE_TMP) {
    Serial.println("ERROR: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
  }
  else {
    secure_disabled = false;
    boilerStatus = 1;
    digitalWrite(LOAD_VCC, 1);
  }
}

/**
   Disable Load
*/
void disableLoad() {
  boilerStatus = 0;
  digitalWrite(LOAD_VCC, 0);
}
//
///**
//   Get Temperature
//*/
//float getTemperature() {
//  sensor.requestTemperatures();
//  return sensor.getTempC(insideThermometer);
//}
/**
   Get Temperature
*/
float getTemperature(int dev/*=0*/) {
  //Serial.println("DEBUG: Requesting device " + String(dev));
  sensor.setWaitForConversion(false);  // makes it async
  sensor.requestTemperatures();
  sensor.setWaitForConversion(true);  // makes it async
  return sensor.getTempCByIndex(dev);
  //return sensor.getTempC(insideThermometer[dev]);
}

/**
   Get end print temperature to serial
*/
String printTemperatureToSerial() {
  int dc = sensor.getDeviceCount();
  for (int i = 0 ; i < dc; i++) {
    current_temp       = getTemperature(i);
    Serial.print("INFO: Temperature[" + String(i) + "] C: ");
    Serial.println(current_temp);
  }
  return String(current_temp);
}

/**
 * 
 */
String get_thermometers_addr() {
  String data = "[";
  int i = 0;
  int counter = sensor.getDeviceCount();
  for (i = 0; i < counter; i++) {
    data = data + String("\"") + String(getAddressString(insideThermometer)) + String("\" , "); // TODO insideThermometer[i]
    //Serial.println("Build  " + String(i) + " : " + String(getAddressString(insideThermometer[i])) + " "  + data);
  }
  data = data + "]";
  return data;
}

/**
  Convert Dallas Address to String
*/
String getAddressString(DeviceAddress deviceAddress) {
  String ret = "";
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) {
      ret += "0";
    }
    ret += String(deviceAddress[i], HEX);
    if (i < 7) {
      ret += ":";
    }
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/**
  Write to file content on SPIFFS
*/
void save_setting(const char* fname, String value) {
  File f = SPIFFS.open(fname, "w");
  if (!f) {
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
  Read file content from SPIFFS
*/
String read_setting(const char* fname) {
  String s      = "";
  File f = SPIFFS.open(fname , "r");
  if (!f) {
    Serial.print("file open failed:");
    Serial.println(fname);
  }
  else {
    s = f.readStringUntil('\n');
    f.close();
  }
  return s;
}

