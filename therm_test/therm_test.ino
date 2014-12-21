#include <EEPROM.h>

#include <LiquidCrystal.h>        // include library for LCD
#include <Time.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int ThermistorPIN = 1;
float pad = 9850;                       // balance/pad resistor value, set this to
float thermr = 10000;                   // thermistor nominal resistance



void setup(){
  pinMode(ThermistorPIN,INPUT);
}

float Thermistor(int RawADC) {
  long Resistance;  
  float Temp;  // Dual-Purpose variable to save space.
  Resistance=((1024 * pad / RawADC) - pad); 
  Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
  return Temp;                                      // Return the Temperature

}


void loop(){  
  int currentTemp;
  currentTemp=Thermistor(analogRead(ThermistorPIN));
  lcd.setCursor (0,0);
  lcd.print(currentTemp);
  sleep(10);

}
