#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <elapsedMillis.h>

const char* ssid = "XXXXX";
const char* password = "XXXXX";
const char* serverURL ="http://XXXXX:XXXX/XXXX";

const long oneSecond = 1000; // a second is a thousand milliseconds
const long oneMinute = oneSecond * 60;
const long oneHour   = oneMinute * 60;

elapsedMillis timerRest;      
const long dataInterval = oneSecond * 5; // 5 seconds

bool isActive = false;
bool blenderRan = false;

String cmd = "";  
String st = "";

#define RELAY_PIN 2

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
	
	// reset all the things
	timerRest = 0;
	cmd = "default";
	st = "off";
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH);

	// delay and setup done
	delay(100);
	Serial.println("setup done");
}

void loop() {

	// poll the server every 5 seconds.
	if (timerRest > dataInterval) {
        timerRest -= dataInterval; 
        getData();
    } 

	if(st == "on"){
		if(cmd == "blender"){
			digitalWrite(RELAY_PIN, LOW);
		}else if(cmd == "pixels"){
			// fire on some pixels
		}else if(cmd == "song"){
			// run a sung
		}else if(cmd == "party"){
			// start a party
		}

	}else if(st == "off"){

		if(cmd != "blender"){
			digitalWrite(RELAY_PIN,HIGH);
		}else if(cmd != "pixels"){
			// turn off some pixels
		}else if(cmd != "song"){
			// turn off a song
		}else if(cmd != "party"){
			// turn off a party
		}
	}
}

void getData(){
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
	Serial.println(payload);
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(payload);
	if (!root.success()) {
		Serial.println("parseObject() failed");
		return;
	}
	String command = root["command"];
	String state = root["state"];
	Serial.println(command);
	Serial.println(state);
	cmd = command;
	st = state;
}
