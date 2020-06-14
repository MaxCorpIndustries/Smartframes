#include <SoftwareSerial.h>
#include <avr/wdt.h>
const byte rxPin = 10;
const byte txPin = 9;

SoftwareSerial BLEserial (rxPin, txPin);  // Declare BLE serial pins

char c = ' ';
char ANCS_8Buffer[16];
void setup() 
{
  Serial.begin(9600);
  BLEserial.begin(9600);  
  
  
  Serial.println("---Sys Online---");

}

void loop() 
{
  if (BLEserial.available())
  {
   c = BLEserial.readBytes(ANCS_8Buffer, 16); //save input to 16 bit char 
   Serial.println("Buffer contents: " + String(ANCS_8Buffer));//print resulting 16 bit char

   Serial.println("Char 7: " + String(ANCS_8Buffer[7]));//print resulting 16 bit char
   Serial.println("Char 8: " + String(ANCS_8Buffer[8]));//print resulting 16 bit char
   Serial.println("Char 9: " + String(ANCS_8Buffer[9]));//print resulting 16 bit char
   Serial.println("Char 10: " + String(ANCS_8Buffer[10]));//print resulting 16 bit char
   Serial.println("Char 11: " + String(ANCS_8Buffer[11]));//print resulting 16 bit char
   Serial.println("Char 12: " + String(ANCS_8Buffer[12]));//print resulting 16 bit char
   Serial.println("Char 13: " + String(ANCS_8Buffer[13]));//print resulting 16 bit char
   Serial.println("Char 14: " + String(ANCS_8Buffer[14]));//print resulting 16 bit char
   Serial.println("Char 15: " + String(ANCS_8Buffer[15]));//print resulting 16 bit char
  }
}
