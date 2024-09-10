#include "esp_timer.h"
#include "driver/gpio.h"


const uint8_t buttonPins[] = {4, 16, 12, 14};
const uint8_t ledPins[] = {0, 17, 13, 27};
const uint8_t buzzerPin = 18;
const int POSITIONS_QUANTITY = 4;
int buttonPressed = -1;
int currentPos = 0;
// This is the time player has to punch the chosen position
int waitTime = 2;

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

void blink() {
  currentPos = randomPosition();
  digitalWrite(ledPins[currentPos], HIGH);
}

void resetLeds() {
  for (int i = 0; i < POSITIONS_QUANTITY; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  attachInterrupt(buttonPins[0], pushButton0, RISING);
  attachInterrupt(buttonPins[1], pushButton1, RISING);
  attachInterrupt(buttonPins[2], pushButton2, RISING);
  attachInterrupt(buttonPins[3], pushButton3, RISING);
}

void loop() {
  blink();
  delay(waitTime * 1000);
  resetLeds();
  if(buttonPressed == currentPos){
    Serial.print("Correct");
    tone(buzzerPin, 1000, waitTime * 100);
  }else{
    Serial.print("Incorrect");
    tone(buzzerPin, 250, waitTime * 100);
  }
  buttonPressed = -1;
}
