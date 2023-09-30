#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 10000;
#define NUM_OF_LEDS 250
#define PIN 33
 
// Web server running on port 80
WebServer server(80);

// Neopixel LEDs strip
Adafruit_NeoPixel pixels(NUM_OF_LEDS, PIN, NEO_RGB + NEO_KHZ800);

// JSON data buffer
StaticJsonDocument<8000> jsonDocument;
char buffer[8000];

void handleNotFound()
{  
  String path = server.uri();
  String dataType = "text/plain";
  if (path.endsWith("/")) {
    path += "index.html";
  }
     Serial.println(path);
  if (path.endsWith(".src")) {
    path = path.substring(0, path.lastIndexOf("."));
  } else if (path.endsWith(".html")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (path.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (path.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (path.endsWith(".zip")) {
    dataType = "application/zip";
  }

  File dataFile = SPIFFS.open(path.c_str());

  bool isfile = dataFile.available();

  if (!dataFile.available()){
    server.send(200,"text/html","File Not Found");
    return;
  }

  if (dataFile.isDirectory()) {
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SPIFFS.open(path.c_str());
  }

  if (server.hasArg("download")) {
    dataType = "application/octet-stream";
  }

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
  return;
}

void connectToWiFi() 
{
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("test", "test");
  wifiMulti.addAP("test", "test");
  wifiMulti.addAP("test", "test");
  wifiMulti.addAP("test", "test");
  
Serial.println("Connecting Wifi...");
   
   // start process by blinking onboard light to show its trying to connect
   for(byte i = 0; i < 5; i++){
  digitalWrite(2,HIGH);
  delay(100);
  digitalWrite(2,LOW);
  delay(500);
  };

  //CONNECT TO WIFI
  uint32_t wifiStatus ;
  wifiStatus = wifiMulti.run(connectTimeoutMs);
  Serial.println(wifiStatus); 
  
  //If Connected turn on Onboard LED and print connected with IP
  if(wifiStatus == WL_CONNECTED) {
    digitalWrite(2,HIGH);
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address:");
    Serial.print(WiFi.localIP());
  } //If not connected blink LED faster than before and print WIFI not connect
  else {
       for(byte i = 0; i < 15; i++){
       digitalWrite(2,HIGH);
       delay(100);
       digitalWrite(2,LOW);
       delay(100);
       };
    Serial.println("");
    Serial.println("WiFi not connected connected");
  }
  
}

//Saving a routed sent to this API
void saveRoute()
{ 
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);

    String filename = jsonDocument["RouteName"];
    String fullpath = "/scripts/"+(String)filename +".json";

    File file = SPIFFS.open(fullpath,FILE_WRITE);
    file.println(body);
    server.send(200, "application/json", "{Result:Sucess}");

}

void setLEDs() 
{
  pixels.clear();
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  JsonArray lights = jsonDocument["lights"];

  for(JsonVariant light : lights) {  
    JsonArray rgb = light["RGB"];
    int lightnum = light["LightNum"];
    int red = rgb[0];
    int green = rgb[1];
    int blue = rgb[2];
  
     pixels.setPixelColor(lightnum,red,green ,blue);
  }
  
  pixels.show();

  // Respond to the client
  server.send(200, "application/json", "{Result:Sucess}");
}

void setLED()
{
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  JsonArray rgb =  jsonDocument["RGB"];
  int lightNumber = jsonDocument["LightNum"];
  int red = rgb[0];
  int green = rgb[1];
  int blue = rgb[2];
  pixels.setPixelColor(lightNumber,red,green ,blue);
  pixels.show();
  server.send(200, "application/json", "{Result:Sucess}");
}

void getLEDs()
{
  String jsonoutput;
  int pixelnumber = 0;
  String color ;
  int Red;
  int Green;
  int Blue;

  DynamicJsonDocument doc(4000);
  JsonObject article = doc.createNestedObject("article");
  JsonArray lightsJson = article.createNestedArray("lights");
  uint8_t * lights =  pixels.getPixels();

  for(int i = 0; i<pixels.numPixels() * 3; i= i+3){
    pixelnumber = i/3;
    Red = lights[i];
    Green = lights[i+1];
    Blue = lights[i+2];

    if (Red + Green + Blue > 0){
      JsonObject light = lightsJson.createNestedObject();
      light["LightNum"] = pixelnumber;
      JsonArray rgb = light.createNestedArray("RGB");
      rgb[0] = Red;
      rgb[1] = Green;
      rgb[2] = Blue;

    }
  } 

  serializeJson(doc,jsonoutput);
  server.send(200, "application/json",jsonoutput);
}
 
// setup API resources
void setup_routing() {
  server.on("/setLEDs", HTTP_POST, setLEDs);
  server.on("/getLights",HTTP_GET, getLEDs);
  server.on("/setLED",HTTP_POST,setLED);
  server.on("/saveRoute", HTTP_POST, saveRoute);
  // start server
  server.onNotFound(handleNotFound);
  server.enableCrossOrigin(true);
  server.begin();
}

void setup() 
{
  SPIFFS.begin(true); //intialize file system
  pinMode(2,OUTPUT) ; //used to intialize the onboard LED GPO02 pin as output

  Serial.begin(9600);
  Serial.print("StartUp Beginning");
  connectToWiFi();
  setup_routing();  

  // Initialize Neopixel
  pixels.begin();
  pixels.clear();
  pixels.show();
}
 
void loop() 
{
  server.handleClient();
   //reconnect to wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
       connectToWiFi();
  }
}

