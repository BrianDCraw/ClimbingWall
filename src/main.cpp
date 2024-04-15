#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>

WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 10000;
#define NUM_OF_LEDS 250
#define PIN 33
int CurrentRouteIndex;
int MaxRouteIndex;
 
// Web server running on port 80
WebServer server(80);

// Neopixel LEDs strip
Adafruit_NeoPixel pixels(NUM_OF_LEDS, PIN, NEO_RGB + NEO_KHZ800);

// JSON dat
StaticJsonDocument<20000> RequestContent;
StaticJsonDocument<20000> RoutesFileJson;
String RoutesFileString;




DynamicJsonDocument getLEDs() {
  int pixelnumber = 0;
  String color ;
  int Red;
  int Green;
  int Blue;
  DynamicJsonDocument LEDS(4000);

  JsonObject article = LEDS.createNestedObject("article");
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
      JsonArray rgb = light.createNestedArray("color");
      rgb[0] = Red;
      rgb[1] = Green;
      rgb[2] = Blue;

    }
  } 
  return LEDS;
}


void setLEDs(JsonArray lights) 
{
  pixels.clear();

  for(JsonVariant light : lights) {  
    JsonArray rgb = light["color"];
    int lightnum = light["LightNum"];
    int red = rgb[0];
    int green = rgb[1];
    int blue = rgb[2];
  
     pixels.setPixelColor(lightnum,red,green ,blue);
  }
  
  pixels.show();

}

void setLED(JsonArray rgb, int lightNumber)
{
  int red = rgb[0];
  int green = rgb[1];
  int blue = rgb[2];
  pixels.setPixelColor(lightNumber,red,green ,blue);
  pixels.show();
}

void LoadRoute(int RouteId)
{
  JsonArray routes = RoutesFileJson.as<JsonArray>();
  
  for(JsonVariant route : routes) { 
      if (route["RouteId"] == RouteId) {   
          JsonArray lights = route["Lights"];
          setLEDs(lights);
      }
  }
}

//Return List of Routes
void getRoutesAPI()
{ 
    server.send(200, "application/json",RoutesFileString);
}
void getCurrentRouteAPI()
{ 
   DynamicJsonDocument Routes(4000);
  JsonObject ReturnObject = Routes.createNestedObject();
  String ReturnString;
  ReturnObject["RouteId"] = CurrentRouteIndex;
  serializeJson(ReturnObject,ReturnString);
  server.send(200, "application/json",ReturnString) ;
}

//Update RouteList File
void updateRoutesAPI()
{ 
    String body = server.arg("plain");
    String fullpath = "/assets/routes.json";
    File file = SPIFFS.open(fullpath,"w");
    file.println(body);
    server.send(200, "application/json", "{Result:Sucess}");
}

//Load Specific Route by RouteId
void loadRouteAPI()
{
  // Parse input
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
  CurrentRouteIndex = RequestContent["RouteId"];
  LoadRoute(CurrentRouteIndex);

  server.send(200, "application/json", "{Result:Sucess}");
}

void LoadNextRouteApi()
{
  if (CurrentRouteIndex != MaxRouteIndex){
    CurrentRouteIndex++;
  }
  else{
    CurrentRouteIndex = 1;
  }
  LoadRoute(CurrentRouteIndex);
  DynamicJsonDocument Routes(4000);
  JsonObject ReturnObject = Routes.createNestedObject();
  String ReturnString;
  ReturnObject["RouteId"] = CurrentRouteIndex;
  serializeJson(ReturnObject,ReturnString);
  server.send(200, "application/json",ReturnString) ;
}
void LoadPreviousRouteApi()
{
  if (CurrentRouteIndex > 1){
    CurrentRouteIndex--;
  }
  else{
    CurrentRouteIndex = MaxRouteIndex;
  }
  LoadRoute(CurrentRouteIndex);
  DynamicJsonDocument Routes(4000);
  JsonObject ReturnObject = Routes.createNestedObject();
  String ReturnString;
  ReturnObject["RouteId"] = CurrentRouteIndex;
  serializeJson(ReturnObject,ReturnString);
  server.send(200, "application/json",ReturnString) ;
}

//Set a specific list of lights to replace existing lights 
void setLEDsAPI() 
{ 
  CurrentRouteIndex = 0;
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
  JsonArray lights = RequestContent["lights"];
  setLEDs(lights);
  server.send(200, "application/json", "{Result:Sucess}");
}

void mirrorRouteAPI() 
{ 
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
  JsonArray lights = RequestContent["lights"];
  setLEDs(lights);
  server.send(200, "application/json", "{Result:Sucess}");
}


void setLEDAPI()
{
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
   JsonArray rgb =  RequestContent["color"];
  int lightNumber = RequestContent["LightNum"];

  setLED(rgb,lightNumber);
  server.send(200, "application/json", "{Result:Sucess}");
}

//get Current LEDs that are on and what color
void getLEDsAPI()
{
  DynamicJsonDocument LedsJson = getLEDs();
  String LEDs;
  serializeJson(LedsJson,LEDs);
  server.send(200, "application/json",LEDs);
}





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
  wifiMulti.addAP("JBHome", "n0ne5ha11pa55");
  wifiMulti.addAP("JBHome5G", "n0ne5ha11pa55");
  wifiMulti.addAP("JBHomeAP", "n0ne5ha11pa55");
  wifiMulti.addAP("JBHome5GAP", "n0ne5ha11pa55");
  
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



// setup API resources
void setup_routing() {
  server.on("/setLEDs", HTTP_POST, setLEDsAPI);
  server.on("/getLights",HTTP_GET, getLEDsAPI);
  server.on("/setLED",HTTP_POST,setLEDAPI);
  server.on("/getRoutes", HTTP_GET, getRoutesAPI);
  server.on("/updateRoutes", HTTP_POST, updateRoutesAPI);
  server.on("/nextRoute", HTTP_POST, LoadNextRouteApi );
  server.on("/prevRoute", HTTP_POST, LoadPreviousRouteApi);
  server.on("/loadRoute", HTTP_POST, loadRouteAPI);
  server.on("/getCurrentRoute",HTTP_GET, getCurrentRouteAPI);
  server.on("/mirrorRoute",HTTP_POST,mirrorRouteAPI);
  // start server
  server.onNotFound(handleNotFound);
  server.enableCrossOrigin(true);
  server.begin();
}

 void loadRoutesFile()
 {
    String fullpath = "/assets/routes.json";
    File file = SPIFFS.open(fullpath,"r");
    RoutesFileString = file.readString(); 
    file.close();
    deserializeJson(RoutesFileJson, RoutesFileString);

   JsonArray routes = RoutesFileJson.as<JsonArray>();
   MaxRouteIndex = 0;

  for(JsonVariant route : routes) { 
      if (route["RouteId"] > MaxRouteIndex) {   
          MaxRouteIndex = route["RouteId"];
      }
  }
 } 


void setup() 
{
  SPIFFS.begin(true); //intialize file system
  pinMode(2,OUTPUT) ; //used to intialize the onboard LED GPO02 pin as output

  Serial.begin(9600);
  Serial.print("StartUp Beginning");
  connectToWiFi();
  setup_routing();  

  //Load Routelist
  loadRoutesFile();
  CurrentRouteIndex = 0;
  // Initialize Neopixel
  pixels.begin();
  pixels.clear();
  pixels.show();
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

 
void loop() 
{
  server.handleClient();
   //reconnect to wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
       connectToWiFi();
  }
    ArduinoOTA.handle();
}

