#ifndef Lights_h
#define Lights_h
#include "Arduino.h"

class Lights
{
  public:
    //Methods
    Lights(int warmPin, int coolPin);
    void setValues(int amplitudeInput, int temperatureInput);
    void writeOutput();
    void shift(long duration, int fromAmp, int toAmp, int fromTemp, int toTemp);
    void refresh();
    int getAmplitude();
    int getTemperature();
  private:
    //Variables
    int pinW;
    int pinC;
    float amplitude;
    int amplitudeSetPoint;
    float temperature;
    int tempSetPoint;
    //Methods
    void curve(int amplitudeInput, int temperatureInput);
    void calculateChannels();

    //Shifting Variables
    long shiftStart;
    long shiftDuration;
    int shiftOriginAmp;
    int shiftTargetAmp;
    int shiftOriginTemp;
    int shiftTargetTemp;
};
#endif

