#include "IMU.h"
#include "LoadCell.h"
#include "Thruster.h"

bool thrusterArm = false;
LoadCell LC = LoadCell();
BNO080_IMU imu = BNO080_IMU();
PWM_Thruster thrusters[2];
unsigned long t = millis();
bool loadCellEnable = true;
float nozzleMomentArm = 0.1; // Nozzle moment arm length in meters (adjust as needed)

void setup()
{
    Serial.begin(57600);
    Serial.println();
    Serial.println("SACS IMU, Load Cell, and Thruster Test");

    Wire.begin();
    Wire.setClock(400000);

    // Initialize IMU
    imu.begin(50); // send interval of 50ms for IMU data updates

    // Initialize Load Cell with a calibration value (adjust as needed)
    LC.begin(410.84);

    // Initialize Thrusters
    thrusters[0] = PWM_Thruster(9, 0, 5, 1.0);
    thrusters[1] = PWM_Thruster(10, 0, 5, 1.0);
}

void serialIO()
{
    const int serialPrintInterval = 1000; // increase value to slow down serial print activity

    if (Serial.available() > 0)
    {
        char inByte = Serial.read();
        if (inByte == 't')
            LC.tareLoadCell();
        else if (inByte == 'r')
            LC.calibrateLC(); // calibrate
        else if (inByte == 'c')
            LC.changeSavedLCCalFactor(); // edit calibration value manually
        else if (inByte == 'a')
        {
            Serial.println("Thrusters Armed");
            thrusterArm = !thrusterArm;
            for (auto &thruster : thrusters)
            {
                if (!thrusterArm)
                    thruster.stopThruster(); // Stop thruster immediately if disarming
            }
        }
        else if (inByte == 'l')
        {
            loadCellEnable = !loadCellEnable;
            Serial.print("Load Cell Enabled: ");
            Serial.println(loadCellEnable);
        }
        else if (inByte == 'm')
        {
            for (auto &thruster : thrusters)
            {
                while (Serial.available() == 0)
                {
                    // Wait for user input to set max thrust
                }
                float inByte = Serial.parseFloat();
                Serial.print("Setting max thrust to: ");
                Serial.println(inByte);
                thruster.setMaxThrust(inByte); // Set the maximum thrust for the thruster
            }
        }
    }

    if (millis() > t + serialPrintInterval)
    {
        imu.printIMU();
        if (loadCellEnable)
        {
            LC.printLoadCellValue();
        }
    }
}

float readTheta()
{
    imu.readIMU();
    float theta = imu.bno080Data[3][0];
    return theta;
}

float readThetaDot()
{
    imu.readIMU();
    float thetaDot = imu.bno080Data[1][0];
    return thetaDot;
}

float calculateDesiredThrust(float desiredTheta, float currentTheta,
                             float currentThetaDot, float desiredThetaDot = 0,
                             float kp = 1, float kd = 1)
{
    float theta_error = desiredTheta - currentTheta;
    float theta_dot_error = desiredThetaDot - currentThetaDot;
    float u = kp * theta_error + kd * theta_dot_error;

    return u / nozzleMomentArm; // return the desired thrust based on the control output and the nozzle moment arm
}

void loop()
{
    serialIO();
    if (thrusterArm)
    {
        float desiredThrust = calculateDesiredThrust(0, readTheta(), readThetaDot()); // Example: desired angle is 0 radians
        if (desiredThrust > 0)
        {
            thrusters[0].fireThruster(desiredThrust); // Fire thruster 1 with the desired thrust
            thrusters[1].stopThruster();                // Ensure thruster 2 is stopped
        }
        else if (desiredThrust < 0)
        {
            thrusters[1].fireThruster(desiredThrust); // Fire thruster 2 with the desired thrust (negate for opposite direction)
            thrusters[0].stopThruster();                // Ensure thruster 1 is stopped
        }
        else
        {
            for (auto &thruster : thrusters)
            {
                thruster.stopThruster(); // Stop both thrusters if desired thrust is zero
            }
        }
    }
    for (auto &thruster : thrusters)
    {
        thruster.writePWM(); // Update the PWM signal for each thruster
    }
}