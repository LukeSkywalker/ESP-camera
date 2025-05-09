#define BUTTON_PIN          D4  // GPIO6
#define CAPTURE_TRIGGER_PIN D0  // GPIO2
#define MODE_TRIGGER_PIN    D1  // GPIO3


void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active LOW
  pinMode(CAPTURE_TRIGGER_PIN, OUTPUT);
  pinMode(MODE_TRIGGER_PIN, OUTPUT);

  digitalWrite(CAPTURE_TRIGGER_PIN, LOW);
  digitalWrite(MODE_TRIGGER_PIN, LOW);
}

void loop() {
  static bool buttonHeld = false;
  static uint32_t pressStartTime = 0;

  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW); // Button active LOW

  if (buttonPressed && !buttonHeld) {
    // Button just pressed
    pressStartTime = millis();
    buttonHeld = true;
  }

  if (!buttonPressed && buttonHeld) {
    // Button just released
    uint32_t pressDuration = millis() - pressStartTime;
    Serial.print("Button held for: ");
    Serial.print(pressDuration);
    Serial.println(" ms");

    if (pressDuration < 1500) {
      // Short press → Capture
      Serial.println("Short press detected: Capture");
      digitalWrite(CAPTURE_TRIGGER_PIN, HIGH);
      delay(100); // Pulse HIGH for 100ms
      digitalWrite(CAPTURE_TRIGGER_PIN, LOW);
    } else {
      // Long press → Mode change
      Serial.println("Long press detected: Mode Change");
      digitalWrite(MODE_TRIGGER_PIN, HIGH);
      delay(100); // Pulse HIGH for 100ms
      digitalWrite(MODE_TRIGGER_PIN, LOW);
    }

    buttonHeld = false;
  }

  delay(10); // Small loop delay
}
