/*
Analog Steering Wheel Controls Bluetooth Interface

This uses an Analog input to measure button presses
then sends the appropiate commands to an BlueCreations.com
BC127 bluetooth module that is running Melody Audio v6 
via Serial.  It also uses HID to do screen unlock and app
switching of a USB OTG connected tablet

I'm using a SparkFun.com PurpleTooth 
Jamboree BC127 Development Board. I've updated it to 
the newest firmware, Melody 6.0.48, and configured it
to support multiple connections for audio and phone.
The Support -> Tools section of their site has the 
firmware update tools and also offers
a tool to activate APTX and CVC codecs for testing
for free. I will include my running configuration as
a text file in this package.

https://www.sparkfun.com/products/11924

Note that I did not use the BC127 Library that is provided.
It was written for an old firmware version and does not
really support the features needed for this task.

Requires https://github.com/NicoHood/HID

The values below are what I measured. If yours differs
use the analog to serial example sketch to retrieve
new numbers.  NOTE: if you enable more Digital Output pins
these values will change.

Vol Up 47-49
Vol Down 55-57
Seek Up 75-77
Seek Down 63-65
Source Up 167-169
Source Down 282-283
Talk 92-94
Prog 118-120 
normal 1018
with input_pullup

On a 05 chevy silverado the two wires are connected into the BCM
on Connector 3 (Brown) Pin A7 (Lite Green), 
and Connector 2 (Grey) Pin A2 (Dark Blue).
I cut the wires just short of the connectors and spliced them
to wires going to GND and A0 on the Leonardo.
*/

#include "HID-Project.h"

const int inputPin = A0;
const int inputPin2 = A1;

const int volumeUp = 1;
const int volumeDown = 2;
const int seekUp = 3;
const int seekDown = 4;
const int sourceUp = 5;
const int sourceDown = 6;
const int talk = 7;
const int prog = 8;
int sensorValue = 0;
int lastSensorValue = 0;
int buttonPress = 0;
int keyHeld = 0;
unsigned long keyHeldTimer = 0;
int playPause = 0;
int onCall = 0;
int callRinging = 0;

String readString, stringNotif , stringVar1, stringVar2;
String EOL = String("\r");
String volumeLinkID = "10";
String musicLinkID = "11";
String callLinkID = "13";

String getValue(String data, char separator, int index)
{
 int found = 0;
  int strIndex[] = {
0, -1  };
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
  if(data.charAt(i)==separator || i==maxIndex){
  found++;
  strIndex[0] = strIndex[1]+1;
  strIndex[1] = (i == maxIndex) ? i+1 : i;
  }
 }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void parseSerial() { //Process any serial data received from the bluetooth module
    if (Serial1.available()) { //see if there is any data waiting
    int waitingEOL = 1;
    while (waitingEOL) { //lets retrieve a full line before continuing
    delay(3);  //delay to allow buffer to fill 
    if (Serial1.available() >0) {
      char c = Serial1.read();  //gets one byte from serial buffer
      if (c == '\n' || c == '\r') {waitingEOL=0;} //we have hit the end of the line
      else if (isPrintable(c)) {readString += c;} //makes the string readString, but only standard characters
    }
    else {waitingEOL=0;} //we missed the carrage return or newline, so let's stop trying.
    }
  }

  if (readString.length() >0) {
    Serial.println(readString); //send what was received to Serial for troubleshooting
    String stringNotif = getValue(readString,' ',0); //split out the sections of the line
    String stringVar1 = getValue(readString,' ',1);
    String stringVar2 = getValue(readString,' ',2);
    readString=""; //clear the received string buffer

    if (stringNotif == String("A2DP_STREAM_START")) { //music has started playing
      volumeLinkID = stringVar1; //update the Link ID for volume control
      Serial.print("Music Link ID: ");
      Serial.println(volumeLinkID);
    }
    
    else if (stringNotif == String("AVRCP_PLAY")) { //music has started playing and can be controlled
      musicLinkID = stringVar1; //update the Link ID for music control
      playPause = 1; //remember that we are now listening to music
      Serial.print("Music RCP Play Link ID: ");
      Serial.println(musicLinkID);
    }
    else if (stringNotif == "AVRCP_STOP" || stringNotif == "AVRCP_PAUSE") { //music has stopped
      playPause = 0; 
      Serial.print("Music RCP Stop/Pause Link ID: ");
      Serial.println(musicLinkID);
    }    
    else if (stringNotif == String("CALL_ACTIVE")) { //we are now talking on the phone
      callLinkID = stringVar2; //update the Link ID for phone calls
      callRinging = 0; //the phone is not ringing any more
      onCall = 1; //we are on a call
      Serial.print("Call Active Link ID: ");
      Serial.println(callLinkID);
    }
    else if (stringNotif == String("CALL_IDLE")) { //the call has ended
      onCall = 0; //call is over
      callRinging = 0; //the phone is not ringing
      Serial.print("Call Idle Link ID: ");
      Serial.println(callLinkID);
    }
    else if (stringNotif == String("CALL_INCOMING")) { //there is a incoming phone call
      callLinkID = stringVar2; //update the Link ID for phone calls
      callRinging = 1; //the phone is ringing
      Serial.print("Call Ringing Link ID: ");
      Serial.println(callLinkID);
    }

  }
}

void readSensor() { //This reads the analog input and makes sure that it's stable before continuing
      lastSensorValue = sensorValue; //remember what the last value was
      sensorValue = analogRead(inputPin); //read the new value
  while ( ! ( sensorValue < lastSensorValue + 3 && sensorValue > lastSensorValue - 3)) { //make sure the value is stable +-3
    lastSensorValue = sensorValue;
    sensorValue = analogRead(inputPin);
    delay(10);
  }

  if (sensorValue >= 45 && sensorValue < 51) {buttonPress = volumeUp;} //sense the values and update the buttonPress value
  else if (sensorValue >= 51 && sensorValue < 60) {buttonPress = volumeDown;}
  else if (sensorValue >= 60 && sensorValue < 70) {buttonPress = seekDown;}
  else if (sensorValue >= 70 && sensorValue < 85) {buttonPress = seekUp;}
  else if (sensorValue >= 85 && sensorValue < 105) {buttonPress = talk;}
  else if (sensorValue >= 105 && sensorValue < 145) {buttonPress = prog;}
  else if (sensorValue >= 145 && sensorValue < 200) {buttonPress = sourceUp;}
  else if (sensorValue >= 200 && sensorValue < 300) {buttonPress = sourceDown;}
  else {buttonPress = 0;} //no button is pressed
  if (Serial.available()) {buttonPress = Serial.parseInt();} //for diagnostics. input a buttonPress value from the Serial console to simulate inputs
  if (buttonPress) {Serial.println(buttonPress);} //confirm what button was pressed
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(inputPin, INPUT_PULLUP);
  Consumer.begin(); //Consumer HID keyboard init
  System.begin(); //System HID keyboard init
  BootKeyboard.begin(); //Boot standard HID keyboard init
  delay(100);
  Serial1.print("RESET\r"); //Make sure that the bluetooth module is starting fresh

  //delay(4000); //Make sure to give your device enough time to wake up
  //BootKeyboard.write(KEY_SPACE); //Prompt the unlock screen
  //delay(1000);
  //BootKeyboard.println("123456"); //Unlock code for your device
  //delay(100);
}

void loop() {
  unsigned long currentMillis = millis();
  parseSerial();
  readSensor(); //Read the current button state

  // For application tabbing, we check to see if the Alt key
  // is still being held
  if (keyHeld && keyHeldTimer > currentMillis - 5000) { //check if we are holding a key and within our timeout
    if (buttonPress == sourceUp) { //the button is pressed again
     keyHeldTimer = millis(); //reset our timer
     BootKeyboard.press(KEY_LEFT_ALT); //redundant, it is all ready heald
     delay(100);
     BootKeyboard.press(KEY_TAB); //press TAB
     delay(100);
     BootKeyboard.release(KEY_TAB); //release TAB
     delay(300);
     }
   }

   //If the key hold timer has expired, release it
   else if (keyHeld) {
    BootKeyboard.releaseAll(); //release all held keys
    keyHeld = 0;
    delay(300);
   }

   //Now on to just detecting key presses
  else if (buttonPress == talk) { //The talk button is pressed. this can do a few different things...
    if (callRinging) { //if the phone is ringing, answer it
      Serial1.print("CALL ");
      Serial1.print(callLinkID);
      Serial1.print(" ANSWER\r");
    }
    else if (onCall) { //if we are talking on the phone, hang up
      Serial1.print("CALL ");
      Serial1.print(callLinkID);
      Serial1.print(" END\r");
    }
    else {
    Serial1.print("TOGGLE_VR "); //otherwise, let's get Google or SIRI to take a voice command
    Serial1.print(callLinkID);
    Serial1.print("\r");
    delay(300);
    }
  }
  else if (buttonPress == volumeUp) { //Volume UP
    Serial1.print("VOLUME ");
    if (onCall) {Serial1.print(callLinkID);} //if we are on a call, change the call volume
    else {Serial1.print(volumeLinkID);} //otherwise let's just change the music volume
    Serial1.print(" UP\r");
    delay(300);
  }
  else if (buttonPress == volumeDown) { //Volume DOWN
    Serial1.print("VOLUME ");
    if (onCall) {Serial1.print(callLinkID);} //same idea as above
    else {Serial1.print(volumeLinkID);}
    Serial1.print(" DOWN\r");
    delay(300);
  }
  else if (buttonPress == seekUp) { //Skip to next track
    Serial1.print("MUSIC ");
    Serial1.print(musicLinkID);
    Serial1.print(" FORWARD\r");
    delay(300);
  }
  else if (buttonPress == seekDown) { //Skip to previous track
    Serial1.print("MUSIC ");
    Serial1.print(musicLinkID);
    Serial1.print(" BACKWARD\r");
    delay(300);
  }
  else if (buttonPress == prog) { //Start or Stop the Music
    Serial1.print("MUSIC ");
    Serial1.print(musicLinkID);
    if (playPause) {Serial1.print(" PAUSE\r");} //If music is playing, then stop it
    else {Serial1.print(" PLAY\r");} //nothing is playing, let's listen to something
    delay(300);
  }
  else if (buttonPress == sourceDown) { //this is a go to the home screen button on Android
    BootKeyboard.releaseAll();
    keyHeld = 0;
    Consumer.write(CONSUMER_BROWSER_HOME);
    delay(300);
  }

  //Here is the Application switching. It's part of
  //a two part process as it requires the alt key to
  //be held for a period of time.
  else if (buttonPress == sourceUp) { //App switching, hit the button to cycle through running apps
    keyHeld = 1; //remember that we are holding a key down
    keyHeldTimer = millis(); //remember when we pressed that key
    BootKeyboard.press(KEY_LEFT_ALT); //holding ALT
    delay(100);
    BootKeyboard.press(KEY_TAB); //pressing TAB
    delay(100);
    BootKeyboard.release(KEY_TAB); //releasing TAB
    delay(300);
  }
  
delay(100);
}


