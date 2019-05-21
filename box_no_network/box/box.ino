
#include <TimerOne.h>
#include <MD_REncoder.h>

#include "Lights.h"

const int defaultTemp = 150;
const int defaultAmplitude = 40;


//Refresh Settings
long lastReach;
const int netUpdateInterval = 2000;

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

  readControls();
  updateOutputs();
  
}

void loop() {
  if (readControls()) {
    updateOutputs();
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
