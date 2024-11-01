#include "esp_timer.h"
#include "driver/gpio.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <WiFi.h>

const uint8_t buttonPins[] = { 4, 16, 12, 14 };
const uint8_t ledPins[] = { 0, 17, 13, 27 };
const int POSITIONS_QUANTITY = 4;
int buttonPressed = -1;
int currentPos = 0;
// This is the time player has to punch the chosen position
int waitTime = 2;
bool usePattern = true;
int pattern[] = { 0, 1, 2, -1, -1, -1, -1, -1, -1, -1 };
int currentPatternPos = 0;
bool reloadingPattern = false;

const char *ssid = "Boxing game";
const char *password = "123456789";

AsyncWebServer server(80);

void IRAM_ATTR pushButton0() {
  buttonPressed = 0;
}

void IRAM_ATTR pushButton1() {
  buttonPressed = 1;
}

void IRAM_ATTR pushButton2() {
  buttonPressed = 2;
}

void IRAM_ATTR pushButton3() {
  buttonPressed = 3;
}

void decodePattern(String input) {
  int start = 1;
  int index = 0;

  if(input.length() == 2){
    usePattern = false;
  }else{
    usePattern = true;
  }

  Serial.println("Pattern:");
  for (int i = 1; i < input.length(); i++) {
    if (input[i] == ',' || i == (input.length() - 1)) {
      String numberStr = input.substring(start, i);
      Serial.println(numberStr);
      pattern[index] = numberStr.toInt();
      index++;
      start = i + 1;
    }
  }
}

// Return a random position for the user to push
int randomPosition() {
  int buf = 0;
  esp_fill_random(&buf, sizeof(buf));
  int pos = buf % POSITIONS_QUANTITY;
  if (pos < 0) {
    pos = pos * -1;
  }
  return (pos);
}

void blinkRandom() {
  currentPos = randomPosition();
  digitalWrite(ledPins[currentPos], HIGH);
}

void blink() {
  if (!reloadingPattern) {
    digitalWrite(ledPins[pattern[currentPatternPos]], HIGH);
    //Serial.println(pattern[currentPatternPos]);
    currentPos=pattern[currentPatternPos];
    currentPatternPos++;
    // -1 represents the end of the pattern too
    if (currentPatternPos >= (sizeof(pattern) / sizeof(int)) || pattern[currentPatternPos] == -1) {
      currentPatternPos = 0;
    }
  }
}

void resetLeds() {
  for (int i = 0; i < POSITIONS_QUANTITY; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  attachInterrupt(buttonPins[0], pushButton0, FALLING);
  attachInterrupt(buttonPins[1], pushButton1, FALLING);
  attachInterrupt(buttonPins[2], pushButton2, FALLING);
  attachInterrupt(buttonPins[3], pushButton3, FALLING);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on(
    "/combination", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      reloadingPattern = true;
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, data);

      if (error) {
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
      }

      String response = jsonDoc["value"].as<String>();
      decodePattern(response);

      request->send(200, "application/json", "{\"status\":\"success\"}");
      reloadingPattern = false;
    });

  server.on(
    "/speed", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, data);

      if (error) {
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
      }

      String response = jsonDoc["value"].as<String>();
      waitTime = response.toInt();
      // Values received come from 0 to 100
      waitTime = waitTime / 10;

      Serial.println(waitTime);
      request->send(200, "application/json", "{\"status\":\"success\"}");
    });

  server.begin();
}

void loop() {
  if (!usePattern) {
    blinkRandom();
    delay(waitTime * 1000);
    resetLeds();
  } else {
    blink();
    delay(waitTime * 1000);
    resetLeds();
  }
  Serial.println("Current Pos: ");
  Serial.println(currentPos);
  Serial.println("ButtonPressed");
  Serial.println(buttonPressed);
  if (buttonPressed == currentPos) {
    Serial.println("Correct");
  } else {
    Serial.println("Incorrect");
  }
  buttonPressed = -1;
}