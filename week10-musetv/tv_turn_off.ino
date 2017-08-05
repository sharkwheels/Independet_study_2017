/************
Turn a TV off if you're really into what you are watching. 
Turn it back on after 5 seconds
*************/


byte incomingByte;            // from python
int command = 0;              // command (1 = open, 2 = close)
int turnOffTv = 0;
int tvIsOn = true;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.flush();
  pinMode(12, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);       // begin serial communication
  Serial.setTimeout(20);    // set the timeout, the default is 1 second which is nuts.

}
  

// the loop function runs over and over again forever
void loop() {

    // if the serial is avaiable and is not disabled. 

    if (Serial.available()>0 && tvIsOn) {
    //incomingByte = Serial.parseInt();   // use if testing from arduino input
    incomingByte = Serial.read();         // use if live
    command = incomingByte;
    turnOffTv = readCom(command);
  }

  if(turnOffTv == 2){
    // snap on the relay and turn off the TV
    digitalWrite(12, HIGH); 
    digitalWrite(LED_BUILTIN, HIGH);
    tvIsOn = false;


  } else {
    // turn the TV back on
    digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(LED_BUILTIN, LOW);
    tvIsOn = true;
  }


if(!tvIsOn){
  Serial.println("tv Off");
  delay(5000);
  turnOffTv = 0;
  tvIsOn = false;

}else{
  //Serial.println("tv On");
}



/*
Serial.print("resetTimerFired");
Serial.println(resetTimerFired);

Serial.print("resetTimer");
Serial.println(resetTimer);

Serial.println(" ");
Serial.println("----------------------");
Serial.println(" ");*/
                   
}

int readCom(int com){
  Serial.println("!inReadCom");   // track that you are in this function
  Serial.println(com);            // print the command
  return com;                     // return the command. 
}

 /*
  digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(5000);                       
  digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
  delay(5000);    */ 
