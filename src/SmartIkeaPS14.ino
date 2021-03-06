#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "index.h" //Our HTML webpage contents

#define IN1  4
#define IN2  14
#define IN3  12
#define IN4  13
#define rPIN8  0 //RELAY

// WiFi parameters
const char* ssid = "UPC8391079";
const char* password = "dhpbacK3kuct";
String time2;
ESP8266WebServer server(80); //Server on port 80

IPAddress ipBroadCast(239, 255, 255, 250);
unsigned int udpRemotePort=1982;
unsigned int udplocalPort=1982;
const int UDP_PACKET_SIZE = 100;
char udpBuffer[ UDP_PACKET_SIZE];
char incomingPacket[255];  // buffer for incoming packets
char  replyPacekt[] = "ESP8266 got multicast message";  // a reply string to send back
boolean discovered = false;
IPAddress * YEELIGHT_IP = new IPAddress(192,168,0,102);
const char* yeelight_id = "0x0000000004a097e2";

WiFiUDP udp;

unsigned int curSpeed = 900; // Notabene: nicht unter 900
unsigned int upwardSpeed = 900; // Notabene: nicht unter 900
unsigned int downwardSpeed = 700; // Notabene: nicht unter 700

//Intervall for checkEndpoints
unsigned long interval=5000; // the time we need to wait
unsigned long previousMillis=0; // millis() returns an unsigned long.
//Intervall for checkEndpoints
unsigned long interval2=250; // the time we need to wait
unsigned long previousMillis2=0; // millis() returns an unsigned long.
//Motor Timeout
unsigned long interval3=10000; // the time we need to wait
unsigned long previousMillis3=0; // millis() returns an unsigned long.
int Steps = 0;
boolean Direction = false; //true=up - false=down
boolean stopMotor = true; 
boolean opening = false;
boolean closing = false;
int light = 1;
int potiMaxOrig = 270;
int potiMinOrig = 163;
int potiMax = potiMaxOrig;
int potiMin = potiMinOrig;
int median[3];

String serialData;

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(rPIN8, OUTPUT);
  //digitalWrite(rPIN8, HIGH);
  Serial.begin(9600);
  Serial.setTimeout(50);
  int sensorValue = analogRead(A0);
  median[0]=sensorValue;median[1]=sensorValue;median[2]=sensorValue;
  
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    print(".");
  }

  //If connection successful show IP address in serial monitor
  println("");
  print("Connected to ");
  println(ssid);
  print("IP address: ");
  println(WiFi.localIP().toString());  //IP address assigned to your ESP

  server.on("/", handleRoot);
  server.on("/openCloseDeathstar", openCloseDeathstar);
  server.on("/setDeathstarPosition", setDeathstarPosition);
  server.on("/lightbulb", lightbulbLogic);
  server.on("/setBrightness", brightnessLogic);
  server.on("/setColor", colorLogic);
  server.on("/setMode", modeLogic);
  server.on("/uptime", uptimeLogic);
  server.on("/discover", discover);
  server.on("/console", consoleLogic);
  server.on("/initPage", initpageLogic);
  server.begin();                  //Start server
  println("HTTP server started");
  
  configTime(2 * 3600, 0, "de.pool.ntp.org", "pool.ntp.org", "time.nist.gov");
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  int time_trials = 0;
  while (timeinfo->tm_year == 70 && time_trials < 10) {
    time(&now);
    timeinfo = localtime(&now);
    print(":");
    delay(1000);
    time_trials++;
  }
  time2 = ctime(&now);
  println("Boot up time: " + time2);

  // set udp port for listen
  udp.begin(udplocalPort);
  //Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), udplocalPort);

  //OTA Code:
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  // println(String(analogRead(A0)));
  
  // check if WLAN is connected
  if (WiFi.status() != WL_CONNECTED)
  
  {
    WiFiStart();
  }
  server.handleClient();          //Handle client requests

  //****STEPPER MOTOR
  stepper(1);
  delayMicroseconds(curSpeed);

  if(!discovered){
    sendAndReceiveUDP();
  }

  if ( Serial.available() ) {  Serial.write( Serial.read() ); }

  ArduinoOTA.handle();
}

//================================================================
// Function to send udp message and discover yeelight
//================================================================
void discover(){
  discovered = false;
  }
void sendAndReceiveUDP(){
  unsigned long currentMillis = millis(); // grab current time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
   //****ESP CONNECTION
   discoverYeelight();
   previousMillis = millis();
  }

   int packetSize = udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    int len = udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    //println("UDP packet contents: ");
    //println(incomingPacket);
    String str(incomingPacket);
    if(str.indexOf("yeelight") >= 0){
      if(str.indexOf(yeelight_id) >= 0){
        print("Yeelight Found: ");
        String ip = udp.remoteIP().toString();
        println(ip);
        int oneIndex = ip.indexOf('.');
        String one = ip.substring(0, oneIndex);
        String rest = ip.substring(oneIndex+1);
        int twoIndex = rest.indexOf('.');
        String two = rest.substring(0, twoIndex);
        rest = rest.substring(twoIndex+1);
        int threeIndex = rest.indexOf('.');
        String three = rest.substring(0, threeIndex);
        rest = rest.substring(threeIndex+1);
        int fourIndex = rest.indexOf('.');
        String four = rest.substring(0, fourIndex);
        discovered = true;
        YEELIGHT_IP = new IPAddress(one.toInt(), two.toInt(),three.toInt(),four.toInt());
      } else {
        print("yeelight found but wrong ID:");
        print(str.substring(str.indexOf("id: "), str.indexOf("id: ")+18));
        print(" != ");
        println(yeelight_id);
      }
    }
  }
}

void discoverYeelight()
{
  //Serial.println("UDP send");
  strcpy(udpBuffer, "M-SEARCH * HTTP/1.1\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb\r\n"); 
  udp.beginPacketMulticast(ipBroadCast, udpRemotePort ,WiFi.localIP());
  udp.write(udpBuffer, sizeof(udpBuffer));
  udp.endPacket();
}

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 println("Connected");
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
void openCloseDeathstar() {
  previousMillis3 = millis();
  String serverArg = server.arg("openclose");
  if(serverArg.indexOf("open") >= 0){
    Direction = true;
    stopMotor = false;
    opening = true;
    closing = false;
    potiMin = potiMinOrig;
    potiMax = potiMaxOrig;
    println( "ACTION EXECUTED: Deathstar Open");
  }
  if (serverArg.indexOf("close") >= 0){
    Direction = false;
    stopMotor = false;
    opening = false;
    closing = true;
    potiMax = potiMaxOrig;
    potiMin = potiMinOrig;
    println( "ACTION EXECUTED: Deathstar Close");
  }
 println("Deathstar Action = " + serverArg);
 server.send(200, "text/plain", "ok");
}
void setDeathstarPosition() {
  String serverArg = server.arg("value");
  int setValue = serverArg.toInt();
  int sensorValue = analogRead(A0);
  if(setValue < sensorValue){
    Direction = false;
    stopMotor = false;
    opening = false;
    closing = true;
    potiMin = setValue;
  } else {
    Direction = true;
    stopMotor = false;
    opening = true;
    closing = false;
    potiMax = setValue;
  }
 println("Set Deathstar Position = " + server.arg("value"));
 println("Current Position: " + String(analogRead(A0)));
 server.send(200, "text/plain", "ok");
}


String YEELIGHT_TOGGLE = "{\"id\": \"1\", \"method\": \"toggle\", \"params\":[]}\r\n";
String YEELIGHT_ON = "{\"id\": \"1\", \"method\": \"set_power\", \"params\":[\"on\", \"smooth\", 500]}\r\n";
String YEELIGHT_OFF = "{\"id\": \"1\", \"method\": \"set_power\", \"params\":[\"off\", \"smooth\", 500]}\r\n";
String YEELIGHT_BRIGHTNESS = "{\"id\": \"1\", \"method\": \"set_bright\", \"params\":[, \"smooth\", 500]}\r\n";
String YEELIGHT_RGB = "{\"id\": \"1\", \"method\": \"set_rgb\", \"params\":[ , \"smooth\", 500]}\r\n";
const uint16_t YEELIGHT_PORT = 55443;
WiFiClient yeelight_client;
void lightbulbLogic() {
  server.send(200, "text/plain", "ok");
  String serverArg =  server.arg("onoff");
  if(serverArg.indexOf("on") >= 0){
    //digitalWrite(rPIN8, HIGH);
    if (!yeelight_client.connect(*YEELIGHT_IP, YEELIGHT_PORT)) {
      println("yeelight connection failed");
    } else {
      println("yeelight connected");
      yeelight_client.print(YEELIGHT_ON);
    }

      yeelight_client.stop();

    light = 1;
  }
  if(serverArg.indexOf("off") >= 0){
    //digitalWrite(rPIN8, LOW);
    if (!yeelight_client.connect(*YEELIGHT_IP, YEELIGHT_PORT)) {
      println("yeelight connection failed");
    } else {
      println("yeelight connected");
      yeelight_client.print(YEELIGHT_OFF);
    }

      yeelight_client.stop();

    light = 0;
  }
 println("Lightbulb Action = " + server.arg("onoff"));
}

void brightnessLogic(){
    server.send(200, "text/plain", "ok");
    String serverArg =  server.arg("value");
    if (!yeelight_client.connect(*YEELIGHT_IP, YEELIGHT_PORT)) {
      println("yeelight connection failed");
      return;
    } else {
      println("yeelight connected");
    }
  //Adjust yeelight String
  String brightnessSendString = YEELIGHT_BRIGHTNESS;
  int insertIndex = brightnessSendString.indexOf("[");
  String tmp = brightnessSendString.substring(insertIndex+1);
  brightnessSendString = brightnessSendString.substring(0, insertIndex+1);
  brightnessSendString += serverArg;
  brightnessSendString += tmp;
    
  yeelight_client.print(brightnessSendString);

    yeelight_client.stop();

  println("Set Brightness = " + server.arg("value"));
}
unsigned long previousMillis4 = millis(); 
unsigned long interval4 = 500;
void colorLogic(){
  server.send(200, "text/plain", "ok");
  unsigned long currentMillis4 = millis(); // grab current time
  if ((unsigned long)(currentMillis4 - previousMillis4) >= interval4) {
    String serverArg =  server.arg("value");

  // Get rid of '#' and convert it to integer
  if (!yeelight_client.connect(*YEELIGHT_IP, YEELIGHT_PORT)) {
    println("yeelight connection failed");
    return;
  } else {
    println("yeelight connected");
  }
  //Hex String to Int
  int number = (int) strtol( &serverArg[1], NULL, 16);
  println((String)number);
  
  //Adjust yeelight String
  String rgbSendString = YEELIGHT_RGB;
  String val = String(number);
  int insertIndex = rgbSendString.indexOf("[");
  String tmp = rgbSendString.substring(insertIndex+1);
  rgbSendString = rgbSendString.substring(0, insertIndex+1);
  rgbSendString += val;
  rgbSendString += tmp;

    
  yeelight_client.print(rgbSendString);
  println("Set Color = " + server.arg("value"));
  previousMillis4 = millis();

    yeelight_client.stop();

  }
}
String YEELIGHT_MODE_FIRE = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"700,2,1700,50,1200,2,1700,30,400,2,1700,50,1500,2,1700,30,700,2,1700,40\"]}\r\n";
String YEELIGHT_MODE_NIGHT = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",1,1,\"1000,1,10271231,44\"]}\r\n";
String YEELIGHT_MODE_SUNRISE = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",2,1,\"1000,2,1700,1,900000,2,1700,100\"]}\r\n";
String YEELIGHT_MODE_SUNSET = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",2,2,\"1000,2,1700,100,900000,2,1700,1\"]}\r\n";
String YEELIGHT_MODE_PARTY_VFAST = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"50,1,16715266,100,50,1,196577,100,50,1,16712443,100,50,1,458498,100,50,1,255,100,50,1,16734355,100,50,1,16514818,100\"]}\r\n";
String YEELIGHT_MODE_PARTY_FAST = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"250,1,16715266,100,250,1,196577,100,250,1,16712443,100,250,1,458498,100,250,1,255,100,250,1,16734355,100,250,1,16514818,100\"]}\r\n";
String YEELIGHT_MODE_PARTY_MED= "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"950,1,16715266,100,950,1,196577,100,950,1,16712443,100,950,1,458498,100,950,1,255,100,950,1,16734355,100,950,1,16514818,100\"]}\r\n";
String YEELIGHT_MODE_PARTY_SLOW = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"2950,1,16715266,100,2950,1,196577,100,2950,1,16712443,100,2950,1,458498,100,2950,1,255,100,2950,1,16734355,100,2950,1,16514818,100\"]}\r\n";
String YEELIGHT_MODE_PARTY_VSLOW = "{\"id\":1,\"method\":\"set_scene\", \"params\":[\"cf\",0,1,\"10950,1,16715266,100,10250,1,196577,100,10250,1,16712443,100,10250,1,458498,100,10250,1,255,100,10250,1,16734355,100,10250,1,16514818,100\"]}\r\n";
void modeLogic() {
  server.send(200, "text/plain", time2);
  String serverArg =  server.arg("value");
  String mode2exec;
  if(serverArg.indexOf("Fire") >= 0){mode2exec =  YEELIGHT_MODE_FIRE;}
  if(serverArg.indexOf("Night") >= 0){mode2exec =  YEELIGHT_MODE_NIGHT;}
  if(serverArg.indexOf("Sunrise") >= 0){mode2exec =  YEELIGHT_MODE_SUNRISE;}
  if(serverArg.indexOf("Sunset") >= 0){mode2exec =  YEELIGHT_MODE_SUNSET;}
  if(serverArg.indexOf("Party(very fast)") >= 0){mode2exec =  YEELIGHT_MODE_PARTY_VFAST;}
  if(serverArg.indexOf("Party(fast)") >= 0){mode2exec =  YEELIGHT_MODE_PARTY_FAST;}
  if(serverArg.indexOf("Party(medium)") >= 0){mode2exec =  YEELIGHT_MODE_PARTY_MED;}
  if(serverArg.indexOf("Party(slow)") >= 0){mode2exec =  YEELIGHT_MODE_PARTY_SLOW;}
  if(serverArg.indexOf("Party(very slow)") >= 0){mode2exec =  YEELIGHT_MODE_PARTY_VSLOW;}
  if (!yeelight_client.connect(*YEELIGHT_IP, YEELIGHT_PORT)) {
      println("yeelight connection failed");
      return;
    } else {
      println("yeelight connected");
    }
    yeelight_client.print(mode2exec);

      yeelight_client.stop();

}

void uptimeLogic() {
 println("Uptime = " + time2);
 server.send(200, "text/plain", time2);
}
void initpageLogic() {
 println("Page Initialization ..." );
 String serialString = Serial.readString();
 int startIndex = serialString.indexOf("{\"position\"");
 /*while( startIndex == -1){
    serialString = Serial.readString();
    startIndex = serialString.indexOf("{\"position\"");
 }
 String jsonAnswer = serialString.substring(startIndex, startIndex+29);*/
 String jsonAnswer = "{\"position\": " + (String)analogRead(A0) +", \"light\": " + (String)light + "}";
 //String jsonAnswer = "{\"position\": 750, \"light\": 1}";
 print("Initializing with: ");
 println(jsonAnswer);
 //{"position": 750, "light": 1}
 server.send(200, "text/json", jsonAnswer);
}

void consoleLogic() {
 String jsonAnswer = "{\"text\": \"" + (String)serialData + "\"}";
 server.send(200, "text/json", jsonAnswer);
}

int lastAverage = 0;
void checkEndpoints(){
   // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  median[1]=median[0];median[2]=median[1];median[0]=sensorValue;
  int MedianSsensorValue = (median[0]+median[1]+median[1]+median[2]+median[2])/5;
  if(opening == 1) {
    if(MedianSsensorValue > potiMax){
      stopMotor = true;
      closing = 1;
      opening = 0;
      Direction = false;
    }
  }
  if(closing == 1) {
    if(MedianSsensorValue < potiMin ){
      stopMotor = true;
      closing = 0;
      opening = 1;
      Direction = true;
    }
  }
  unsigned long currentMillis3 = millis(); // grab current time
  if ((unsigned long)(currentMillis3 - previousMillis3) >= interval3) {
    if(lastAverage == MedianSsensorValue || lastAverage == MedianSsensorValue+1 || lastAverage == MedianSsensorValue-1) {
      println("STOP AUTOMATIC");
      stopMotor = false; // true
    }
    lastAverage = MedianSsensorValue;
    previousMillis3 = millis();
  }
}

void stepper(int xw){
  //Serial.println(xw);
  for (int x=0;x<xw;x++){
    switch(Steps){
       case 0:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, HIGH);
       break; 
       case 1:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, HIGH);
       break; 
       case 2:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, LOW);
       break; 
       case 3:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, HIGH);
         digitalWrite(IN4, LOW);
       break; 
       case 4:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
       case 5:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, HIGH);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
         case 6:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
       case 7:
         digitalWrite(IN1, HIGH); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, HIGH);
       break; 
       default:
         digitalWrite(IN1, LOW); 
         digitalWrite(IN2, LOW);
         digitalWrite(IN3, LOW);
         digitalWrite(IN4, LOW);
       break; 
    }
    SetDirection();
  }
} 
void SetDirection(){
  if(stopMotor==0){
    if(Direction==1){ Steps--; curSpeed = upwardSpeed;}
    if(Direction==0){ Steps++; curSpeed = downwardSpeed;}
    if(Steps>7){Steps=0;}
    if(Steps<0){Steps=7; }
    unsigned long currentMillis2 = millis(); // grab current time
    if ((unsigned long)(currentMillis2 - previousMillis2) >= interval2) {
     //****ESP CONNECTION
     checkEndpoints();
     previousMillis2 = millis();
    }
  }
}

void println(String text){
  Serial.println(text);
  text.trim();
  text.replace("\"", "");
  serialData = serialData + text + "<br>";
}
void print(String text){
  Serial.print(text);
  serialData = serialData + text;
}

int ulReconncount = 0;
void WiFiStart()
{
  ulReconncount++;

  // Connect to WiFi network
  println("");
  println("");
  print("Connecting to ");
  println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    print(".");
  }
  println("");
  println("WiFi connected");

  // Start the server
  server.begin();
  println("Server started");

  // Print the IP address
  println(WiFi.localIP().toString());
  print("Reconnect count: ");
  println((String)ulReconncount);
}
