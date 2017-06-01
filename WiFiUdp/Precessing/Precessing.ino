/**
 * https://thearduinoandme.wordpress.com/tutorials/esp8266-send-receive-binary-data/
 * 
 * 
 * 
 */
// Processing UDP example to send and receive binary data
// press f to toggel between LED flickering twice per second or not
// the LED will illuminate when any other key is held down and
// turn back off when the key is released.

//import hypermedia.net.*;

UDP udp; // define the UDP object

String ip = "192.168.1.17"; // the remote IP address
int port = 8888; // the destination port

long previousMillis = 0;
int light = 0;
long interval = 500;
int flicker = 0;
int held = 0;

/**
 * 
 */
void setup() {
  udp = new UDP( this, 8888 ); // create a new datagram connection on port 8888
  //udp.log( true ); // <– printout the connection activity
  udp.listen( true ); // and wait for incoming message
}

/**
 * 
 */
void draw()
{
  if (flicker == 1) {
//    if (previousMillis < millis() – interval){
//      previousMillis = previousMillis + interval;
//      if (light == 0) {
//        byte[] message = new byte[2];
//        message[0] = 0;
//        message[1] = 0;
//        udp.send(message, ip , port);
//        light = 1;
//      }
//      else {
//        byte[] message = new byte[2];
//        message[0] = 0;
//        message[1] = 1;
//        udp.send(message, ip , port);
//        light = 0;
//      }
//    }
  }
}

/**
 * 
 */
void keyPressed() {

  if (key == 'f') {
    if (flicker == 1) {
      flicker = 0;
      byte[] message = new byte[2];
      message[0] = 0;
      message[1] = 0;
      udp.send(message, ip , port);
      light = 0;
    }
    else {
      flicker = 1;
    }
  }
  else {
    if (flicker == 0 && held == 0) {
      byte[] message = new byte[2];
      message[0] = 0;
      message[1] = 1;
      udp.send(message, ip , port);
      held = 1;
    }
  }
}

/**
 * 
 */
void keyReleased() {
  if (key != 'f') {
    if (flicker == 0) {
      byte[] message = new byte[2];
      message[0] = 0;
      message[1] = 0;
      udp.send(message, ip , port);
      held = 0;
    }
  }
}

/**
 * 
 */
void receive( byte[] data ) { // <– default handler
  //void receive( byte[] data, String ip, int port ) { // <– extended handler

  for (int i = 0; i < data.length; i++)
    print(char(data[i]));
  println();
}
