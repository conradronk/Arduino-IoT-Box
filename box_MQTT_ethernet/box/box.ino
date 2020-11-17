#include <SPI.h>
#include <PubSubClient.h>

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

#include <TimerOne.h>
#include <MD_REncoder.h>

#include "Lights.h"

const int defaultTemp = 150;
const int defaultAmplitude = 40;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xED, 0xAD };


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "conradronk"
#define AIO_KEY         "34e3c0e31c5e4c6882d1bae2ece71757"

//Refresh Settings
long lastReach;
const int netUpdateInterval = 3000;

long lastRefresh;
const int lightRefreshInterval = 200;

//Global Variables
//Global Controls
const int switchShift = 44;
const int switchNet = 45;
//AC Controls
const int switchAC1 = 46;
const int switchAC2 = 47;
const int switchAC3 = 48;
const int switchAC4 = 49;
//LED controls
const int buttonA1 = 30;
const int buttonA2 = 33;
const int buttonB1 = 36;
const int buttonB2 = 39;
//Encoders
MD_REncoder AmpKnobA(18,32);
MD_REncoder TempKnobA(19,35);
MD_REncoder AmpKnobB(20,38);
MD_REncoder TempKnobB(21,41);

//Light outputs with associated PWM pins
Lights StripA(4,5);
Lights StripB(6,7);
//AC Relay Outputs
const int relayA = A4;
const int relayB = A3;
const int relayC = A2;
const int relayD = A1;

boolean shiftState;
boolean netState;
boolean ac1State;
boolean ac2State;
boolean ac3State;
boolean ac4State;

boolean ac1PhysState;
boolean ac2PhysState;
boolean ac3PhysState;
boolean ac4PhysState;

bool a1Saved;
bool b1Saved;
bool a2Saved;
bool b2Saved;

int ampAState;
int tempAState;
int ampBState;
int tempBState;
int prevKnobSum;


String splitString(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}



void callback (char* topic, byte* payload, unsigned int length) {
  Serial.println("recieved smth");
  if (netState) {
    payload[length] = '\0';
    String message = String((char*)payload);

    //Single feed for all AC channels. Mapping payloads to channels and states

    //Working with topic, value, and length
    if(strcmp(topic, "conradronk/feeds/box")==0){
      //could split string into key-values. Yeah, kinda have to
      //String key = splitStriing(
    }
    
    
    //Handles the LED PWM channels
    if (strcmp(topic,"conradronk/feeds/channel_a_amplitude")==0){
      ampAState = message.toInt();
    }
  
    if (strcmp(topic,"conradronk/feeds/channel_a_temperature")==0) {
      //Serial.println("Change in temp");
      tempAState = message.toInt();
    }
    
    
    //Previous code: handles boolean vales, mapping AC channels to feeds
    if (strcmp(topic,"conradronk/feeds/ac_1_setting")==0) {
      ac1State = message.toInt();
    }
    if (strcmp(topic,"conradronk/feeds/ac_2_setting")==0) {
      ac2State = message.toInt();
    }
    if (strcmp(topic,"conradronk/feeds/ac_3_setting")==0) {
      ac3State = message.toInt();
    }
    if (strcmp(topic,"conradronk/feeds/ac_4_setting")==0) {
      ac4State = meassage.toInt();
    }
  
    //Custom shift Routines
    if (strcmp(topic,"conradronk/feeds/sunrise_start")==0 && shiftState) {
      StripA.shift(600000L, 0, 40, 240, 180);
    }
    if (strcmp(topic,"conradronk/feeds/sunset")==0 && shiftState) {
      StripA.shift(30000L, ampAState, (ampAState - 0.5*ampAState), tempAState, 240);
    }
  }
}

EthernetClient client;
PubSubClient mqttclient(AIO_SERVER, AIO_SERVERPORT, callback, client);

void setup() {
  //pinMode Stuff
  pinMode(switchNet, INPUT);
  pinMode(switchShift, INPUT);
  pinMode(switchAC1, INPUT);
  pinMode(switchAC2, INPUT);
  pinMode(switchAC3, INPUT);
  pinMode(switchAC4, INPUT);
  pinMode(buttonA1, INPUT_PULLUP);
  pinMode(buttonA2, INPUT_PULLUP);
  pinMode(buttonB1, INPUT_PULLUP);
  pinMode(buttonB2, INPUT_PULLUP);
  pinMode(relayA, OUTPUT);
  pinMode(relayB, OUTPUT);
  pinMode(relayD, OUTPUT);
  pinMode(relayC, OUTPUT);

  //MD_REncoder Inits
  AmpKnobA.begin();
  TempKnobA.begin();
  AmpKnobB.begin();
  TempKnobB.begin();
  AmpKnobA.setPeriod(100);
  TempKnobA.setPeriod(100);
  AmpKnobB.setPeriod(100);
  TempKnobB.setPeriod(100);

  //TimerOne Inits
  Timer1.initialize(200);
  Timer1.attachInterrupt(tick);

  Serial.begin(9600);
  readControls();
  updateOutputs();
  
  //Network Inits
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac);
  delay(1000);

  if (mqttclient.connect("box")) {
     Serial.println(F("MQTT Connected"));
   } else {
    Serial.println(F("failed initial connection"));
    Serial.println(mqttclient.state());
   }
}

void loop() {
  if (readControls()) {
    updateOutputs();
  }
  
  
  
  //Only runs if the network/lockout switch is down, and at the network frequency
  if (netState && (lastReach + netUpdateInterval) < millis()) {
    lastReach = millis();
    reconnect();
    
    lastReach = millis();
    updateOutputs();
  }
  
  mqttclient.loop();

  //Code to run at the select light update frequency
  if (shiftState && (lastRefresh + lightRefreshInterval) < millis()) {
    StripA.refresh();
    StripB.refresh();
    
    ampAState = StripA.getAmplitude();
    tempAState = StripA.getTemperature();
    ampBState = StripB.getAmplitude();
    tempBState = StripB.getTemperature();
    
    lastRefresh = millis();
  }
}

bool reconnect() {
  Ethernet.maintain();
  if (mqttclient.connect("", AIO_USERNAME,AIO_KEY)) {
    mqttclient.subscribe("conradronk/feeds/channel_a_amplitude");
    //mqttclient.subscribe("conradronk/feeds/channel_a_temperature");
    mqttclient.subscribe("conradronk/feeds/ac_1_setting");
    mqttclient.subscribe("conradronk/feeds/ac_2_setting");
    mqttclient.subscribe("conradronk/feeds/ac_3_setting");
    mqttclient.subscribe("conradronk/feeds/ac_4_setting");
    //mqttclient.subscribe("conradronk/feeds/sunrise_start");
    //mqttclient.subscribe("conradronk/feeds/sunset");
  } else {
    Serial.println("unable to connect");
    //Serial.println(client.localIP());
    //client.stop();
    //client.connect();
    //PubSubClient mqttclient(AIO_SERVER, AIO_SERVERPORT, callback, client);
  }
}

void tick() {
  int velocity = (AmpKnobA.speed() + TempKnobA.speed() + AmpKnobB.speed() + TempKnobB.speed()) + 1;
  switch (AmpKnobA.read()) {
    case 16:
      ampAState -= 1 * velocity;
      break;
    case 32:
      ampAState += 1 * velocity;
      break;
    default:
      ampAState += 0;
  }

  switch (TempKnobA.read()) {
    case 16:
      tempAState -= 1 * velocity;
      break;
    case 32:
      tempAState += 1 * velocity;
      break;
    default:
      tempAState += 0;
  }
  switch (AmpKnobB.read()) {
    case 16:
      ampBState -= 1 * velocity;
      break;
    case 32:
      ampBState += 1 * velocity;
      break;
    default:
      ampBState += 0;
  }
  switch (TempKnobB.read()) {
    case 16:
      tempBState -= 1 * velocity;
      break;
    case 32:
      tempBState += 1 * velocity;
      break;
    default:
      tempBState += 0;
  }
}

bool readControls() {
  bool changed = false;
  bool individualChange = false;

  bool newShiftState = digitalRead(switchShift);
  changed = (shiftState != newShiftState || changed);
  shiftState = newShiftState;

  bool newNetState = digitalRead(switchNet);
  changed = (netState != newNetState || changed);
  netState = newNetState;

  bool newAC1PhysState = digitalRead(switchAC1);
  individualChange = (ac1PhysState != newAC1PhysState);
  changed = individualChange || changed;
  ac1PhysState = newAC1PhysState;
  if (individualChange) {
    ac1State=ac1PhysState;
    individualChange = false;
  }
  
  bool newAC2PhysState = digitalRead(switchAC2);
  individualChange = (ac2PhysState != newAC2PhysState);
  changed = individualChange || changed;
  ac2PhysState = newAC2PhysState;
  if (individualChange) {
    ac2State=ac2PhysState;
    individualChange = false;
  }

  bool newAC3PhysState = digitalRead(switchAC3);
  individualChange = (ac3PhysState != newAC3PhysState);
  changed = individualChange || changed;
  ac3PhysState = newAC3PhysState;
  if (individualChange) {
    ac3State=ac3PhysState;
    individualChange = false;
  }

  bool newAC4PhysState = digitalRead(switchAC4);
  individualChange = (ac4PhysState != newAC4PhysState);
  changed = individualChange || changed;
  ac4PhysState = newAC4PhysState;
  if (individualChange) {
    ac4State=ac4PhysState;
    individualChange = false;
  }
  
  bool newA1Saved = digitalRead(buttonA1);
  changed = (a1Saved != newA1Saved || changed);
  a1Saved = newA1Saved;

  bool newB1Saved = digitalRead(buttonB1);
  changed = (b1Saved != newB1Saved || changed);
  b1Saved = newB1Saved;

  bool newA2Saved = digitalRead(buttonA2);
  changed = (a2Saved != newA2Saved || changed);
  a2Saved = newA2Saved;

  bool newB2Saved = digitalRead(buttonB2);
  changed = (b2Saved != newB2Saved || changed);
  b2Saved = newB2Saved;

  int newKnobSum = ampAState + tempAState + ampBState + tempBState;
  changed = (newKnobSum != prevKnobSum || changed);
  prevKnobSum = newKnobSum;
  
  return changed;
}

void updateOutputs() {
  //AC Relays 
  if(ac1State) {
    digitalWrite(relayA,HIGH);
  } else {
    digitalWrite(relayA,LOW);
  }
  if(ac2State) {
    digitalWrite(relayB,HIGH);
  } else {
    digitalWrite(relayB,LOW);
  }

  if(ac3State) {
    digitalWrite(relayC,HIGH);
  } else {
    digitalWrite(relayC,LOW);
  }
  if(ac4State) {
    digitalWrite(relayD,HIGH);
  } else {
    digitalWrite(relayD,LOW);
  }

  if(!a1Saved) {
    ampAState = defaultAmplitude;
  }
  if(!a2Saved) {
    tempAState = defaultTemp;
  }
  if(!b1Saved) {
    ampBState = defaultAmplitude;
  }
  if(!b2Saved) {
    tempBState = defaultTemp;
  }
  
  
  ampAState = sanitizeNumber(ampAState);
  tempAState = sanitizeNumber(tempAState);
  ampBState = sanitizeNumber(ampBState);
  tempBState = sanitizeNumber(tempBState);

  
  
  StripA.setValues(sanitizeNumber(ampAState),sanitizeNumber(tempAState));
  StripA.writeOutput();

  StripB.setValues(sanitizeNumber(ampBState),sanitizeNumber(tempBState));
  StripB.writeOutput();
}

int sanitizeNumber(int input) {
  int output;
  if (input > 255) {
    output = 255;
  } else if (input < 0) {
    output = 0;
  } else {
    output = input;
  }
  return output;
}
    //Publish Code
    //char charBuff[8];
    //String(ampAState).toCharArray(charBuff, 8);
    //if (!mqttclient.connected()) {
    //  mqttclient.connect("", AIO_USERNAME, AIO_KEY);
    //  mqttclient.publish("conradronk/feeds/photocell", charBuff);
    //} else {
    //  mqttclient.publish("conradronk/feeds/photocell", charBuff);
    //}
    //reconnect();
    //mqttclient.subscribe("conradronk/feeds/channel_a_amplitude");
