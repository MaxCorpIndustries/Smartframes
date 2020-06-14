

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
 
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
 
 
float AccelMinX = 0;
float AccelMaxX = 0;
float AccelMinY = 0;
float AccelMaxY = 0;
float AccelMinZ = 0;
float AccelMaxZ = 0;
  
 
int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
 
void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  // begin the serial monitor @ 9600 baud
  Serial.begin(9600);
  Serial.println("ADXL345 Accelerometer Calibration"); 
  Serial.println("");
  
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }  
}
 
void loop() 
{

    sensors_event_t accelEvent;  
    accel.getEvent(&accelEvent);
    
  // read the value from the sensor:
  sensorValue = accelEvent.acceleration.z;
  if ((accelEvent.acceleration.z > 10) || (accelEvent.acceleration.z < -20) )
  {
  Serial.println("AWAKE!!!!!!!!");
  }
  Serial.println(sensorValue);
  Serial.print(" ");
 
  delay(20);
}
