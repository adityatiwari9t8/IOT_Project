#pragma once
// =====================================================================
// config.h — Non-secret hardware & timing configuration
//
// Every #define here must match the wiring described in diagram.json.
// If you change a pin here, update diagram.json too (and vice versa).
// =====================================================================

// ---------------------------------------------------------------------
// LEDs & Buzzer
// ---------------------------------------------------------------------
#define PIN_LED_GREEN   13   // Attendance accepted / success
#define PIN_LED_RED     12   // Rejected: unknown card or duplicate tap
#define PIN_LED_BLUE    14   // Busy: Wi-Fi connecting / cloud sync
#define PIN_BUZZER      15   // Piezo buzzer (active via ledc tone())

// ---------------------------------------------------------------------
// Attendance reset button
// Wired to a discrete wokwi-pushbutton part (see diagram.json) rather
// than the board's silkscreen BOOT button — we couldn't confirm the
// devkit's built-in BOOT button is clickable inside Wokwi's UI, and a
// real part is guaranteed to be. Wired active-LOW with INPUT_PULLUP,
// so the other leg just needs to go to GND.
// ---------------------------------------------------------------------
#define PIN_BUTTON_RESET  4

// ---------------------------------------------------------------------
// MFRC522 RFID Reader (SPI)
// NOTE: RST is on GPIO27, NOT GPIO21 — GPIO21 is used by the OLED (SDA).
// Sharing that pin was a real bug in an earlier revision of this project.
// ---------------------------------------------------------------------
#define PIN_RFID_SS     5
#define PIN_RFID_RST    27
// SCK=18, MOSI=23, MISO=19 use the ESP32's default HSPI/VSPI mapping
// and are wired directly in diagram.json — no #define needed for them.

// ---------------------------------------------------------------------
// SSD1306 OLED Display (I2C)
// ---------------------------------------------------------------------
#define PIN_OLED_SDA        21
#define PIN_OLED_SCL        22
#define OLED_SCREEN_WIDTH    128
#define OLED_SCREEN_HEIGHT   64
#define OLED_RESET_PIN       -1     // Shares ESP32 reset, no dedicated pin
#define OLED_I2C_ADDRESS      0x3C

// ---------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------
#define WIFI_CONNECT_TIMEOUT_MS   15000  // Give up and go offline after this
#define HTTP_REQUEST_TIMEOUT_MS   10000  // Abort a hung cloud request
#define RESULT_DISPLAY_MS          1500  // How long ACCESS DENIED / DUPLICATE stays up
#define SUCCESS_DISPLAY_MS         2000  // How long WELCOME / LOGGED stays up
#define SUCCESS_BEEP_HOLD_MS        500  // LED/beep hold time on success
#define ERROR_BEEP_HOLD_MS          600  // LED/beep hold time on error
#define RESET_HOLD_MS              3000  // Hold reset button this long to wipe attendance

// ---------------------------------------------------------------------
// Buzzer tones
// ---------------------------------------------------------------------
#define TONE_SUCCESS_HZ   1500
#define TONE_SUCCESS_MS    150
#define TONE_ERROR_HZ      500
#define TONE_ERROR_MS      400
#define TONE_WIFI_OK_HZ   2000
#define TONE_WIFI_OK_MS    100
