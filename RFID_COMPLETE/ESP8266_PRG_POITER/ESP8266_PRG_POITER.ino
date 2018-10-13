
#include <dummy.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

const char WiFiAPPSK[] = "918273465"; //mat khau wifi
const char* c3museverAddress = "diemdanh.nghiachi.com";//sever  296437-36980  296437-36980 ?????
const String Keyconnect = "TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ"; //it not randoms

byte WRITEMODE; // 0 : no  ; 1 : writeone ; 2 : writeallnewcard 3:writealloldcard
boolean firsttimewrite,severconnect;
boolean trytocnt = true;
//String statusp = ""; // for update write startup  or not use
unsigned long placeinfile, timehpg, timesever; // for back to last place read data

//String dessid     = "Snowfoxsd";
//String depassword = "11112222";

String ssid, pass, batr;

char xd = '\n';

byte numofpullclient;

WiFiServer server(80); //open gate at 80
IPAddress listip[6];
WiFiClient mainclient;
LiquidCrystal_I2C lcd(0x27, 16, 2);

////////////////////khai bao mot so cau truc

struct rfiddata {
  String NAME;
  String COURSE;
  String PHONE;
};

struct alient {
  WiFiClient client;
  IPAddress ip; // ngan ngua tai lai trang ton tai nguyen v:
  byte pull = 0;
  /* PULL mode | WRITE | HOME
         0                      null or delete
         1                 un update
         2                 updateable
         3        writingone
         4        fullwrite un update
         5        fullwriteupdate
  */
  unsigned long timeout; //time to pull
  //struct alient *back;
  struct alient *next;
};

alient* makenewclient() {
  alient *p = new alient;
  if (p == NULL)
  {
    return NULL;
  }
  //  p->timeout = millis();
  p->next = NULL;
  return p;
};


/////////////////khai bao mot so vat the v:
int GATE;
alient *firstclient;//dia chi cua client dau tien
alient *la = firstclient;
struct rfiddata RFIDDATA;
///////////////////////////////////////////////////////////////////sys

void serialevento() {
  
  if (Serial.available()) {
    String Sdata = "";
    //String Sdata = Serial.readStringUntil('\r');
    while (Serial.available()) {
      Sdata  = Sdata + char(Serial.read());
      yield();
    }
    Serial.flush();
    //Serial.println(Sdata);
    //getdata
    if (Sdata.indexOf("Data:") != -1) { //Data:NGUYENDOQUOCANH+1234-01682956371#
      RFIDDATA.NAME = RFIDDATA.COURSE = RFIDDATA.PHONE = "";
      for (int i = Sdata.indexOf("Data:") + 5; i < Sdata.indexOf("+"); i++) {
        RFIDDATA.NAME = RFIDDATA.NAME + Sdata[i];
      }
      for (int i = Sdata.indexOf("+") + 1; i < Sdata.indexOf("-"); i++) {
        RFIDDATA.COURSE = RFIDDATA.COURSE + Sdata[i];
      }
      for (int i = Sdata.indexOf("-") + 1; i < Sdata.indexOf("#"); i++) {
        RFIDDATA.PHONE = RFIDDATA.PHONE + Sdata[i];
      }

      //save data to sd card
      
      String timea = String(hour()) + ":" + String(minute()) + ":" + String(second());
      String datas = timea + "\t" +  RFIDDATA.PHONE + "\n";
      if (WiFi.status() == WL_CONNECTED)
        {
        String tsData = "TIME=" + timea + "&PHONE=" +  RFIDDATA.PHONE;
        savedata(datas);
        sendtosever(tsData);
        }
      else
        {
        savewaitdata(datas);
        }
      lcd.show("WELCOME",RFIDDATA.NAME);
      
      //said to anyone waiting for data
      for (alient *p = firstclient; p != NULL; p = p -> next)
      {
        yield();
        if ((p->pull) == 2) {
          p->pull = 1;
        }
      }
     return;
    }

    if (Sdata.indexOf("WRITE_done") != -1) {

      for (alient *p = firstclient; p != NULL; p = p -> next) // CANCLETHE PULL
      {
        yield();
        if (p->pull == 3) {
          if (WRITEMODE == 1) {
            p->client.println(F("HTTP/1.1 200 \nServer: ESP8266\nStatup:WRITE_DONE\nConnection: close\n"));
            p->client.flush();
            p->client.stop();      
            lcd.show(F("WRITE COMPLETE"));
            break;
          }
          else if (WRITEMODE != 0) {
            String rps =  readas();
            if (rps != "") {
              p->client.println("HTTP/1.1 204 \nServer: ESP8266\nStatup:" + rps + "\nConnection: Keep-Alive\nKeep-Alive:  timeout=60, max=70\n");
              lcd.show(F("NOW WRITE"),rps);
            } else {
              p->client.println(F("HTTP/1.1 200 \nServer: ESP8266\nStatup:DONE\nConnection: close\n"));
              WRITEMODE = 0;
              lcd.show(F("WRITE COMPLETE"));
            }
            p->client.flush();
            p->client.stop();
            break;
          }

        }
      }
      if (WRITEMODE == 1) {
        WRITEMODE = 0;
      }
      return;
    }

    if (Sdata.indexOf("LASTCARD") != -1) {
      lcd.show(F("card readed ..."));
      //print to lcd
      return;
    }

    if (Sdata.indexOf("WRITE_DATA_FALSE") != -1) {
      lcd.show(F("WRITEFALSE"));
      for (alient *p = firstclient; p != NULL; p = p -> next) // CANCLETHE PULL
      {
        yield();
        if (p->pull == 3) {
          p->client.println(F("HTTP/1.1 200  \nServer: ESP8266\nStatup:WRITE_FALSE\nConnection: close\n"));
          p->client.flush();
          p->client.stop();
        }
      }
      WRITEMODE = 0;
    }
        
    if(Sdata.indexOf("btr:") != -1){
    batr = " ";
      for (unsigned int i = Sdata.indexOf("btr:")+4; i <= Sdata.indexOf("%"); i++) {
        batr = batr + Sdata[i];
      }
    }

    if(Sdata.indexOf("USINGUSB") != -1){
      if(trytocnt == false){ 
      if (WiFi.status() != WL_CONNECTED) {
          loadlastwifidata(ssid, pass);
          connectwifi(ssid,pass);
        }
        trytocnt = true;
      }
    }else{
    if (Sdata.indexOf("LOWbatery") != -1) {
      if (WiFi.status() == WL_CONNECTED) {
        trytocnt = false;
        disconnectwifi();
        }
      if(Sdata.indexOf("USINGbatery") != -1) {
        batr = "LOW!";
        if(trytocnt == true){
          trytocnt = false;
          disconnectwifi();
        }
      }
    }
    }

  }
  
}

void disconnectwifi() {
  WiFi.setAutoConnect(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("disconected");
  lcd.show(F("WIFI disconected"), F(""));
  lcd.checkdislay();
  return;
}

void connectwifi(String &ssid, String &pass) {
  WiFi.mode(WIFI_STA);
  WiFi.reconnect();
  WiFi.softAPdisconnect(true); 
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long tcn = millis();
  Serial.print(F("conectingWIFI"));
  lcd.show(F("conectingWIFI..."), F(""));
  while ((WiFi.status() != WL_CONNECTED) && ((millis() - tcn) < 20000)) {
    lcd.checkdislay();
    delay(500);
    lcd.show(String((millis() - tcn)));
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.setAutoConnect(false);
    WiFi.disconnect();
    Serial.println(F("cantconnect"));
    lcd.show(F("cantconnect WIFI"), F(""));
  } else {
    checkseverconect();
    Serial.println(F("connected"));
    lcd.show(F("WIFI connected"), F(""));
    syncfiledata();
    gettime();
    if(year() == 1970){
    gettime();
    }
  }
  lcd.checkdislay();
}

void setupWiFi() {
  lcd.show(F("change to AP mode"), F("ssid DDHS*"));
  lcd.checkdislay();
WiFi.softAPdisconnect(false);
IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];//6bytes char
  WiFi.softAPmacAddress(mac);//get
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);//lay 2 byte cuoi
  macID.toUpperCase();// chuyen macId thanh viet hoa
  String AP_NameString = "DDHS" + macID;//tranh trung lap dia chi ssid
  char AP_NameChar[AP_NameString.length() + 1];// do dai cua string + 1
  memset(AP_NameChar, 0, AP_NameString.length() + 1);// tuy chinh vung nho tu
  for (unsigned int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);// String vao char
  //  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);//hamchichapnhanmangchar
}

void turnwifi(){
  if(WiFi.status() != WL_CONNECTED){
  Serial.print("turnWIFI");
  disconnectwifi();
  setupWiFi(); 
  }
}

/////////////////////////////////////////////////////////////////////for SD card

void savedata(String datas) {
  using sd::File;
  SD.begin(15);
  // check the /data folder
  if (!SD.exists("./DATA")) {
    SD.mkdir("./DATA");
  }
  // open the file to add data into
  String ye = String(year());
  String filename = "./DATA/" + String(day()) + "-" + String(month()) + "-" + String(ye[2]) + String(ye[3]) + ".TXT"; // that it
  File file = SD.open(filename, FILE_WRITE); // open file at the end to write more.
  if (!file) {
    Serial.println(F("false to open file !"));
    return;
  }
  file.println(datas);
  file.close();
}

void savewaitdata(String datas) {
  using sd::File;  
  SD.begin(15);
  //try to begin sdcard two time
  //ya copy
  File file = SD.open("./WAITDATA.TXT", FILE_WRITE); // open file at the end to write more.
  String ye = String(year());
  String filename = String(day()) + "-" + String(month()) + "-" + String(ye[2]) + String(ye[3]) + "\t"; // that it
  file.print(filename);
  file.println(datas);
  file.close();// yes ... done !!!
}

void syncfiledata() { //put save wait data to data
  using sd::File;
  SD.begin(15);
  Serial.println("begin syncdata");
  //ya copy
  File file1 = SD.open("./WAITDATA.TXT", FILE_READ); // open file at the end to write more.
  File file2;
  lcd.show(F("syncdata..."), F(""));
  lcd.checkdislay();
  unsigned int tis = 0;
  while (true) {
    tis++;
    Serial.println(tis);
    yield();
    file1.readStringUntil('\r');
    String datename = file1.readStringUntil('\t');
    String fname = "./DATA/" + datename;
    String data = file1.readStringUntil('\n');
    if (data == "") {
      file2.close();
      break;
    }

    if (!sendtosever("SYNC=" + datename +"&="+ data)) { //updata the data to sever!
      file1.close();
      file2.close();
      return;
    }

    fname = fname + ".TXT";
    file2 = SD.open(fname, FILE_WRITE);
    file2.println(data);
    file2.close();
    Serial.println(data);
  }
  
  file1.close();
  if (SD.remove("WAITDATA.TXT")) {
    Serial.println("remove complete");
  }
  Serial.println("complete syncdata");
  /*read and check the
    date / time + data
    -> try to put the dat
  */
}

String readas() { //for write
  using sd::File; 
  SD.begin(15);
  rfiddata dat; //hmmm just 3xString
  // try to begin sdcard two time
  File file = SD.open("./WRITE.TXT", FILE_READ); // open file at the end to write more.
  if (!file) {
    return "";
  }
  if (firsttimewrite == false) {
    placeinfile = 0;
    file.readStringUntil('\n');
    placeinfile = file.position();
    firsttimewrite = true;
  }
  file.seek(placeinfile);
  dat.NAME = file.readStringUntil('\t');
  dat.COURSE = file.readStringUntil('\t');
  if (dat.COURSE == "") {
    file.close();
    return "";
  }
  dat.PHONE = file.readStringUntil('\n');
  placeinfile = file.position();
  file.close();
  Serial.print((WRITEMODE == 2) ? "NEWCARD" : "OLDCARD");
  Serial.println("reWRITE:" + dat.NAME + "+" + dat.COURSE + "-" + dat.PHONE + "#");
  String returnd = "WRITE:" + dat.NAME + "  " + dat.COURSE + "  " + dat.PHONE;
  return returnd;
}


//////////////////////////////////////////////////////////////////// connect with sever

void gettime() { // rebuild code at http://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/ By LUCIAN VADUVA
  const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
  static byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
  static WiFiUDP udp;
  const char* ntpServerName = "vn.pool.ntp.org";
  unsigned int localPort = 2390;
  udp.begin(localPort);

  struct X { // struct's as good as class
    // send an NTP request to the time server at the given address
    static void sendNTPpacket(IPAddress& address)
    {
      // set all bytes in the buffer to 0
      memset(packetBuffer, 0, NTP_PACKET_SIZE);
      // Initialize values needed to form NTP request
      // (see URL above for details on the packets)
      packetBuffer[0] = 0b11100011;   // LI, Version, Mode
      packetBuffer[1] = 0;     // Stratum, or type of clock
      packetBuffer[2] = 6;     // Polling Interval
      packetBuffer[3] = 0xEC;  // Peer Clock Precision
      // 8 bytes of zero for Root Delay & Root Dispersion
      packetBuffer[12]  = 49;
      packetBuffer[13]  = 0x4E;
      packetBuffer[14]  = 49;
      packetBuffer[15]  = 52;

      // all NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      udp.beginPacket(address, 123); //NTP requests are to port 123
      udp.write(packetBuffer, NTP_PACKET_SIZE);
      udp.endPacket();
    }
  };

  IPAddress timeServerIP; // time.nist.gov NTP server address
  WiFi.hostByName(ntpServerName, timeServerIP);
  X::sendNTPpacket(timeServerIP);
  unsigned long tim = millis();
    delay(1000);
    int cb = udp.parsePacket();
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      if(secsSince1900 != 0){
      // now convert NTP time into everyday time:
      //Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      //VN time = GTM+7 => that's 2208963600
      const unsigned long seventyYears = 2208963600UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      setTime(epoch);
      tim += 1000;
    }
  return;
}

boolean sendtosever(String tsData) {
  if (mainclient.connect(c3museverAddress, 80)) {
    Serial.println("senddata");
    mainclient.print(F("POST /index.php HTTP/1.1\nHost: diemdanh.nghiachi.com\nConnection: close\nAuthorization: Basic TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ==\nContent-Type: application/x-www-form-urlencoded\nContent-Length: "));/*Authorization: Basic TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ==\n*/
    mainclient.print(tsData.length());
    mainclient.print("\n\n");
    mainclient.print(tsData); 
    mainclient.readStringUntil('\n');
    mainclient.stop();  
    
    return true;
    }
return false;
}

void checkseverconect(){
    if ((unsigned long)(millis() - timesever) > 60000){ //resetconnection every 1m
    timesever = millis();
    if(mainclient.connect(c3museverAddress, 80)){
    mainclient.print(F("GET /pull.php?KEEPSTREAM=TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ HTTP/1.1\nHost:diemdanh.nghiachi.com\nConnection: keep-alive\nKEEPSTREAM:YES\nAuthorization: Basic TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ==\nContent-Type: application/x-www-form-urlencoded\n\n"));
    severconnect = true;
    mainclient.read();
    mainclient.flush();
    mainclient.stop();
    }else severconnect = false;
   }
}

/////////////////////////////////////////////////////////////////////// connect with client

void checkclient(alient *p1, alient *plast) {

  if (GATE <= 5) { // if out of gate then stop

    p1->client = server.available();//open gate and check

    if (p1->client) {

      if (p1 != firstclient) { // delete struct when complete request
        plast->next = p1->next;
        delete p1;
      }

      String line;
      for (int i = 1; i <= 6; i++) {
        line += p1->client.readStringUntil('\r');////();//
      }
      ///checktheadminaddrest//////////////////////////////////////////////
      if (line.indexOf(Keyconnect) != -1) { //true key then go
        listip[GATE] = p1->ip = p1->client.remoteIP();
        //////////////////////////XMLhttprequestrepont! there have two

        /*if(line.indexOf("UpLoadFiLe") != -1){
          Serial.println(line);
          while(true){
            String dataa = p1->client.readStringUntil('\r');
            if(dataa == ""){Serial.println("BREAKFAST");break;}
            Serial.println(dataa);
          }
          senddata("/write.html", p1);
          return;
          }*/

        if (line.indexOf("/home") != -1) { // complete

          if (line.indexOf("Checkdata") != -1) {
            p1->timeout = millis();
            p1->pull = 2;
            alient *pas = makenewclient();
            firstclient = pas;
            pas->next = p1;
            return;  //backtoloop &&keeep conection
          }

          if (line.indexOf("GetDayData") != -1) {

            for (alient *p = firstclient; p != NULL; p = p -> next) // CANCLETHE PULL
            {
              yield();
              if ((p1->ip == p->ip ) && (p->pull == 2)) {
                p->client.println(F("HTTP/1.1 204 No new data\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70\n"));
                p->client.flush();
                p->client.stop();
              }
            }

            senddaydata(p1);
            p1->client.flush();
            p1->client.stop();
            return;
          }
          senddata("/home.html", p1);
          return;
        }

        if (line.indexOf("/writepg=") != -1) {

          if (line.indexOf("WrItEdAtA") != -1) {
            //readmodelwrite
            String writedata = "";
            if (line.indexOf("tycrd=true") != -1) {
              writedata = "NEWCARD";
            } else {
              writedata = "OLDCARD";
            }
            //readdatawrite
            writedata += "reWRITE:";
            for (int i = line.indexOf("Data:") + 5; i < line.indexOf("&&&END"); i++) {
              writedata = writedata + line[i];
            }
            if(writedata.indexOf("%20")!= -1){
              writedata.replace("%20", " ");
            }
            writedata += "#";
            Serial.println(writedata);
            lcd.show(F("now write"),writedata);
            WRITEMODE = 1; // 0 : no  ; 1 : writeone ; 2 : writeall
            //prepairfordataget
            p1->client.println(F("HTTP/1.1 204 New data\ncontent-type:text\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70"));
            p1->client.println(F("Statup: please input card to write"));
            p1->client.println(F(""));
            p1->client.flush();
            p1->client.stop();
            return;
          }

          if (line.indexOf("StarWriteHangLoat") != -1) {
            //readmodelwrite
            if (line.indexOf("tycrd=true") != -1) {
              WRITEMODE = 2;
            } else {
              WRITEMODE = 3;
            }
            firsttimewrite = false;
            //prepairfordataget
            p1->client.println(F("HTTP/1.1 204 New data\ncontent-type:text\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70"));
            p1->client.println(F("Statup: NowWrite..."));
            p1->client.println(F(""));
            p1->client.flush();
            p1->client.stop();
            return;
          }

          if (line.indexOf("PULL") != -1) { // pullllllllllllllllllllllllllll
            if (firsttimewrite == false) {
              if (WRITEMODE != 1) {
                String rps =  readas();
                lcd.show(F("now write"),rps);
                if (rps == "") {
                  p1->client.println(F("HTTP/1.1 200 DONE \ncontent-type:text\nServer: ESP8266\nConnection: Close"));
                  p1->client.println(F("Statup: WRITE DONE"));
                  p1->client.println(F(""));
                  p1->client.flush();
                  p1->client.stop();
                  WRITEMODE = 0;
                  lcd.show(F("write"),F("DONE"));
                  return;
                } else {
                  p1->client.println(F("HTTP/1.1 204 DONE \ncontent-type:text\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70"));
                  p1->client.println("Statup: " + rps);
                  p1->client.println(F(""));
                  p1->client.flush();
                  p1->client.stop();
                  return;
                }
              }
            }

            p1->timeout = millis();
            p1->pull = 3;
            alient *pas = makenewclient();
            firstclient = pas;
            pas->next = p1;
            return;
          }

          if (line.indexOf("SkipThisCard") != -1) { //Skip this card
            Serial.println(F("cancle"));
            delay(20);
            for (alient *p = firstclient; p != NULL; p = p -> next) // CANCLETHE PULL
            {
              yield();
              if (p->pull == 3) {
                p->client.println(F("HTTP/1.1 200 cancle \nServer: ESP8266\nStatup  :Skip\nConnection: close\n"));
                p->client.flush();
                p->client.stop();
              }
            }
            if (WRITEMODE != 1) {
              String rps =  readas();
              lcd.show(F("now write"),rps);
              if (rps == "") {
                p1->client.println(F("HTTP/1.1 200 DONE \ncontent-type:text\nServer: ESP8266\nConnection: Close"));
                p1->client.println(F("Statup: WRITE DONE"));
                p1->client.println(F(""));
                p1->client.flush();
                p1->client.stop();
                WRITEMODE = 0;
                lcd.show(F("write done"),rps);
                return;
              } else {
                p1->client.println(F("HTTP/1.1 204 ctn \ncontent-type:text\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70"));
                p1->client.println("Statup: " + rps);
                p1->client.println(F(""));
                p1->client.flush();
                p1->client.stop();
                return;
              }
            }
          }

          if (line.indexOf("Cancle") != -1) { //Skip this card
            //prepairfordataget
            lcd.show("Cancle write");
            Serial.print("cancle");
            p1->client.println(F("HTTP/1.1 200 cancle\ncontent-type:text\nStatup:WRITE_CANCLE\nServer: ESP8266\nConnection: close\n"));
            p1->client.flush();
            p1->client.stop();
            for (alient *p = firstclient; p != NULL; p = p -> next) // CANCLETHE PULL
            {
              yield();
              if (p->pull == 3) {
                p->client.println(F("HTTP/1.1 200 cancle \nServer: ESP8266\nStatup:WRITE_CANCLE\nConnection: Keep-Alive\nKeep-Alive: close\n"));
                p->client.flush();
                p->client.stop();
              }
            }
            WRITEMODE = 0;
            return;
          }
        
        }
        
        if (line.indexOf("download=") != -1) {
          String dow = "./DATA/";
          using sd::File;
          SD.begin(15);
          for (int i = line.indexOf("load=") + 5; i < line.indexOf("load=") + 5 + 11; i++) {
            dow = dow + line[i];
          }
          p1->client.println(F("HTTP/1.1 200 OK"));
          p1->client.println(F("Server: ESP8266"));
          p1->client.println(F("Connection: Close"));
          p1->client.println();//end of headers
          File file = SD.open(dow, FILE_READ);
          if (!file) {
            p1->client.println(F("Nodata"));
          }
          p1->client.write(file);
          p1->client.flush();
          p1->client.stop();
          file.close();
          return;
        }

        if (line.indexOf("setup=") != -1) { //this is not get new ram plc
          //get date and time to find and send


          if (line.indexOf("FuLLLiStDaTa") != -1) {
            sendlistdata(p1);
            p1->client.flush();
            p1->client.stop();
            return;
          }
          if (line.indexOf("Ssid=") != -1) {
            if(line.indexOf("%20")!= -1){
              line.replace("%20", " ");
            }
            ssid = pass = "";
            for (int i = line.indexOf("Ssid=") + 5; i < line.indexOf("&"); i++) {
              ssid = ssid + line[i];
            }
            for (int i = line.indexOf("&Pass=") + 6; i < line.indexOf(" HTTP"); i++) {
              pass = pass + line[i];
            }
            for (int i = 0; i < 96; i++) {
              EEPROM.write(i, 0); delay(3);
            }
            for (unsigned int i = 0; i < ssid.length(); i++)
            {
              EEPROM.write(i, ssid[i]); delay(3);
            }
            for (unsigned int i = 0; i < pass.length(); i++)
            {
              EEPROM.write(i + 32, pass[i]); delay(3);
            }
            EEPROM.end();
            connectwifi(ssid,pass);
            return;
          }
        }

        ///request page///////////////////////////////////////////////////

        //most using pager
        if (line.indexOf("/style.css") != -1) {
          senddata("/style.css", p1);
          return;
        }

        else if (line.indexOf("/write") != -1) {
          senddata("/write.html", p1);
          return;
        }

        else if (line.indexOf("/setup") != -1) {
          senddata("/setup.html", p1);
          return;
        }

        else if (line.indexOf("/scan") != -1) {
          p1->client.println(F("HTTP/1.1 200 OK"));
          p1->client.println(F("Server: ESP8266"));
          p1->client.println(F("Connection: Close\n"));
          p1->client.println(scanPg());
          p1->client.stop();
          return;
        }

        //most unuse
        else if ((line.indexOf("icon.png") != -1) || (line.indexOf("favicon.ico") != -1)) {
          senddata("/icon.png", p1);
          return;
        }
        // unknowdata ??? --- go home kid/////////////////////////////
        else {
          senddata("/next.html", p1);
        }
      }

      else {//or key false then authenticate
        Authenticate(p1->client);//request to authentica
      }
    }
  }
}

void sendlistdata(alient *pa) {
  using sd::File;
  SD.begin(15);
  File root = SD.open("./DATA");
  if (!root) {
    pa->client.println(F("HTTP/1.1 204 No data\r"));
    pa->client.println(F("content-type:text/xml"));
    pa->client.println(F("Server: ESP8266"));
    pa->client.println(F("Connection: close"));
    pa->client.println(F("Statup:nodata"));
    pa->client.println();
    return;
    root.close();
  }
  pa->client.println(F("HTTP/1.1 200 New data\r"));
  pa->client.println(F("content-type:text/xml"));
  pa->client.println(F("Server: ESP8266"));
  pa->client.println(F("Connection: Keep-Alive"));
  pa->client.println(F("Keep-Alive: timeout=60, max=70"));
  pa->client.println();
  pa->client.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<CATALOG>\n");
  File entry ;
  while (true) {
    yield();
    entry =  root.openNextFile();
    if (! entry) {
      break;       // no more files
    }
    String data = data +
                  "<CD>\n" +
                  "<D>" + entry.name() +
                  "</D>\n" +
                  "</CD>\n";
    pa->client.print(data);
  }
  pa->client.print("</CATALOG>\n");
  entry.close();
}

void Authenticate(WiFiClient &client) {
  client.println(F("HTTP/1.0 401 Unauthorized"));/// xac thuc
  client.println(F("WWW-Authenticate: Basic realm=\"Cemu#628173\""));
  client.println(F(""));
  client.println(F("YOU MUST ENTER RIGHT PASSWORD AND USES"));
  client.flush();
  client.stop();
}

void senddaydata(alient *po) { 
  //readdatafromsdcard and send to client
  // send heder
  using sd::File;

  // prepair data
  SD.begin(15);
  //ya copy
  String ye = String(year());
  String filename = "./DATA/" + String(day()) + "-" + String(month()) + "-" + String(ye[2]) + String(ye[3]) + ".txt"; // that it
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    po->client.println(F("HTTP/1.1 204 No data\r"));
    po->client.println(F("content-type:text/xml"));
    po->client.println(F("Server: ESP8266"));
    po->client.println(F("Connection: close"));
    po->client.println(F("Statup:nodaydata"));
    po->client.println();
    return;
  }
  po->client.println(F("HTTP/1.1 201 New data\r"));
  po->client.println(F("content-type:text/xml"));
  po->client.println(F("Server: ESP8266"));
  po->client.println(F("Connection: Keep-Alive"));
  po->client.println(F("Keep-Alive: timeout=60, max=70"));
  po->client.println();
  yield();
  // delay(20); // forwifisysreaaaaaaady
  po->client.println(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<CATALOG>\n"));
  while (file.available()) {
    // int i = 0;
    file.read();
    file.read();
    String timedata = file.readStringUntil('\t'); // read time
    if (timedata == "") {
      break;
    }
    String namedata = file.readStringUntil('\t'); //read name
    String classdata = file.readStringUntil('\t'); // read class
    String phonedata = file.readStringUntil('\n'); // read phone
    String data = data +
                  "<CD>\n" +
                  "<NAME>" +
                  namedata +
                  "</NAME>\n" +
                  "<COURSE>" +
                  classdata +
                  "</COURSE>\n" +
                  "<PHONE>" +
                  phonedata +
                  "</PHONE>\n" +
                  "<TIME>" + timedata +
                  "</TIME>\n" +
                  "</CD>\n";
    po->client.print(data);
  }
  po->client.print("</CATALOG>\n");
  file.close();
}

String scanPg() {
  int n = WiFi.scanNetworks();
  String Scand;
  if (n == 0) {
    Scand = " No Network found ";
  } else {
    Scand = "&emsp;&emsp;" + String(n) + " Network found! <br/>";
    for (int i = 0; i < n; ++i)
    {
      Scand = Scand + +"&emsp;" + String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm) <br/> \n";
    }
  }
  String page = page +
                "<!DOCTYPE HTML>" + xd +
                "<html>" + xd +
                "<head>" + xd +
                "<meta http - equiv = 'Content-Type' content = 'text/html; charset = utf-8'>" + xd +
                "<TITLE> STUDENT ATTENDANCE </TITLE>" + xd +
                "<link href = \"style.css\" rel = \"stylesheet\">" + xd +
                "<link rel=\"shortcut icon\" href=\"icon.png\"/>" + xd +
                "</head> " + xd +
                "<body> " + xd +
                "<div> " + xd +
                "<p> " + xd +
                "<br/> STUDENT ATTENDANCE <br/> <br/> " + xd +
                "</p> " + xd +
                "<HR/> " + xd +
                "<a href='/home' > HOME </a> " + xd +
                "<a href='/write'> WRITE </a> " + xd +
                "<a href='/setup' > SETUP </a> " + xd +
                "<HR/>  " + xd +
                Scand +
                "<HR/> " + xd +
                "<p class='end'> " + xd +
                "<br/> " + xd +
                "&emsp;All design & code by Cemu<br/> " + xd +
                "&emsp;Copyright 2017-Infinity" + xd +
                "<br/><br/> " + xd +
                "</p> " + xd +
                "</div> " + xd +
                "</body> " + xd +
                "</html>" ;

  return page;
}

void senddata(String names, alient *p2) {
  p2->client.println(F("HTTP/1.1 200 OK"));
  p2->client.println(F("Server: ESP8266"));
  p2->client.println(F("Connection: Close"));
  if (names.indexOf("next.html") != -1) {
    p2->client.println(F("Location: /home")); //dobble changepage
  }
  p2->client.println();//end of headers
  File file = SPIFFS.open(names, "r");
  p2->client.write(file);
  p2->client.flush();
  p2->client.stop();
  file.close();
  return;
}

void loadlastwifidata(String &ssid, String &pass) {
  for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) {
    pass += char(EEPROM.read(i));
  }
}

void homepg(){
  String d1 = "IP:"+ipToString(WiFi.localIP());
  String d2 = String(day())+"-"+String(month())+" "+String(hour())+":"+String(minute())+ batr + (severconnect?" Y":" N") ;
  unsigned long lastime = lcd.timedoffdislay;
  lcd.show(d1,d2);
  lcd.timedoffdislay = lastime;
}

/////////////////////////////////////////////////////////////////////support function

String ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

///////////////MAIN///////////MAIN////////////////////MAIN//////////////////////MAIN FUNCTION

void setup() {
  // put your setup code here, to run once:  
  pinMode(0,INPUT_PULLUP);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(F("Starting..."));
  Serial.begin(115200);
  EEPROM.begin(100);
  SPIFFS.begin();
  if (!SD.begin(15)) {
    Serial.print(F("No sd card"));
      lcd.setCursor(0, 0);
      lcd.print(F("No sd card"));
      delay(700);
  }
  // Turn on the blacklight and print a message.
  loadlastwifidata(ssid, pass);
  connectwifi(ssid,pass);
  //connectwifi(dessid, depassword);  
  //Start the server
  mainclient.setNoDelay(true);
  server.begin();
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  /// fast fast fast yay .. no: client.setNoDelay(1);
  //  client = server.available();
  alient *p = makenewclient();
  firstclient = p;
  // numofpullclient = 1;
  //savedata("testing test  test");
//  syncfiledata();
  homepg();
  attachInterrupt(0,turnwifi, FALLING);
//  SD.remove("DATA/9:22:22.TXT");
//  SD.remove("DATA/0:6:17.TXT");
}

void loop() {
  IPAddress deleteIP, NULLIP;
  serialevento();

  if ((unsigned long)(millis() - lcd.timedoffdislay) > 40000){
    homepg();
  }
  
  if ((unsigned long)(millis() - timehpg) >= 1000){
    lcd.checkdislay();
    timehpg = millis();
  }
  
  if(WiFi.status() == WL_CONNECTED){
    checkseverconect();
  }
   
  for (alient *p = firstclient; p != NULL; p = p -> next)
  {
    GATE += 1;
    yield();
    if ((p->pull) == 3) {
      //keep pull when not complete
      if (millis() - p->timeout >= 50000) {
        p->client.println(F("HTTP/1.1 204 New data\ncontent-type:text\nServer: ESP8266\nConnection: Keep-Alive\nKeep-Alive: timeout=60, max=70"));
        p->client.println(F("Statup: please input card to write!"));
        p->client.println(F(""));
        p->client.flush();
        p->client.stop();
      }
      
    }

    else if ((p->pull) == 1) {
      p->client.println(F("HTTP/1.1 200 New data\r"));
      p->client.println(F("content-type:text/xml"));
      p->client.println(F("Server: ESP8266"));
      p->client.println(F("Connection: Keep-Alive"));
      p->client.println(F("Keep-Alive: timeout=60, max=70"));
      p->client.println();
      String data = data + "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
                    "<CATALOG>\n" +
                    "<CD>\n" +
                    "<NAME>" +
                    RFIDDATA.NAME +
                    "</NAME>\n" +
                    "<COURSE>" +
                    RFIDDATA.COURSE +
                    "</COURSE>\n" +
                    "<PHONE>" +
                    RFIDDATA.PHONE +
                    "</PHONE>\n" +
                    "<TIME>" + String(hour()) + ":" + String(minute()) + ":" +  String(second()) +
                    "</TIME>\n" +
                    "</CD>\n" +
                    "</CATALOG>\n";
      p->client.print(data);
      p->client.flush();
      p->client.stop();
      p->pull = 2;
    }

    else if ((p->pull) == 2) {
      if (millis() - p->timeout >= 50000) { //reset the connection every 50s
        p->client.println(F("HTTP/1.1 204 No new data\r"));
        p->client.println(F("Server: ESP8266"));
        p->client.println(F("Connection: Keep-Alive"));
        p->client.println(F("Keep-Alive: timeout=60, max=70"));
        p->client.println();
        p->client.flush();
        p->client.stop();
        //  p->pull = false;
        if ((GATE == 2) && (p != firstclient)) {
          firstclient->next = NULL;
          delete p;
        } else if (p != firstclient) {
          la->next = p->next;
          delete p;
        }
      }
      if ((p->ip == deleteIP) && (GATE >= 2)) {
        if (p == firstclient) {
          firstclient = p->next;
          delete p;
        } else {
          la->next = p->next;
          delete p;
        }
      }
    }
    
    else {
      checkclient(p, la);
    }

    if (GATE >= 2) {
      for (byte i = 1; i <= GATE; i++) {
        if (p->ip == listip[i]) {
          if (p->pull != 0) {
            deleteIP = p->ip;
          }
        }
      }
      for (byte i = GATE; i <= 6; i++) {
        listip[i] = NULLIP;
      }
    }

    la = p;
    
  }

  GATE = 0;
//  Serial.println(GATE);
//  Serial.println(ESP.getFreeHeap());
}

