#include <WallLeds.h>
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#define NUM_OF_LEDS 250
#define PIN 33

// Neopixel LEDs strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_OF_LEDS, PIN, NEO_RGB + NEO_KHZ800);



void WallLeds::initalizePixels()
{
  pixels.begin();
  pixels.clear();
  pixels.show();
}

JsonDocument WallLeds::getLEDs() {
  int pixelnumber = 0;
  String color ;
  int Red;
  int Green;
  int Blue;
  JsonDocument LEDS;

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


void  WallLeds::setLEDs(JsonArray lights) 
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

void  WallLeds::setLED(JsonArray rgb, int lightNumber)
{
  int red = rgb[0];
  int green = rgb[1];
  int blue = rgb[2];
  pixels.setPixelColor(lightNumber,red,green ,blue);
  pixels.show();
}
