#ifndef THRUSTER_H
#define THRUSTER_H

#include <Arduino.h>

class PWM_Thruster
{
public:
    PWM_Thruster();
    PWM_Thruster(uint16_t pin, int pwmValue, int pwmOutFreq, float maxThrust);
    void setPWMValue(int newPWMValue);
    void setMaxThrust(float maxThrust);
    float getDesiredThrust();
    float getCurrentThrottle();
    void writePWM();
    void thrustToPWM();
    void fireThruster(float commandedThrust);
    void stopThruster();

private:
    uint16_t pin;
    int pwmValue = 0;
    unsigned long lastPWM = 0;
    unsigned long lastPWMChange = 0;
    int pwmOutfreq = 5;
    float desiredThrust = 0;
    float maxThrust = 1.0;
};

#endif // THRUSTER_H
