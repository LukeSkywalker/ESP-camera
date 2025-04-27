// #include <esp32cam.h>
// #include <WebServer.h>
// #include <WiFi.h>

// #define AP_SSID "Boeing"
// #define AP_PASS "bunrieugiotrungvitlon"

// WebServer server(80);
// unsigned long lastCaptureTime = 0;
// const unsigned long captureInterval = 5000; // 5 seconds

// void handleCapture() {
//   captureAndSendImage();
// }

// void setup() {
//   pinMode(4, OUTPUT);

//   // Camera config
//   auto res = esp32cam::Resolution::find(1024, 768); // Higher res: UXGA
//   esp32cam::Config cfg;
//   cfg.setPins(esp32cam::pins::AiThinker);
//   cfg.setResolution(res);
//   cfg.setJpeg(80); // Better quality (lower number = higher quality)
//   cfg.setBufferCount(2); // Help with stability

//   bool ok = esp32cam::Camera.begin(cfg);
//   if (!ok) {
//     Serial.println("Camera init failed");
//     while (true) delay(100);
//   }

//   WiFi.softAP(AP_SSID, AP_PASS);
//   Serial.println("WiFi AP started");
//   Serial.println(WiFi.softAPIP());

//   server.on("/capture.jpg", handleCapture);
//   server.begin();
// }

// void loop() {
//   server.handleClient();

//   unsigned long currentMillis = millis();
//   if (currentMillis - lastCaptureTime >= captureInterval) {
//     lastCaptureTime = currentMillis;
//     captureAndSendImage();  // Auto capture every interval
//   }
// }

// void captureAndSendImage() {
//   digitalWrite(4, HIGH);
//   delay(250);
//   auto img = esp32cam::capture();
//   digitalWrite(4, LOW);

//   if (img == nullptr) {
//     Serial.println("Camera capture failed");
//     return;
//   }

//   // Decode JPEG to RGB, convert to grayscale manually here (placeholder)
//   // Currently sending original JPEG directly for testing
//   Serial.printf("Captured %d bytes\n", img->size());

//   // OTA transmission (to a client hitting /capture.jpg)
//   server.setContentLength(img->size());
//   server.send(200, "image/jpeg");
//   WiFiClient client = server.client();
//   img->writeTo(client);
// }






// #define BUTTON_PIN 2

// void setup() {
//   Serial.begin(115200);
//   pinMode(BUTTON_PIN, INPUT_PULLUP);   // internal 45 kΩ pull-up
// }

// void loop() {
//   if (digitalRead(BUTTON_PIN) == LOW) {
//     Serial.println("Button pressed!");
//   } else {
//     Serial.println("Button not pressed.");
//   }
//   delay(300);
// }





/* ──────────  libraries  ────────── */
#include <Arduino.h>
#include <esp32cam.h>          // pulls in esp_camera.h internally
#include <WiFi.h>
#include <WebServer.h>
#include <memory>              // for std::unique_ptr

/* ──────────  Wi-Fi AP  ────────── */
#define AP_SSID  "Boeing"
#define AP_PASS  "bunrieugiotrungvitlon"

/* ──────────  GPIOs  ───────────── */
#define BUTTON_PIN  2      // push-button to GND
#define FLASH_PIN   4      // white LED on ESP32-CAM

/* ──────────  globals  ─────────── */
WebServer server(80);
std::unique_ptr<esp32cam::Frame> lastFrame;   // stores latest JPEG

/* ──────────  helper: shoot one photo  ────────── */
void takePhoto()
{
  digitalWrite(FLASH_PIN, HIGH);
  delay(120);

  lastFrame.reset();                      // drop previous frame
  lastFrame = esp32cam::capture();        // grab new one
  digitalWrite(FLASH_PIN, LOW);

  if (lastFrame) {
    Serial.printf("Captured %d bytes\n", lastFrame->size());
  } else {
    Serial.println("Capture failed");
  }
}

/* ──────────  HTTP handlers  ────────── */
void handleCapture()          // /capture.jpg : shoot + send
{
  takePhoto();
  if (!lastFrame) {
    server.send(500, "text/plain", "capture failed");
    return;
  }
  server.setContentLength(lastFrame->size());
  server.send(200, "image/jpeg");
  lastFrame->writeTo(server.client());
}

void handleImage()            // /image.jpg : send cached frame
{
  if (!lastFrame) {
    server.send(503, "text/plain",
                "no photo yet – press the button!");
    return;
  }
  server.setContentLength(lastFrame->size());
  server.send(200, "image/jpeg");
  lastFrame->writeTo(server.client());
}

/* ──────────  setup  ────────── */
void setup()
{
  Serial.begin(115200);
  pinMode(FLASH_PIN , OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  /* camera config */
  auto res = esp32cam::Resolution::find(1024, 768);
  esp32cam::Config cfg;
  cfg.setPins(esp32cam::pins::AiThinker);
  cfg.setResolution(res);
  cfg.setJpeg(80);            // lower = better quality
  cfg.setBufferCount(2);

  if (!esp32cam::Camera.begin(cfg)) {
    Serial.println("Camera init failed! Halting.");
    while (true) delay(1000);
  }

  /* Wi-Fi AP */
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: ");  Serial.println(WiFi.softAPIP());

  /* routes */
  server.on("/capture.jpg", handleCapture);
  server.on("/image.jpg",   handleImage);
  server.begin();
}

/* ──────────  main loop  ────────── */
void loop()
{
  server.handleClient();

  /* simple edge-detect with debounce */
  static bool prev = HIGH;
  bool now = digitalRead(BUTTON_PIN);

  if (now == LOW && prev == HIGH) {      // button just pressed
    delay(30);                           // debounce
    if (digitalRead(BUTTON_PIN) == LOW)
      takePhoto();
  }
  prev = now;
}
