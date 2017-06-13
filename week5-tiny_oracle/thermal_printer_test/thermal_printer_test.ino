/************
Tiny Oracles 1.0
https://learn.adafruit.com/adafruit-neopixel-featherwing
http://richardhayler.blogspot.ca/2016/04/getting-started-with-adafruit-feather.html
**************/
#include <SoftwareSerial.h>
#include <Adafruit_Thermal.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEO_PIN 15
#define TX_PIN 14 	// printer rx = blue / yellow
#define RX_PIN 12  	// printer tx = green

Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, NEO_PIN, NEO_GRB + NEO_KHZ800);
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);

void setup() {
	Serial.begin(115200);
	Serial.println(F("PRINTER TEST"));
	mySerial.begin(9600); 
	printer.begin();  
	printer.println(F("HIYA! I'M A PRINTER!"));
 printer.feed(1);
 delay(500);
 strip.begin();
 strip.show(); // Initialize all pixels to 'off'
  	
}

void loop() {
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  rainbow(20);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}


void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


