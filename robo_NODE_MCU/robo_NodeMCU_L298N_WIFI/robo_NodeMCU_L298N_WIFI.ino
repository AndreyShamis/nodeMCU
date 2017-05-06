#include <ESP8266WiFi.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

WiFiServer server(80);


//подключение драйвера двигателя L298N
#define L9110_A_IA 14 // Pin D12 --> выход int1 на мотор "А"
#define L9110_A_IB 12 // Pin D11 --> выход int2 на мотор "А"
#define L9110_B_IA 13 // Pin D10 --> выход int3 на мотор "В"
#define L9110_B_IB 15 // Pin D9 --> выход int4 на мотор "В"
 
// какие контакты за что отвечают
#define MOTOR_A_PWM L9110_A_IA // контроль скорости моторов "А"
#define MOTOR_A_DIR L9110_A_IB // питание мотора
#define MOTOR_B_PWM L9110_B_IA // контроль скорости моторов "В"
#define MOTOR_B_DIR L9110_B_IB // питание мотора

int t = 0;
int rnd;

void setup ()  {
   Serial.begin(115200);


  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
 
 pinMode( MOTOR_A_DIR, OUTPUT );
 pinMode( MOTOR_A_PWM, OUTPUT );
 pinMode( MOTOR_B_DIR, OUTPUT );
 pinMode( MOTOR_B_PWM, OUTPUT );
 stop(10);


}

void loop ()  { 

    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
//-------------------------------------------------------------------------------------------//
//                                                                                           //
//                   Это основной цикл нашей программы!!!                                    //
//                                                                                           //
//-------------------------------------------------------------------------------------------//

  if (req.indexOf("/fforward") != -1)
    fforward(1023);


  if (req.indexOf("/fback") != -1)
    fback(1023);


  if (req.indexOf("/fleft") != -1)
    fleft(1023);


  if (req.indexOf("/fright") != -1)
    fright(1023);

  if (req.indexOf("/stop") != -1)
    stop(10);


    if (req.indexOf("/test") != -1)  {
forward(1023,500); //вперед 
stop(100);
back(1023,500);    //назад
stop(100);         //стоп на 0.5 секунд
left(1023,500);    //влево
stop(100);         //стоп на 0.5 секунд
right(1023,500);   //вправо
stop(100);         //стоп на 0.5 секунд

    }



client.flush();
client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n OK </html>\n");
  delay(1);
  Serial.println("Client disonnected");


//-------------------------------------------------------------------------------------------//
//                                                                                           //
//                        Завершаем наш основной цикл  !!!                                   //
//                                                                                           //
//-------------------------------------------------------------------------------------------//

}
//***************************************************
void move(int motor, int speed, int rotate){
//создаем функцию движения с
//тремя параметрами (моторы, скорость, направление вращения)
// моторы - 1 или 0
// скорость - от 0 до 255
// направление вращения - 1 или 0

  boolean inPin1 = LOW;
  
  if(rotate == 1){
    inPin1 = HIGH;
    
  if(motor == 1){
    digitalWrite(MOTOR_A_DIR, inPin1);
    analogWrite(MOTOR_A_PWM, 1023-speed);
          }else{
            digitalWrite(MOTOR_B_DIR, inPin1);
            analogWrite(MOTOR_B_PWM, 1023-speed);
          }    
    }
    
  else{
    inPin1 = LOW;
  
  if(motor == 1){
    digitalWrite(MOTOR_A_DIR, inPin1);
    analogWrite(MOTOR_A_PWM, 1023-(1023-speed));
          }else{
            digitalWrite(MOTOR_B_DIR, inPin1);
            analogWrite(MOTOR_B_PWM, 1023-(1023-speed));
          }  
 }
 
}
//***************************************************
void stop(int t){  //создаем функцию "стоп"

  move(1,0,0);
  move(2,0,0);
 delay(t);
}

//***************************************************
void forward(int s, int time){//создаем функцию "движение вперед"
  move(1,s,0);
  move(2,s,0);
  delay(time);
}
//***************************************************
void fforward(int s){//создаем функцию "движение вперед без временного ограничения"
  move(1,s,0);
  move(2,s,0);
}
//***************************************************
void back(int s, int time){ //создаем функцию "движение назад"
  move(1,s,1);
  move(2,s,1);
  delay(time);
}
//***************************************************
void fback(int s){ //создаем функцию "движение назад без временного ограничения"
  move(1,s,1);
  move(2,s,1);
}
//***************************************************
void left(int s, int time){ //создаем функцию "поворот вправо"
  move(1,s,1);
  move(2,s,0);
  delay(time);
}
//***************************************************
void fleft(int s){ //создаем функцию "поворот вправо без временного ограничения"
  move(1,s,1);
  move(2,s,0);
}
//***************************************************
void right(int s, int time){ //создаем функцию "поворот лево"
  move(1,s,0);
  move(2,s,1);
  delay(time);
}
//***************************************************
void fright(int s){ //создаем функцию "поворот лево без временного ограничения"
  move(1,s,0);
  move(2,s,1);
}


