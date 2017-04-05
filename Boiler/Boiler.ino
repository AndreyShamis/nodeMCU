#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
#include <OneWire.h>
#include <DallasTemperature.h>
const int led = 13;
const char* ssid = "RadiationG";
const char* password = "polkalol";
#define ONE_WIRE_BUS D4 //D4 2
// Time to sleep (in seconds):
const int sleepTimeS = 10;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
ESP8266WebServer server(80);
// arrays to hold device address
DeviceAddress insideThermometer;

float getTemperature() {
  float temp;
//  do {
    sensor.requestTemperatures();
    temp = sensor.getTempC(insideThermometer);
//    delay(100);
//  } while (temp == 85.0 || temp == (-127.0));
//  return temp;
}

void handleRoot() {
  //digitalWrite(led, 1);
  //delay(20);
  float tempC = getTemperature();
  Serial.print("Temp C: ");
  Serial.println(tempC);
  String tmp = String(tempC);
  String message = "<h1> Temperature : " + tmp + "</h1><form action='/enable_led'><input style='font-size:62px' type='submit' value='Enable Led'></form><form action='/disable_led'><input style='font-size:62px' type='submit' value='Disable Led'></form>";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(200, "text/html", message);
  //digitalWrite(led, 0);


}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void save_setting(const char* fname,String value)
{
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
String read_setting(const char* fname)
{
  String s = "";
  File f = SPIFFS.open(fname , "r");
  if (!f){
    Serial.print("file open failed:");
    Serial.println(fname);
  }
  else{
    s=f.readStringUntil('\n');
    f.close();
  }
  return s;
}
void setup(void){

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
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
  server.on("/enable_led", [](){
    digitalWrite(led, 1);
    handleRoot();
  });
  server.on("/disable_led", [](){
    digitalWrite(led, 0);
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
  //ESP.deepSleep(sleepTimeS * 1000000);
}
int counter = 0;
void loop(void){
  
  server.handleClient();
  if(counter > 1000){
    counter = 0;
    float tempC = getTemperature();
    Serial.print("INFO: Temperature C: ");
    Serial.println(tempC);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI DISCONNECTED");
    delay(500);
  }
  delay(100);
  counter++;

}
