
#include "Arduino.h"
#include "LightSensor.h"

LightSensor::LightSensor(int pin)
{
  _pin = pin;
}

LightSensor::LightSensor(int pin, int dataPoints, int measTime)
{
  _pin = pin;
  _dataPoints = dataPoints;
  _measTime = measTime;
}

int LightSensor::instantMeasure()
{
  return analogRead(_pin);
}

double* LightSensor::measure()
{
  int buffer[_dataPoints]; // Store "_dataPoints" max values of individual measures
  double results[2]; // Used to return mean and variance of measure
  double aux[2]; // Used to compare current result with new measurements
  
  long inicio = millis();  //time of the begin
    
  while((millis() - inicio) < _measTime)
    {
    //writes in bufferSensor all the information individualy
    for (int m = 0; m < _dataPoints; m++){

      // read the sensors and saves one information individualy in every buffer
	buffer[m] = analogRead(_pin);
	
      }
      
      aux[0] = mean(buffer); //takes a mean of the sensor's info
      
     if( results[0] < aux[0] )//search the higher mean of all the groups of meditions and calculates the variance of the group
     {
       results[0] = aux[0];
       results[1] = variance(buffer, aux[0]);
       }
    }
    
    return results;
}

double LightSensor::mean(int* data)
{
  int dataLength = sizeof(data)/sizeof(data[0]);
  double sum = 0;
  
  for(int i = 0; i < dataLength; i++)
  {
    sum += data[i];
  }
  
  return sum/dataLength;
  
}

double LightSensor::variance(int* data)
{
  int dataLength = sizeof(data)/sizeof(data[0]);
  
  double meanAux = mean(data);
  
  double aux = 0;
  double sum = 0;
  
  for(int i = 0; i < dataLength; i++){
    aux = (data[i] - meanAux);
    aux = aux*aux;
    sum += aux;
  }
  
  return sum/(dataLength-1);
  
}

double LightSensor::variance(int* data, double mean)
{
  int dataLength = sizeof(data)/sizeof(data[0]);
  
  double aux = 0;
  double sum = 0;
  
  for(int i = 0; i < dataLength; i++){
    aux = (data[i] - mean);
    aux = aux*aux;
    sum += aux;
  }
  
  return sum/(dataLength-1);
  
}