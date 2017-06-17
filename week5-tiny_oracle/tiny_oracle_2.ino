/************
Tiny Oracles 1.0
https://learn.adafruit.com/adafruit-neopixel-featherwing
http://richardhayler.blogspot.ca/2016/04/getting-started-with-adafruit-feather.html
**************/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_Thermal.h>
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};

#define NEO_PIN 15
#define TX_PIN 14   // printer rx = blue / yellow
#define RX_PIN 12   // printer tx = green


const char* ssid = "XXX";
const char* password = "xXX";      

const long oneSecond = 1000; // a second is a thousand milliseconds
const long oneMinute = oneSecond * 60;
const long oneHour   = oneMinute * 60;

SoftwareSerial pSerial(RX_PIN, TX_PIN); 
Adafruit_Thermal printer(&pSerial);

void Strip1Complete();
NeoPatterns Strip1(32, NEO_PIN, NEO_GRB + NEO_KHZ800, &Strip1Complete);

elapsedMillis timerRest;      
const long dataInterval = oneMinute * 30;    

elapsedMillis pixelTimer;
const long pixelRunTime = oneSecond * 20;  
bool pixelTimerFired = false;

String msg = "";
String feels = "";

bool isActive = false;
bool printerRan = false;
bool pixelsRan = false;

void setup() {
	
    Serial.begin(115200);
    Serial.setDebugOutput(true);

     // connect to the wifi
    Serial.println();
    Serial.print("Connecting to wifi");
    Serial.println(ssid);
    WiFi.begin(ssid,password);
    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("Wifi connected");
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    delay(100);

     // neopixel cick off
    Serial.println("starting pixels");
    Strip1.begin();
    Strip1.Fade(Strip1.Color(50,0,50), Strip1.Color(0,0,0),200,20);
    delay(100);

    // Printer runs on 9600 baud and uses software serial
    Serial.println("starting printer");
    pSerial.begin(9600); 
    printer.begin();  

    // begin the repeating data timer on setup.
    timerRest = 0;

    Serial.println("setup done");
}

void loop() {
    Strip1.Update();
    if(isActive){

        if(!printerRan){
            Serial.println(msg);
            printer.println(msg);
            printer.feed(3);
            Serial.println("stopping printer");
            printerRan = true;
        }
        if(!pixelsRan){
            //Serial.println(clr); 
            if(feels == "angry"){
            Strip1.ActivePattern = SCANNER;
            Strip1.Color1 = Strip1.Color(100,0,0); 
            Strip1.Interval = 5;

            }else if(feels == "sad"){
                Strip1.ActivePattern = FADE;
                Strip1.Color1 = Strip1.Color(0,0,100);
                Strip1.Color2 = Strip1.Color(0,0,0); 
                Strip1.Interval = 20;

            }else if(feels == "happy"){
                Strip1.ActivePattern = RAINBOW_CYCLE;
                Strip1.TotalSteps = 255;
                Strip1.Interval = 5;

            }
        }

        if((!pixelTimerFired) && (pixelTimer > pixelRunTime)){
            Serial.println("stopping pixels");
            pixelTimerFired = true;
            pixelsRan = true;
            Serial.println("setting to inactive");
            isActive = false; 
        }
          
    }else{
        Strip1.ActivePattern = FADE;
        Strip1.Color1 = Strip1.Color(100,0,100); 
        Strip1.Color2 = Strip1.Color(0,0,0);
        Strip1.Interval = 50;
        if (timerRest > dataInterval) {
            timerRest -= dataInterval; 
            getData();
        } 
    }    
}

void getData(){
    if((WiFi.status() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        http.begin("http://0.0.0.0:5000/cityfeels"); 
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                parsePayload(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}

void parsePayload(String payload){
    Serial.println(payload);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
    }
    String feeling = root["feeling"];
    String message = root["message"];
    msg = message;
    feels = feeling;
    setTheThings();
}

void setTheThings(){
    pixelTimer = 0;
    pixelTimerFired = false;
    printerRan = false;
    pixelsRan = false;
    isActive = true; 
}

void Strip1Complete(){
  Strip1.Reverse();
}

