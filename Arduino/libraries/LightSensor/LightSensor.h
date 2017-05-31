
#ifndef LightSensor_h
#define LightSensor_h

#include "Arduino.h"

class LightSensor
{
  public:
    LightSensor(int pin);
    LightSensor(int pin, int dataPoints, int measTime); // Manual asign of parameters
    int instantMeasure();
    double* measure();	//First value of int[] returned is mean and the second one is the measurement's variance
  private:
    double mean(int* data); //Calculates mean of data
    double variance(int* data); // Calculates variance of data
    double variance(int* data, double mean); //Calculates variance of data with given mean
    
    int _pin;  //Analog pin connected to sensor, in Arduino uno goes from 0 to 5
    int _dataPoints = 3;   //Number of data singletons to consider in meas result, default is 3, max is 32
    int _measTime = 10000;   //Measurement time in millis, default is 10 seconds
    
};

#endif
