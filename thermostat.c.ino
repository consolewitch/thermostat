/*
Thermostat control software for Arduino
Origional Author: Alexander N. Speaks
Contact: alex@sneaksneak.org, twitter: LogicalMethods

Methodology:

Define a set of default display types
1) current temp & target temp
2) graph _-~ variance of 12 degrees (2 degrees per step) over the last 12 hours.  bottom row left shows low, top row left shows high.
3) timer mode (timer mode definitions set in code for now - not configurable from display)

Set default temperature start points.

up/down buttons change target temperature

left/right buttons change display

select button selects between modes
1) dumb mode, stays near target temperature
2) timer mode




*/




#include <math.h>						// include math library
#include <LiquidCrystal.h> 				// include library for LCD
#include <time.h>

#define ThermistorPIN 1                 // Analog Pin 1


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int count = 0;						// counter
char lastLoop = 'z';
int currentTemp = 65;					// current temperature
int targetTemp = 65;					// set target temperature
int tempVariance = 2;					// how low does the temperature go before system kicks back on
int buttonVoltage = 0;					// stores the raw button voltage
int systemState = 0;					// variable to turn the system on or off
int waitTime = 5;					// how long we wait before going on
int relay = 2;                                          // relay controled by pin 

float pad = 9850;                       // balance/pad resistor value, set this to
float thermr = 10000;                   // thermistor nominal resistance



float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.

  Resistance=((1024 * pad / RawADC) - pad); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
  return Temp;                                      // Return the Temperature

}

void snark (){
  
}



void setup() {
lcd.begin(16, 2);						// set up the LCD's number of columns and rows
lcd.setCursor (0,0);					// Print a message to the LCD
lcd.print("Current Temp:");
lcd.setCursor (0,1);
lcd.print("Target Temp:");
pinMode(relay, OUTPUT);
digitalWrite(relay,0); //debug

//setTime(1387061244);

}

void loop() {

        //get current temperature
        currentTemp=Thermistor(analogRead(ThermistorPIN));
	currentTemp = (currentTemp * 9.0)/ 5.0 + 32.0;                  // converts to  Fahrenheit

        //turn on / off heat
        if (currentTemp < (targetTemp - tempVariance)){
          systemState=1;
          lcd.setCursor (15,1);
          lcd.print("|");
          digitalWrite(relay, systemState);
        }
        else{
          systemState=0;
          lcd.setCursor (15,1);
          lcd.print("o");
          digitalWrite(relay, systemState);
        }

        //check for input from buttons
	buttonVoltage = analogRead(0);
          if (buttonVoltage < 100) {
           //right button does nothing yet 
          }
          else if (buttonVoltage < 200 && lastLoop != 'u' && lastLoop != 'd') {
                  targetTemp++;
                  lastLoop='u';      // this is to keep the temperature from changing rapidly while the button is held down
    		//fUp();
          }
          else if (buttonVoltage < 400 && lastLoop != 'd' && lastLoop != 'u'){
                  targetTemp--;
                  lastLoop='d';      // this is to keep the temperature from changing rapidly while the button is held down
    		//fDown();
          }
          else if (buttonVoltage < 600){
            //left button does nothing yet
          }
          else if (buttonVoltage < 800){
/*            currentTime = now();
            lcd.clear();
            lcd.setCursor (1,0)
            lcd.print( hour(currentTime), ':', minute(currentTime));
            lastLoop='t' */
          }
          else{
            lastLoop = 'n';      // this is to keep the temperature from changing rapidly while the button is held down
          }
        lcd.setCursor (13,0);
        lcd.print(currentTemp);
        
	lcd.setCursor (12,1);
        lcd.print(targetTemp);
/*
	//GET COUNT
	if (delay >= count) {
		//GET TEMPERATURE
		if (targetTemp - tempVariance < currentTemp && offTime > minOffTime)
			systemState = 1;			// if it's too cold, turn on the heat
		else if (currentTemp >= targetTemp)
			systemState = 0;			// if it's hot enough or too hot, turn off the system.
		
		if (systemState = 0)
			offTime++;				// count how long the system has been off THIS NEEDS WORK		

		if (systemState = 1)
			runTime++;				// Count how long the system has been on THIS NEEDS WORK
	}
*/
	
}
/*

int fUp(int tempChange) {						// Change the target temperature up one degree
	tempChange++;	
	return tempChange;
}

int fDown(int tempChange) {						// Change the target temperature down one degree
	tempChange--;
	return tempChange;
}

int emulate(emulatedTemp) {
	count++								// timer
	emulatedTemp
}



*/
