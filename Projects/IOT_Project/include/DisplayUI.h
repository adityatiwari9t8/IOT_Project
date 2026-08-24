#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

// Thin wrapper around the SSD1306 OLED. Every screen in this project is
// a 4-line layout: a title, an underline, and three content lines — so
// we expose exactly that instead of raw Adafruit_GFX calls everywhere.
class DisplayUI {
 public:
  DisplayUI();

  // Initializes I2C and the display. Returns false if the OLED didn't
  // respond (wrong wiring / address) — caller should halt in that case.
  bool begin();

  void showMessage(const String& title, const String& line1,
                    const String& line2, const String& line3);

  // Convenience wrapper for the idle/home screen.
  void showReady(bool wifiConnected);

 private:
  Adafruit_SSD1306 _display;
};
