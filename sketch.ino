const uint8_t buttonPins[] = {4, 16, 12, 14};
const uint8_t ledPins[] = {0,17,13,27};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

int readButtons() {
  while (true) {
    for (int i = 0; i < 4; i++) {
      int buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}

void loop() {
  readButtons();
}
