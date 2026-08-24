#pragma once

// Drives the three status LEDs and the piezo buzzer. Keeping this in
// one place means main.cpp never touches digitalWrite()/tone() directly,
// which makes the pin wiring easy to change in one file (config.h).
class FeedbackIndicators {
 public:
  void begin();

  void success();          // green LED + short high beep (blocking ~500ms)
  void error();             // red LED + longer low beep (blocking ~600ms)
  void wifiConnected();     // short confirmation beep, no LED
  void networkBusy(bool on);  // blue LED on/off while Wi-Fi/HTTP is in flight
};
