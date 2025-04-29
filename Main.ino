/*------------------------------------Version 1--------------------------------*/
/* Capture image, switch mode by button press only on ESP CAM */
// #include <WiFi.h>
// #include <WebServer.h>
// #include "CamController.h"

// #define AP_SSID  "Boeing"
// #define AP_PASS  "bunrieugiotrungvitlon"

// #define BTN_PIN   2          // momentary switch → GND
// #define FLASH_PIN 4          // white LED

// //Initialization
// WebServer        server(80);
// CamController    cam;
// std::unique_ptr<esp32cam::Frame> lastFrame;

// /* ---------- HTTP ---------- */
// void sendFrame()
// {
//   if (!lastFrame) {
//     server.send(503,"text/plain","no picture yet");
//     return;
//   }
//   server.setContentLength(lastFrame->size());
//   server.send(200,"image/jpeg");
//   lastFrame->writeTo(server.client());
// }

// void shootAndSend()
// {
//   lastFrame = cam.capture();
//   sendFrame();
// }

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(BTN_PIN,   INPUT_PULLUP);
//   pinMode(FLASH_PIN, OUTPUT);

//   if (!cam.begin(1024,768,75)) {        // XGA + mid-quality = sharp w/ grain
//     Serial.println("Cam init failed");  
//     while(true) delay(1000);
//   }

//   WiFi.softAP(AP_SSID, AP_PASS);
//   Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

//   server.on("/image.jpg",   sendFrame);
//   server.on("/capture.jpg", shootAndSend);
//   server.begin();
// }

// void loop()
// {
//   server.handleClient();

//   /* Button handling:
//    * <1.5 s press  → take photo
//    * ≥1.5 s press  → cycle mode (Colour → B&W → Vintage → …)
//    */
//   static uint32_t t_press = 0;
//   static bool     held    = false;

//   bool pressed = (digitalRead(BTN_PIN) == LOW);

//   if (pressed && !held) {               // just went down
//     t_press = millis();  held = true;
//     digitalWrite(FLASH_PIN, HIGH);      // tiny half-press LED cue
//   }
//   if (!pressed && held) {               // just released
//     uint32_t dt = millis() - t_press;
//     digitalWrite(FLASH_PIN, LOW);

//     if (dt < 1500) {                     // short click → shutter
//       lastFrame = cam.capture();
//     } else {                            // long press → next mode
//       cam.setMode( static_cast<CamController::Mode>((cam.mode()+1)%3) );
//       Serial.printf("Mode → %u\n", cam.mode());
//     }
//     held = false;
//   }
// }








/*------------------------------------Version 2--------------------------------*/
/* Capture image, switch mode by receiving external signals on GPIO */

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

  // if (!cam.begin(1024, 768, 75)) {  // XGA + mid-quality = sharp w/ grain
  //   Serial.println("Camera init failed");
  //   while (true) delay(1000); // Halt forever
  // }

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

