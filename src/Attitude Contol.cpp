#include "IMU.h"
// #include "LoadCell.h"
#include "Thruster.h"

bool thrusterArm = true;
// LoadCell LC = LoadCell();
BNO080_IMU imu = BNO080_IMU();
PWM_Thruster thrusters[2];
unsigned long t = millis();
bool loadCellEnable = true;
float nozzleMomentArm = 0.1; // Nozzle moment arm length in meters (adjust as needed)
u_int16_t solenoid1_pin = 3; // Pin for thruster 1
u_int16_t solenoid2_pin = 4; // Pin for thruster 2

void setup()
{
    // while (!Serial)
    //     delay(10);
    Serial.begin(57600);
    Serial.println();
    Serial.println("SACS IMU, Load Cell, and Thruster Test");

    Wire.begin();
    Wire.setClock(400000);

    // Initialize IMU
    imu.begin(50); // send interval of 50ms for IMU data updates

    // Initialize Load Cell with a calibration value (adjust as needed)
    // LC.begin(410.84);

    // Initialize Thrusters
    thrusters[0] = PWM_Thruster(solenoid1_pin, 0, 5, 1.0);
    thrusters[1] = PWM_Thruster(solenoid2_pin, 0, 5, 1.0);
}

void serialIO()
{
    const int serialPrintInterval = 1000; // increase value to slow down serial print activity

    if (Serial.available() > 0)
    {
        char inByte = Serial.read();
        // if (inByte == 't')
        //     LC.tareLoadCell();
        // else if (inByte == 'r')
        //     LC.calibrateLC(); // calibrate
        // else if (inByte == 'c')
        //     LC.changeSavedLCCalFactor(); // edit calibration value manually
        // else if (inByte == 'a')
        if (inByte == 'a')
        {
            thrusterArm = !thrusterArm;
            for (auto &thruster : thrusters)
            {
                if (!thrusterArm)
                {
                    Serial.println("Thrusters disarmed. Stopping all thrusters immediately...");
                    thruster.stopThruster(); // Stop thruster immediately if disarming
                }
                else
                    Serial.println("Thrusters armed. Ready to fire.");
            }
        }
        // else if (inByte == 'l')
        // {
        //     loadCellEnable = !loadCellEnable;
        //     Serial.print("Load Cell Enabled: ");
        //     Serial.println(loadCellEnable);
        // }
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
        t = millis();
        // if (loadCellEnable)
        // {
        //     LC.printLoadCellValue();
        // }
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

void fireThrusters()
{
    float desiredThrust = calculateDesiredThrust(0, readTheta(), readThetaDot(), 0, 0.002, 0.002); // Example: desired angle is 0 radians
    if (desiredThrust > 0)
    {
        // Serial.println("Firing Thruster 1 with Desired Thrust: " + String(desiredThrust));
        thrusters[0].fireThruster(desiredThrust); // Fire thruster 1 with the desired thrust
        thrusters[1].stopThruster();              // Ensure thruster 2 is stopped
    }
    else if (desiredThrust < 0)
    {
        // Serial.println("Firing Thruster 2 with Desired Thrust: " + String(desiredThrust));
        thrusters[1].fireThruster(desiredThrust); // Fire thruster 2 with the desired thrust (negate for opposite direction)
        thrusters[0].stopThruster();              // Ensure thruster 1 is stopped
    }
    else
    {
        for (auto &thruster : thrusters)
        {
            thruster.stopThruster(); // Stop both thrusters if desired thrust is zero
        }
    }
    for (auto &thruster : thrusters)
    {
        thruster.writePWM(); // Update the PWM signal for each thruster
    }
}

void loop()
{
    imu.readIMU();
    serialIO();
    if (thrusterArm)
    {
        fireThrusters();
    }
}