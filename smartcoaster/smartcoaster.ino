/*
  This is code for an Arduino UNO R4 using a Grove Kit V3.
  Allows user to set desired temperature of drink in a range of 0-100 and displays it on a 16x2 LCD screen.

  created 17 February 2026
  by Mariusz Matczak
  modified 19 February 2026
  by Kritika Saini
  modified 25 February 2026
  by Salsabil Amer
  modified 21 April 2026
  by Mariusz Matczak

  Find the full Smart Coaster documentation here:
  https://github.com/mariusz-tm/arduino-smart-coaster
*/

#include "arduino_secrets.h"

#include <WiFiS3.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include "wifi-form.h"

#include <BlynkSimpleWifi.h>
#include <math.h>

rgb_lcd lcd;

// WiFi credentials of coaster's hotspot
char hotspotSSID[] = "Coaster-Setup";
char hotspotPASS[] = "";

WiFiServer server(80);

String newSSID = "";
String newPASS = "";

bool wifiReady = false;
bool blynkReady = false;

// Google Sheets
WiFiSSLClient sheetClient;
bool sheetSent = false;

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

// Buzzer disable via Blynk
bool buzzerDisabled = false;

// ================= FUNCTION DECLARATIONS (FIX COMPILER ERROR) =================
void parseCredentials(String req);
void sendFormPage(WiFiClient client);

// ================= BLYNK CONTROL =================
BLYNK_WRITE(V6)
{
  buzzerDisabled = param.asInt();
}

// ================= WIFI PARSE =================
void parseCredentials(String req)
{
  int ssidIndex = req.indexOf("ssid=");
  int passIndex = req.indexOf("pass=");

  if (ssidIndex > 0 && passIndex > 0) {
    newSSID = req.substring(ssidIndex + 5, req.indexOf('&', ssidIndex));
    newPASS = req.substring(passIndex + 5, req.indexOf(' ', passIndex));

    newSSID.replace("%20", " ");
    newPASS.replace("%20", " ");
  }
}

// ================= HOTSPOT =================
void startHotspot()
{
  WiFi.beginAP(hotspotSSID, hotspotPASS);

  server.begin();

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Connect to");
  lcd.setCursor(1, 1);
  lcd.print("Coaster-Setup");

  delay(10000); // short pause before showing IP

  // New screen with IP
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Connect to");
  lcd.setCursor(2, 1);
  lcd.print(WiFi.localIP());
}

// ================= WIFI TEST =================
bool testConnection()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Connecting...");

  WiFi.end();

  int status = WiFi.begin(newSSID.c_str(), newPASS.c_str());

  int retries = 0;
  while (status != WL_CONNECTED && retries < 10) {
    delay(1000);
    status = WiFi.status();
    retries++;
  }

  if (status == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Connected!");
    delay(1000);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("No Connection");

    delay(3000);

    WiFi.disconnect();
    startHotspot();
    return false;
  }
}

// ================= GOOGLE SHEETS =================
void sendToSheet(float startTemp)
{
  if (sheetClient.connect("script.google.com", 443)) {

    String url = String(GOOGLE_SCRIPT_URL) +
                 "?IDtag=" + String(temperatureValue) +
                 "&TimeStamp=" + String(millis()) +
                 "&TempC=" + String(startTemp);

    sheetClient.println("GET " + url + " HTTP/1.1");
    sheetClient.println("Host: script.google.com");
    sheetClient.println("Connection: close");
    sheetClient.println();

    sheetClient.stop();
  }
}

// ================= BLYNK DATA =================
void sendBlynkData()
{
  int tempSensorValue = analogRead(tempSensorPin);

  float thermistorResistance =
    (float)(1023 - tempSensorValue) * 10000 / tempSensorValue;

  float sensorTemperature =
    1 / (log(thermistorResistance / 10000) / B + 1 / 298.15) - 273.15;

  Blynk.virtualWrite(V4, sensorTemperature);
  Blynk.virtualWrite(V5, temperatureValue);
}

// ================= CLIENT HANDLER =================
void handleClient()
{
  WiFiClient client = server.available();
  if (!client) return;

  String request = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      request += c;

      if (c == '\n') {

        if (request.indexOf("GET /save?") >= 0) {

          parseCredentials(request);

          if (testConnection()) {

            WiFi.end();

            wifiReady = true;
          }
        } else {
          sendFormPage(client);
        }

        break;
      }
    }
  }

  client.stop();
}

// ================= WEB PAGE =================
void sendFormPage(WiFiClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.print(WIFI_FORM);
}

// ================= SETUP =================
void setup()
{
  Serial.begin(9600);

  pinMode(touchPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);

  startHotspot();
}

// ================= LOOP =================
void loop()
{
  if (!wifiReady) {
    handleClient();
    return;
  }

  if (!blynkReady) {
    Blynk.begin(BLYNK_AUTH_TOKEN, newSSID.c_str(), newPASS.c_str());
    blynkReady = true;
  }

  Blynk.run();

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
    waitingForStart = true;
    lcd.clear();
    return;
  }

  if (!locked) {
    potentiometerValue = analogRead(potentiometerPin);
    potentiometerValue = constrain(potentiometerValue, 0, 1023);
    temperatureValue = map(potentiometerValue, 0, 1023, 0, 99);

    if (temperatureValue != lastTemperatureValue) {
      lastTemperatureValue = temperatureValue;
      lastChangeTime = millis();
    }
    else if (millis() - lastChangeTime >= lockDelay) {
      locked = true;
    }

    lcd.setCursor(0, 0);
    lcd.print("Temp Goal:");
    lcd.setCursor(10, 0);
    lcd.print(temperatureValue);
    lcd.setCursor(12, 0);
    lcd.print("C");
  }
  else {
    int tempSensorValue = analogRead(tempSensorPin);

    float thermistorResistance =
      (float)(1023 - tempSensorValue) * 10000 / tempSensorValue;

    float sensorTemperature =
      1 / (log(thermistorResistance / 10000) / B + 1 / 298.15) - 273.15;

    lcd.setCursor(0, 1);
    lcd.print("Temp Now :");
    lcd.setCursor(10, 1);
    lcd.print(sensorTemperature);
    lcd.setCursor(14, 1);
    lcd.print("C");

    if (!sheetSent) {
      sendToSheet(sensorTemperature);
      sheetSent = true;
    }

    if (!buzzerDisabled && hasBuzzed == false && temperatureValue >= sensorTemperature) {
      digitalWrite(buzzerPin, HIGH);
      delay(3000);
      digitalWrite(buzzerPin, LOW);
      hasBuzzed = true;
    }
  }
}