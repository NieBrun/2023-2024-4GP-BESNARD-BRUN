// Variables pour éviter le delay 
unsigned long previousMillis = 0;        // will store last time LED was updated

// Variable pour cadencer l'acquisition
const long interval = 500;           // interval at which to blink (milliseconds)

// POTAR
#include <SPI.h>

const byte csPin           = 10;      // MCP42100 chip select pin
const int  maxPositions    = 256;     // wiper can move from 0 to 255 = 256 positions
const long rAB             = 47000;   // 100k pot resistance between terminals A and B, 
                                      // mais pour ajuster au multimètre, je mets 92500
const byte rWiper          = 125;     // 125 ohms pot wiper resistance
const byte pot0            = 0x11;    // pot0 addr // B 0001 0001
const byte pot0Shutdown    = 0x21;    // pot0 shutdown // B 0010 0001
long R2;


void setPotWiper(int addr, int pos) {
  pos = constrain(pos, 0, 255);            // limit wiper setting to range of 0 to 255
  digitalWrite(csPin, LOW);                // select chip
  SPI.transfer(addr);                      // configure target pot with wiper position
  SPI.transfer(pos);
  digitalWrite(csPin, HIGH);               // de-select chip

  // print pot resistance between wiper and B terminal
  R2 = ((rAB * pos) / maxPositions ) + rWiper;
  Serial.print("Wiper position: ");
  Serial.print(pos);
  Serial.print(" Resistance wiper to B terminal: ");
  Serial.print(R2);
  Serial.println(" ohms");
}

//Bluetooth
#include <SoftwareSerial.h>
#define rxPin 8 // Correspondant à la broche tx du module bluetooth
#define txPin 7 // Correspondant à la broche Rx du module bluetooth
#define baudrate 9600 
SoftwareSerial mySerial(rxPin ,txPin); //Definition du software serial

// Pin pour mesure de la tension du capteur
int capteurgraphitePin = 1;
int val=0;

float R3=100000;
int Vcc=5;
float R1=100000;
int R5=10000;
float Vadc=0;
float Res=-1;


void setup() {
  mySerial.begin(baudrate); // Initialiser le port bluetooth
  Serial.begin(baudrate); // Initialiser le port série
  digitalWrite(csPin, HIGH);        // chip select default to de-selected
  pinMode(csPin, OUTPUT);           // configure chip select as output
  SPI.begin();
  setPotWiper(pot0,127);
  delay(100);
  val=Calibration();

}


void loop() {

  // setPotWiper(pot0,int(10));
  Vadc = analogRead(capteurgraphitePin)*5.0/1024.0;
  Res=R1*(1+R3/R2)*Vcc/Vadc-R1-R5;
  Serial.println(Res);
  delay(1000);
  // Serial.print("POT POS   ");
  // Serial.print(val);
  // unsigned long currentMillis = millis ();
  
  //  if (currentMillis - previousMillis >= interval) {
  //   previousMillis = currentMillis;
  //   Vadc = analogRead(capteurgraphitePin);
  //   Serial.println(Vadc); // Afficher sur le port série la valeur de la tension mesuré en bytes
  //   mySerial.write(Vadc); // Envoyer sur le port bluetooth la valeur acquise

  // compteur++;
  //   if (compteur%2==0){
  //     val++;
  //     setPotWiper(pot0,val);
  //     Serial.print("Pot :");
  //     Serial.println(val);
  //     }
  //   if (val>64){val=0;}
  // }
  
}

int tolerance=20; //Tolerance : 512+-20
int V_Cible = 512;
int PotPos = 127;

int Calibration(){
 int Vadc=analogRead(capteurgraphitePin);
  int tolerance=20; //Tolerance : 512+-20
  int V_Cible = 512;
  int a = 0;
  int b = 255;
  int ite=0;
  while(abs(Vadc - V_Cible) > tolerance)
  {
    Vadc=0;
    ite++;
    PotPos = (a + b)/2;
    setPotWiper(pot0,int(PotPos));
    for(int i=0;i<3;i++)
    {
      Vadc += analogRead(capteurgraphitePin);
      delay(100);
    }
    Vadc = int(Vadc/3);
    Serial.print(F("Position Pota :"));
    Serial.print(PotPos);
    Serial.print(F(". Valeur lue :"));
    Serial.println(Vadc); // Afficher sur le port série la valeur de la tension mesuré en bytes
    if(Vadc > V_Cible){
      a = PotPos;}
    else{
      b = PotPos;}
    if(ite>12){break;}
  }
  Serial.print(F("Finie. Pota en "));
  Serial.println(PotPos); // Afficher sur le port série la valeur de la tension mesuré en bytes
  return PotPos;

}
