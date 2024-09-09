#include "esp_timer.h"
#include "driver/gpio.h"


const uint8_t buttonPins[] = {4, 16, 12, 14};
const uint8_t ledPins[] = {0, 17, 13, 27};
const int POSITIONS_QUANTITY = 4;
int buttonPressed = 0;
int currentPos = 0;
// This is the time player has to punch the chosen position
int waitTime = 2;

bool outOfTime = false;

// Timer handle
esp_timer_handle_t timerHandle;

// This function is called when the player is out of time to tap de piezoelectric
void setOutOfTime(void* arg) {
  outOfTime = true;
  Serial.print("Out!");
}

void readButtons() {
  while (!outOfTime) {
    for (int i = 0; i < 4; i++) {
      int buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        if (i == currentPos) {
          Serial.print("Correct");
        } else {
          Serial.print("Incorrect");
        }
        return;
      }
    }
  }
}

// Return a random position for the user to push
int randomPosition() {
  int buf = 0;
  esp_fill_random(&buf, sizeof(buf));
  Serial.println(buf);
  int pos = buf % POSITIONS_QUANTITY;
  if (pos < 0) {
    pos = pos * -1;
  }
  Serial.println(pos);
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
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
    // Create and configure the timer
    const esp_timer_create_args_t timerArgs = {
        .callback = &setOutOfTime, // Callback function
        .name = "setOutOfTime"        // Timer name
    };

    // Create the timer
    esp_timer_create(&timerArgs, &timerHandle);

    // Start the timer with a 1-second interval (1000000 microseconds)
    esp_timer_start_periodic(timerHandle, 1000000); // 1,000,000 microseconds = 1 second
}

void loop() {
  blink();
  readButtons();
  outOfTime = false;
  resetLeds();
}