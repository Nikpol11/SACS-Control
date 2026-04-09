#ifndef LOADCELL_H
#define LOADCELL_H

#include <Arduino.h>
#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif


class LoadCell
{
private:
    const int calVal_eepromAdress = 0;
    unsigned long t = 0;
    float loadCellValue = 0;
    const int HX711_dout = 3; // mcu > HX711 dout pin, must be external interrupt capable!
    const int HX711_sck = 5;  // mcu > HX711 sck pin
    HX711_ADC loadcell = HX711_ADC(HX711_dout, HX711_sck);
    void loadCellDataReadyISR();

public:
    LoadCell();
    void begin(float callibrationValue = 410.84);
    void calibrateLC();
    void changeSavedLCCalFactor();
    float getLoadCellValue();
    void printLoadCellValue();
    void tareLoadCell();
};

#endif // LOADCELL_H
