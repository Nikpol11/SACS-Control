#include "IMU.h"

void BNO080_IMU::begin(int sendInterval)
{

    if (!bno080.begin(0x4A))
    {
        while (1)
        {
            Serial.println(F("bno080 not detected at address 0x4A. freezing..."));
            delay(5000);
        }
    }
    else
    {
        Serial.println(F("IMU Connection Sucessful"));
        delay(1000);
    }

    bno080.enableAccelerometer(sendInterval);    // We must enable the accel in order to get MEMS readings even if we don't read the reports.
    bno080.enableRawAccelerometer(sendInterval); // Send data update every 50ms
    bno080.enableGyro(sendInterval);
    bno080.enableRawGyro(sendInterval);
    bno080.enableMagnetometer(sendInterval);
    bno080.enableRawMagnetometer(sendInterval);
    bno080.enableRotationVector(sendInterval);
    bno080.calibrateAccelerometer();
    bno080.calibrateMagnetometer();

    Serial.println(F("Raw IMU Data Reads Enabled"));
    Serial.println(F("Output is [[X, Y, Z], [Rot Vel X, Rot Vel Y, Rot Vel Z], [Mag X, Mag Y, Mag Z], [Yaw, Pitch, Roll]]"));
}

void BNO080_IMU::readIMU()
{
    if (bno080.dataAvailable() == true)
    {
        bno080Data[0][0] = bno080.getAccelX();
        bno080Data[0][1] = bno080.getAccelY();
        bno080Data[0][2] = bno080.getAccelZ();

        bno080Data[1][0] = bno080.getGyroX() / PI * 180;
        bno080Data[1][1] = bno080.getGyroY() / PI * 180;
        bno080Data[1][2] = bno080.getGyroZ() / PI * 180;

        bno080Data[2][0] = bno080.getMagX();
        bno080Data[2][1] = bno080.getMagY();
        bno080Data[2][2] = bno080.getMagZ();

        bno080Data[3][0] = bno080.getYaw()   / PI * 180;
        bno080Data[3][1] = bno080.getPitch() / PI * 180;
        bno080Data[3][2] = bno080.getRoll()  / PI * 180;
    }
}

void BNO080_IMU::printIMU()
{
    readIMU();
    Serial.print("IMU Data: ");
    String labels[4] = {"Accel", "Gyro", "Mag", "Attitude"};
    for (int i = 0; i < 4; i++)
    {
        Serial.print(labels[i]);
        Serial.print(": ");
        for (int j = 0; j < 3; j++)
        {
            Serial.print(bno080Data[i][j]);
            if (j < 2)
                Serial.print(", ");
        }
        Serial.print("| ");
    }
    Serial.println();
}
