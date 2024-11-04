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

CustomKeyboard::CustomKeyboard() {
    _debug = false;
    _lastKey = -1;
    _resetRequired = false;
    _pressed = false;
}

void CustomKeyboard::setupKeyboard() {
    Serial.begin(115200);

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

    Serial.println("Starte Tastenkalibrierung...");
    for (int i = 0; i < _numKeys; i++) {
        while (true) {
            calibrateKey(i);
            Serial.print("Möchtest du zu Taste ");
            Serial.print(i + 1);
            Serial.println(" fortfahren oder 'redo' eingeben? (Enter für fortfahren / redo eingeben):");
            while (Serial.available() == 0) {}
            String input = Serial.readStringUntil('\n');
            input.trim();
            if (input.equalsIgnoreCase("redo")) {
                continue;
            }
            break;
        }
    }
    Serial.println("Kalibrierung abgeschlossen.");
    printKey();
}

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

int CustomKeyboard::readKeyValue() {
    std::vector<int> values;
    int value;

    while (true) {
        value = readAnalogValue();
        if (value > 0) {
            values.push_back(value);
        }
        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();
            break;
        }
    }

    return calculateRobustMean(values);
}

int CustomKeyboard::calculateRobustMean(std::vector<int>& values) {
    if (values.size() < 3) {
        long sum = 0;
        for (int value : values) {
            sum += value;
        }
        return sum / values.size();
    }

    std::sort(values.begin(), values.end());
    int median = values[values.size() / 2];

    int sum = 0;
    int count = 0;
    for (int value : values) {
        if (abs(value - median) < _tolerance) {
            sum += value;
            count++;
        }
    }

    return (count > 0) ? (sum / count) : median;
}

int CustomKeyboard::readAnalogValue() {
    return analogRead(_pin);  // Reads the analog value from the specified pin
}

bool CustomKeyboard::readBoolWithPrompt(String prompt) {
    char value;
    while (true) {
        promptUser(prompt);
        if (Serial.available() > 0) {
            value = Serial.read();
            while (Serial.available() > 0) {
                Serial.read();  // Clear the buffer
            }
            if (value == 'y' || value == 'Y') {
                return true;
            } else if (value == 'n' || value == 'N') {
                return false;
            }
        }
    }
}

int CustomKeyboard::readIntWithPrompt(String prompt) {
    int value;
    while (true) {
        promptUser(prompt);
        if (Serial.available() > 0) {
            value = Serial.parseInt();
            while (Serial.available() > 0) {
                Serial.read();  // Clear the buffer
            }
            break;
        }
    }
    return value;
}

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
    for (int i = 0; i < 16; i++) {
        Serial.print(_keyValues[i]);
        Serial.print(" ");
    }
    Serial.println();
}

void CustomKeyboard::promptUser(String message) {
    Serial.println(message);
    while (Serial.available() == 0) {}
}

void CustomKeyboard::setupKey(const String& key) {
    int values[20] = {0}; // Initialize all to zero
    int index = 0;
    int startIndex = 0;
    int spaceIndex = key.indexOf(' ', startIndex);

    // Parse the key string and fill the values array
    while (spaceIndex != -1 && index < 20) {
        String token = key.substring(startIndex, spaceIndex);
        values[index++] = token.toInt();
        startIndex = spaceIndex + 1;
        spaceIndex = key.indexOf(' ', startIndex);
    }
    // Capture the last value if there's no trailing space
    if (startIndex < key.length() && index < 20) {
        String token = key.substring(startIndex);
        values[index++] = token.toInt();
    }

    // Check if we have all the required values
    if (index < 20) {
        Serial.println("Error: Key string does not have enough values.");
        return;
    }

    // Assign the values to the appropriate variables
    _pin = values[0];
    _numKeys = values[1];
    _tolerance = values[2];
    _debounceMode = (values[3] != 0);
    for (int i = 0; i < 16; i++) {
        _keyValues[i] = values[4 + i];
    }

    // Initialize the pin
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

int CustomKeyboard::getKeyValue(int index) {
    if (index >= 0 && index < 16) {
        return _keyValues[index];
    }
    return -1;
}

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
