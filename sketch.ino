#include "esp_timer.h"
#include "driver/gpio.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>
#include <WiFi.h>

const uint8_t buttonPins[] = {4, 16, 12, 14};
const uint8_t ledPins[] = {0, 17, 13, 27};
const int POSITIONS_QUANTITY = 4;
int buttonPressed = -1;
int currentPos = 0;
// This is the time player has to punch the chosen position
int waitTime = 2;

const char *ssid = "Boxing game";
const char *password = "123456789";

AsyncWebServer server(80);

void IRAM_ATTR pushButton0()
{
  buttonPressed = 0;
}

void IRAM_ATTR pushButton1()
{
  buttonPressed = 1;
}

void IRAM_ATTR pushButton2()
{
  buttonPressed = 2;
}

void IRAM_ATTR pushButton3()
{
  buttonPressed = 3;
}

// Return a random position for the user to push
int randomPosition()
{
  int buf = 0;
  esp_fill_random(&buf, sizeof(buf));
  int pos = buf % POSITIONS_QUANTITY;
  if (pos < 0)
  {
    pos = pos * -1;
  }
  return (pos);
}

void blink()
{
  currentPos = randomPosition();
  digitalWrite(ledPins[currentPos], HIGH);
}

void resetLeds()
{
  for (int i = 0; i < POSITIONS_QUANTITY; i++)
  {
    digitalWrite(ledPins[i], LOW);
  }
}

void setup()
{
  Serial.begin(9600);
  for (int i = 0; i < 4; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  attachInterrupt(buttonPins[0], pushButton0, RISING);
  attachInterrupt(buttonPins[1], pushButton1, RISING);
  attachInterrupt(buttonPins[2], pushButton2, RISING);
  attachInterrupt(buttonPins[3], pushButton3, RISING);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on(
      "/combination", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, data);

      if (error) {
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
      }

      String response = "Received data:\n";
      response += jsonDoc["param1"].as<String>();
      response += "\n";
      response += jsonDoc["param2"].as<String>();

      Serial.println(response);
      request->send(200, "application/json", "{\"status\":\"success\"}"); });

  server.on(
      "/speed", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
      {
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, data);

      if (error) {
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
      }

      String response = "Received data:\n";
      response += jsonDoc["value"].as<String>();

      Serial.println(response);
      request->send(200, "application/json", "{\"status\":\"success\"}"); });

  server.begin();
}

void loop()
{
  blink();
  delay(waitTime * 1000);
  resetLeds();
  if (buttonPressed == currentPos)
  {
    Serial.print("Correct");
  }
  else
  {
    Serial.print("Incorrect");
  }
}