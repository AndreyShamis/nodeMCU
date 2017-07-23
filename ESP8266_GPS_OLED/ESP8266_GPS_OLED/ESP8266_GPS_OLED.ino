#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <TinyGPS.h>                                  // Tiny GPS Plus Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

static const int RXPin = D7, TXPin = D8;                // Ublox 6m GPS module to pins 12 and 13
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

const double Home_LAT = 32.27922;                      // Your Home Latitude
const double Home_LNG = 34.8523;                     // Your Home Longitude

TinyGPS gps;                                        // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device

void setup()
{
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();                                      // Clear OLED display
  display.setTextSize(1);                               // Set OLED text size to small
  display.setTextColor(WHITE);                          // Set OLED color to White
  display.setCursor(0, 0);                              // Set cursor to 0,0
  display.println("GPS example");
  display.println("Version" + TinyGPS::library_version());
  display.display();                                     // Update display
  delay(1500);                                          // Pause 1.5 seconds
  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600
  Serial.print("Simple TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  delay(3500);  
}

void loop()
{
    float flat, flon;
    unsigned long age, date, time, chars = 0;
      print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
    gps.f_get_position(&flat, &flon, &age);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Latitude  : ");
  display.println(flat, 5);
  display.print("Longtite  : ");
  display.println(flon, 4);
    print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  display.print("Satellites: ");
  display.println(gps.satellites());
  display.print("Elevation : ");
  display.println(gps.f_altitude());
  //display.println("ft");
//  display.print("Time UTC  : ");
//  display.print(gps.time.hour());                       // GPS time UTC
//  display.print(":");
//  display.print(gps.time.minute());                     // Minutes
//  display.print(":");
//  display.println(gps.time.second());                   // Seconds
  display.print("Heading   : ");
  display.println(gps.f_course());
  display.print("Speed     : ");
  display.println(gps.f_speed_kmph());

  unsigned long Distance_To_Home = (unsigned long)TinyGPS::distance_between(flat, flon, Home_LAT, Home_LNG)/1000;
  display.print("KM to Home: ");                        // Have TinyGPS Calculate distance to home and display it
  display.print(Distance_To_Home);
  display.display();                                     // Update display
    print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, Home_LAT, Home_LNG) / 1000, 0xFFFFFFFF, 9);
  delay(2000);

  smartdelay(1500);                                      // Run Procedure smartDelay

//  if (millis() > 5000 && gps.charsProcessed() < 10)
//    display.println(F("No GPS data received: check wiring"));
}

static void smartdelay(unsigned long ms)                // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  Serial.println("");
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len-1] = ' ';
  Serial.println(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
  Serial.println("");
}

