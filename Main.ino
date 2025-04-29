#include <WiFi.h>
#include <WebServer.h>
#include "CamController.h"

#define AP_SSID  "Boeing"
#define AP_PASS  "bunrieugiotrungvitlon"

#define CAPTURE_TRIGGER_PIN 2  // External trigger for capture
#define MODE_TRIGGER_PIN    12 // External trigger for mode change
#define FLASH_PIN           4  // White LED

// Initialization
WebServer        server(80);
CamController    cam;
std::unique_ptr<esp32cam::Frame> lastFrame;

/* ---------- HTTP ---------- */
void sendFrame()
{
  if (!lastFrame) {
    server.send(503, "text/plain", "no picture yet");
    return;
  }
  server.setContentLength(lastFrame->size());
  server.send(200, "image/jpeg");
  lastFrame->writeTo(server.client());
}

void shootAndSend()
{
  lastFrame = cam.capture();
  sendFrame();
}

void setup()
{
  Serial.begin(115200);

  pinMode(CAPTURE_TRIGGER_PIN, INPUT_PULLDOWN);
  pinMode(MODE_TRIGGER_PIN, INPUT_PULLDOWN);
  pinMode(FLASH_PIN, OUTPUT);

  if (!cam.begin(1024, 768, 75)) {  // XGA + mid-quality = sharp w/ grain
    Serial.println("Camera init failed");
    while (true) delay(1000); // Halt forever
  }

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  server.on("/image.jpg", sendFrame);
  server.on("/capture.jpg", shootAndSend);
  server.begin();
}

void loop()
{
  server.handleClient();

  static bool captureTriggered = false;
  static bool modeTriggered = false;

  // --- Capture Handling ---
  bool captureSignal = (digitalRead(CAPTURE_TRIGGER_PIN) == HIGH);
  if (captureSignal && !captureTriggered) {
    Serial.println("Capture trigger received");
    digitalWrite(FLASH_PIN, HIGH); // Turn ON LED

    lastFrame = cam.capture();
    if (lastFrame) {
      Serial.println("Image captured!");
    } else {
      Serial.println("Failed to capture image.");
    }

    captureTriggered = true; // Prevent retrigger until signal goes LOW
  }
  if (!captureSignal) {
    captureTriggered = false;
    digitalWrite(FLASH_PIN, LOW); // Turn OFF LED
  }

  // --- Mode Change Handling ---
  bool modeSignal = (digitalRead(MODE_TRIGGER_PIN) == HIGH);
  if (modeSignal && !modeTriggered) {
    Serial.println("Mode change trigger received");

    // Switch camera mode: COLOUR -> BW -> VINTAGE -> COLOUR -> ...
    cam.setMode(static_cast<CamController::Mode>((cam.mode() + 1) % 3));
    Serial.printf("Mode switched to: %u\n", cam.mode());

    modeTriggered = true;
  }
  if (!modeSignal) {
    modeTriggered = false;
  }

  delay(10); // Short delay for stability
}

