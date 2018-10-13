#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
File myFile;
String do1, do2;
unsigned int ld1, ld2;
unsigned long  timed1,timed2;
const char WiFiAPPSK[] = "123456789"; //mat khau wifi
unsigned long  tcn;
String ssid, pass, namewifi;
int button = 10;
WiFiServer server(80);
WiFiClient client;
IPAddress ip;
String xd = "\n";//xuong dong cho de dc xD
void disconnectwifi() {
  WiFi.setAutoConnect(false);
  WiFi.disconnect();
  Serial.print("disconected");
  lcd.show("Wifidisconnected","");
}
void connectwifi() {
  Serial.print("connected");
  WiFi.reconnect();
  WiFi.setAutoConnect(true);
  if ( ssid.length() > 1 ) {
    WiFi.begin(ssid.c_str(), pass.c_str());
    tcn = millis();
    while ((WiFi.status() != WL_CONNECTED) && ((millis() - tcn) < 10000)) {
      lcd.show("conecting...","...");
      delay(10);
    }
    if (WiFi.status() != WL_CONNECTED) {
      lcd.show("No wifi connect","Turnoff auto reconnet");
      WiFi.setAutoConnect(false);
      WiFi.disconnect();
    }else
    lcd.show("Wifi connected","");
  }
}
void setupWiFi() {
  lcd.show("Wifi AP mode","loading...");
  WiFi.mode(WIFI_AP_STA);
  uint8_t mac[WL_MAC_ADDR_LENGTH];//6bytes char
  WiFi.softAPmacAddress(mac);//get
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);//lay 2 byte cuoi
  macID.toUpperCase();// chuyen macId thanh viet hoa
  String AP_NameString = "DSHS" + macID;//tranh trung lap dia chi ssid
  char AP_NameChar[AP_NameString.length() + 1];// do dai cua string + 1
  memset(AP_NameChar, 0, AP_NameString.length() + 1);// tuy chinh vung nho tu
  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);// String vao char
  //  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);//hamchichapnhanmangchar
  lcd.show("","complete");
}
void readdata() {
if (Serial.available()) {
  delay(10);
    String data = Serial.readStringUntil('\r');
    Serial.flush();
//    Serial.println(data);

if (data.indexOf("UID:") != -1) {
  String d1,d2;
       for (int i = data.indexOf("UID:"); i < data.indexOf("#"); i++) {
      d1 = d1 + data[i];
    }
       for (int i = data.indexOf("Data:"); i < data.indexOf("*"); i++) {
      d2 = d2 + data[i];
    lcd.show(d1,d2); 
    }
} 
}   
}
void checkbutton(){
  if(!digitalRead(button)){
    lcd.show("buttonon","Ok");
      connectwifi();
      WiFi.softAPdisconnect(false);
      setupWiFi();
      server.begin();
     
  }
  /*
      if (data.indexOf("mode0") != -1) {
      Serial.print("mode0#");
      wifi = 0;
      WiFi.softAPdisconnect(true);
      disconnectwifi();
      server.close();
    }
  */
}
void checkclient(){
  WiFiClient client = server.available();
 if (client) {
    String line = client.readStringUntil('\r');
    client.flush();
    lcd.show("Wifi cleint","get data");
    if (line.indexOf("/setup") != -1) {     
      client.println("");
      client.print(style());
      client.print(setupPage());
      if (line.indexOf("ssid=") != -1) {
        ssid = pass = "";
        for (int i = line.indexOf("ssid=") + 5; i < line.indexOf("&"); i++) {
          ssid = ssid + line[i];
        }
        for (int i = line.indexOf("&pass=") + 6; i < line.indexOf(" HTTP"); i++) {
          pass = pass + line[i];
        }
        for (int i = 0; i < 96; i++) {
          EEPROM.write(i, 0);
        }
        for (int i = 0; i < ssid.length(); i++)
        {
          EEPROM.write(i, ssid[i]);
        }
        for (int i = 0; i < pass.length(); i++)
        {
          EEPROM.write(i + 32, pass[i]);
        }
        EEPROM.commit();
        connectwifi();
      }
      
    }
    else if (line.indexOf("/scan") != -1) {
      client.print(style());
      client.print(scanPg());
    }
    if (line.indexOf("/OK") != -1 ) {
      Serial.print("mode0");
      client.print("disconected xD");
      delay(1);
      client.stop();
      WiFi.softAPdisconnect(true);
      disconnectwifi();
    }
    if (line.indexOf("write:") != -1 ) {
      String writes;
      for (int i = line.indexOf("write:"); i < line.indexOf("#"); i++) {
          writes = writes + line[i];
        }
        lcd.show("Write ting","input card");
        Serial.print(writes);
    }
    if (line.indexOf("/wps") != -1) { // text ---> not working !!! = hide mode
      WiFi.mode(WIFI_STA);
      WiFi.setAutoConnect(true);
      WiFi.begin("foobar", "");
      WiFi.beginWPSConfig();
    }
    delay(1);
    client.stop();
  }
  }
String style() {
  String htmlPage = htmlPage +
                    "<!DOCTYPE HTML>" + xd +
                    "<html>" + xd +
                    "<head>" + xd +
                    "<meta http-equiv = 'Content-Type' content = 'text / html; charset = utf-8'>" + xd +
                    "<TITLE> HỆ THỐNG CHĂM SÓC CÂY THÔNG MINH </TITLE>" + xd +
                    "<style>" + xd +
                    "body {" + xd +
                    "background-color: green;" + xd +
                    "font-family: Arial, Helvetica, sans-serif;" + xd +
                    "}" + xd +
                    "HR {" + xd +
                    "border: 1px solid green;" + xd +
                    "background-color: lightgreen;" + xd +
                    "margin-left: 1%;" + xd +
                    "margin-right: 1%;" + xd +
                    "}" + xd +
                    "HR.mini{" + xd +
                    "width: 200px " + xd +
                    "}" + xd +
                    "p {" + xd +
                    " font-size: 40px;" + xd +
                    " color: blue;" + xd +
                    "-webkit-margin-before: 0em;" + xd +
                    "-webkit-margin-after: 0em;" + xd +
                    "background: lightgreen; " + xd +
                    "background: -webkit-linear-gradient(lightgreen, white); " + xd +
                    "background: linear-gradient(lightgreen, white);" + xd +
                    "text-align:center" + xd +
                    "}" + xd +
                    "p.end{" + xd +
                    " font-size: 10px;" + xd +
                    "font-style: italic;" + xd +
                    "background: -webkit-linear-gradient(white, lightgreen); " + xd +
                    "background: linear-gradient(white, lightgreen);" + xd +
                    "text-align:left" + xd +
                    "}" + xd +
                    "a {" + xd +
                    "color: black;" + xd +
                    "border: 2px solid #4CAF50;" + xd +
                    "padding: 10px 10px;" + xd +
                    "text-align: center;" + xd +
                    "text-decoration: none;" + xd +
                    "display: inline-block;" + xd +
                    "font-size: 20px;" + xd +
                    "margin-left: 1px;" + xd +
                    "margin-top: 1px;" + xd +
                    "text-indent: 0px;" + xd +
                    "}" + xd +
                    "a:hover{" + xd +
                    "color: blue;" + xd +
                    "background-color: #4CAF50;" + xd +
                    "}" + xd +
                    "a.hightlight{" + xd +
                    "color: red;" + xd +
                    "background-color: lightgreen;" + xd +
                    "}" + xd +
                    "a.buton{" + xd +
                    "font-size: 10px;" + xd +
                    "margin-left: 5px;" + xd +
                    "border: 1px solid red;" + xd +
                    "}" + xd +
                    "div {" + xd +
                    "border: 2px solid lightgreen;" + xd +
                    "margin-left: 8%;" + xd +
                    "margin-right: 8%;" + xd +
                    "background-color: white;" + xd +
                    "padding: 10px 10px;" + xd +
                    "}" + xd +
                    "input {" + xd +
                    "padding: 2px 2px;" + xd +
                    "border: 1px solid red;" + xd +
                    "margin-left: 5px;" + xd +
                    "margin-bottom: 2px;" + xd +
                    "}" + xd +
                    "input:hover{" + xd +
                    "background-color: lightyellow;" + xd +
                    "}" + xd +
                    "</style>" + xd;
  return htmlPage;
}
String scanPg() {
  int n = WiFi.scanNetworks();
  String Scand;
  if (n == 0) {
    Scand = " không có mạng nào được tìm thấy";
  } else {
    Scand = String(n) + " mạng đã được tìm thấy <br/>";
    for (int i = 0; i < n; ++i)
    {
      Scand = Scand + +"&emsp;" + String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + "dBm) <a href='/setup" + WiFi.SSID(i) + "*' class = 'buton'> sử dụng tên WIFI </a> <br/> \n";
    }
  }
  String page = page +
                "</head> " + xd +
                "<body> " + xd +
                "<div> " + xd +
                "<p> " + xd +
                "<br/> HỆ THỐNG ĐIỂM DANH HỌC SINH <br/> <br/> " + xd +
                "</p> " + xd +
                "<HR/> " + xd +
                "<a href='/write' > GHI DỮ LIỆU </a> " + xd +
                "<a href='/read'> ĐỌC DỮ LIỆU </a> " + xd +
                "<a href='/setup' > CÀI ĐẶT WIFI </a> " + xd +
                "<HR/>  " + xd +
                Scand +
                "<HR/> " + xd +
                "<p class='end'> " + xd +
                "<br/> " + xd +
                "&emsp;Website design & code by Cemu<br/> " + xd +
                "&emsp;Copyright 2016 " + xd +
                "<br/><br/> " + xd +
                "</p> " + xd +
                "</div> " + xd +
                "</body> " + xd +
                "</html>" ;
  return page;
}
String setupPage() {
  String htmlPage = htmlPage +
                    "</head> " + xd +
                    "<body>" + xd +
                    "<div>" + xd +
                    "<p>" + xd +
                    "<br/> HỆ THỐNG ĐIỂM DANH HỌC SINH <br/> <br/> " + xd +
                     "</p> " + xd +
                     "<HR/> " + xd +
                     "<a href='/write' > GHI DỮ LIỆU </a> " + xd +
                     "<a href='/read'> ĐỌC DỮ LIỆU </a> " + xd +
                     "<a href='/setup' class = 'hightlight'> CÀI ĐẶT WIFI </a> " + xd +
                    "<HR/>  " + xd +
                    "<br/><strong> Nhập tên WIFI (SSID) và PASSWORD của bạn </strong> <br/>  " + xd +
                    "<form action='#'> " + xd +
                    "<br/> &emsp;SSID <input type='text' name='ssid' size='20'value='" + namewifi + "'/> " + xd +
                    "<HR class = 'mini'/> " + xd +
                    "&emsp;PASS<input type='password' name='pass' size='20'/>  " + xd +
                    "<input type='submit' value = ' OK ' class = 'button' onclick='reset'><br/><br/> " + xd +
                    "<EM> LƯU Ý : Điều này sẽ làm mất pass và ssid trước đó của bạn !</EM> " + xd +
                    "</form> " + xd +
                    "<a href='/scan' class = 'buton'> Scan WIFI </a>" + xd +
                    "<HR/> " + xd +
//                    "<br/> <Strong> Chỉnh giờ hệ thống : </Strong><br/><br/> " + xd +
//                    "<br/> &emsp; Thời gian hệ thống : " + times + "  " + xd +
//                    "<form action='#'>&emsp; " + xd +
//                    "<input type='text' name='h' size='2'/> giờ  " + xd +
//                    "<input type='text' name='m' size='2'/> phút  " + xd +
//                    "<input type='text' name='s' size='2'/> giây  " + xd +
//                    "<input type='submit' value=' OK '> " + xd +
                    "</form> " + xd +
                    "<br/> " + xd +
                    "<HR/> " + xd +
//                    "<strong> Cài đặt thời gian gửi dữ liệu lên server </Strong><br/><br/> " + xd +
//                    "<form action='#'> " + xd +
//                    "&emsp; <select name= 'tup'> " + xd +
//                    "<option value='0'" + se1 + ">off</option> " + xd +
//                    "<option value='1'" + se2 + ">30 giây</option> " + xd +
//                    "<option value='2'" + se3 + ">1 phút</option> " + xd +
//                    "<option value='3'" + se4 + ">1 giờ</option> " + xd +
//                    "<option value='4'" + se5 + ">1 ngày</option> " + xd +
//                    "</select> " + xd +
//                    "<input type='submit' value=' OK '> " + xd +
//                    "</form> " + xd +
//                    "<br/> " + xd +
//                    "<HR/> " + xd +
                    "<p class='end'> " + xd +
                    "<br/> " + xd +
                "&emsp;Website design & code by Cemu<br/> " + xd +
                "&emsp;Copyright 2016 " + xd +
                    "<br/><br/> " + xd +
                    "</p> " + xd +
                    "</div> " + xd +
                    "</body> " + xd +
                    "</html>" ;
  return htmlPage;
}
void setup() {
   Serial.begin(115200); // Initialize serial communications with the PC
   EEPROM.begin(512);
   lcd.begin();
      lcd.backlight();
      lcd.noAutoscroll();
   WiFi.setOutputPower(20);
   pinMode(button,INPUT_PULLUP);
    SPI.begin();        // Init SPI bus
  if (!SD.begin(15)) {
    Serial.println("initialization failed!");
    lcd.show("ERROR","SD card fail");
  }
  lcd.show("initialization SD CARD done.","");
 for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) {
    pass += char(EEPROM.read(i));
  }
  WiFi.softAPdisconnect(true);
  disconnectwifi();
  lcd.show("Setup complete","READY...");
}
void loop() {
    readdata();
    checkbutton();
    checkclient();
    lcd.checkdislay();
}

