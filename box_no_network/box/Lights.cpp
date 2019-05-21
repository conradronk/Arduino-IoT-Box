#include <Arduino.h>
#include "Lights.h"

long shiftStart;
long shiftDuration;
int shiftOriginAmp;
int shiftTargetAmp;
int shiftOriginTemp;
int shiftTargetTemp;

int pinW;
int pinC;
float amplitude;
int amplitudeSetPoint;
float temperature;
int tempSetPoint;
int warmVal;
int coolVal;


//Public Methods
Lights::Lights(int warmPin, int coolPin) {
  pinW = warmPin;
  pinC = coolPin;
  
  pinMode(pinW, OUTPUT);
  pinMode(pinC,OUTPUT);
}

void Lights::setValues(int amplitudeInput, int temperatureInput) {
  amplitudeSetPoint = amplitudeInput;
  tempSetPoint = temperatureInput;
  curve(amplitudeSetPoint,tempSetPoint);
  calculateChannels();
}

void Lights::shift(long duration, int fromAmp, int toAmp, int fromTemp, int toTemp) {
  shiftStart = (long) millis();
  shiftDuration = duration;
  shiftOriginAmp = fromAmp;
  shiftTargetAmp = toAmp;
  shiftOriginTemp = fromTemp;
  shiftTargetTemp = toTemp;
  curve(fromAmp,fromTemp);
  calculateChannels();
  writeOutput();
}

void Lights::refresh() {
  int transAmp = amplitudeSetPoint;
  int transTemp = tempSetPoint; 
  if (millis() < (shiftStart + shiftDuration)) {
    float progress = ((millis()-shiftStart)/(shiftDuration*1.0))*1000.0;
    transAmp = map(progress, 0, 1000, shiftOriginAmp,shiftTargetAmp);
    transTemp = map(progress, 0, 1000, shiftOriginTemp,shiftTargetTemp);
  }
  amplitudeSetPoint = transAmp;
  tempSetPoint = transTemp;
  curve(transAmp,transTemp);
  calculateChannels();
  writeOutput();
}

int Lights::getAmplitude() {
  return amplitudeSetPoint;
}

int Lights::getTemperature() {
  return tempSetPoint;
}


//Private Methods
void Lights::curve(int amplitudeInput, int temperatureInput) {
  amplitude = amplitudeInput/255.0;
  temperature = temperatureInput/255.0;
}

void Lights::calculateChannels() {
  //int overhead = 60;
  warmVal = (amplitude*255.0)*temperature;
  coolVal = (amplitude*255.0)*(temperature-1.0)*-1.0;
}

void Lights::writeOutput() {
  analogWrite(pinW,warmVal);
  analogWrite(pinC,coolVal);
}
