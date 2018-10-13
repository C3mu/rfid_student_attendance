#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "FS.h"
const char WiFiAPPSK[] = "918273465"; //mat khau wifi
char c3museverAddress[] = "c3mu.esy.es";//sever
String Keyconnect = "TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ"; //it not randoms
     //              TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ
//#define 

unsigned long timeout;
boolean pull,newdata;

String dessid     = "Snowfoxsd";
String depassword = "11112222";



WiFiServer server(80);
WiFiClient client; //main client per time 
WiFiClient client1;
IPAddress ip; //save password

void loadlastwifidata(String &ssid, String &pass){
  for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) {
    pass += char(EEPROM.read(i));
  }
}

void connectwifi(String &ssid, String &pass){ 
  WiFi.reconnect();
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long tcn = millis();
  while ((WiFi.status() != WL_CONNECTED) && ((millis() - tcn) < 20000)) { 
    delay(500);
    Serial.print(F("."));
  }
  if (WiFi.status() != WL_CONNECTED){
    WiFi.setAutoConnect(false);
    WiFi.disconnect();
    Serial.println("cantconnect");
  }else Serial.println("connected");
}
  
void setup() {
  // put your setup code here, to run once:
  digitalWrite(12,LOW);
  Serial.begin(115200);
  EEPROM.begin(100);
  SPIFFS.begin();
  String ssid,pass;
  loadlastwifidata(ssid,pass);
  Serial.println(F("connecting wifi"));
  //connectwifi(ssid,pass);
  connectwifi(dessid,depassword);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //Start the server
  server.begin();
  digitalWrite(12,HIGH);
  client.setNoDelay(1); /// fast fast fast yay
//  client = server.available();
  }

void loop() {
  if (pull){ // keep connection for faster data transpace

      client1 = server.available(); //open new gate
      
      if (client1){ 
        String dat;      
        for(int i=1;i<=6;i++){
         dat += client1.readStringUntil('\r');
        }
        if(ip == client.remoteIP()){
          //client.println("HTTP/1.1 403 OVER\r \nServer: ESP8266 \nConnection: close\n\n");
          client.stop();
          client = client1;
          pull = false;
          return;
        }
        if(dat.indexOf(Keyconnect) != -1){ //seaching the key
          client.println("HTTP/1.1 403 OVER\r \nServer: ESP8266 \nConnection: close\n\n");  
          client.stop();
          client1.println("HTTP/1.1 200 Oke\r \nServer: ESP8266 \nConnection: close\n\n");  
            File file = SPIFFS.open("/next.html", "r");
          client1.write(file);
          client1.stop();  
          client1.flush();
          file.close();
          ip = client.remoteIP();
          client = client1;
          pull = false;
          return;
        }else{
          Authenticate(client1);
        }
      }   
      
      if (millis() - timeout >= 50000){//reset the connection every 50s
      client.println("HTTP/1.1 204 No new data\r");
      client.println("Server: ESP8266");
      client.println("Connection: Keep-Alive");      
      client.println("Keep-Alive: timeout=60, max=70");  
      client.println();          
      client.flush();
      client.stop();
      Serial.println("stop pull");  
      pull = false; 
    }
    
}
else{
  cliendworddata();
}
}
void cliendworddata(){
  client = server.available();
    if (client) {
      String line;
    for(int i=1;i<=6;i++){
    line += client.readStringUntil('\r');////();//
    }
     ///checktheadminaddrest//////////////////////////////////////////////
     if(ip != client.remoteIP()){   
      //checkthe password
       if (line.indexOf(Keyconnect) != -1){  
         ip = client.remoteIP();
         cliendworddata();
         return;
         //set admin ip && replay 
      }else{      
      Authenticate(client);
      }
      }
     else{

///XMLhttprequestrepont!

    if (line.indexOf("Checkdata") != -1){   
      timeout = millis();
      pull = true;
      return;  //backtoloop &&keeep conection
    }
    if (line.indexOf("GetDayData") != -1){   
      senddaydata();
      //client.flush();
      return;
    }

 ///request page

    //most using pager
    if (line.indexOf("/style.css") != -1) {
      senddata("/style.css");
      return;
    }
  
    else if (line.indexOf("/home") != -1) {
      senddata("/home.html");
      return;
    }
    
    else if (line.indexOf("/write") != -1) {
      senddata("/write.html");
      return;
    }
    
    else if (line.indexOf("/setup") != -1) {
      senddata("/setup.html");
      return;
    }
    
    //most unuse
    else if ((line.indexOf("icon.png") != -1)||(line.indexOf("favicon.ico") != -1)) {
      senddata("/icon.png");
      return;
    }  
    
    else { // unknowdata ??? --- go home kid
      senddata("/next.html");
    }
 client.flush();
}
}
}
void Authenticate(WiFiClient &client){
      client.println("HTTP/1.0 401 Unauthorized");/// xac thuc
      client.println("WWW-Authenticate: Basic realm=\"Cemu#628173\""); 
      client.stop();  
      client.flush();
}
void senddaydata(){////////////////////////////////////////////////////////uncomplete
  //readdatafromsdcard and send to client
  
}

void senddata(String names){
client.println("HTTP/1.1 200 OK");
client.println("Server: ESP8266");
client.println("Connection: Keep-Alive");
if (names.indexOf("next.html") != -1) {
    client.println("Location: /home"); //dobble changepage
}
client.println();//end of headers
File file = SPIFFS.open(names, "r");
client.write(file);
client.stop();
file.close();
return;
}

//String peeks(){
//  String returns;
//    uint8_t buffer[100];//peek the data to know what to do
//    client.peekBytes(buffer,100);
//    for(int i=0;i<=100;i++){
//      returns += char(buffer[i]);
//    }
//   return returns;
//}
