// SoftPathElectronics.h
#ifndef SOFTPATH_ELECTRONICS_H
#define SOFTPATH_ELECTRONICS_H

#include <Arduino.h>
#include <string.h>

#if defined(ARDUINO_ARCH_AVR)
  #define SPK_PLATFORM 0
#elif defined(ESP32)
  #define SPK_PLATFORM 1
#else
  #error "Nicht unterstützte Plattform"
#endif

/*
 * SoftPathElectronics
 * Laufzeit-Bibliothek zum Einlesen eines analogen Keypads.
 */
class SoftPathElectronics {
public:
  SoftPathElectronics();

  bool    loadKey(const char* key);
  void    begin();
  uint8_t read();
  void    setDebug(bool en) { _debug = en; }

private:
  uint8_t  _pin;
  uint8_t  _numKeys;
  uint16_t _values[16];
  uint16_t _tol;
  bool     _debounceAfterZero;
  bool     _debug;
  bool     _waitingZero;

  uint16_t analogReadNow() const;
};

#endif // SOFTPATH_ELECTRONICS_H
