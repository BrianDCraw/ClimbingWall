#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Routes.h>

int CurrentRouteIndex;
int MaxRouteIndex;
JsonDocument RoutesFileJson;
String RoutesFileString;

String pgetCurrentRouteIdAsString()
{
 JsonDocument Routes;
  String ReturnString;
  Routes["RouteId"] = CurrentRouteIndex;
  serializeJson(Routes,ReturnString);
  return ReturnString;
} 

JsonDocument Routes::getallRoutes(){
    return RoutesFileJson;
}


void Routes::saveAllRoutes(String routes)
{
      SPIFFS.begin();
    String fullpath = "/assets/routes.json";
    File file = SPIFFS.open(fullpath,"w");
    deserializeJson(RoutesFileJson,routes);
    file.println(routes);

}

JsonArray  Routes::getLightsForRoute(int RouteId)
{
    JsonArray lights;
      JsonArray routes = RoutesFileJson.as<JsonArray>();
      for(JsonVariant route : routes) { 
      if (route["RouteId"] == RouteId) {   
          lights = route["Lights"];

      }
      }
      CurrentRouteIndex = RouteId;
      return lights;
}

String Routes::getCurrentRouteIdAsString()
{
  String ReturnString = pgetCurrentRouteIdAsString();
  return ReturnString;
}


String Routes::getNextRouteIdAsString()
{
  if (CurrentRouteIndex != MaxRouteIndex){
    CurrentRouteIndex++;
  }
  else{
    CurrentRouteIndex = 1;
  }
   String ReturnString = pgetCurrentRouteIdAsString();
  return ReturnString;    
}

String Routes::getPreviousRouteIdAsString()
{
  if (CurrentRouteIndex > 1){
    CurrentRouteIndex--;
  }
  else{
    CurrentRouteIndex = MaxRouteIndex;
  }
  String ReturnString = pgetCurrentRouteIdAsString();
  return ReturnString;    
}

void Routes::loadRoutesFile()
 {
    SPIFFS.begin();
    String fullpath = "/assets/routes.json";
    File file = SPIFFS.open(fullpath,"r");
    RoutesFileString = file.readString(); 
    file.close();
    deserializeJson(RoutesFileJson, RoutesFileString);

   JsonArray routes = RoutesFileJson.as<JsonArray>();
   MaxRouteIndex = 0;
   CurrentRouteIndex = 0;
  for(JsonVariant route : routes) { 
      if (route["RouteId"] > MaxRouteIndex) {   
          MaxRouteIndex = route["RouteId"];
      }
  }
 } 