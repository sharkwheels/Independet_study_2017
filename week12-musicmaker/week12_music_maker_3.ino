
// Specifically for use with the Adafruit Feather, the pins are pre-set here!
// include SPI, MP3 and SD libraries

#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <elapsedMillis.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS      16     // VS1053 chip select pin (output)
#define VS1053_DCS     15     // VS1053 Data/command select pin (output)
#define CARDCS          2     // Card chip select pin
#define VS1053_DREQ     0     // VS1053 Data request, ideally an Interrupt pin // <-- i think this is the issue?

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

const char* ssid = "XXX";
const char* password = "XXX";
const char* serverURL ="XXX";     

const long oneSecond = 1000; // a second is a thousand milliseconds
const long oneMinute = oneSecond * 60;
const long oneHour   = oneMinute * 60;

elapsedMillis timerRest;      
const long dataInterval = oneSecond * 30;   // x seconds

String overRideState = "";
int count = 0;
int allStarCount = 0;
bool isInOverRide = false;

char *names[3]={"track001", "track002", "track003"};
char *files[3]={"track001.mp3", "track002.mp3", "track003.mp3"};

char *overrideName = "override";
char *overrideTrack = "override.mp3";

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


	Serial.println("\n\nAdafruit VS1053 Feather Test");
	if (! musicPlayer.begin()) { // initialise the music player
		 Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
		 while (1);
	}

	Serial.println(F("VS1053 found"));
	musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
	if (!SD.begin(CARDCS)) {
		Serial.println(F("SD failed, or not present"));
		while (1);  // don't do anything more
	}
	Serial.println("SD OK!");
	printDirectory(SD.open("/"), 0);

	// Set volume for left, right channels. lower numbers == louder volume!
	musicPlayer.setVolume(10,10);
	musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

    // begin the repeating data timer on setup.
    timerRest = 0;

	Serial.println("setup done");


}

void loop() {
	if(count >= 3){ count = 0; }

	if (timerRest > dataInterval) {
        timerRest -= dataInterval; 
        getData();
    } 

    if(overRideState == "on"){
    	isInOverRide = true;
    }else{
    	isInOverRide = false;
    }

    if(isInOverRide){
    	Serial.print("Playing: ");
    	Serial.println(overrideName);
    	musicPlayer.stopPlaying();
    	musicPlayer.playFullFile("override.mp3");
    	allStarCount ++;
    	isInOverRide = false;
    	overRideState = "off";
    	
    }else{
    	if(musicPlayer.stopped() && count < 3){
    		Serial.print("Playing: ");
        	Serial.println(names[count]);
        	musicPlayer.startPlayingFile(files[count]);	// this refuses to play a song
        	count ++; 
        	Serial.print("file count: ");
        	Serial.println(count);
    	}
	}
}


void getData(){
	/// Get Data w/ a get request
    if((WiFi.status() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        http.begin(serverURL); 
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
	/// Parse JSON
    Serial.println(payload);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    
    if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
    }

    String message = root["state"];
    overRideState = message;				// will either be off or on
}



void printDirectory(File dir, int numTabs) {
	/// File listing helper
	 while(true) {
		 
		 File entry =  dir.openNextFile();
		 if (! entry) {
			 // no more files
			 //Serial.println("**nomorefiles**");
			 break;
		 }
		 for (uint8_t i=0; i<numTabs; i++) {
			 Serial.print('\t');
		 }
		 Serial.print(entry.name());
		 if (entry.isDirectory()) {
			 Serial.println("/");
			 printDirectory(entry, numTabs+1);
		 } else {
			 // files have sizes, directories do not
			 Serial.print("\t\t");
			 Serial.println(entry.size(), DEC);
		 }
		 entry.close();
	 }
}
