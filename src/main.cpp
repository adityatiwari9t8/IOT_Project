#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================
// PIN DEFINITIONS (Matching Wokwi diagram.json)
// ============================================
#define PIN_LED_GREEN 13  // On Time / Success
#define PIN_LED_RED   12  // Invalid / Duplicate
#define PIN_LED_BLUE  14  // Processing / Network
#define PIN_BUZZER    15  // Piezo Buzzer

// RFID Pins (SPI)
#define SS_PIN        5
#define RST_PIN       27
MFRC522 rfid(SS_PIN, RST_PIN);

// OLED Display (I2C) - SCL: GPIO 22, SDA: GPIO 21
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDR   0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ============================================
// WI-FI & CLOUD SETTINGS
// ============================================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
bool wifiConnected = false;

// Paste your deployed Google Apps Script Web App URL here:
String GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbztuadEKdYnplTQnfGBNoqNhyFn6njOKJKXBlCvyWCpuNw-58kDp0ROIYUAgswAPrGshA/exec";

// ============================================
// STUDENT DATABASE (Preset Wokwi Card UIDs)
// ============================================
struct Student {
  String uid;
  String name;
  bool logged;
};

Student students[] = {
  {"01020304", "Aisyah", false},
  {"11223344", "Balqis", false},
  {"55667788", "Danish", false},
  {"AABBCCDD", "Farah",  false}
};
const int TOTAL_STUDENTS = 4;

// ============================================
// HELPER FUNCTIONS (OLED & BUZZER)
// ============================================

void showOledMessage(String title, String line1, String line2, String line3) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setCursor(0, 16);
  display.println(line1);
  
  display.setCursor(0, 32);
  display.println(line2);
  
  display.setCursor(0, 48);
  display.println(line3);
  display.display();
}

void triggerBeep(int freq, int durationMs) {
  tone(PIN_BUZZER, freq, durationMs);
}

void indicateSuccess() {
  digitalWrite(PIN_LED_GREEN, HIGH);
  triggerBeep(1500, 150);
  delay(500);
  digitalWrite(PIN_LED_GREEN, LOW);
}

void indicateError() {
  digitalWrite(PIN_LED_RED, HIGH);
  triggerBeep(500, 400);
  delay(600);
  digitalWrite(PIN_LED_RED, LOW);
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);

  // Initialize LEDs & Buzzer
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Initialize OLED (I2C)
  Wire.begin(21, 22); // SDA = 21, SCL = 22
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  showOledMessage("SMART ATTENDANCE", "Booting System...", "Please wait", "");

  // Initialize RFID (SPI)
  SPI.begin();
  rfid.PCD_Init();

  // Connect to Wi-Fi
  digitalWrite(PIN_LED_BLUE, HIGH);
  showOledMessage("WI-FI CONNECTION", "Connecting to:", "Wokwi-GUEST", "");
  
  WiFi.begin(ssid, password);
  const unsigned long wifiTimeoutMs = 15000;
  const unsigned long wifiStartMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartMs < wifiTimeoutMs) {
    delay(400);
    Serial.print(".");
  }
  digitalWrite(PIN_LED_BLUE, LOW);

  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected) {
    Serial.println("\n[SUCCESS] Wi-Fi Connected!");
    triggerBeep(2000, 100);
  } else {
    Serial.println("\n[INFO] Wi-Fi unavailable; offline mode");
  }

  showOledMessage("SYSTEM READY", wifiConnected ? "Wi-Fi: Connected" : "Wi-Fi: Offline", "Tap your ID Card", "SDG 4: Education");
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  // Check if a card is placed on reader
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Extract UID
  String cardUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    cardUID += String(rfid.uid.uidByte[i], HEX);
  }
  cardUID.toUpperCase();

  Serial.print("\nCard Detected! UID: ");
  Serial.println(cardUID);

  // Find student in local database
  int studentIndex = -1;
  for (int i = 0; i < TOTAL_STUDENTS; i++) {
    if (students[i].uid == cardUID) {
      studentIndex = i;
      break;
    }
  }

  // Handle unregistered card
  if (studentIndex == -1) {
    showOledMessage("ACCESS DENIED", "Unknown Card!", "UID: " + cardUID, "Not Registered");
    indicateError();
    delay(1500);
    showOledMessage("SYSTEM READY", wifiConnected ? "Wi-Fi: Connected" : "Wi-Fi: Offline", "Tap your ID Card", "");
    rfid.PICC_HaltA();
    return;
  }

  // Handle duplicate scan
  if (students[studentIndex].logged) {
    showOledMessage("ALREADY LOGGED", students[studentIndex].name, "Status: Present", "Duplicate scan");
    indicateError();
    delay(1500);
    showOledMessage("SYSTEM READY", wifiConnected ? "Wi-Fi: Connected" : "Wi-Fi: Offline", "Tap your ID Card", "");
    rfid.PICC_HaltA();
    return;
  }

  // Valid, new attendance entry
  showOledMessage("WELCOME!", students[studentIndex].name, wifiConnected ? "Syncing Cloud..." : "Offline attendance", "");
  digitalWrite(PIN_LED_BLUE, HIGH);

  // Cloud Transmission (Google Sheets)
  if (wifiConnected && WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = GOOGLE_SCRIPT_URL + "?uid=" + cardUID + "&name=" + students[studentIndex].name;
    
    http.begin(serverPath.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int httpResponseCode = http.GET();
    digitalWrite(PIN_LED_BLUE, LOW);

    if (httpResponseCode >= 200 && httpResponseCode < 300) {
      Serial.println("[SUCCESS] Logged to Google Sheets!");
      showOledMessage("ATTENDANCE LOGGED", students[studentIndex].name, "Status: On Time", "Cloud: Synced");
      indicateSuccess();
    } else {
      Serial.print("[ERROR] HTTP Error: ");
      Serial.println(httpResponseCode);
      showOledMessage("CLOUD ERROR", students[studentIndex].name, "Recorded Locally", "Err: " + String(httpResponseCode));
      indicateSuccess(); // Still grant entry locally
    }
    http.end();
  } else {
    Serial.println("[INFO] Attendance recorded locally; cloud unavailable");
    showOledMessage("ATTENDANCE SAVED", students[studentIndex].name, "Recorded Locally", "Cloud: Offline");
    indicateSuccess();
  }

  students[studentIndex].logged = true;

  delay(2000);
  showOledMessage("SYSTEM READY", wifiConnected ? "Wi-Fi: Connected" : "Wi-Fi: Offline", "Tap your ID Card", "");
  
  rfid.PICC_HaltA();
}