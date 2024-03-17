int voltPin = 0;
float voltage[500];
float moyenne=0;
float volti=0;



void setup()
{
  Serial.begin(9600);  //Start the serial connection with the copmuter
}

void loop()                     // run over and over again
{
  volti=analogRead(voltPin);
  Serial.println(volti);
  delay(200);                       
  // for (int i = 0; i < 50; i++) {
  //   //voltage[i] = analogRead(temperaturePin);
  //   moyenne=moyenne+analogRead(voltPin);
  //   delay(50);}   
  
  // Serial.println(moyenne/50);
  // moyenne=0;                     //printing the result
                                  //waiting a second
}