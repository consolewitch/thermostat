/*
Thermostat control software for Arduino
Origional Author: Alexander N. Speaks
Contact: alex@sneaksneak.org, twitter: LogicalMethods
*/


#include <math.h>                 // include math library
#include <LiquidCrystal.h>        // include library for LCD
#include <Time.h>                 // include time library


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int count = 0;                // counter
char lastLoop = 'z';          // initialize lastLoop variable
int lastCurrentTemp = 0;      // initialize lastCurrentTemp
int currentTemp = 65;         // current temperature
int targetTemp = 60;          // set target temperature
int tempVariance = 2;         // how low does the temperature go before system kicks back on
int buttonVoltage = 0;        // stores the raw button voltage
int systemState = 0;          // variable to turn the system on or off
int waitTime = 5;             // how long we wait before going on
int relay = 2;                // relay controled by pin 2 (which is labeled 3 on the digital pin side)
int ThermistorPIN = 1;        // Thermsistor input on analog pin 1
float pad = 9850;             // balance/pad resistor value, set this to
float thermr = 10000;         // thermistor nominal resistance
boolean updateDisp = false;   // bool to specify if we're updating the screen this loop
int stateChangeTime = 0;
int inStateTime = 0;
int minStateTime = 0;


float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;                                       // Dual-Purpose variable to save space.
  Resistance=((1024 * pad / RawADC) - pad); 
  Temp = log(Resistance);                           // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;                             // Convert Kelvin to Celsius
  Temp = (Temp * 9.0)/ 5.0 + 32.0;                  // converts to  Fahrenheit
                      
  return Temp;                                      // Return the Temperatur
}

void displayDefault(int currentTemp, int targetTemp, boolean systemState) {
  time_t t = now();
  //first line
  lcd.setCursor (0,0);
  lcd.print("Target:");
  lcd.setCursor (8,0);
  lcd.print(targetTemp);
  lcd.setCursor (11,0);
  lcd.print('     ');
  lcd.setCursor (11,0);
  lcd.print(hour(t));
  lcd.print (':');
  lcd.print (minute(t));
  //second line
  lcd.setCursor (0,1);                              // Print a message to the LCD
  lcd.print("Current:");
  lcd.setCursor (9,1);
  lcd.print("   ");
  lcd.setCursor (9,1);
  lcd.print(currentTemp);
  if (systemState) {
    lcd.setCursor (15,1);
    lcd.print("|");
  }
  else {
    lcd.setCursor (15,1);
    lcd.print("0"); 
  }
  
}

/*
int displaySetTime(int needs_work) { // Display the time-setting system
  int currentTime = now();
  lcd.clear();
  lcd.setCursor (1,0);
  lcd.print( hour(currentTime), ':', minute(currentTime));
  lastLoop='t' ;
} 
*/
int fUp(int tempChange) {           // Change the target temperature up one degree
  tempChange++; 
  return tempChange;
}

int fDown(int tempChange) {           // Change the target temperature down one degree
  tempChange--;
  return tempChange;
}

void fLeft() {            // Do something on left button
  adjustTime(+3600);
}

void fRight() {           // do something on right button
  adjustTime(+60);
}

void timeShift() {        // shift the temperature during certain times of day

}

void setup() {
  lcd.begin(16, 2);                                 // set up the LCD's number of columns and rows
  pinMode(relay, OUTPUT);
  pinMode(ThermistorPIN, INPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}


void loop() {
  //get current temperature
  currentTemp=Thermistor(analogRead(ThermistorPIN));

  //turn on or off heat
  if (currentTemp < (targetTemp - tempVariance)){
    systemState=1;
    digitalWrite(relay, !systemState);            // using ! (aka:not) because of the design of our pre-built relay
  }
  else{
    systemState=0;
    digitalWrite(relay, !systemState);            // using ! (aka:not) because of the design of our pre-built relay
  }

  //check for input from buttons
  buttonVoltage = analogRead(0);
  if (buttonVoltage < 100 && lastLoop != 'u' && lastLoop != 'd' && lastLoop != 'l' && lastLoop != 'r') { //right button
    fRight(); //right button
    //Serial.print("R");
    lastLoop = 'r';
    updateDisp=true;
  }
  else if (buttonVoltage < 200 && lastLoop != 'u' && lastLoop != 'd' && lastLoop != 'l' && lastLoop != 'r') { //up button
    targetTemp++;
    //Serial.print("U");
    lastLoop='u'; // this is to keep the temperature from changing rapidly while the button is held down
    updateDisp=true;
    //fUp();lastLoop
  }
  else if (buttonVoltage < 400 && lastLoop != 'd' && lastLoop != 'u' && lastLoop != 'l' && lastLoop != 'r'){ //down button
    targetTemp--;
    //Serial.print("D");
    lastLoop='d';      // this is to keep the temperature from changing rapidly while the button is held down
    updateDisp=true;
    //fDown();
  }
  else if (buttonVoltage < 600 && lastLoop != 'd' && lastLoop != 'u' && lastLoop != 'r' && lastLoop != 'l'){ //left button
    fLeft(); //left button
    //Serial.print("L");
    lastLoop = 'l';
    updateDisp=true;
  }
  else if (buttonVoltage < 800 && lastLoop != 'd' && lastLoop != 'u' && lastLoop != 'l' && lastLoop != 'r'){ //select button
    //Serial.print("S");
    lastLoop = 's';
    updateDisp=true;
  }
  else if (buttonVoltage <1000) {
    //allow button press to escape without changing the lastLoop var.
  }
  else {
    lastLoop = 'n';      // this is to keep the temperature from changing rapidly while the button is held down
  }
  
  
  if (updateDisp || lastCurrentTemp != currentTemp){
    displayDefault(currentTemp,targetTemp,systemState);
    updateDisp=false;
  }
  Serial.print(lastLoop);
  lastCurrentTemp=currentTemp;


/*  
  if (delay >= count) {
    //GET TEMPERATURE
    if (targetTemp - tempVariance < currentTemp && offTime > minOffTime)
      systemState = 1;      // if it's too cold, turn on the heat
      stateChangeTime = now();
    else if (currentTemp >= targetTemp)
      systemState = 0;      // if it's hot enough or too hot, turn off the system.
      stateChangeTime = now();
    
    if (systemState = 0)
      offTime++;        // count how long the system has been off THIS NEEDS WORK   

    if (systemState = 1)
      runTime++;        // Count how long the system has been on THIS NEEDS WORK
  }
*/
  
} //end loop function