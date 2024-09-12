/*
 * HAFTUNGSAUSSCHLUSS:
 * Diese Software wird "wie besehen" ohne ausdrückliche oder stillschweigende Garantie bereitgestellt.
 * In keinem Fall haftet der Autor oder das Unternehmen für Schäden, die durch die Verwendung dieser Software entstehen.
 * Es liegt in der Verantwortung des Nutzers sicherzustellen, dass die Software für seine Zwecke geeignet ist und dass
 * er sie vor der Verwendung ausreichend getestet hat.
 * Durch die Nutzung dieser Software erklären Sie, dass Sie die Sicherheitswarnungen und Gebrauchsanweisungen gelesen und verstanden haben.
 * 
 * DISLAIMER:
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

#ifndef SOFT_PATH_ELECTRONICS_H
#define SOFT_PATH_ELECTRONICS_H

#include <Arduino.h>
#include <vector>

/*
 * DISCLAIMER:
 * This software is provided "as-is" without any express or implied warranty.
 * In no event shall the author or company be held liable for any damages
 * arising from the use of this software. It is the user's responsibility
 * to ensure that the software is suitable for their needs and that they
 * have tested it sufficiently before using it in any project. By using this
 * software, you agree that you have read and understood the safety warnings
 * and usage instructions.
 * 
 * This library is licensed for personal, non-commercial use only. Commercial use,
 * reproduction, or redistribution without prior written consent is prohibited.
 */

class CustomKeyboard {
public:
    CustomKeyboard();
    void setupKeyboard();
    void setupKey(const String& key);
    int getKeyPressed();
    int getKeyValue(int index);
    int getPin();
    int getNumKeys();
    int getTolerance();
    bool getDebounceMode();
    void printKey();

private:
    int _pin;
    int _numKeys;
    int _tolerance;
    bool _debounceMode;
    int _keyValues[16];
    bool _debug;
    int _lastKey;
    bool _resetRequired;
    bool _pressed;
    void calibrateKey(int keyIndex);
    int readAnalogValue();
    void promptUser(String message);
    int readIntFromSerial();
    int readIntWithPrompt(String prompt);
    bool readBoolWithPrompt(String prompt);
    int calculateRobustMean(std::vector<int>& values);
    int readKeyValue();
};

#endif
