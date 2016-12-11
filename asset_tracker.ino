/*
 * Asset Tracker
 * Uses Arduino, Adafruit Ultimate GPS Breakout, and Adafruit FONA phone module
 * Allows a phone to send an SMS, which is received by the FONA.
 * The FONA then gets the GPS coordinates via Arduino, and sends them back to the phone.
 * This code is based on the SMS functionality from Adafruit's "Open Sesame" code
 * https://learn.adafruit.com/open-sesame-a-sms-controlled-door-lock/software
 * By Alex Voto, 12/10/16
*/

#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define FONA_TX 3
#define FONA_RX 2
#define FONA_RST 4
#define FONA_RI 5

#define LED 13
#define actuator 12

#define BUSYWAIT 5000  // milliseconds

// this is a large buffer for replies
char replybuffer[255];
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

//This is the FONA initialization function
boolean fonainit(void) {
  Serial.println(F("Initializing....(May take 3 seconds)"));
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  // make it slow so its easy to read!
  fonaSS.begin(4800); // if you're using software serial
  // See if the FONA is responding
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    Serial.println(F("Couldn't find FONA"));
    return false;
  }
  Serial.println(F("FONA is OK"));
  return true; 
}
//end of FONA initialization Function

void setup() {

  // set LED output for debugging
  pinMode(LED, OUTPUT);
  pinMode(actuator,OUTPUT);  
  
  Serial.begin(115200);
  Serial.println(F("FONA basic test"));

  while (! fonainit()) {
    delay(5000);
  }
  
  // Print SIM card IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }
  
  pinMode(FONA_RI, INPUT);
  digitalWrite(FONA_RI, HIGH); // turn on pullup on RI
  // turn on RI pin change on incoming SMS!
  fona.sendCheckReply(F("AT+CFGRI=1"), F("OK"));
}

int8_t lastsmsnum = 0;

void loop() {
   digitalWrite(LED, HIGH);
   delay(100);
   digitalWrite(LED, LOW);
       
  while (fona.getNetworkStatus() != 1) {
    Serial.println("Waiting for cell connection");
    delay(2000);
  }
  
  // this is a 'busy wait' loop, we check if the interrupt
  // pin went low, and after BUSYWAIT milliseconds break out to check
  // manually for SMS's and connection status
  // This would be a good place to 'sleep'
  for (uint16_t i=0; i<BUSYWAIT; i++) {
     if (! digitalRead(FONA_RI)) {
        // RI pin went low, SMS received?
        Serial.println(F("RI went low"));
        break;
     } 
     delay(1);
  }
  
  int8_t smsnum = fona.getNumSMS();
  if (smsnum < 0) {
    Serial.println(F("Could not read # SMS"));
    return;
  } else {
    Serial.print(smsnum); 
    Serial.println(F(" SMS's on SIM card!"));
  }
  
  if (smsnum == 0) return;

  // if you've gotten this far, there's an SMS!
  uint8_t n = 1; 
  while (true) {
     uint16_t smslen;
     char sender[25];
     
     Serial.print(F("\n\rReading SMS #")); Serial.println(n);
     uint8_t len = fona.readSMS(n, replybuffer, 250, &smslen); // pass in buffer and max len!
     // if the length is zero, its a special case where the index number is higher
     // so increase the max we'll look at!
     if (len == 0) {
        Serial.println(F("[empty slot]"));
        n++;
        continue;
     }
     if (! fona.getSMSSender(n, sender, sizeof(sender))) {
       // failed to get the sender?
       sender[0] = 0;
     }
     
     Serial.print(F("***** SMS #")); Serial.print(n);
     Serial.print(" ("); Serial.print(len); Serial.println(F(") bytes *****"));
     Serial.println(replybuffer);
     Serial.print(F("From: ")); Serial.println(sender);
     Serial.println(F("*****"));
     
//Trigger!!! Change text for different text commands. Change enclosed function to do different things.
     if (strcasecmp(replybuffer, "do action 1") == 0) {
       // turn on the led for a second
       action1();
     }
     if (strcasecmp(replybuffer, "do action 2") == 0) {
       // turn on the led for a second
       action2();
     }
     delay(3000);
     break;
  }  
  fona.deleteSMS(n);
 
  delay(1000); 
}


void action1() {
  digitalWrite(actuator, HIGH);
  delay(1000);
  digitalWrite(actuator,LOW);
}

void action2() {
  digitalWrite(actuator, HIGH);
  delay(10000);
  digitalWrite(actuator,LOW);
}
