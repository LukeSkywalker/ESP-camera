#pragma once
#include <Arduino.h>
#include <esp32cam.h>

/*  Tiny wrapper around esp32cam::capture()
 *  Gives three “looks” and hides the sensor-register voodoo.
 */
class CamController {
public:
  enum Mode : uint8_t { COLOUR = 0, NEG = 1, BW = 2, VINTAGE = 3 };

  bool begin(uint16_t x = 1600, uint16_t y = 1200, uint8_t jpegQ = 85);
  void  setMode(Mode m);
  Mode  mode() const                 { return current; }
  std::unique_ptr<esp32cam::Frame> capture();   // JPEG ptr

private:
  void applyMode();
  Mode current = COLOUR;
};
