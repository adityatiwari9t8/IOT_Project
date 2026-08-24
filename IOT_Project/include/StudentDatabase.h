#pragma once
#include <Arduino.h>

// A single enrolled student and their attendance state for today's session.
struct Student {
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

// Small in-memory "database" of enrolled students.
//
// NOTE: this list resets to `logged = false` for everyone on every
// power cycle / reset, because it lives in RAM only. That's fine for a
// single class session in the Wokwi simulator. For a real deployment
// you'd persist `logged` (and the date) using ESP32 Preferences/NVS or
// re-derive "already logged today" from the Google Sheet itself — see
// the "Future Improvements" section of README.md.
class StudentDatabase {
 public:
  StudentDatabase(Student* roster, int count);

  // Looks up `uid`. Returns UNKNOWN_CARD / ALREADY_LOGGED / NEW_ENTRY.
  // On ALREADY_LOGGED or NEW_ENTRY, `outIndex` is set to the matching
  // roster index so the caller can read the student's name. On
  // UNKNOWN_CARD, outIndex is -1 (no match exists).
  LookupResult lookup(const String& uid, int& outIndex) const;

  // Marks the student at `index` as logged (present) for this session.
  void markLogged(int index);

  const Student& at(int index) const;
  int count() const;

 private:
  Student* _roster;
  int _count;
};
