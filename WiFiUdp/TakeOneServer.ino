//-- Libraries Included --------------------------------------------------------------
  #include <ESP8266WiFi.h>
//------------------------------------------------------------------------------------
  // Define I/O Pins
  #define     LED0      2           // WIFI Module LED
  #define     LED1      D0          // Connectivity With Client #1
  #define     LED2      D2          // Connectivity With Client #2
  #define     BUTTON    D1          // Connectivity ReInitiate Button
//------------------------------------------------------------------------------------
  // Authentication Variables
  char*       TKDssid;              // SERVER WIFI NAME
  char*       TKDpassword;          // SERVER PASSWORD
//------------------------------------------------------------------------------------
  #define     MAXSC     6           // MAXIMUM NUMBER OF CLIENTS

  /*
   * MAXSC => Maximum Server Clients
   * They Are 6 Because
   * 1. Blue Fighter Suit
   * 2. Red Fighter Suit
   * 3. Corner Referee 1
   * 4. Corner Referee 2
   * 5. Corner Referee 3
   * 6. Corner Referee 4
   */

  WiFiServer  TKDServer(9001);      // THE SERVER AND THE PORT NUMBER
  WiFiClient  TKDClient[MAXSC];     // THE SERVER CLIENTS (Devices)
//====================================================================================

  void setup()
  {
    // Setting The Serial Port
    Serial.begin(115200);           // Computer Communication

    // Setting The Mode Of Pins
    pinMode(LED0, OUTPUT);          // WIFI OnBoard LED Light
    pinMode(LED1, OUTPUT);          // Indicator For Client #1 Connectivity
    pinMode(LED2, OUTPUT);          // Indicator For Client #2 Connectivity
    pinMode(BUTTON, INPUT_PULLUP);  // Initiate Connectivity

    // Print Message Of I/O Setting Progress
    Serial.println();
    Serial.println("I/O Pins Modes Set .... Done");

    // Setting Up A Wifi Access Point
    SetWifi("TAKEONE", "");
  }

//====================================================================================

  void loop()
  {
    // Checking For Available Clients
    AvailableClients();
    // Checking For Available Client Messages
    AvailableMessage();
  }

//====================================================================================

  void SetWifi(char* Name, char* Password)
  {
    // Stop Any Previous WIFI
    WiFi.disconnect();

    // Setting The Wifi Mode
    WiFi.mode(WIFI_AP_STA);
    Serial.println("WIFI Mode : AccessPoint Station");

    // Setting The AccessPoint Name & Password
    TKDssid      = Name;
    TKDpassword  = Password;

    // Starting The Access Point
    WiFi.softAP(TKDssid, TKDpassword);
    Serial.println("WIFI < " + String(TKDssid) + " > ... Started");

    // Wait For Few Seconds
    delay(1000);

    // Getting Server IP
    IPAddress IP = WiFi.softAPIP();

    // Printing The Server IP Address
    Serial.print("AccessPoint IP : ");
    Serial.println(IP);

    // Printing MAC Address
    Serial.print("AccessPoint MC : ");
    Serial.println(String(WiFi.softAPmacAddress()));

    // Starting Server
    TKDServer.begin();
    TKDServer.setNoDelay(true);
    Serial.println("Server Started");
  }

//====================================================================================

  void AvailableClients()
  {
    if (TKDServer.hasClient())
    {
      // Read LED0 Switch To Low If High.
      if(digitalRead(LED0) == HIGH) digitalWrite(LED0, LOW);

      // Light Up LED1
      digitalWrite(LED1, HIGH);

      for(uint8_t i = 0; i < MAXSC; i++)
      {
        //find free/disconnected spot
        if (!TKDClient[i] || !TKDClient[i].connected())
        {
          // Checks If Previously The Client Is Taken
          if(TKDClient[i])
          {
            TKDClient[i].stop();
          }

          // Checks If Clients Connected To The Server
          if(TKDClient[i] = TKDServer.available())
          {
            Serial.println("New Client: " + String(i));
          }

          // Continue Scanning
          continue;
        }
      }

      //no free/disconnected spot so reject
      WiFiClient TKDClient = TKDServer.available();
      TKDClient.stop();
    }
    else
    {
      // This LED Blinks If No Clients Where Available
      digitalWrite(LED0, HIGH);
      delay(250);
      digitalWrite(LED0, LOW);
      delay(250);
    }
  }

//====================================================================================

  void AvailableMessage()
  {
    //check clients for data
    for(uint8_t i = 0; i < MAXSC; i++)
    {
      if (TKDClient[i] && TKDClient[i].connected() && TKDClient[i].available())
      {
          while(TKDClient[i].available())
          {
            String Message = TKDClient[i].readStringUntil('\r');
            TKDClient[i].flush();
            Serial.println("Client No " + String(i) + " - " + Message);
          }
      }
    }
  }

//====================================================================================
