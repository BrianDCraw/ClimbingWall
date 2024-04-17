#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
//#pragma once

//WallLeds.cpp
    class Routes
    {
        
        public: 
        JsonDocument getallRoutes();

        public:
        void saveAllRoutes(String routes);

        public:
        JsonArray getLightsForRoute(int RouteId);

        public:
        String getCurrentRouteIdAsString();

        public:
        String getNextRouteIdAsString();

        public: 
        String getPreviousRouteIdAsString();

        public:
        void loadRoutesFile();
    };

