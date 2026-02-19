/*
  This is code for an Arduino UNO R4 WiFi using a Grove Kit V3.
  Allows user to set desired temperature of drink in a range of 0-100 and displays it on a 16x2 LCD screen.

  created 17 February 2026
  by Mariusz Matczak
  modified 19 February 2026
  by Kritika Saini

  Find the full Smart Coaster documentation here:
  https://github.com/mariusz-tm/arduino-smart-coaster
 */

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// Set variables for temperature dial (potentiometer)
const int potentiometerPin = A0;
int potentiometerValue = 0;

// Set variable for reset pad (touch sensor)
const int touchPin = 2;

// Set variables for temperature
int temperatureValue = 0;
int lastTemperatureValue = -999;

// Set variables for time required to lock, time check and lock state
const int lockDelay = 3000;
unsigned long lastChangeTime = 0;
bool locked = false;

// Set variables for buzzer
bool hasBuzzed = false;
const int buzzerPin = 5;

void resetState() {
  // Resets all variables to default states
  potentiometerValue = 0;
  temperatureValue = 0;
  lastTemperatureValue = -999;
  lastChangeTime = millis();
  locked = false;

  // Resets lcd screen to initial display
  lcd.clear();
  lcd.setCursor(6, 0); 
  lcd.print("Welcome"); // #####Welcome#####
  lcd.setCursor(2, 1); 
  lcd.print("Press to start"); // ##Push#to#start##
 
  // Waits until user presses reset pad (touch sensor) before moving onto main loop
  int touchState = digitalRead(touchPin);
  while (touchState != 1)
  {
    touchState = digitalRead(touchPin);
  }
  lcd.clear();
}

void setup() {
  // Initializing touch sensor as input for the resetState
  pinMode(touchPin, INPUT); 

  // Initializing buzzer as output
  pinMode(bunnerPin, OUTPUT);

  // Initializing lcd screen
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);

  // Welcome interaction requiring user input
  lcd.setCursor(6, 0); 
  lcd.print("Welcome"); // #####Welcome#####
  lcd.setCursor(2, 1); 
  lcd.print("Push to start"); // ##Push#to#start##
 
  // Waits until user presses reset pad (touch sensor) before moving onto main loop
  int touchState = digitalRead(touchPin);
  while (touchState != 1)
  {
    touchState = digitalRead(touchPin);
  }
  lcd.clear();

  // Initializing resetState function  
  resetState();
}

void loop() {
  // Activating resetState if touch value is 1 (is being pressed)
  int touchState = digitalRead(touchPin);
  if (touchState == 1)
  {
    delay(20);
    resetState();
  }

  if (!locked) 
  {
    // Takes the reading from potentiometer and maps it onto a 0-100 range for temperature
    potentiometerValue = analogRead(potentiometerPin);
    potentiometerValue = constrain(potentiometerValue, 0, 1023);
    temperatureValue = map(potentiometerValue, 0, 1013, 0, 100);

    // If-statement to check if dial is being turned
    if (temperatureValue != lastTemperatureValue) 
    {
      lastTemperatureValue = temperatureValue;
      lastChangeTime = millis();
    }
    // locks user input if dial has not been turned for lockDelay milliseconds
    else if (millis() - lastChangeTime >= lockDelay) 
    {
      locked = true;
    }

    // Printing display to show temperature set by user
    lcd.setCursor(0, 0);
    lcd.print("Temp Goal:");
    lcd.setCursor(10, 0);
    lcd.print(temperatureValue);
    lcd.setCursor(14, 0);
    lcd.print("°C");
  } 
  else 
  {
    lcd.setCursor(0, 0);
    lcd.print("Temp Goal: ");
    lcd.setCursor(10, 0);
    lcd.print(temperatureValue); // REQUIRES CHANGE TO TEMPERATURE SENSOR VALUE
    lcd.setCursor(14, 0);
    lcd.print("°C"); // Temp#Goal:000°C#

    // Sounds the buzzer when the temperature of the cup is the same as the desired temperature
    if (hasBuzzed = false && temperatureValue = temperatureValue) // REQUIRES CHANGE TO TEMPERATURE SENSOR VALUE
    {
      digitalWrite(buzzerPin, HIGH);
      delay(3000);
      digitalWrite(buzzerPin, LOW);
      hasBuzzer = true;
    }
  }
}
