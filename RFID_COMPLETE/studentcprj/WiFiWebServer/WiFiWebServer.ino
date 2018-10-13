/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
//
const char* ssid = "Snowfoxsd";
const char* password = "11112222";
unsigned long i;
const char xd = '\n';
//const char* ssid = "Cemu";
//const char* password = "khongchopass";
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
String test(){
  String page = page + xd +
  " <!DOCTYPE html>"  +xd +
   "<html>"  +xd +
  "<body>"  + xd +
  "<div id=\"demo\">"+ xd +
"<h1>The XMLHttpRequest Object</h1>"+ xd +
"<button type=\"button\" onclick=\"loadDoc()\">Change Content</button>"+ xd +
"</div>"+ xd +
"<script>"+ xd +
"setInterval(loadDoc,500);"+ xd+//was set to 2
"function loadDoc() {"+ xd +
  "var xhttp = new XMLHttpRequest();"+ xd +
  "xhttp.onreadystatechange = function() {"+ xd +
    "if (this.readyState == 4 && this.status == 200) {"+ xd + 
      "document.getElementById(\"demo\").innerHTML ="+ xd +
      "this.responseText;"+ xd +
    "}"+ xd +
  "};"+ xd +
  "xhttp.open('GET', \"ajax_info.txt\", true);"+ xd +
//  "xhttp.open('POST', \"ajax_info.txt\", false);"+ xd +
  "xhttp.send();"+ xd +
"}"+ xd +
"</script>"+ xd +
"<form action='/justdoit' method='post'>"  +xd +
  "First name: <input type='text' name='fname'><br>"  +xd +
  "Last name: <input type='text' name='lname'><br>"  +xd +
 " <input type='submit' value='Submit'>"  +xd +
"</form>"  +xd +
"<p> Time: "+ String(millis()) +"loop : "+String(i)+".</p>"  +xd +
"</body>"  +"</html>";
  return page;

}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void setup() {
Serial.begin(115200);
WiFi.setPhyMode(WIFI_PHY_MODE_11N);
WiFi.softAPdisconnect(true);
WiFi.disconnect();
WiFi.mode(WIFI_STA);
//WiFi.setOutputPower(25);
//  WiFi.reconnect();
WiFi.setAutoConnect(true);
  // prepare GPIO2
//  pinMode(13, OUTPUT);
//  digitalWrite(13, 0);
  
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  printWifiStatus();
  // Start the server
  server.begin();

  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected\
//  delay(1);
  WiFiClient client = server.available(); 
//WiFiClient client = server.client();
if (client) {
  i = i+1;
  String req = client.readString();
  client.flush();
//  Serial.print("CLIENT IP ");
//  Serial.println(client.remoteIP());
Serial.println(i);
//  delay(1);
   
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
//  client.println("Refresh: 30");  // refresh the page automatically max time was test
  client.println();//header done
  client.print(test()); 
//  delay(1);
  client.stop();
}
}
