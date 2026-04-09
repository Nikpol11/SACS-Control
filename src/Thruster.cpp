#include "Thruster.h"

PWM_Thruster::PWM_Thruster(uint16_t pin, int pwmValue, int pwmOutFreq, float maxThrust)
{
    this->pin = pin;
    this->pwmValue = pwmValue;
    this->pwmOutfreq = pwmOutFreq;
    this->maxThrust = maxThrust;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
};

PWM_Thruster::PWM_Thruster()
{
    pin = 0;
    pwmValue = 0;
    pwmOutfreq = 5;
    maxThrust = 1.0;
}

void PWM_Thruster::setPWMValue(int newPWMValue)
{
    this->pwmValue = newPWMValue;
};

void PWM_Thruster::setMaxThrust(float maxThrust)
{
    this->maxThrust = maxThrust;
    thrustToPWM(); // Update PWM value based on the new maximum thrust
};

void PWM_Thruster::fireThruster(float commandedThrust)
{
    this->desiredThrust = abs(commandedThrust);
    thrustToPWM(); // Convert desired thrust to corresponding PWM value and update
    writePWM();    // Command thruster to fire with the new PWM value
};

float PWM_Thruster::getDesiredThrust()
{
    return this->desiredThrust;
};

float PWM_Thruster::getCurrentThrottle()
{
    return (this->pwmValue / 255.0); // Return current throttle as a value between 0 and 1 based on the current PWM value
};

void PWM_Thruster::thrustToPWM()
{
    // Convert desired thrust to corresponding PWM value based on the maximum thrust
    if (desiredThrust > maxThrust)
        desiredThrust = maxThrust; // Cap desired thrust at maximum
    else if (desiredThrust < 0)
        desiredThrust = 0; // Ensure desired thrust is not negative

    this->pwmValue = static_cast<int>((desiredThrust / maxThrust) * 255); // Scale desired thrust to a PWM value between 0 and 255
};

void PWM_Thruster::stopThruster()
{
    this->desiredThrust = 0;
    this->setPWMValue(0); // Set PWM value to 0 to stop the thruster
    writePWM();           // Command thruster to stop immediately
};

void PWM_Thruster::writePWM()
{
    int freqDelay = 1000 / pwmOutfreq;
    unsigned int dutyTime = digitalRead(pin) == LOW ? ((pwmValue / 256.0) * freqDelay) : ((256 - pwmValue) / 256.0) * freqDelay;
    unsigned long currentTime = millis();
    if (currentTime - lastPWM >= dutyTime)
    {
        lastPWM = currentTime;
        digitalWrite(pin, !digitalRead(pin)); // Toggle the pin state
    }
};