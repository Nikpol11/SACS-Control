#include <Arduino.h>
#include <SparkFun_BNO080_Arduino_Library.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

BNO080 bno080;
const int pwmOut_1 = 9;
const int HX711_dout = 3; //mcu > HX711 dout pin, must be external interrupt capable!
const int HX711_sck = 5; //mcu > HX711 sck pin
const int button1Pin = 8;
const int button2Pin = 7;

int pwmValue = 128;
int pwmDirection = 1;
unsigned long lastPWM = 0;
unsigned long lastPWMChange = 0;
int pwmOut_1_freq = 20; //frequency in Hz of the PWM signal to be generated on pwmOut_1 pin, increase value to decrease frequency

float bno080Data[3][3] = {{0,0,0},{0,0,0},{0,0,0}};

HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;
unsigned long t = 0;
volatile boolean newDataReady;

void loadCellDataReadyISR() {
  if (LoadCell.update()) {
    newDataReady = 1;
  }
}

void setup() {
  pinMode(pwmOut_1, OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);

  digitalWrite(pwmOut_1, LOW);

  Serial.begin(57600);
  Serial.println();
  Serial.println("SACS bno080, HX711_ADC Test");

  // Wire.begin();

  // Wire.setClock(400000);

  // if (!bno080.begin(0x4A)){
  //   while (1) {
  //     Serial.println(F("bno080 not detected at address 0x4A. freezing..."));
  //     delay(5000);
  //   }
  // } else {
  //   Serial.println(F("IMU Connection Sucessful"));
  //   delay(1000);
  // }

  // bno080.enableAccelerometer(50);    //We must enable the accel in order to get MEMS readings even if we don't read the reports.
  // bno080.enableRawAccelerometer(50); //Send data update every 50ms
  // bno080.enableGyro(50);
  // bno080.enableRawGyro(50);
  // bno080.enableMagnetometer(50);
  // bno080.enableRawMagnetometer(50);
  // bno080.calibrateAccelerometer();
  // bno080.calibrateMagnetometer();

  // Serial.println(F("Raw Data Reads Enabled"));
  // Serial.println(F("Output is [[X, Y, Z], [Rot X, Rot Y, Rot Z], [Mag X, Mag Y, Mag Z]]"));

  // float calibrationValue; // calibration value
  // calibrationValue = 410.84; //696.0; // uncomment this if you want to set this value in the sketch

  // LoadCell.begin();
  // unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  // boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  // LoadCell.start(stabilizingtime, _tare);
  // if (LoadCell.getTareTimeoutFlag()) {
  //   Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  //   while (1);
  // }
  // else {
  //   LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
  //   Serial.println("Startup is complete");
  // }

  // attachInterrupt(digitalPinToInterrupt(HX711_dout), loadCellDataReadyISR, FALLING);

}

void calibrateLC() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  // LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  // Serial.println("Refreshed Data Set");
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (inByte == 'n') {
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

void changeSavedLCCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}

void pwmWrite(int pin, int duty_cycle, int frequency) {
  int freqDelay = 1000/frequency;
  unsigned int dutyTime = digitalRead(pin) == LOW ? ((duty_cycle/256.0)*freqDelay) : ((256-duty_cycle)/256.0)*freqDelay;
  // Serial.print("freq: ");
  // Serial.println(freqDelay);
  // Serial.print("duty time: ");
  // Serial.println(dutyTime);
  unsigned long currentTime = millis();
  if (currentTime - lastPWM >= dutyTime) {
    lastPWM = currentTime;
    digitalWrite(pin, !digitalRead(pin)); // Toggle the pin state
  }
}

// void readIMU(){
//   if (bno080.dataAvailable() == true){
//     bno080Data[0][0] = bno080.getAccelX();
//     bno080Data[0][1] = bno080.getAccelY();
//     bno080Data[0][2] = bno080.getAccelZ();

//     bno080Data[1][0] = bno080.getRawGyroX()/PI*180;
//     bno080Data[1][1] = bno080.getRawGyroY()/PI*180;
//     bno080Data[1][2] = bno080.getRawGyroZ()/PI*180;

//     bno080Data[2][0] = bno080.getMagX();
//     bno080Data[2][1] = bno080.getMagY();
//     bno080Data[2][2] = bno080.getMagZ();
//   }
// }

// void printIMU(){
//   readIMU();
//   for (int i = 0; i < 3; i++) {
//     for (int j = 0; j < 3; j++) {
//       Serial.print(bno080Data[i][j]);
//       if (j < 2) Serial.print(", ");
//     }
//     Serial.print("| ");
//   }
//   Serial.println();
// }

void loop() {
  const int serialPrintInterval = 1000; //increase value to slow down serial print activity

  // if (Serial.available() > 0) {
  //   char inByte = Serial.read();
  //   if (inByte == 't') LoadCell.tareNoDelay();
  //   else if (inByte == 'r') calibrateLC(); //calibrate
  //   else if (inByte == 'c') changeSavedLCCalFactor(); //edit calibration value manually
  // }

  // //check if last tare operation is complete
  // if (LoadCell.getTareStatus() == true) {
  //   Serial.println("Tare complete");
  // }

  // int button1State = digitalRead(button1Pin);
  // int button2State = digitalRead(button2Pin);

  // if (button1State == HIGH) {
  //   analogWrite(pwmOut_1, pwmValue);
  // } else {
  //   analogWrite(pwmOut_1, 0);
  // }
  // if (button2State == HIGH) {
  //   analogWrite(pwmOut_2, 0);
  // }

  pwmWrite(pwmOut_1, pwmValue, pwmOut_1_freq);

  if (millis() - lastPWMChange >= 100) {
    lastPWMChange = millis();
    pwmValue+=5*pwmDirection;
    if (pwmValue >= 255 || pwmValue <= 0) {
      pwmDirection*=-1;
      pwmValue = max(0, min(255, pwmValue));
    }
    Serial.print("PWM value: ");
    Serial.println(pwmValue);
  }
}