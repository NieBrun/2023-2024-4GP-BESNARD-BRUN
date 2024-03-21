//Bluetooth
#include <SoftwareSerial.h>
#define rxPin 7 // Correspondant à la broche tx du module bluetooth
#define txPin 8 // Correspondant à la broche Rx du module bluetooth
#define baudrate 9600 
SoftwareSerial mySerial(rxPin ,txPin); //Definition du software serial

//OLED ATTENTION, la librarie utilisée (d'origine) consomme énormement de mémoire RAM en buffer qui n'est pas affiché sur arduino (possible même si affiché seulement 40%)
// Si le programme ne lance rien, même pas le setup, le problème peut venir de là.
// On peut soit baisser le nombre de pixels dans l'écran, soit utiliser d'autre librairie moins gourmande facilement trouvable sur internet.
#include <Adafruit_SSD1306.h>
#define nombreDePixelsEnLargeur 128        // Taille de l'écran OLED, en pixel, au niveau de sa largeur
#define nombreDePixelsEnHauteur 64        // Taille de l'écran OLED, en pixel, au niveau de sa hauteur
#define brocheResetOLED         -1          // Reset de l'OLED partagé avec l'Arduino (d'où la valeur à -1, et non un numéro de pin)
#define adresseI2CecranOLED     0x3C        // Adresse de "mon" écran OLED sur le bus i2c (généralement égal à 0x3C ou 0x3D)
Adafruit_SSD1306 ecranOLED(nombreDePixelsEnLargeur, nombreDePixelsEnHauteur, &Wire, brocheResetOLED);  

// MESURE Pin pour mesure de la tension du capteur //////////
const int capteurgraphitePin = 1;  // Pin A1
float calibre = pow(10,-6); // pour avoir des Mohms
// Variables pour éviter le delay 
unsigned long previousMillis = 0;        // will store last time LED was updated
// Variable pour cadencer l'acquisition
const long interval = 500;           // interval at which to blink (milliseconds) for MesureINST

// MESURE FLEX SENSOR
const int flexPin = A0;      // Pin connected to voltage divider output
const float VCC = 5;      // voltage at Ardunio 5V line
const float R_DIV = 10000.0;  // resistor used to create a voltage divider
const float flatResistance = 30000.0; // resistance when flat
const float bendResistance = 1000.0;  // resistance at 90 deg

//BOUTONS
const int buttonPin[] = {6,5,4};  // Pin connected to button
// Variables will change:
bool buttonState[3]= {LOW,LOW,LOW};            // the current reading from the input pin
bool lastButtonState[3] = {LOW,LOW,LOW};  // the previous reading from the input pin
bool reading; // variable pour stocker la lecture de l'etat des boutons
//Variable debouncing
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// MENU ///////////////////////////
int Position = 1; // Position par défaut dans le menu
int OK = 0;        // Si le bouton central a été appuyé ou pas
String text1="1 : Graphite INST"; // Les différents menus
String text2="2 : Graphite MOY";
String text3="3 : Flex Sensor";
String text4="4 : Calibration";

//Android
bool OK_TEL=0; // Si un bouton sur téléphone a été cliqué.

// Digital Potentiometer
#include <SPI.h>
const byte csPin           = 10;      // MCP42100 chip select pin
const int  maxPositions    = 256;     // wiper can move from 0 to 255 = 256 Positions
const long rAB             = 47000;   // 50k pot resistance between terminals A and B, mais pour ajuster au multimètre, je mets 47500
const byte rWiper          = 125;     // 125 ohms pot wiper resistance
const byte pot0            = 0x11;    // pot0 addr // B 0001 0001
const byte pot0Shutdown    = 0x21;    // pot0 shutdown // B 0010 0001
long R2;                              // Valeur de R2 calculé en sortir du potentiomètre
int PotPos;                           // Valeur de la position du potentiomètre entre 0 et 255

// Permet de gérer le potentiomètre
void setPotWiper(int addr, int pos) {
  pos = constrain(pos, 0, 255);            // limit wiper setting to range of 0 to 255
  digitalWrite(csPin, LOW);                // select chip
  SPI.transfer(addr);                      // configure target pot with wiper Position
  SPI.transfer(pos);
  digitalWrite(csPin, HIGH);               // de-select chip

  R2 = ((rAB * pos) / maxPositions ) + rWiper; // Calcul de R2 (résistance entre wiper et B)
  Serial.print(F(" Resistance wiper to B terminal: "));
  Serial.print(R2);
}


void setup() {
    // Initialisation de l'écran OLED
  if(!ecranOLED.begin(SSD1306_SWITCHCAPVCC, adresseI2CecranOLED))
    while(1);   
  mySerial.begin(baudrate); // Initialiser le port bluetooth
  Serial.begin(baudrate); // Initialiser le port série


  pinMode(buttonPin[0], INPUT);     // Initialise les boutons
  pinMode(buttonPin[1], INPUT);
  pinMode(buttonPin[2], INPUT);
  digitalWrite(csPin, HIGH);        // chip select default to de-selected
  SPI.begin();
  PotPos=Calibration();             // Calibration du potentiomètre en fonction du capteur
  }
  
void loop() {
  ChoixMenu(); // Affiche le menu et ses choix en boucle
}

// Permet de se balader dans les 4 menus différents
//MENU 1 : MesureINST : Mesure du capteur graphite toutes les 700ms~~
//MENU 2 : MesureMoyenne : Mesure du capteur graphite en faisant 50 mesures sur 2500ms
//MENU 3 : MesureINST : Mesure du capteur flex sensor
//MENU 4 : Calibration : Calibration du potentiomètre en fonction du capteur
void ChoixMenu(){
  // // MENU
  unsigned long currentMillis = millis ();
  if (currentMillis - previousMillis >= 500) 
  {
    previousMillis = currentMillis;
    ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
    ecranOLED.setTextSize(1);                           // Taille du texte
    ecranOLED.setCursor(0, 0);                          // Placement du texte
    ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
    CheckBoutons();   // Fonction primordiale : vérifie si les boutons physiques ne sont pas appuyé et ajuste la variable position, OK et OK_TEL (si un bouton sur tél n'est pas cliqué)
    if(Position==1)                       // Si nous sommes en position 1, affiche le texte correspondant (effet de surbrillance)
    {
      ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      ecranOLED.println(text1);
      ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
      ecranOLED.println(text2);
      ecranOLED.println(text3);
      ecranOLED.println(text4);
      ecranOLED.display();                            // Transfert le buffer à l'écran
      if(OK_TEL==1) // Si un bouton sur téléphone est cliqué
      {
        OK_TEL=0; // Remet à 0
        MesureINST(); 
      }
      else if(OK==1) // Si le bouton central est cliqué
      {
        OK=0;     // Remet à 0
        MesureINST();
      }
    }
    else if (Position==2)// Si nous sommes en position 2, affiche le texte correspondant (effet de surbrillance)
    {
      ecranOLED.println(text1);
      ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);   // Couleur du texte, et couleur du fond
      ecranOLED.println(text2);
      ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
      ecranOLED.println(text3);
      ecranOLED.println(text4);
      ecranOLED.display();                            // Transfert le buffer à l'écran
       if(OK_TEL==1)
      {
        OK_TEL=0;
        MesureMoyenne();
      }
      else if(OK==1)
      {
        OK=0;
        MesureMoyenne();
      }
    }
    else if (Position==3)// Si nous sommes en position 3, affiche le texte correspondant (effet de surbrillance)
    {
      ecranOLED.println(text1);
      ecranOLED.println(text2);
      ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      ecranOLED.println(text3);
      ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK); 
      ecranOLED.println(text4);
      ecranOLED.display();                            // Transfert le buffer à l'écran
       if(OK_TEL==1)
      {
        OK_TEL=0;
        FlexSensor();
      }
      else if(OK==1)
      {
        OK=0;
        FlexSensor();
      }
    }
    else if (Position==4)// Si nous sommes en position 4, affiche le texte correspondant (effet de surbrillance)
    {
      ecranOLED.println(text1);
      ecranOLED.println(text2);
      ecranOLED.println(text3);
      ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
      ecranOLED.println(text4);
      ecranOLED.display();
      if(OK_TEL==1)
      {
        OK_TEL=0;
        Calibration();
      }
      if(OK==1)
      {
        OK=0;
        Calibration();
      }    
    }
  }
}


// Vérifie si un bouton a été appuyé
void CheckBoutons()
{
  for (int i = 0; i < 3; i++) {
    reading = digitalRead(buttonPin[i]);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:
    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState[i]) {
      // reset the debouncing timer
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
      buttonState[i] = reading;
    }
    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState[i] = reading;
  }
  if(buttonState[0]==HIGH && Position < 4){ // Permet de descendre dans les menus sauf si en position 4
    Position++;
    buttonState[0]=LOW;}
  else if(buttonState[0]==HIGH && Position == 4){ // Si en position 4 et que l'utilisateur veut descendre, cela le fait remonter en position 1
    Position=1;
    buttonState[0]=LOW;
  }
  if(buttonState[1]==HIGH){ // si bouton central appuyé, alors cela correspond à une validation
    OK=1;
    buttonState[1]=LOW;
    }
  if(buttonState[2]==HIGH && Position > 1){ // Permet de monter dans les menus sauf si en position 1
    Position--;
    buttonState[2]=LOW;
    }
  else if(buttonState[2]==HIGH && Position == 1){// Si en position 1 et que l'utilisateur veut monter, cela le fait descendre en position 4
    Position=4;
    buttonState[2]=LOW;
  }
    if(mySerial.available()>0){ // Lis ce qui a été envoyé sur le module Bluetooth si nécessaire
    switch (mySerial.read()) {
    case 1: // si arduino reçois le chiffre 1 alors
      Position=1;
      //OK=1;
      OK_TEL=1;
      Serial.println(1);
      break;
    case 2: // si arduino reçois le chiffre 2 alors
      Position=2;
      //OK=1;
      OK_TEL=1;
      Serial.println(2);
      break;
    case 3: // si arduino reçois le chiffre 3 alors
      Position=3;
      //OK=1;
      OK_TEL=1;
      Serial.println(3);
      break;
    case 4: // si arduino reçois le chiffre 4 alors 
      Position=4;
      //OK=1;
      OK_TEL=1;
      Serial.println(4);
      break;
    }                   // wait for a second
  }
}


//Amelioration: faire une fonction de tout ce qui est affichage répété dans les 3 fonctions suivantes
// Permet de mesurer toutes les 500ms la valeur du capteur graphite
void MesureINST()
{
  float R3=100000;
  int Vcc=5;
  float R1=100000;
  int R5=10000;
  float Vadc=0;
  float Res=-1;
  // Affiche l'affichage de mesureINST avant les mesures réelles, comme cela, l'utilisateur peut arreter d'appuyer sur le bouton OK et cela évite de le faire sortir du programme
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(1);                   // Taille des caractères (1:1, puis 2:1, puis 3:1)
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(F("Mesure instantannee : "));
  ecranOLED.display();    
  delay(1000);
  do
  {    
    // MESURE
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Vadc = analogRead(capteurgraphitePin)*5.0/1024.0; // Valeur lu en voltage sortant du capteur
      Res=(R1*(1+R3/R2)*Vcc/Vadc-R1-R5)*calibre; // Calcul de la résistance
      DisplayAndTransmitter(Res,1); // Transmet la donnée à afficher à l'ordi, à l'écran et au bluetooth
      CheckBoutons();
      }
  }while(!(OK == 1 || OK_TEL == 1)); // Tant que le bouton OK n'est pas appuyé, continue
  //Position=1;
  OK=0;
  //OK_TEL=0;
  //Permet de donner l'impression à l'utilisateur qu'il est sorti de l'écran MesureINST
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextSize(1);                 
  ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);   // Couleur du texte, et couleur du fond
  ecranOLED.println(text1);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(text2);
  ecranOLED.println(text3);
  ecranOLED.println(text4);
  ecranOLED.display();    
  delay(1000);
 
} 

//Mesure du capteur graphite sur 50 mesures en 2500ms, forme similaire à MesureINST
void MesureMoyenne(){
  float R3=100000;
  int Vcc=5;
  float R1=100000;
  int R5=10000;
  float Vadc=0;
  float Res=-1;
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(1);                   // Taille des caractères (1:1, puis 2:1, puis 3:1)
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(F("Mesure moyennee :"));
  ecranOLED.display();    
  delay(1000);
  do
  { 
    float moyenne=0;  
    for (int i = 0; i < 50; i++) {
      moyenne=moyenne+(analogRead(capteurgraphitePin)*5/1024); 
      CheckBoutons();
      delay(50);
    }
    moyenne=moyenne/50; // Fais la moyenne sur les 50 mesures
    Res=(R1*(1+R3/R2)*Vcc/moyenne-R1-R5)*calibre; // Calcul de la résistance à partir de la valeur lue moyenne, possible de faire la moyenne des résistance calculées
    //Serial.print(F("Mesure moyennee :"));
    //Serial.println(Res);
    DisplayAndTransmitter(Res,2); // Transmet la donnée à afficher à l'ordi, à l'écran et au bluetooth
  }while(!(OK == 1 || OK_TEL == 1));
  //Position=2;
  OK=0;
  //OK_TEL=0;
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextSize(1);                 
  ecranOLED.println(text1);
  ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);   // Couleur du texte, et couleur du fond
  ecranOLED.println(text2);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(text3);
  ecranOLED.println(text4);
  ecranOLED.display();    
  delay(1000);
}

// Mesure du flex sensor
void FlexSensor(){
  int ADCflex;
  float Vflex;
  float Rflex;
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(1);                   // Taille des caractères (1:1, puis 2:1, puis 3:1)
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(F("Mesure Flex Sensor:"));
  ecranOLED.display();    
  delay(1000);
  do
  {  
    // Read the ADC, and calculate voltage and resistance from it
    ADCflex = analogRead(flexPin);
    Vflex = ADCflex * VCC / 1023.0;
    Rflex = R_DIV * (VCC / Vflex - 1.0); // Calcul de la résistance du flex
    // Use the calculated resistance to estimate the sensor's bend angle:
    // float angle = map(Rflex, flatResistance, bendResistance, 0, 90.0);
    // Serial.println("Bend: " + String(angle) + " degrees");
    delay(500);
    DisplayAndTransmitter(Rflex,3);
    CheckBoutons();
  }while(!(OK == 1 || OK_TEL == 1));
  //Position=3;
  OK=0;
  //OK_TEL=0;    
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextSize(1);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond                 
  ecranOLED.println(text1);
  ecranOLED.println(text2);
  ecranOLED.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  ecranOLED.println(text3);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK); 
  ecranOLED.println(text4);
  ecranOLED.display();
  delay(1000);
}
// Transmet la donnée à afficher à l'ordi, à l'écran et au bluetooth
void DisplayAndTransmitter(float VALUE, int choix){
  char ResASCII[10];
  dtostrf(VALUE, 5, 2, ResASCII); //Transforme le float en chaine de caractères
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(1);                   // Taille des caractères (1:1, puis 2:1, puis 3:1)
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  if(choix==1){ecranOLED.println(F("Mesure instantannee :"));}
  else if(choix==2){ecranOLED.println(F("Mesure moyennee :"));}
  else if(choix==3){ecranOLED.println(F("Mesure Flex Sensor :"));}
  else if(choix==4){ecranOLED.println(F("Calibration en cours"));}
  ecranOLED.setTextSize(3);                   // Ecris en gros
  ecranOLED.setCursor(0, 30); // Ecris à gauche de l'écran
  ecranOLED.print(ResASCII); // Envoyer sur l'écran la valeur
  mySerial.write(ResASCII); // Envoyer sur le port bluetooth la valeur acquise
  Serial.println(ResASCII); // Afficher sur le port série la valeur de la tension mesuré en bytes
  ecranOLED.display();                            // Transfert le buffer à l'écran
  delay(50);

}

//Calibration basé sur un principe de dichotimie, voir internet. Essaie de se placer au centre de la plage des 1024 valeurs
int Calibration(){
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(1);                   // Taille des caractères (1:1, puis 2:1, puis 3:1)
  ecranOLED.setCursor(0, 0);
  ecranOLED.setTextColor(SSD1306_WHITE, SSD1306_BLACK);   // Couleur du texte, et couleur du fond
  ecranOLED.println(F("Calibration en cours"));
  ecranOLED.display();    
  delay(2000);   
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
    PotPos = (a + b)/2; // Prend le milieu  des 2 valeurs précédentes
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
    DisplayAndTransmitter(PotPos,4);
    if(Vadc > V_Cible){ // on change le minimum ou le maximum
      a = PotPos;}
    else{
      b = PotPos;}
    if(ite>12){break;}
  }
  Serial.print(F("Finie. Pota en "));
  Serial.println(PotPos); // Afficher sur le port série la valeur de la tension mesuré en bytes
  return PotPos;

}


