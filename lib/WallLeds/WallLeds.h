#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
//#pragma once

//WallLeds.cpp
    class WallLeds
    {
        public:
        JsonDocument getLEDs();

        public:
        void setLEDs(JsonArray lights);

        public:
        void setLED(JsonArray rgb, int lightNumber);

        public: 
        void initalizePixels();

    
    };

