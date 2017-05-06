//deep sleep include
extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
}
#include <Time.h>
#define __LED_MATRIX

#ifdef __LED_MATRIX
#include <LedMatrix.h>
#endif
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_HMC5883_U.h>
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

#define               ONE_WIRE_BUS  D4 //D4 2
#define               BOILER_VCC    D7 //D7 13
#define               NUMBER_OF_DEVICES 1
#define               CS_PIN        D3
#define TIME_MSG_LEN 11 // time sync to PC is HEADER followed by Unix time_t as ten ASCII digits
#define TIME_HEADER 'T' // Header tag for serial time sync message
#define TIME_REQUEST 7 // ASCII bell character requests a time sync message 

const char  *ssid             = "RadiationG";
const char  *password         = "polkalol";
const int   sleepTimeS        = 10;  // Time to sleep (in seconds):
int         counter           = 0;
bool        boilerStatus      = 0;
float       MAX_POSSIBLE_TMP  = 65;
bool        secure_disabled   = false;
float       temperatureKeep   = 40;
float       current_temp      = -10;
OneWire             oneWire(ONE_WIRE_BUS);
DallasTemperature   sensor(&oneWire);
ESP8266WebServer    server(80);
DeviceAddress       insideThermometer; // arrays to hold device address
WiFiUDP ntpUDP;

// SDA connect D2
// SCL connect to D1

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12346);

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800, 60000);


#ifdef __LED_MATRIX
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);
#endif

BoilerModeType boilerMode = MANUAL;
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

String build_index() {
  String ret_js = String("") + "boiler = {" +
                  "'boiler_mode': '" + String(boilerMode) + "'," +
                  "'boiler_status': '" + String(boilerStatus) + "'," +
                  "'disbaled_by_watch': '" + String(secure_disabled) + "'," +
                  "'max_temperature': '" + String(MAX_POSSIBLE_TMP) + "'," +
                  "'keep_temperature': '" + String((int)temperatureKeep) + "'," +
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
               " <script src='http://tm.hldns.ru/js/boiler.js'></script>\n" +
               " <link rel='stylesheet' type='text/css' href='http://tm.hldns.ru/css/boiler.css'>\n" +
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
/**

*/
void setup(void) {
  //ADC_MODE(ADC_VCC);
  pinMode(BOILER_VCC, OUTPUT);
  disableBoiler();
  sensor.begin();
  Serial.begin(115200);
  Serial.println("");
  Serial.println("PASS: Serial communication started.");
#ifdef __LED_MATRIX
  ledMatrix.init();
  //ledMatrix.setTextAlignment(TEXT_ALIGN_RIGHT);
  ledMatrix.setIntensity(3); // range is 0-15
  ledMatrix.setText(" AnDrEy ");
#endif
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
  if (sensor.isParasitePowerMode()) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
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
    enableBoiler();
    boilerMode = MANUAL;
    handleRoot();
  });
  server.on("/db", []() {
    disableBoiler();
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
  Serial.println(read_setting("/ssid"));
  Serial.println(read_setting("/password"));
  // Sleep
  //Serial.println("ESP8266 in sleep mode");
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  timeClient.begin();
  timeClient.update();


  /* Initialise the sensor */
  if (!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while (1);
  }

  /* Initialise the sensor */
  if (!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // displaySetRange(ADXL345_RANGE_8_G);
  // displaySetRange(ADXL345_RANGE_4_G);
  // displaySetRange(ADXL345_RANGE_2_G);

  /* Display some basic information on this sensor */
  sensor_t sensor;
  
  accel.getSensor(&sensor);
  displaySensorDetails(sensor);
  
  mag.getSensor(&sensor);
  displaySensorDetails(sensor);
  /* Display additional settings (outside the scope of sensor_t) */
  displayDataRate();
  displayRange();
  Serial.println("");
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
  if (boilerMode == KEEP && !boilerStatus) {
    if (current_temp < temperatureKeep &&  current_temp < MAX_POSSIBLE_TMP && current_temp > 0) {
      Serial.println("WARNING: Keep enabled, enable boiler");
      enableBoiler();
    }
  }
  if (boilerStatus) {
    if (current_temp > MAX_POSSIBLE_TMP || (boilerMode == KEEP && current_temp > temperatureKeep)) {
      Serial.println("WARNING: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
      Serial.println("WARNING: Disabling Load");
      disableBoiler();
      secure_disabled = true;
    }
  }

  if (current_temp < 1) {
    Serial.println("WARNING: Very LOW temperatute. " + String(current_temp));
    Serial.println("WARNING: Disabling Load");
    disableBoiler();
    secure_disabled = true;
  }

  if (counter > 1000) {
#ifdef __LED_MATRIX
    ledMatrix.setNextText(String(getTemperature()) + " C");
#endif
    counter = 0;
    printTemperatureToSerial();
  }
  if (counter == 5) {
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
#ifdef __LED_MATRIX
  ledMatrix.clear();

  ledMatrix.scrollTextLeft();
  //ledMatrix.scrollTextRight();
  ledMatrix.drawText();
  ledMatrix.commit(); // commit transfers the byte buffer to the displays
#endif
  //ESP.deepSleep(sleepTimeS * 1000000, RF_DEFAULT);
  delay(100);
  counter++;
  if (counter % 200 == 0) {
    if (Serial.available() )
    {
      processSyncMessage();
    }
    if (timeStatus() == timeNotSet) {
      Serial.println("waiting for sync message");
    }
    else {
      digitalClockDisplay();
    }

    Serial.println(timeClient.getFormattedTime());
    //Serial.println(timeClient.getEpochTime());

  }
  if (counter % 1 == 0) {
    /* Get a new sensor event */
    sensors_event_t event;

    mag.getEvent(&event);
    /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
    Serial.print("MAGNETIC X: "); Serial.print(event.magnetic.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  "); Serial.println("uT");

    // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
    // Calculate heading when the magnetometer is level, then correct for signs of axis.
    float heading = atan2(event.magnetic.y, event.magnetic.x);

    // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
    // Find yours here: http://www.magnetic-declination.com/
    // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
    // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
    float declinationAngle = 0.22;
    heading += declinationAngle;

    // Correct for when signs are reversed.
    if (heading < 0)
      heading += 2 * PI;

    // Check for wrap due to addition of declination.
    if (heading > 2 * PI)
      heading -= 2 * PI;

    // Convert radians to degrees for readability.
    float headingDegrees = heading * 180 / M_PI;

    Serial.print("Heading (degrees): "); Serial.println(headingDegrees);

    accel.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("ACCELERATION X: "); Serial.print(event.acceleration.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  "); Serial.println("m/s^2 ");

    //        /* Display the results (acceleration is measured in m/s^2) */
    //    Serial.print("magnetic X: "); Serial.print(event.magnetic.x); Serial.print("  ");
    //    Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
    //    Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  "); Serial.println("m/s^2 ");

    //    /* Display the results (acceleration is measured in m/s^2) */
    //    Serial.print("orientation ROLL: "); Serial.print(event.orientation.roll); Serial.print("  ");
    //    Serial.print("PITCH: "); Serial.print(event.orientation.pitch); Serial.print("  ");
    //    Serial.print("HEADING: "); Serial.print(event.orientation.heading); Serial.print("  "); Serial.println("");

    //                /* Display the results (acceleration is measured in m/s^2) */
    //    Serial.print("gyro X: "); Serial.print(event.gyro.x); Serial.print("  ");
    //    Serial.print("Y: "); Serial.print(event.gyro.y); Serial.print("  ");
    //    Serial.print("Z: "); Serial.print(event.gyro.z); Serial.print("  "); Serial.println("m/s^2 ");
  }
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/**

*/
String getAddressString(DeviceAddress deviceAddress) {
  String ret = "";
  for (uint8_t i = 0; i < 8; i++) {
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

/**
   Enable boiler
*/
void enableBoiler() {
  float current_temp = getTemperature();
  if (current_temp > MAX_POSSIBLE_TMP) {
    Serial.println("ERROR: Current temperature is bigger of possible maximum. " + String(current_temp) + ">" + String(MAX_POSSIBLE_TMP));
  }
  else {
    secure_disabled = false;
    boilerStatus = 1;
    digitalWrite(BOILER_VCC, 1);
  }
}

/**
   Disable Boiler
*/
void disableBoiler() {
  boilerStatus = 0;
  digitalWrite(BOILER_VCC, 0);
}


/**
   Get Temperature
*/
float getTemperature() {
  sensor.requestTemperatures();
  return sensor.getTempC(insideThermometer);
}

/**
   Get end print temperature to serial
*/
String printTemperatureToSerial() {
  current_temp       = getTemperature();
  Serial.print("INFO: Temperature C: ");
  Serial.println(current_temp);
  return String(current_temp);
}


/**

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

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void processSyncMessage() {
  // if time sync available from serial port, update time and return true
  while (Serial.available() >= TIME_MSG_LEN ) { // time message consists of header & 10 ASCII digits
    char c = Serial.read() ;
    Serial.print(c);
    if ( c == TIME_HEADER ) {
      time_t pctime = 0;
      for (int i = 0; i < TIME_MSG_LEN - 1; i++) {
        c = Serial.read();
        if ( c >= '0' && c <= '9') {
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number
        }
      }
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}
/**
   Adafruit_ADXL345
*/
void displaySensorDetails(sensor_t sensor)
{

  Serial.println("------------------------------------");
  Serial.println  ("Sensor:       " + String(sensor.name));
  Serial.println  ("Driver Ver:   " + sensor.version);
  Serial.println  ("Unique ID:    " + sensor.sensor_id);
  Serial.println  ("Max Value:    " + String(sensor.max_value) + " m/s^2");
  Serial.println  ("Min Value:    " + String(sensor.min_value) + " m/s^2");
  Serial.println  ("Resolution:   " + String(sensor.resolution) + " m/s^2");
  Serial.println("------------------------------------\n");
}

void displayDataRate(void)
{

  String data_rate = "";
  switch (accel.getDataRate())
  {
    case ADXL345_DATARATE_3200_HZ:
      data_rate = "3200";
      break;
    case ADXL345_DATARATE_1600_HZ:
      data_rate = "1600";
      break;
    case ADXL345_DATARATE_800_HZ:
      data_rate = "800";
      break;
    case ADXL345_DATARATE_400_HZ:
      data_rate = "400";
      break;
    case ADXL345_DATARATE_200_HZ:
      data_rate = "200";
      break;
    case ADXL345_DATARATE_100_HZ:
      data_rate = "100";
      break;
    case ADXL345_DATARATE_50_HZ:
      data_rate = "50";
      break;
    case ADXL345_DATARATE_25_HZ:
      data_rate = "25";
      break;
    case ADXL345_DATARATE_12_5_HZ:
      data_rate = "12.5";
      break;
    case ADXL345_DATARATE_6_25HZ:
      data_rate = "6.25";
      break;
    case ADXL345_DATARATE_3_13_HZ:
      data_rate = "3.13";
      break;
    case ADXL345_DATARATE_1_56_HZ:
      data_rate = "1.56";
      break;
    case ADXL345_DATARATE_0_78_HZ:
      data_rate = "0.78";
      break;
    case ADXL345_DATARATE_0_39_HZ:
      data_rate = "0.39";
      break;
    case ADXL345_DATARATE_0_20_HZ:
      data_rate = "0.20";
      break;
    case ADXL345_DATARATE_0_10_HZ:
      data_rate = "0.10";
      break;
    default:
      data_rate = "???";
      break;
  }
  Serial.println("Data Rate:  " + data_rate + " Hz");
}

void displayRange(void)
{
  String val = "";
  switch (accel.getRange())
  {
    case ADXL345_RANGE_16_G:
      val = "16";
      break;
    case ADXL345_RANGE_8_G:
      val = "8";
      break;
    case ADXL345_RANGE_4_G:
      val = "4";
      break;
    case ADXL345_RANGE_2_G:
      val = "2";
      break;
    default:
      val = "???";
      break;
  }
  Serial.println("Range:  +/- " + val + " g");
}


