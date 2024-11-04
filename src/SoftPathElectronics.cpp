/*
 * HAFTUNGSAUSSCHLUSS:
 * Diese Software wird "wie besehen" ohne ausdrückliche oder stillschweigende Garantie bereitgestellt.
 * In keinem Fall haftet der Autor oder das Unternehmen für Schäden, die durch die Verwendung dieser Software entstehen.
 * Es liegt in der Verantwortung des Nutzers sicherzustellen, dass die Software für seine Zwecke geeignet ist und dass
 * er sie vor der Verwendung ausreichend getestet hat.
 * Durch die Nutzung dieser Software erklären Sie, dass Sie die Sicherheitswarnungen und Gebrauchsanweisungen gelesen und verstanden haben.
 * 
 * DISCLAIMER:
 * This software is provided "as-is" without any express or implied warranty.
 * In no event shall the author or company be held liable for any damages arising from the use of this software.
 * It is the user's responsibility to ensure that the software is suitable for their needs and that they have tested it
 * sufficiently before using it in any project. By using this software, you agree that you have read and understood the safety warnings
 * and usage instructions.
 * 
 * LIZENZ:
 * Diese Software ist nur für den persönlichen, nicht-kommerziellen Gebrauch lizenziert. Eine kommerzielle Nutzung, 
 * Reproduktion oder Weiterverbreitung ist ohne vorherige schriftliche Zustimmung untersagt.
 * 
 * LICENSE:
 * This library is licensed for personal, non-commercial use only. Commercial use, reproduction, or redistribution without
 * prior written consent is prohibited.
 */
#include "SoftPathElectronics.h"

// Konstruktor
CustomKeyboard::CustomKeyboard() {
    _debug = false;  // Setzen Sie dies auf 'true', um Debugging zu aktivieren
    _lastKey = -1;
    _resetRequired = false;
    _pressed = false;
}

// Initialisierung der Tastatur
void CustomKeyboard::setupKeyboard() {
    Serial.begin(115200);
    // Entfernen der 'while (!Serial) { ; }' Zeile, um Hängenbleiben zu vermeiden

    if (!readBoolWithPrompt("Haben Sie alle Sicherheitshinweise gelesen? (y/n):")) {
        Serial.println("Bitte lesen Sie die Sicherheitshinweise, bevor Sie fortfahren.");
        return;
    }
    if (!readBoolWithPrompt("Haben Sie die Bedienungsanleitung gelesen? (y/n):")) {
        Serial.println("Bitte lesen Sie die Bedienungsanleitung, bevor Sie fortfahren.");
        return;
    }

    _pin = readIntWithPrompt("Gib den analogen Eingangspin ein:");

    _numKeys = readIntWithPrompt("Gib die Anzahl der Tasten ein (3, 4, 6, 12 oder 16):");
    while (_numKeys != 3 && _numKeys != 4 && _numKeys != 6 && _numKeys != 12 && _numKeys != 16) {
        _numKeys = readIntWithPrompt("Ungültige Anzahl der Tasten. Bitte erneut eingeben:");
    }

    _tolerance = readIntWithPrompt("Gib die Toleranz ein (empfohlen: 8):");

    _debounceMode = readBoolWithPrompt("Soll eine Taste erst gezählt werden, wenn der analoge Wert zwischendurch 0 war? (y/n):");

    // Initialisieren des Pins
    pinMode(_pin, INPUT);

    Serial.println("Starte Tastenkalibrierung...");
    for (int i = 0; i < _numKeys; i++) {
        while (true) {
            calibrateKey(i);
            Serial.print("Taste ");
            Serial.print(i + 1);
            Serial.println(" wurde kalibriert.");

            if (i < _numKeys - 1) {
                Serial.print("Möchtest du mit Taste ");
                Serial.print(i + 2);
                Serial.print(" fortfahren oder 'redo' eingeben für Taste ");
                Serial.print(i + 1);
                Serial.println("? (Enter für fortfahren / redo eingeben):");
            } else {
                Serial.println("Kalibrierung abgeschlossen.");
            }

            while (Serial.available() == 0) {
                // Warten auf Benutzereingabe
            }
            String input = Serial.readStringUntil('\n');
            input.trim();
            if (input.equalsIgnoreCase("redo")) {
                continue; // Wiederhole die Kalibrierung der aktuellen Taste
            }
            break; // Gehe zur nächsten Taste
        }
    }
    Serial.println("Kalibrierung aller Tasten abgeschlossen.");
    printKey();
}

// Kalibriert eine einzelne Taste
void CustomKeyboard::calibrateKey(int keyIndex) {
    int value1, value2;

    Serial.print("Drücke Taste ");
    Serial.print(keyIndex + 1);
    Serial.println(" und drücke Enter, um fortzufahren.");
    value1 = readKeyValue();
    Serial.println("Drücke die Taste nochmals und drücke Enter.");
    value2 = readKeyValue();

    _keyValues[keyIndex] = (value1 + value2) / 2;
    if (_debug) {
        Serial.print("Kalibrierter Wert für Taste ");
        Serial.print(keyIndex + 1);
        Serial.print(": ");
        Serial.println(_keyValues[keyIndex]);
    }
}

// Liest einen einzelnen Tastendruck-Wert
int CustomKeyboard::readKeyValue() {
    int count = 0;
    int value;

    while (true) {
        value = readAnalogValue();
        if (value > 0 && count < MAX_SAMPLES) {
            _values[count++] = value;
        }
        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();
            break;
        }
    }

    return calculateRobustMean(_values, count);
}

// Berechnet den robusten Mittelwert (Median-basierte Methode)
int CustomKeyboard::calculateRobustMean(int* values, int count) {
    if (count == 0) {
        return 0;
    }

    // Sortieren der Werte (Bubble Sort)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (values[j] > values[j + 1]) {
                int temp = values[j];
                values[j] = values[j + 1];
                values[j + 1] = temp;
            }
        }
    }

    // Berechnung des Medians
    int median;
    if (count % 2 == 0) {
        median = (values[count / 2 - 1] + values[count / 2]) / 2;
    } else {
        median = values[count / 2];
    }

    // Summieren der Werte innerhalb der Toleranz um den Median
    long sum = 0;
    int validCount = 0;
    for (int i = 0; i < count; i++) {
        if (abs(values[i] - median) < _tolerance) {
            sum += values[i];
            validCount++;
        }
    }

    if (validCount > 0) {
        return sum / validCount;
    } else {
        return median;
    }
}

// Liest den analogen Wert vom angegebenen Pin
int CustomKeyboard::readAnalogValue() {
    return analogRead(_pin);
}

// Liest eine boolesche Eingabe vom Benutzer (y/n)
bool CustomKeyboard::readBoolWithPrompt(String prompt) {
    char value;
    while (true) {
        promptUser(prompt);
        while (Serial.available() == 0) {
            // Warten auf Eingabe
        }
        value = Serial.read();
        while (Serial.available() > 0) {
            Serial.read();  // Puffer leeren
        }
        if (value == 'y' || value == 'Y') {
            return true;
        } else if (value == 'n' || value == 'N') {
            return false;
        } else {
            Serial.println("Ungültige Eingabe. Bitte 'y' oder 'n' eingeben.");
        }
    }
}

// Liest eine Ganzzahl vom Benutzer
int CustomKeyboard::readIntWithPrompt(String prompt) {
    int value = 0;
    while (true) {
        promptUser(prompt);
        while (Serial.available() == 0) {
            // Warten auf Eingabe
        }
        value = Serial.parseInt();
        while (Serial.available() > 0) {
            Serial.read();  // Puffer leeren
        }
        if (value != 0) {
            break;
        } else {
            Serial.println("Ungültige Eingabe. Bitte eine gültige Zahl eingeben.");
        }
    }
    return value;
}

// Gibt die konfigurierten Schlüsselwerte aus
void CustomKeyboard::printKey() {
    Serial.print("Key: ");
    Serial.print(_pin);
    Serial.print(" ");
    Serial.print(_numKeys);
    Serial.print(" ");
    Serial.print(_tolerance);
    Serial.print(" ");
    Serial.print(_debounceMode ? 1 : 0);
    Serial.print(" ");
    for (int i = 0; i < _numKeys; i++) { // Nur bis _numKeys iterieren
        Serial.print(_keyValues[i]);
        Serial.print(" ");
    }
    Serial.println();
}

// Zeigt eine Nachricht an und wartet auf Benutzereingabe
void CustomKeyboard::promptUser(String message) {
    Serial.println(message);
}

// Initialisiert die Tastatur mit gespeicherten Schlüsselwerten
void CustomKeyboard::setupKey(const String& key) {
    int values[20] = {0}; // Alle Werte mit 0 initialisieren
    int index = 0;
    int startIndex = 0;
    int spaceIndex = key.indexOf(' ', startIndex);

    // Parse des Key-Strings und Füllen des values-Arrays
    while (spaceIndex != -1 && index < 20) {
        String token = key.substring(startIndex, spaceIndex);
        values[index++] = token.toInt();
        startIndex = spaceIndex + 1;
        spaceIndex = key.indexOf(' ', startIndex);
    }
    // Letzten Wert erfassen, falls kein abschließendes Leerzeichen vorhanden ist
    if (startIndex < key.length() && index < 20) {
        String token = key.substring(startIndex);
        values[index++] = token.toInt();
    }

    // Überprüfen, ob alle erforderlichen Werte vorhanden sind
    if (index < 20) {
        Serial.println("Error: Key string does not have enough values.");
        return;
    }

    // Zuweisen der Werte zu den entsprechenden Variablen
    _pin = values[0];
    _numKeys = values[1];
    _tolerance = values[2];
    _debounceMode = (values[3] != 0);
    for (int i = 0; i < 16; i++) {
        _keyValues[i] = values[4 + i];
    }

    // Initialisieren des Pins
    pinMode(_pin, INPUT);

    if (_debug) {
        Serial.println("Keyboard Configuration:");
        Serial.print("Pin: ");
        Serial.println(_pin);
        Serial.print("Number of Keys: ");
        Serial.println(_numKeys);
        Serial.print("Tolerance: ");
        Serial.println(_tolerance);
        Serial.print("Debounce Mode: ");
        Serial.println(_debounceMode ? "Enabled" : "Disabled");
        Serial.print("Key Values: ");
        for (int i = 0; i < _numKeys; i++) {
            Serial.print(_keyValues[i]);
            if (i < _numKeys - 1) Serial.print(", ");
        }
        Serial.println();
    }
}

// Gibt die Nummer der gedrückten Taste zurück
int CustomKeyboard::getKeyPressed() {
    int value = readAnalogValue();
    if (_debug) {
        Serial.print("Analog Value Read: ");
        Serial.println(value);
    }
    for (int i = 0; i < _numKeys; i++) {
        if (abs(value - _keyValues[i]) < _tolerance) {
            if (_lastKey != i + 1 || _resetRequired) {
                _lastKey = i + 1;
                _resetRequired = false;
                if (_debounceMode && value == 0) {
                    _resetRequired = true;
                    continue;
                }
                return i + 1;  // Rückgabe der Tastennummer, beginnend mit 1
            }
        }
    }
    _lastKey = -1;
    return -1;  // Keine Taste erkannt
}

// Gibt den Wert einer bestimmten Taste zurück
int CustomKeyboard::getKeyValue(int index) {
    if (index >= 0 && index < 16) {
        return _keyValues[index];
    }
    return -1;
}

// Getter-Methoden
int CustomKeyboard::getPin() {
    return _pin;
}

int CustomKeyboard::getNumKeys() {
    return _numKeys;
}

int CustomKeyboard::getTolerance() {
    return _tolerance;
}

bool CustomKeyboard::getDebounceMode() {
    return _debounceMode;
}
