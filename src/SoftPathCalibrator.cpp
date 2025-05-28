// SoftPathCalibrator.cpp
#include "SoftPathCalibrator.h"

SoftPathCalibrator::SoftPathCalibrator() :
  _platform(SPK_PLATFORM),
  _pin(0),
  _numKeys(0),
  _tol(8),                    // empfohlener Standardwert
  _debounceAfterZero(false)
{
  _keyStr[0] = '\0';
}

/* ---------- öffentlich ---------- */

void SoftPathCalibrator::begin() {
  Serial.begin(115200);
  while (!Serial) { }

  Serial.println(F("SoftPath Keypad Kalibrator v1.3"));
  printPlatform();
  overridePlatform();
  askPin();
  askKeys();
  askTolerance();
  askDebounce();
  calibrateKeys();
  buildKeyString();

  Serial.println();
  Serial.println(F("Kalibrierung abgeschlossen. Key-String:"));
  Serial.println(_keyStr);
}

/* ---------- generische I/O-Helfer ---------- */

void SoftPathCalibrator::printPlatform() {
  Serial.print(F("Erkannte Plattform: "));
  Serial.println(_platform == 0 ? F("UNO/AVR") : F("ESP32"));
}

void SoftPathCalibrator::overridePlatform() {
  Serial.println(F("Plattform überschreiben? "
                   "'u' = UNO, 'e' = ESP32 (3 s)…"));
  unsigned long t0 = millis();
  while (millis() - t0 < 3000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'u' || c == 'U') { _platform = 0; break; }
      if (c == 'e' || c == 'E') { _platform = 1; break; }
    }
  }
}

void SoftPathCalibrator::readLine(String& out) {
  while (!Serial.available()) { delay(10); }
  out = Serial.readStringUntil('\n');
  out.trim();
}

bool SoftPathCalibrator::askYesNo(const __FlashStringHelper* prompt) {
  Serial.println(prompt);
  String line; readLine(line);
  if (!line.length()) return false;
  char c = line[0];
  return (c == 'j' || c == 'J' || c == 'y' || c == 'Y');
}

/* ---------- Hilfetexte ---------- */

void SoftPathCalibrator::helpTolerance() {
  Serial.println(F("\nToleranz beschreibt, wie stark der gemessene ADC-Wert\n"
                   "vom Sollwert einer Taste abweichen darf, damit sie noch\n"
                   "als gedrückt erkannt wird.  Ein zu kleiner Wert führt\n"
                   "zu Fehlschlägen, ein zu großer kann Mehrfach­zuordnungen\n"
                   "verursachen.\n"
                   "Beispiel bei 10-Bit-ADC und Sollwert 512:\n"
                   "Toleranz 8  → gültig 504…520\n"
                   "Toleranz 20 → gültig 492…532\n"));
}

void SoftPathCalibrator::helpDebounce() {
  Serial.println(F("\n„Nach Nullwert entprellen“ sorgt dafür, dass zwischen\n"
                   "zwei Tastendrücken erst ein eindeutiger Null-ADC-Wert\n"
                   "erkannt werden muss.  Dadurch werden Fehl­erkennungen\n"
                   "bei sehr schnellen oder über­schneidenden Drücken\n"
                   "vermieden.\n"
                   "Aktiviert (j):  gedrückt → loslassen (ADC≈0) → nächste Taste\n"
                   "Deaktiviert (n): jede Taste sofort gültig.\n"));
}

/* ---------- Fragen ---------- */

void SoftPathCalibrator::askPin() {
  if (_platform == 0) {
    Serial.println(F("Analogen Pin eingeben (A0–A5 oder 0–5):"));
    String line; readLine(line); line.toUpperCase();
    if (line.startsWith("A")) line.remove(0, 1);
    _pin = constrain(line.toInt(), 0, 5);
  } else {
    Serial.println(F("GPIO-Nummer eingeben (0–39):"));
    String line; readLine(line);
    _pin = constrain(line.toInt(), 0, 39);
  }
}

void SoftPathCalibrator::askKeys() {
  Serial.println(F("Anzahl der Tasten (3 / 4 / 6 / 12 / 16):"));
  String line; readLine(line);
  uint8_t n = line.toInt();
  if (n == 3 || n == 4 || n == 6 || n == 12 || n == 16) _numKeys = n;
  else                                                   _numKeys = 4;
}

void SoftPathCalibrator::askTolerance() {
  for (;;) {
    Serial.println(F("Toleranz 4–60 (Standard 8, 'h' für Hilfe):"));
    String line; readLine(line);
    if (!line.length()) { _tol = 8; break; }           // Standard
    if (line[0] == 'h' || line[0] == 'H') { helpTolerance(); continue; }
    uint16_t v = line.toInt();
    if (v >= 4 && v <= 60) { _tol = v; break; }
    Serial.println(F("Ungültige Eingabe.\n"));
  }
}

void SoftPathCalibrator::askDebounce() {
  for (;;) {
    Serial.println(
      F("Nach Nullwert entprellen? (j/n, 'h' für Hilfe):"));
    String line; readLine(line);
    if (!line.length()) { _debounceAfterZero = false; break; }
    char c = line[0];
    if (c == 'h' || c == 'H') { helpDebounce(); continue; }
    if (c == 'j' || c == 'J' || c == 'y' || c == 'Y') {
      _debounceAfterZero = true;  break;
    }
    if (c == 'n' || c == 'N') { _debounceAfterZero = false; break; }
    Serial.println(F("Bitte 'j' oder 'n' eingeben.\n"));
  }
}

/* ---------- ADC-Helfer ---------- */

uint16_t SoftPathCalibrator::readAnalogOnce() {
#if SPK_PLATFORM == 0
  return analogRead(_pin + A0);
#else
  return analogRead(_pin);
#endif
}

void SoftPathCalibrator::gatherReadings(uint16_t* buf) {
  uint8_t i = 0;
  while (i < 6) {
    uint16_t v = readAnalogOnce();
    if (v > 0) {
      buf[i++] = v;
      Serial.print('.');
    }
    delay(8);
  }
  Serial.println();
}

uint16_t SoftPathCalibrator::medianOfSix(uint16_t* arr) {
  uint16_t t[6];
  for (uint8_t i = 0; i < 6; ++i) t[i] = arr[i];
  for (uint8_t i = 0; i < 5; ++i)
    for (uint8_t j = i + 1; j < 6; ++j)
      if (t[j] < t[i]) { uint16_t x = t[i]; t[i] = t[j]; t[j] = x; }
  return (uint16_t)((t[2] + t[3]) / 2);
}

/* ---------- ASCII-Layout ---------- */

void SoftPathCalibrator::drawLayout(uint8_t highlight) {
  uint8_t cols, rows;
  switch (_numKeys) {
    case 3:  cols = 3; rows = 1; break;
    case 4:  cols = 4; rows = 1; break;
    case 6:  cols = 3; rows = 2; break;
    case 12: cols = 3; rows = 4; break;
    default: cols = 4; rows = 4; break;
  }

  Serial.println();
  for (uint8_t r = 0; r < rows; ++r) {
    Serial.print('+');
    for (uint8_t c = 0; c < cols; ++c) Serial.print(F("----+"));
    Serial.println();

    Serial.print('|');
    for (uint8_t c = 0; c < cols; ++c) {
      uint8_t idx = r * cols + c;
      char cell[5] = "    ";
      if (idx < _numKeys) {
        char d[4]; sprintf(d, "%u", idx + 1);
        if (idx == highlight) cell[0] = '>';
        memcpy(cell + 1, d, strlen(d));
      }
      Serial.print(cell); Serial.print('|');
    }
    Serial.println();
  }
  Serial.print('+');
  for (uint8_t c = 0; c < cols; ++c) Serial.print(F("----+"));
  Serial.println("\n");
}

/* ---------- Kalibrierung ---------- */

void SoftPathCalibrator::calibrateKeys() {
  uint16_t buf[6];

  Serial.println(F("=== Kalibrierung ==="));
  for (uint8_t k = 0; k < _numKeys; ++k) {
    bool done = false;
    while (!done) {
      drawLayout(k);
      uint16_t med1 = 0, med2 = 0;

      for (uint8_t p = 0; p < 2; ++p) {
        Serial.print(F("Taste ")); Serial.print(k + 1);
        Serial.print(F(", Durchgang ")); Serial.print(p + 1);
        Serial.println(F(": drücken & halten…"));
        while (readAnalogOnce() == 0) { delay(5); }
        gatherReadings(buf);
        (p == 0 ? med1 : med2) = medianOfSix(buf);
        Serial.println(F("Loslassen…"));
        while (readAnalogOnce() > 0) { delay(5); }
      }

      uint16_t diff = (med1 > med2) ? (med1 - med2) : (med2 - med1);
      if (diff > _tol) {
        Serial.print(F("Warnung: Abweichung "));
        Serial.print(diff);
        Serial.print(F(" > Toleranz "));
        Serial.println(_tol);
        if (askYesNo(
              F("Evtl. falsche Taste?  Nochmals versuchen? (j/n):")))
          continue;
      }
      _values[k] = (uint16_t)((med1 + med2) / 2);
      Serial.print(F(" → gespeicherter Wert: "));
      Serial.println(_values[k]);
      done = true;
    }
  }
}

/* ---------- Key-String ---------- */

void SoftPathCalibrator::buildKeyString() {
  char* p = _keyStr;
  int n = sprintf(p, "SPK1 %u %u %u %u %u",
                  _platform, _pin, _numKeys,
                  _tol, _debounceAfterZero ? 1 : 0);
  p += n;
  for (uint8_t i = 0; i < _numKeys; ++i) {
    n = sprintf(p, " %u", _values[i]); p += n;
  }
  sprintf(p, "\r\n");
}
