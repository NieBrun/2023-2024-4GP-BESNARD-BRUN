/*
MCP41xxx Digital Pot Control
 
This example controls an MCP41xxx digital potentiometer.
PBn - One side of potentiometer
PWn - Pot wiper
PAn - Other side of potentiometer
 
The MCP41xxx is SPI-compatible. The hardware is managed by 
sending two bytes:
BYTE 1 - is a command byte (NOP, WRITE, SHUTDOWN)
BYTE 2 - is the data byte (new setpoint value 0-255). 
 
The MCP IC circuit:
* PB0 to ground 
* PA0 to Vcc
* PW0 Connect a multimeter between PW0 and ground to measure resistance 
* CS - to digital pin 10 (SS pin)
* SDI - to digital pin 11 (MOSI pin)
* CLK - to digital pin 13 (SCK pin)
*/

#include <SPI.h>

// Define the MCP41100 OP command bits (only one POT)
// Note: command byte format xxCCxxPP, CC command, PP pot number (01 if selected) 
#define MCP_NOP 0b00000000
#define MCP_WRITE 0b00010001
#define MCP_SHTDWN 0b00100001

const int ssMCPin = 10; // Define the slave select for the digital pot

#define WAIT_DELAY 5000

void SPIWrite(uint8_t cmd, uint8_t data, uint8_t ssPin) // SPI write the command and data to the MCP IC connected to the ssPin
{
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0)); //https://www.arduino.cc/en/Reference/SPISettings
  
  digitalWrite(ssPin, LOW); // SS pin low to select chip
  
  SPI.transfer(cmd);        // Send command code
  SPI.transfer(data);       // Send associated value
  
  digitalWrite(ssPin, HIGH);// SS pin high to de-select chip
  SPI.endTransaction();
}

void setup() 
{
  pinMode (ssMCPin, OUTPUT); //select pin output
  digitalWrite(ssMCPin, HIGH); //SPI chip disabled
  SPI.begin(); 
 
  Serial.begin(57600);
  Serial.println(F("[MCP Digital Pot Test]")); //print to serial
}

int val =0;
void loop() 
{
  char someChar[32] ={0};
  int i = 0;
  // step through the range of the digital pot
  // for (int i = 0; i < 256; i++) 
  // {
  //   Serial.println(i);
  //   SPIWrite(MCP_WRITE, i, ssMCPin);
  //   delay(WAIT_DELAY);
  // }
  // delay (WAIT_DELAY*5);

  while (Serial.available()) {
	  do{
      someChar[i++] = Serial.read();
      delay(2);		
	   }while (Serial.available() > 0);
	  val = atoi(someChar);	}
  Serial.println(val);
  SPIWrite(MCP_WRITE, val, ssMCPin);
  delay(WAIT_DELAY);
}
