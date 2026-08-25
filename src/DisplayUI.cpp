#include "DisplayUI.h"
#include "config.h"

DisplayUI::DisplayUI()
    : _display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET_PIN) {}

bool DisplayUI::begin() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    return false;
  }
  _display.cp437(true);  // correct glyph mapping for extended characters
  return true;
}

void DisplayUI::showMessage(const String& title, const String& line1,
                             const String& line2, const String& line3) {
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.setTextSize(1);

  _display.setCursor(0, 0);
  _display.println(title);
  _display.drawLine(0, 10, OLED_SCREEN_WIDTH, 10, SSD1306_WHITE);

  _display.setCursor(0, 16);
  _display.println(line1);

  _display.setCursor(0, 32);
  _display.println(line2);

  _display.setCursor(0, 48);
  _display.println(line3);

  _display.display();
}

void DisplayUI::showReady(bool wifiConnected) {
  showMessage("SYSTEM READY",
              wifiConnected ? "Wi-Fi: Connected" : "Wi-Fi: Offline",
              "Tap your ID Card", "");
}
