#ifndef IMU_H
#define IMU_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_BNO080_Arduino_Library.h>

class BNO080_IMU {
public:
    void begin(int sendInterval = 50);
    void readIMU();
    void printIMU();

    float bno080Data[3][3];

private:
    BNO080 bno080;
};

#endif // IMU_H
