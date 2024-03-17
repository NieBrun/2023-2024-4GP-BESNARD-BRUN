/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
//Bluetooth
#include <SoftwareSerial.h>
#define rxPin 8 // Correspondant à la broche tx du module bluetooth
#define txPin 7 // Correspondant à la broche Rx du module bluetooth
#define baudrate 9600 
SoftwareSerial mySerial(rxPin ,txPin); //Definition du software serial

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(baudrate);
  mySerial.begin(baudrate); // Initialiser le port bluetooth
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
int c=0;
char val[10];
void loop() {

  if(mySerial.available()>0){
    switch (mySerial.read()) {
    case 1: // si arduino reçois le chiffre 1 alors
      digitalWrite(LED_BUILTIN, HIGH); // allume sur la broche "ledpin"
      Serial.println(1);
      break;
    case 2: // si arduino reçois le chiffre 2 alors
      digitalWrite(LED_BUILTIN, LOW); // allume sur la broche "ledpin"
      Serial.println(2);
      break;
    case 3: // si arduino reçois le chiffre 3 alors "clignote"
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      Serial.println(3);
    }                   // wait for a second
  }
  itoa(c,val,10);
  mySerial.write(val);
  Serial.println(val);
  c++;
  delay(1500);

}
