/*
  This is code for an Arduino UNO R4 WiFi using a Grove Kit V3.
  Allows user to set desired temperature of drink in a range of 0-100 and displays it on a 16x2 LCD screen.

  created 17 February 2026
  by Mariusz Matczak
  modified 19 February 2026
  by Kritika Saini
  modified 25 February 2026
  by Salsabil Amer
  modified 2 March 2026
  by Eleonora Salaka

  Find the full Smart Coaster documentation here:
  https://github.com/mariusz-tm/arduino-smart-coaster
*/

#include <WiFiS3.h>           
#include "arduino_secrets.h"  // WiFi credentials
#include "webpage.h"
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

//  Set WiFi credientals loaded from arduino-secrets.h
char ssid[] = SECRET_SSID;    
char pass[] = SECRET_PASS;    

//
WiFiServer server(80);
bool isTurnedOff = false;

// Set variables for temperature dial (potentiometer)
const int potentiometerPin = A0;
int potentiometerValue = 0;

// Set variables for the temperature sensor
const int tempSensorPin = A1;
const int B = 3975;

// Set variable for reset pad (touch sensor)
const int touchPin = D2;

// Set variables for temperature
int temperatureValue = 0;
int lastTemperatureValue = -999;

// Set variables for time required to lock, time check and lock state
const int lockDelay = 3000;
unsigned long lastChangeTime = 0;
bool locked = false;

// Set variables for buzzer
bool hasBuzzed = false;
const int buzzerPin = A3;

bool waitingForStart = true;
bool lastTouch = false;

void resetState() {
  // Resets all variables to default states
  potentiometerValue = 0;
  temperatureValue = 0;
  lastTemperatureValue = -999;
  lastChangeTime = millis();
  locked = false;
  hasBuzzed = false;

  waitingForStart = true;

  // Resets lcd screen to initial display
  lcd.clear();
  lcd.setCursor(6, 0); 
  lcd.print("Welcome"); // #####Welcome#####
  lcd.setCursor(2, 1); 
  lcd.print("Press to start"); // ##Push#to#start##
}

void setup() {
  Serial.begin(9600);


  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("ERROR: WiFi module communication failed!");
    Serial.println("Check hardware connections.");
    while (true) {
      delay(1000);
    }
  }

  String fv = WiFi.firmwareVersion();
  Serial.print("WiFi firmware version: ");
  Serial.println(fv);

  int status = WL_IDLE_STATUS;  
  Serial.println("Connecting to WiFi...");
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting connection to SSID: ");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid, pass);
    
    delay(5000);
  }

  Serial.println("WiFi connected successfully!");

  server.begin();
  Serial.println("Web server started");

  IPAddress ip = WiFi.localIP();  
  Serial.print("Server IP address: ");
  Serial.println(ip);
  Serial.print("Open browser: http://");
  Serial.println(ip);
  Serial.println("=====================================");

  // Initializing touch sensor as input for the resetState
  pinMode(touchPin, INPUT); 

  // Initializing buzzer as output
  pinMode(buzzerPin, OUTPUT);

  // Initializing lcd screen
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);

  // Welcome interaction requiring user input
  lcd.setCursor(6, 0); 
  lcd.print("Welcome"); // #####Welcome#####
  lcd.setCursor(2, 1); 
  lcd.print("Push to start"); // ##Push#to#start##

  potentiometerValue = 0;
  temperatureValue = 0;
  lastTemperatureValue = -999;
  lastChangeTime = millis();
  locked = false;
  hasBuzzed = false;
  waitingForStart = true;
  lastTouch = false;
  isTurnedOff = false;
}

void loop() {
  bool currentTouch = (digitalRead(touchPin) == HIGH);
  bool touchPressedNow = currentTouch && !lastTouch;
  lastTouch = currentTouch;

  if (waitingForStart) {
    if (touchPressedNow) {
      waitingForStart = false;
      lcd.clear();
    }
    return;
  }

  if (touchPressedNow) {
    resetState();
    return;
  }

  if (!locked) 
  {
    // Takes the reading from potentiometer and maps it onto a 0-99 range for temperature
    potentiometerValue = analogRead(potentiometerPin);
    potentiometerValue = constrain(potentiometerValue, 0, 1023);
    temperatureValue = map(potentiometerValue, 0, 1013, 0, 99);

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
    lcd.setCursor(12, 0);
    lcd.print("C"); // Temp#Goal:00C###
  } 
  else 
  {
    // Takes value of temperature sensor and converts it to Celsius value (SEE README.md)
    int tempSensorValue = analogRead(tempSensorPin);
    float thermistorResistance = (float)(1023-tempSensorValue)*10000/tempSensorValue;
    float sensorTemperature = 1/(log(thermistorResistance/10000)/B+1/298.15)-273.15;

    WiFiClient client = server.available();

    String currentLine = "";      
    String firstLine = "";        
    bool isPostFlag = false;      

    while (client.connected()) {
    if (client.available()) {
      char c = client.read();   

      if (c == '\n') {          
        if (firstLine.length() == 0 && currentLine.length() > 0) {
          firstLine = currentLine;  

          if (firstLine.startsWith("POST /flag")) {
            isPostFlag = true;
            Serial.println("Detected POST /flag request (button pressed)");
          }
        }

        if (currentLine.length() == 0) {
          if (isPostFlag) {
            isTurnedOff = true;
            Serial.print("isTurnedOff changed to: ");
            Serial.println(isTurnedOff ? "true" : "false");
            sendFlagResponse(client);
            
          } else if (firstLine.startsWith("GET /data")) {
            sendJsonData(client);
            
          } else {
            sendHtmlPage(client);
          }
          break;
        } 
        else {
          currentLine = "";
        }
      } 
      else if (c != '\r') {   
        currentLine += c;
      }
    }
  }
  client.stop();

  lcd.setCursor(0, 1);
  lcd.print("Temp Now :");
  lcd.setCursor(10, 1);
  lcd.print(sensorTemperature);
  lcd.setCursor(14, 1);
  lcd.print("C"); // Temp#Now#:000#C#

  // Sounds the buzzer when the temperature of the cup is the same as the desired temperature
  if (hasBuzzed == false && temperatureValue >= sensorTemperature && isTurnedOff == false)
  {
    digitalWrite(buzzerPin, HIGH);
    delay(3000);
    digitalWrite(buzzerPin, LOW);
    hasBuzzed = true;
  }
}
}

void sendHtmlPage(WiFiClient &client) {
  Serial.println("sendHtmlPage called");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  
  client.println();                     
  
  client.print(MAIN_page);
}

// Sends data from dial, temperature sensor and is(Buzzer)TurnedOff to webpage
void sendJsonData(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  client.print("{\"temperatureValue\":");
  client.print(temperatureValue);
  client.print(",\"sensorTemperature\":");
  client.print(sensorTemperature);
  client.print(",\"isTurnedOff\":");
  client.print(isTurnedOff ? "true" : "false");  
  client.println("}");
}

void sendFlagResponse(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("OK");  
}
