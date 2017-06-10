/**************
Weather Servos 1.0
Nadine Lessio
June 2017

Feather pulls and parses JSON data from Weather Underground. 
Servos respond to wind speed. Like a weather vein. 

https://github.com/bblanchon/ArduinoJson
https://gist.github.com/bbx10/149bba466b1e2cd887bf
******************/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const char* ssid = "***";
const char* password = "***";
const char* host = "api.wunderground.com";

String apiKey = "XXXXX";
String location = "Canada/YYZ";


static char respBuf[4096];

bool collectionPaused = false;

/******* Servo Stuff ********/

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);    // initialize the servo drivers

#define SERVOMIN  200         // (out of 4096) pulse length
#define SERVOMAX  500         // (out of 4096) pulse length
int servonum = 0;             // servo counter
int degrees = 0;              // initialize. 180 and 75 are closest to 180 and 90 for these servos
bool ud = true;               // up/down boolean (servo position alternator)
bool end = false;             // is this the end of the wave?
bool servosActive = false;    // is the servoroutine active?
int servoRoutineCount = 0;    // how many times has it run the active routine?

elapsedMillis timerRest;      // rest for servo routine (X seconds on X seconds off)
int servoInterval = 30000;    // 30 seconds on // 30 seconds off. 

elapsedMillis timerWave;  
int waveSpeed = 100;      // speed of the servo wave. Lower is faster (keep it above 1). >~200 is one servo at a time without overlap

elapsedMillis collectionTimer;
int collectionWait = 180000; // 3 minutes

int tempC = 0;
int windSpeed = 0;


void setup() {
	Serial.begin(115200);
  delay(10);
  Serial.println("setting pwm...");
  pwm.begin();           
  pwm.setPWMFreq(60);    
  delay(100);
  for(int i; i < 7; i++){
    pwm.setPWM(i,0,SERVOMIN);
    delay(1);
  }
  // ok so for some reason you must declare this and do the wfi AFTER any PWM. Maybe a library smash? 
  delay(100);
  
  #ifdef ESP8266
      Wire.pins(2, 14);   // ESP8266 can use any two pins, such as SDA to #2 and SCL to #14
  #endif
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
  getData();
  delay(100);
  Serial.println("ready");
}

void loop() 
{
  if(servosActive){
    runServos();
  }else{
    restServos();
  }

  if(!collectionPaused){
    if((!collectionPaused) && collectionTimer > collectionWait ){
      getData();
    }
  }else if(collectionPaused){
    // if the servo routine count is 
    // end the servos
    // begin the countdown to the next collection
    if(servoRoutineCount == 10){
        servoRoutineCount = 0;
        collectionTimer = 0;
        collectionPaused = false;
        servosActive = false;
    }

    if (timerRest > servoInterval) {
      timerRest -= servoInterval; 
      if(!servosActive){
        servoRoutineCount++;
      } 
      servosActive = !servosActive;
      
      Serial.print("servosActive: ");
      Serial.println(servosActive);
      Serial.print("servoRoutineCount: ");
      Serial.println(servoRoutineCount);
    }
  }
}

void runServos(){
  waveSpeed = map(windSpeed,5,50,1000,100);
  if (servonum > 7) {
    servonum = 7;
    end = true;            
  }else if(servonum < 0){
    servonum = 0;
    end = false;
  }
  if(end){
    ud = false; 
  }else{
    ud = true; 
  }

  if (ud == true) {       
    degrees = 180;        
  } else {
    degrees = 10;         
  }

  int pulselength = map(degrees,0,180,SERVOMIN,SERVOMAX);    
  
  if (servonum <= 7) {                                      
    pwm.setPWM(servonum, 0, pulselength);                   
    if (timerWave > waveSpeed) {
      timerWave -= waveSpeed; //reset the timer
      if(end){
        servonum--;
      }else{
        servonum ++; 
      }
      //Serial.println(servonum);
      //Serial.print("ud: ");
      //Serial.println(ud); 
      //Serial.print("degrees: ");
      //Serial.println(degrees);
      //Serial.print("waveSpeed: ");   
      //Serial.println(waveSpeed);
    }
  }
}

void restServos(){
  for(int i; i < 7; i++){
    pwm.setPWM(i,0,SERVOMIN);
    delay(1);
  }
}

void getData(){
  Serial.println("!getData: getting data!");
  // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;

    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
      }
      Serial.println("Getting Data...");

    // We now create a URI for the request
      String url = "/api/" + apiKey + "/conditions/q/"+ location + ".json";
      Serial.print("Requesting URL: ");
      Serial.println(url);

      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.0\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

      delay(500); // need this delay or else it doesn't grab stuff. 

      int respLen = 0;
      bool skip_headers = true;

      while (client.available()){
        if(skip_headers){
          String response = client.readStringUntil('\n');
          //Serial.print(response);
          if(response.length() <= 1){
            skip_headers = false;
          }

        } else {
          int bytesIn;
          bytesIn = client.read((uint8_t *)&respBuf[respLen], sizeof(respBuf) - respLen);
          Serial.print(F("bytesIn ")); Serial.println(bytesIn);
          if (bytesIn > 0) {
            respLen += bytesIn;
              if (respLen > sizeof(respBuf)) respLen = sizeof(respBuf);
          }else if (bytesIn < 0) {
            Serial.print(F("read error "));
              Serial.println(bytesIn);
          }
        }
        delay(1);
      }
      client.stop();
      if (respLen >= sizeof(respBuf)) {
        Serial.print(F("respBuf overflow "));
        Serial.println(respLen);
        return;
      }
      // Terminate the C string
      respBuf[respLen++] = '\0';
      Serial.print(F("respLen "));
      Serial.println(respLen);
      //Serial.println(respBuf);
      
      if(showWeather(respBuf));

      Serial.println(" ");
      Serial.println("closing connection");
      Serial.println("shutting off collection routine...");   

      collectionPaused = true;
      servosActive = true;
      timerRest = 0;
}


bool showWeather(char *json){
	StaticJsonBuffer<3*1024> jsonBuffer;
	char *jsonstart = strchr(json, '{');
  	//Serial.print(F("jsonstart ")); Serial.println(jsonstart);
  	if (jsonstart == NULL) {
    	Serial.println(F("JSON data missing"));
    	return false;
  	}
  	json = jsonstart;
  	// Parse JSON
  	JsonObject& root = jsonBuffer.parseObject(json);
  	
  	if (!root.success()) {
    	Serial.println(F("jsonBuffer.parseObject() failed"));
    	return false;
  	}
  	// Extract weather info from parsed JSON
  	JsonObject& current = root["current_observation"];
  	//const float temp_f = current["temp_f"];
  	//Serial.print(temp_f, 1); Serial.print(F(" F, "));
  	const int temp_c = current["temp_c"];
  	Serial.print(temp_c, 1); Serial.print(F(" C, "));
  	const char *humi = current[F("relative_humidity")];
  	Serial.print(humi);   Serial.println(F(" RH"));
  	const char *weather = current["weather"];
  	Serial.println(weather);
  	const char *pressure_mb = current["pressure_mb"];
  	Serial.println(pressure_mb);
  	const char *observation_time = current["observation_time_rfc822"];
  	Serial.println(observation_time);
    const int wind_kph = current["wind_kph"];
    Serial.print(wind_kph,1); Serial.print(F(" KPH "));
    const int wind_degrees = current["wind_degrees"];
    Serial.print(wind_degrees,1); Serial.print(F(" Deg "));

  	// Extract local timezone fields
  	const char *local_tz_short = current["local_tz_short"];
  	Serial.println(local_tz_short);
  	const char *local_tz_long = current["local_tz_long"];
  	Serial.println(local_tz_long);
  	const char *local_tz_offset = current["local_tz_offset"];
  	Serial.println(local_tz_offset);

    tempC = temp_c;
    windSpeed = wind_kph;

  	return true;

}









