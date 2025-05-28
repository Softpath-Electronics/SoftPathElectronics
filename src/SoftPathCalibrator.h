// SoftPathCalibrator.h
#ifndef SOFTPATH_CALIBRATOR_H
#define SOFTPATH_CALIBRATOR_H

#include <Arduino.h>

#if defined(ARDUINO_ARCH_AVR)
  #define SPK_PLATFORM 0      // UNO / AVR (10-Bit-ADC)
#elif defined(ESP32)
  #define SPK_PLATFORM 1      // ESP32 (12-Bit-ADC)
#else
  #error "Nicht unterstützte Plattform"
#endif

/*
 * SoftPathCalibrator
 * Interaktive Kalibrier-Bibliothek für analoge Keypads.
 * Alle Dialoge sind auf Deutsch.  Bei den Fragen „Toleranz“ und
 * „Nach Nullwert entprellen?“ kann der Benutzer „h“ eingeben,
 * um eine kurze Hilfe angezeigt zu bekommen.
 */
class SoftPathCalibrator {
public:
  SoftPathCalibrator();
  void     begin();                   // startet den Dialog
  const char* getKey() const { return _keyStr; }

private:
  uint8_t  _platform;                 // 0 = UNO, 1 = ESP32
  uint8_t  _pin;                      // Kanal (UNO) bzw. GPIO (ESP32)
  uint8_t  _numKeys;                  // 3,4,6,12,16
  uint16_t _tol;                      // Toleranz
  bool     _debounceAfterZero;        // Debounce-After-Zero
  uint16_t _values[16];               // Zielwerte
  char     _keyStr[192];              // Key-String

  /* interne Helfer */
  void printPlatform();
  void overridePlatform();
  void askPin();
  void askKeys();
  void askTolerance();
  void askDebounce();
  void calibrateKeys();
  void buildKeyString();

  uint16_t readAnalogOnce();
  void     gatherReadings(uint16_t* buf);
  uint16_t medianOfSix(uint16_t* arr);
  void     readLine(String& out);
  void     drawLayout(uint8_t highlight);
  bool     askYesNo(const __FlashStringHelper* prompt);

  /* Hilfetexte */
  void helpTolerance();
  void helpDebounce();
};

#endif // SOFTPATH_CALIBRATOR_H
