#include <WiFi.h>
#include <WebServer.h>
#include "CamController.h"

#define AP_SSID  "Boeing"
#define AP_PASS  "bunrieugiotrungvitlon"

#define BTN_PIN   2          // momentary switch → GND
#define FLASH_PIN 4          // white LED

WebServer        server(80);
CamController    cam;
std::unique_ptr<esp32cam::Frame> lastFrame;

/* ---------- HTTP ---------- */
void sendFrame()
{
  if (!lastFrame) {
    server.send(503,"text/plain","no picture yet");
    return;
  }
  server.setContentLength(lastFrame->size());
  server.send(200,"image/jpeg");
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
  pinMode(BTN_PIN,   INPUT_PULLUP);
  pinMode(FLASH_PIN, OUTPUT);

  if (!cam.begin(1024,768,75)) {        // XGA + mid-quality = sharp w/ grain
    Serial.println("Cam init failed");  while(true) delay(1000);
  }

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  server.on("/image.jpg",   sendFrame);
  server.on("/capture.jpg", shootAndSend);
  server.begin();
}

void loop()
{
  server.handleClient();

  /* Button handling:
   * <0.8 s press  → take photo
   * ≥0.8 s press  → cycle mode (Colour → B&W → Vintage → …)
   */
  static uint32_t t_press = 0;
  static bool     held    = false;

  bool pressed = (digitalRead(BTN_PIN) == LOW);

  if (pressed && !held) {               // just went down
    t_press = millis();  held = true;
    digitalWrite(FLASH_PIN, HIGH);      // tiny half-press LED cue
  }
  if (!pressed && held) {               // just released
    uint32_t dt = millis() - t_press;
    digitalWrite(FLASH_PIN, LOW);

    if (dt < 800) {                     // short click → shutter
      lastFrame = cam.capture();
    } else {                            // long press → next mode
      cam.setMode( static_cast<CamController::Mode>((cam.mode()+1)%3) );
      Serial.printf("Mode → %u\n", cam.mode());
    }
    held = false;
  }
}
