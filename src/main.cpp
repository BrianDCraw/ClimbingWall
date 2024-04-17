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
#include <WallLeds.h>
#include <Routes.h>
#include <PathFunctions.h>

WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 10000;
WebServer server(80);
ArduinoOTAClass OTA; 
WallLeds WallFunctions;
PathFunctions pathFunc;
Routes RoutesFunction;
JsonDocument RequestContent;

void LoadRoute(int RouteId)
{
    JsonArray lights = RoutesFunction.getLightsForRoute(RouteId);
    WallFunctions.setLEDs(lights);
}
//Return List of Routes
void getRoutesAPI()
{ 
  JsonDocument routes = RoutesFunction.getallRoutes();
  String RoutesFileString;
  serializeJson(routes,RoutesFileString);
  server.send(200, "application/json",RoutesFileString);
}
void getCurrentRouteAPI()
{ 
  String ReturnString = RoutesFunction.getCurrentRouteIdAsString();
  server.send(200, "application/json",ReturnString) ;
}
//Update RouteList File
void updateRoutesAPI()
{ 
    String body = server.arg("plain");
    RoutesFunction.saveAllRoutes(body);
    server.send(200, "application/json", "{Result:Sucess}");
}

//Load Specific Route by RouteId
void loadRouteAPI()
{
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
  LoadRoute(RequestContent["RouteId"]);
  server.send(200, "application/json", "{Result:Sucess}");
}

void LoadNextRouteApi()
{
  String ReturnString = RoutesFunction.getNextRouteIdAsString();
  server.send(200, "application/json",ReturnString) ;
}
void LoadPreviousRouteApi()
{
  String ReturnString = RoutesFunction.getPreviousRouteIdAsString();
  server.send(200, "application/json",ReturnString) ;
}


void mirrorRouteAPI() 
{ 
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
  JsonArray lights = RequestContent["lights"];
  WallFunctions.setLEDs(lights);
  server.send(200, "application/json", "{Result:Sucess}");
}

void setLEDAPI()
{
  String body = server.arg("plain");
  deserializeJson(RequestContent, body);
   JsonArray rgb =  RequestContent["color"];
  int lightNumber = RequestContent["LightNum"];
  WallFunctions.setLED(rgb,lightNumber);
  server.send(200, "application/json", "{Result:Sucess}");
}

//get Current LEDs that are on and what color
void getLEDsAPI()
{
  JsonDocument LedsJson = WallFunctions.getLEDs();
  String LEDs;
  serializeJson(LedsJson,LEDs);
  server.send(200, "application/json",LEDs);
}

void handleNotFound()
{  
  String path = pathFunc.GetCleanPath(server.uri());
  String dataType = pathFunc.GetFileType(path);
  File dataFile = SPIFFS.open(path.c_str());

  if (!dataFile.available()){
    server.send(200,"text/html","File Not Found");
    return;
  }
  //stream file 
   size_t streamsize = server.streamFile(dataFile, dataType);;

  if (streamsize != dataFile.size()) {
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
    Serial.print("IP address:");
    Serial.println(WiFi.localIP());
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
void startupOTA()
{
   OTA
    .onStart([]() {
      String type;
      if (OTA.getCommand() == U_FLASH)
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

  OTA.begin();
}

// setup API resources
void setup_routing() {
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

void setup() 
{
  SPIFFS.begin(true); //intialize file system
  pinMode(2,OUTPUT) ; //used to intialize the onboard LED GPO02 pin as output
  Serial.begin(9600);
  Serial.println("StartUp Beginning");
  connectToWiFi();
  startupOTA();
  setup_routing();  
  RoutesFunction.loadRoutesFile();
  WallFunctions.initalizePixels();
  Serial.println("Controller Loaded");

}

void loop() 
{
  server.handleClient();
  OTA.handle();
   //reconnect to wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
       connectToWiFi();
  }

}