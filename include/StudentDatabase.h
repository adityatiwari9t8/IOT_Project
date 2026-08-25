#pragma once
#include <Arduino.h>
#include <Preferences.h>

// A single enrolled student and their attendance state for today's session.
struct Student {
  // NOTE: this UID doubles as the NVS (flash) storage key in
  // StudentDatabase::begin()/markLogged(). ESP32 NVS keys are capped at
  // 15 characters — our UIDs are 8 hex chars, so we're safe, but don't
  // grow the roster to longer UID formats without checking this.
  String uid;     // RFID card UID, uppercase hex, no separators (e.g. "01020304")
  String name;
  bool logged;    // true once they've been marked present this session
};

// Result of looking a card up against the roster.
enum class LookupResult {
  UNKNOWN_CARD,     // UID not in the roster
  ALREADY_LOGGED,   // valid student, already marked present
  NEW_ENTRY         // valid student, first tap this session
};

// "Database" of enrolled students, backed by the ESP32's NVS flash (via
// the Preferences library) so attendance survives a reset or power loss
// mid-class — not just an in-memory RAM array.
//
// Each student's `logged` flag lives under the "attendance" NVS
// namespace, keyed by their UID. begin() restores whatever was saved
// last; markLogged() writes straight through to flash; resetAttendance()
// wipes the whole namespace to start a fresh session (e.g. the next
// class/day) — see the reset button wired to PIN_BUTTON_RESET.
class StudentDatabase {
 public:
  StudentDatabase(Student* roster, int count);

  // Opens the NVS namespace and restores each student's saved `logged`
  // state. Call once in setup(), after Serial.begin().
  void begin();

  // Wipes all saved attendance (flash + in-memory) for a new session.
  void resetAttendance();

  // Looks up `uid`. Returns UNKNOWN_CARD / ALREADY_LOGGED / NEW_ENTRY.
  // On ALREADY_LOGGED or NEW_ENTRY, `outIndex` is set to the matching
  // roster index so the caller can read the student's name. On
  // UNKNOWN_CARD, outIndex is -1 (no match exists).
  LookupResult lookup(const String& uid, int& outIndex) const;

  // Marks the student at `index` as logged (present) for this session,
  // both in RAM and persisted to flash.
  void markLogged(int index);

  const Student& at(int index) const;
  int count() const;

 private:
  Student* _roster;
  int _count;
  Preferences _prefs;
};
