#include "CamController.h"
#include <esp_camera.h>                 // sensor_t *

bool CamController::begin(uint16_t x, uint16_t y, uint8_t jpegQ)
{
  auto res = esp32cam::Resolution::find(x, y);   // pick closest match
  esp32cam::Config cfg;
  cfg.setPins(esp32cam::pins::AiThinker);
  cfg.setResolution(res);
  cfg.setJpeg(jpegQ);                  // 80 ≈ visually lossless, 60 gives grain
  cfg.setBufferCount(2);

  if (!esp32cam::Camera.begin(cfg)) return false;
  applyMode();
  return true;
}

void CamController::setMode(Mode m)
{
  if (m != current) {
    current = m;
    applyMode();
  }
}

void CamController::applyMode()
{
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return;

  switch (current) {
    case COLOUR:                       // default look
      s->set_special_effect(s, 0);     // none
      s->set_saturation(s, 0);
      s->set_contrast  (s, 0);
      break;

    case NEG:
      s->set_special_effect(s, 1);
      break;

    case BW:                           // classic B&W
      s->set_special_effect(s, 2);     // grayscale 
      break;

    case VINTAGE:                      // warm sepia + punch
      s->set_special_effect(s, 3);     // sepia    
      break;
  }
}

std::unique_ptr<esp32cam::Frame> CamController::capture()
{
  /*  For a subtle “film grain”, capture at UXGA then compress down with
   *  JPEG quality 60-70 — OV2640’s in-sensor encoder adds nice analogue-looking
   *  noise. If you need heavier grain you can dither the JPEG buffer in RAM.
   */
  return esp32cam::capture();
}
