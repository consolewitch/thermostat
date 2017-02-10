/*
Thermostat control software for Arduino
Origional Author: Alexander N. Speaks
Contact: alex@sneaksneak.org, twitter: LogicalMethods
*/


#include <math.h>                 // include math library
#include <LiquidCrystal.h>        // include library for LCD
#include <Time.h>                 // include time library
#include <TimeLib.h>              // include time library


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//initialize variables that need to persist from one loop to the next
char lastLoop = 'z';          // initialize lastLoop variable
int lastCurrentTemp = 0;      // initialize lastCurrentTemp
int currentTemp = 65;         // current temperature
int targetTemp = 60;          // set target temperature
int tempVariance = 2;         // how low does the temperature go before system kicks back on
boolean updateDisp = false;   // bool to specify if we're updating the screen this loop
int systemState = 0;          // variable to turn the system on or off
int waitTime = 5;             // how long we wait before going on
long snarkLastMill = 0;                 // The last (or first) time snark was triggered
int snarkChance = 5;                    // Likelyhood of snark being triggered in percent (10 = 10%)
long snarkTimeout = 5000000;             // Number of mills to wait before checking snark again
int stateChangeTime = 0;      // store the last time the state changed
char displayMode = 's';       // store the current display mode z = none, s = set time, d = default screen
char lastDisplayMode = 'd';   // store the display mode setting from the previous loop
int setDatePointer = 0;    // the pointer that tells us what we're adjusting in set date mode (0)year (1)month (2)day (3)hour (4)min
int hr = hour();
int mn = minute();
int sc = second();
int dy = day();
int mo = month();
int yr = year();


//Constants
int relay = 2;                // relay controled by pin 2 (which is labeled 3 on the digital pin side)
int ThermistorPIN = 1;        // Thermsistor input on analog pin 1
float pad = 9850;             // balance/pad resistor value, set this to
float thermr = 10000;         // thermistor nominal resistance
int minStateTime = 300;       // heater must stay in one state for at least 5 minutes

int buttonVoltage = 0;        // stores the raw button voltage note(this doesn't need to be a global var)


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
  lcd.clear();
  time_t t = now();
  //first line
  lcd.setCursor (0,0);
  lcd.print("Target:");
  lcd.setCursor (8,0);
  lcd.print(targetTemp);
  lcd.setCursor (11,0);
  lcd.print("     ");
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

void displaySetTime() {
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print(year());
  lcd.print("/");
  lcd.print(month());
  lcd.print("/");
  lcd.print(day());
  lcd.setCursor(0,1);
  lcd.print(hour());
  lcd.print(":");
  lcd.print(minute());
}

void fUp() {           // Change the target temperature up one degree
    switch (displayMode) {
    case 's': //set time mode
      Serial.print("date pointer: ");
      Serial.print(setDatePointer%5);
      Serial.print("\n");
      switch(setDatePointer%5) {
        case 0:
          yr++;
          break;
        case 1:
          mo++;
          break;
        case 2:
          dy++;
          break;
        case 3:
          hr++;
          break;
        case 4:
          mn++;
          break;
      }
      Serial.print(hr);
      Serial.print(":");
      Serial.print(mn);
      Serial.print(" ");
      Serial.print(yr);
      Serial.print("/");
      Serial.print(mo);
      Serial.print("/");
      Serial.print(dy);
      Serial.print("\n");
      setTime(hr, mn, sc, dy, mo, yr);
      break;
    case 'd':
      targetTemp++;
      break;
    default: 
      Serial.print("fup case statement failedover to the default. displaymode var: ");
      Serial.print(displayMode); 
      Serial.print("\n");
      ;//do nothing (this should never be called)
  }
}

void fDown() {           // Change the target temperature down one degree
    switch (displayMode) {
    case 's':
      switch(setDatePointer%5) {
        case 0:
          yr--;
          break;
        case 1:
          mo--;
          break;
        case 2:
          dy--;
          break;
        case 3:
          hr--;
          break;
        case 4:
          mn--;
          break;
      }
      Serial.print(hr);
      Serial.print(":");
      Serial.print(mn);
      Serial.print(" ");
      Serial.print(yr);
      Serial.print("/");
      Serial.print(mo);
      Serial.print("/");
      Serial.print(dy);
      Serial.print("\n");
      setTime(hr, mn, sc, dy, mo, yr);
      break;
    case 'd':
      targetTemp--;
      break;
    default: 
      Serial.print("fdown case statement failedover to the default. displaymode var: ");
      Serial.print(displayMode); 
      Serial.print("\n");
      ;//do nothing (this should never be called)
  }
}

void fLeft() {            // Do something on left button
    switch (displayMode) {
    case 's':
      if (setDatePointer > 0) {
          setDatePointer--;
      }
      break;
    case 'd':
      ;
      break;
    default:
      ;//do nothing (this should never be called)
  }
}

void fRight() {           // do something on right button
    switch (displayMode) {
    case 's':
      if (setDatePointer < 4) {
          setDatePointer++;
      }
      break;
    case 'd':
      ;//right doesn't do anything in display mode
      break;
    default:
      ;//do nothing (this should never be called)
  }
}

void fSelect(){
    switch (displayMode) {
    case 's':
      displayMode='d'; //switch to default mode
      break;
    case 'd':
      displayMode='s'; //switch to date set mode
      break;
    default:
      Serial.print("fselect case statement failedover to the default. displaymode var: ");
      Serial.print(displayMode); 
      Serial.print("\n");
      ;//do nothing (this should never be called)
  }
}

int timeShift() {        // shift the temperature during certain times of day
}

/*
//Averages analog reads with a definable count and delay
//Example: Count = 100, DelayMills = 10 would read 100 times over 1000ms (1 second)
float avgAnalogRead(int InputPin, int Count, int DelayMills){
  float total = 0;
  
  for(int i = 0; i < Count; i++){
    total += analogRead(InputPin);
    delay(DelayMills);
  }
  
  return total / Count;
}
*/

void setup() {
  lcd.begin(16, 2);                                 // set up the LCD's number of columns and rows
  pinMode(relay, OUTPUT);
  pinMode(ThermistorPIN, INPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  setTime(1419204900-28800);   // set to current unix epoch time in seconds compensating for pst
  stateChangeTime=now();
  Serial.print("StateChangeTime:");
  Serial.print(stateChangeTime);
  Serial.print("\n");
}

boolean snark () {
  //every now and then this function gives you some snark and the user has to choose to cancel or continue
  
  if((snarkLastMill + snarkTimeout) < millis()){
    //Time to check for snark

    if(random(100) < snarkChance){
      //yes, it is snark time
      int snarkChoice = random(10); //total choices + 1
      
      //default language - response should be 16 or less chars, and confim+cancel < 16 chars
      String snarkResponse = " ";
      String snarkConfirm = "Yeah";
      String snarkCancel = "Forget it";
      String snarkConfirmed = "Confirmed";
      String snarkCanceled = "Cancelled";
      
      if(snarkChoice == 0){
          snarkResponse = "You sure?";
      } else if(snarkChoice == 1){
          snarkResponse = "Error: Success!";
          snarkConfirm = "Error";
    snarkCancel = "Success";
      } else if(snarkChoice == 2){
          snarkResponse = "Base are belong!";
    snarkConfirm = "Destroy";
    snarkCancel = "You say!";
      } else if(snarkChoice == 3){
          snarkResponse = "Gorblax commands";
    snarkConfirm = "It is";
    snarkCancel = "Destroy";     
      } else if(snarkChoice == 4){
          snarkResponse = "The cost man!";
    snarkConfirm = "Im rich";
    snarkCancel = "Im poor";
          snarkConfirmed = "Yeah, sure.";
      } else if(snarkChoice == 5){
          snarkResponse = "T' power Capt'n!";
    snarkConfirm = "Do it!";
    snarkCancel = "Klingons.";
          snarkConfirmed = "Eye eye captain!";
          snarkCanceled = "Damn skippy.";
      } else if(snarkChoice == 6){
          snarkResponse = "Its opposite day";
    snarkConfirm = "Yes";
    snarkCancel = "No";
      } else if(snarkChoice == 7){
          snarkResponse = "Its opposite day";
    snarkConfirm = "No";
    snarkCancel = "Yes";
      } else if(snarkChoice == 8){
         snarkResponse = "I frown on this";
          snarkConfirmed = "Fine.";
          snarkCanceled = "Good.";
      } else if(snarkChoice == 9){
         snarkResponse = "Fuck!";          
      } //you can add more snark here

      //print snark to console
      lcd.clear();
      lcd.setCursor (0,0);
      lcd.print(snarkResponse);
      lcd.setCursor (1,0);
      lcd.print(snarkConfirm);
      lcd.setCursor(1, 16 - snarkCancel.length());
      lcd.print(snarkCancel);
      
      //jp note: this is to read the response of the user (confirm or cancel), not sure if it works or not
      for(int i = 0; i < 60000; i++){
        //Wait for response for 60 seconds
    buttonVoltage = analogRead(0);
        
        if (buttonVoltage < 100) {
          //right button pressed - confirm and exit lop
      delay(1000); //wait for button release
          return true; 
        } else if ((buttonVoltage > 400) && (buttonVoltage < 600)){
          //left button pressed - cancel and exit loop
      delay(1000);
          return false;
        }
      
        delay(1);
      }
      
      //user did not respond, cancel
      return false;
      
    }

  }

  //remember the last time we checked for snark so that we don't do it too often
  snarkLastMill = millis();
}

char newButton(){
  //check for input from buttons
  buttonVoltage = analogRead(0);
  Serial.println(buttonVoltage);

  char thisLoop = 'x';
  
  if(buttonVoltage < 100){
    thisLoop = 'r';  
  } else if (buttonVoltage < 200){
    thisLoop = 'u';
  } else if (buttonVoltage < 400){
    thisLoop = 'd';    
  } else if (buttonVoltage < 600){
    thisLoop = 'l';
  } else if (buttonVoltage < 800){
    thisLoop = 's';
  }

  bool valueChanged = thisLoop != lastLoop;
  lastLoop = thisLoop;
  
  if(valueChanged){
    return thisLoop;    
  } else {
    return 'n';
  }
}

void loop() {
    char choice=newButton();      //keeps the button activity for this loop

  //get current temperature
  currentTemp=Thermistor(analogRead(ThermistorPIN));

  if (now() < 10000) {
    displayMode='s'; 
  } 

  if (choice == 'r') { //right button
    fRight(); 
    updateDisp=true;
  }
  else if (choice == 'u') { //up button
    fUp();
    updateDisp=true;
  }
  else if (choice == 'd'){ //down button
    fDown(); 
    updateDisp=true;
  }
  else if (choice == 'l'){ //left button
    fLeft();
    updateDisp=true;
  }
  else if (choice == 's'){ //select button
    fSelect(); 
    updateDisp=true;
  }

  if (now()-stateChangeTime >= minStateTime){
    //Serial.print("now: ");
    //Serial.print(now());
    //Serial.print("\n");
    //Serial.print("stateChangeTime: ");
    //Serial.print(stateChangeTime);
    //Serial.print("\n");
    if (currentTemp < (targetTemp - tempVariance)){
      systemState=1;                                // its too cold, turn on the heat.
      digitalWrite(relay, !systemState);            // using ! (aka:not) because of the design of our pre-built relay
      stateChangeTime=now();
    }
    else{
      systemState=0;                                // its too warm, turn off the heat.
      digitalWrite(relay, !systemState);            // using ! (aka:not) because of the design of our pre-built relay
      stateChangeTime=now();
    }
  }
  
  if (updateDisp || (lastCurrentTemp != currentTemp)){
    if (displayMode == 's'){
      displaySetTime();
    }
    else if (displayMode == 'd'){
      displayDefault(currentTemp,targetTemp,systemState);
    }
    updateDisp=false;
  }
  lastCurrentTemp=currentTemp;
  lastDisplayMode=displayMode;
} //end loop function



/*
  
  if (inStateTime >= minStateTime) {
    //GET TEMPERATURE
    if (targetTemp - tempVariance < currentTemp && minStateTime > inStateTime)
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
  
