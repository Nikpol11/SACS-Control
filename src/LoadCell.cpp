#include "LoadCell.h"

void LoadCell::begin(float calibrationValue)
{
    Serial.println();
    Serial.println("HX711_ADC Initialization");

    loadcell.begin();
    unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
    boolean _tare = true;                 // set this to false if you don't want tare to be performed in the next step
    loadcell.start(stabilizingtime, _tare);
    if (loadcell.getTareTimeoutFlag())
    {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
        while (1)
            ;
    }
    else
    {
        loadcell.setCalFactor(calibrationValue); // set calibration value (float)
        Serial.println("Startup is complete");
    }
}

float LoadCell::getLoadCellValue()
{
    bool newDataReady = false;
    if (loadcell.update())
        newDataReady = true;

    // get smoothed value from the dataset:
    if (newDataReady)
    {
        loadCellValue = loadcell.getData();
        newDataReady = 0;
    }

    return loadCellValue;
}

void LoadCell::printLoadCellValue()
{
    getLoadCellValue();
    Serial.print("Load Cell Value: ");
    Serial.println(loadCellValue);
}

void LoadCell::tareLoadCell()
{
    loadcell.tareNoDelay();
    while (loadcell.getTareStatus() == false)
        ;
    Serial.println("Tare complete");
}

void LoadCell::calibrateLC()
{
    Serial.println("***");
    Serial.println("Start calibration:");
    Serial.println("Place the load cell an a level stable surface.");
    Serial.println("Remove any load applied to the load cell.");
    Serial.println("Send 't' from serial monitor to set the tare offset.");

    boolean _resume = false;
    while (_resume == false)
    {
        loadcell.update();
        if (Serial.available() > 0)
        {
            if (Serial.available() > 0)
            {
                char inByte = Serial.read();
                if (inByte == 't')
                    loadcell.tareNoDelay();
            }
        }
        if (loadcell.getTareStatus() == true)
        {
            Serial.println("Tare complete");
            _resume = true;
        }
    }

    Serial.println("Now, place your known mass on the loadcell.");
    Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

    float known_mass = 0;
    _resume = false;
    while (_resume == false)
    {
        loadcell.update();
        if (Serial.available() > 0)
        {
            known_mass = Serial.parseFloat();
            if (known_mass != 0)
            {
                Serial.print("Known mass is: ");
                Serial.println(known_mass);
                _resume = true;
            }
        }
    }

    float newCalibrationValue = loadcell.getNewCalibration(known_mass); // get the new calibration value

    Serial.print("New calibration value has been set to: ");
    Serial.print(newCalibrationValue);
    Serial.println(", use this as calibration value (calFactor) in your project sketch.");
    Serial.print("Save this value to EEPROM adress ");
    Serial.print(calVal_eepromAdress);
    Serial.println("? y/n");

    _resume = false;
    while (_resume == false)
    {
        if (Serial.available() > 0)
        {
            char inByte = Serial.read();
            if (inByte == 'y')
            {
#if defined(ESP8266) || defined(ESP32)
                EEPROM.begin(512);
#endif
                EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
                EEPROM.commit();
#endif
                EEPROM.get(calVal_eepromAdress, newCalibrationValue);
                Serial.print("Value ");
                Serial.print(newCalibrationValue);
                Serial.print(" saved to EEPROM address: ");
                Serial.println(calVal_eepromAdress);
                _resume = true;
            }
            else if (inByte == 'n')
            {
                Serial.println("Value not saved to EEPROM");
                _resume = true;
            }
        }
    }

    Serial.println("End calibration");
    Serial.println("***");
    Serial.println("To re-calibrate, send 'r' from serial monitor.");
    Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
    Serial.println("***");
}

void LoadCell::changeSavedLCCalFactor()
{
    float oldCalibrationValue = loadcell.getCalFactor();
    boolean _resume = false;
    Serial.println("***");
    Serial.print("Current value is: ");
    Serial.println(oldCalibrationValue);
    Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
    float newCalibrationValue;
    while (_resume == false)
    {
        if (Serial.available() > 0)
        {
            newCalibrationValue = Serial.parseFloat();
            if (newCalibrationValue != 0)
            {
                Serial.print("New calibration value is: ");
                Serial.println(newCalibrationValue);
                loadcell.setCalFactor(newCalibrationValue);
                _resume = true;
            }
        }
    }
    _resume = false;
    Serial.print("Save this value to EEPROM adress ");
    Serial.print(calVal_eepromAdress);
    Serial.println("? y/n");
    while (_resume == false)
    {
        if (Serial.available() > 0)
        {
            char inByte = Serial.read();
            if (inByte == 'y')
            {
#if defined(ESP8266) || defined(ESP32)
                EEPROM.begin(512);
#endif
                EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
                EEPROM.commit();
#endif
                EEPROM.get(calVal_eepromAdress, newCalibrationValue);
                Serial.print("Value ");
                Serial.print(newCalibrationValue);
                Serial.print(" saved to EEPROM address: ");
                Serial.println(calVal_eepromAdress);
                _resume = true;
            }
            else if (inByte == 'n')
            {
                Serial.println("Value not saved to EEPROM");
                _resume = true;
            }
        }
    }
    Serial.println("End change calibration value");
    Serial.println("***");
}
