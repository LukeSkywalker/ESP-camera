# Film-Inspired Camera

A DIY digital camera inspired by analog film, powered by ESP32-CAM and XIAO ESP32-C3. It captures OTA-only JPG images with retro-style filters and runs fully on a rechargeable battery.

## Features
- ESP32-CAM handles capture and Wi-Fi OTA image transmission
- ESP32-C3 handles user input (shutter + mode switch)
- Three image modes: Negative, Black & White, Retro
- Battery-powered with physical power switch
- Compact 3D-printed enclosure with viewfinder

## Setup
1. Flash `Main_esp32c3.ino` to ESP32-C3 (button/controller logic)
2. Flash `Main.ino` to ESP32-CAM (camera + OTA logic)
3. Press shutter to capture and send an image

## Files
- `CamController.{h,cpp}` – Image style control
- `Main.ino` – Camera logic (ESP32-CAM)
- `Main_esp32c3.ino` – Controller logic (ESP32-C3)

## Demo
![image](https://github.com/user-attachments/assets/9e0f881e-a3b8-4785-aa5c-3e32b1f4505b)




