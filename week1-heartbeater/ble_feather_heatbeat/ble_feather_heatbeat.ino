#include <Arduino.h>
#include <SPI.h>

#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================*/
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define GAPDEVNAME                  "heartbeat"
/*=========================================================================*/

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// Variables 
int pulsePin = A0;         // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
//int threshold = 550;  
//int oldSensorValue = 0;

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

void setup(void) 
{
  while (!Serial);  // required for Flora & Micro
  delay(500);
  Serial.begin(9600);
  Serial.println("heartbeat!"); 
  Serial.println("");
  Serial.println(F("Adafruit Bluefruit Command <-> Data Mode communication"));
  Serial.println(F("------------------------------------------------"));
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if (!ble.begin(VERBOSE_MODE))
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println(F("OK!"));


  /*if (FACTORYRESET_ENABLE)
  {
    // Perform a factory reset to make sure everything is in a known state 
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }*/

  /* Disable command echo from Bluefruit */
  ble.echo(false);
  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println();
  ble.verbose(false);  // debug info is a little annoying after this point!
  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }
  Serial.println(F("******************************"));
  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }
  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);
  Serial.println(F("******************************"));

  interruptSetup(); 

  }



void loop(void)
{
  if (QS == true){     // A Heartbeat Was Found
                       // BPM and IBI have been Determined
                       // Quantified Self "QS" true when arduino finds a heartbeat
      ble.println(BPM);
      //ble.print(",");
      //ble.print(IBI);
      //ble.print(",");
      //ble.println(Signal);

    QS = false;                      // reset the Quantified Self flag for next time
  }

  delay(20);             

}

