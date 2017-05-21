/* ======================= INCLUDES ================================= */
#include <application.h>
#include "neopixel.h"
#include "elapsedMillis.h"
#include "Adafruit_IO_Client.h"

TCPClient client;

/* ======================= IO SETUP ================================= */
#define AIO_SERVER      "io.adafruit.com"       // server
#define AIO_SERVERPORT  8883                   // use 8883 for SSL / 1883 not
#define AIO_USERNAME    "miss_n"              // username
#define AIO_KEY         "6b4813d66eae4b1ab931ed5ce1f51eb8"    // adafruit key

Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);
//current gas reading
Adafruit_IO_Feed heartbeat = aio.getFeed("heartbeat");
/* ======================= NEOPATTERNS CLASS ================================= */

/* This class extends Nexpixels to be non-blocking.
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-3/overview
 *
 * DON'T DECLARE A TYPE: this bungs it up for some weird ass reason
 * To reduce NeoPixel burnout risk, add 1000 uF capacitor across pixel power leads
 * Add 300 - 500 Ohm resistor on first pixel's data input
*/
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
    NeoPatterns(uint16_t pixels, uint8_t pin, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin)
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


/* ======================= NEOPATTERNS ================================= */
void Strip1Complete();
NeoPatterns Strip1(12, 5, &Strip1Complete); // pin 5
/* ======================= VARIABLES ================================= */
int motor = D3;              // motor pin
int buttonPin = D6;          // button pin
bool oldState = HIGH;        // button old state
bool motorOn = false;        // is the motor on?
int getRate = 6000;          // look for a new value every minute
int pixelInterval;                // speed for the pixels
int BASE_THRESHOLD = 50;     // the lowest heartrate
int MAX_THRESHOLD = 300;     // the highest heartrate
int currentReading;          // the current value from the feed
int lastGet;                 // the last time we checked for a value from i/o
bool isActive = false;      // is the program active
int mappedSpeed;            // speed the raw data is mapped to

elapsedMillis motorTimer;
int vibrateTime = 1000;

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

void setup()
{
  Serial.begin();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(motor, OUTPUT);         // set the motor pin to output
  digitalWrite(motor, HIGH);
  pixelInterval = map(vibrateTime,100,1000, 0, 3.5);
  Strip1.begin();
  Strip1.Fade(Strip1.Color(0,0,0), Strip1.Color(0,0,0), 255,pixelInterval); // don't ask I'm just tired of dealing with it.
  motorTimer = 0;

}

void loop()
{

  // always update the Strip
  Strip1.Update();


  //Serial.print("vibrateTime: "); Serial.println(vibrateTime);
  //Serial.print("pixelInterval: "); Serial.println(pixelInterval);


  if(isActive)
  {

    // get a new reading from feed
    int newReading;
    int newPixel;
    if(millis()-lastGet>=getRate) {
      // set the new reading
      FeedData latest = heartbeat.receive();
      // reset timer
      lastGet = millis();
      // convert to int
      if (latest.isValid()){
        if (latest.intValue(&newReading)){
          // is it above the thresh?
          if(newReading >= BASE_THRESHOLD){
            // set the upper bound
            if(newReading >= MAX_THRESHOLD){
              newReading = MAX_THRESHOLD;
            }
            vibrateTime = map(newReading, BASE_THRESHOLD, MAX_THRESHOLD, 1000, 200);
            newPixel = map(vibrateTime,100,1000, 0, 3.5);
            Strip1.Interval = newPixel;
            Serial.println("newPixel: "); Serial.println(newPixel);
          }// base threshold
        }// lastest&newReading
      }//isvalid
    } // end of millis/lastGet
    if(motorOn && (motorTimer >= vibrateTime))
    {
      motorTimer-=vibrateTime;
      int currentState = digitalRead(motor);
      digitalWrite(motor, !currentState);
    }
  }
  bool newState = digitalRead(buttonPin);
  if (newState == LOW && oldState == HIGH)
  {
    delay(50);
    newState = digitalRead(buttonPin);
    if (newState == LOW)
    {
      if(!isActive)
      {
        isActive = true;
        Serial.println("is Active!");
        motorOn = true;
        Strip1.ActivePattern = FADE;
        Strip1.Color1 = Strip1.Color(255,0,0);
        Strip1.Color2 = Strip1.Color(0,0,0);




      } else if(isActive){
        isActive = false;
        Serial.println("not active!");
        motorOn = false;
        digitalWrite(motor, HIGH);
        Strip1.ActivePattern = FADE;
        Strip1.Color1 = Strip1.Color(0,0,0);
        Strip1.Color2 = Strip1.Color(0,0,0);
        Strip1.Interval = 10;          // Doesn't update for some reason?
        //Strip1.ActivePattern = COLOR_WIPE;
        //Strip1.Color1 = Strip1.Color(0,0,0);  //updates
        //Strip1.TotalSteps = Strip1.numPixels();
        //Strip1.Interval = 20;
        //Strip1.Color2 = Strip1.Color(0,0,0);  //updates
        //Strip1.Interval = 10;
      }
    }
  }
  oldState = newState;
} // end of void

void Strip1Complete(){
  // reverse the breathe
  Strip1.Reverse();

}
//Serial.print(F("newReading: ")); Serial.println(newReading, DEC);
/*if(newReading != currentReading){
    // map the heart rate to a pixel speed

    //vibrateTime = mappedSpeed;
    currentReading = newReading;
}else{
  Serial.println(F("nothing different keep values"));
} // newReading vs currentReading*/
