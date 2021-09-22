/**
 * @file AMG88XX.cpp
 * @author Thomas Novotny
 * @brief AMG88XX library for Arduino/Genuino v0.2
 * @version 0.2
 *
 * This library allows interfacing of an AMG88XX infrared thermopile array with
 * an Arduino board. It should be easily portable.
 *
 * Features supported:
 *   * Reading temperature values as °C
 *  * Reading internal thermistor value in °C
 *  * Setting power mode (Normal, Sleep, Stand-by)
 *  * Setting frame rate (1FPS or 10FPS)
 *  * Setting moving average processing option
 *  * Putting the AMG88xx into sleep mode
 *
 *  Features currently not supported:
 *   * Interrupt handling
 *
 */

#include "AMG88XX.h"

char amg_addr = 0x69;
float temps[64];

/**
 * @brief Resets module and puts all registers into their original boot up states.
 * Useful for debugging
 */
void AMG88XX_reset(){
  Wire.beginTransmission(amg_addr);
  Wire.write(AMG_REGISTER_RST);
  Wire.write(AMG_RST_INITIALRESET);
  Wire.endTransmission();
}

/**
 * @brief Writes the temperature values in °C into a float array.
 * 
 * @param result Float array of the size 64 used for storing results
 */
void readValues(float result[]){
  byte pixelAddrL = AMG_REGISTER_T01L;
  for (int pixel = 0; pixel < 64; pixel++) {
    delay(1);
    Wire.beginTransmission(amg_addr);
    Wire.write(pixelAddrL);
    Wire.endTransmission();
    Wire.requestFrom(amg_addr, 2);
    byte lowerLevel = Wire.read();
    byte upperLevel = Wire.read();

    int temperature = ((upperLevel << 8) | lowerLevel);
    if (temperature > 2047) {
      temperature = temperature - 4096;
    }
    result[pixel] = temperature * 0.25;
    pixelAddrL +=2;
  }
}

/**
 * @brief Calculates average of the 64 temperature values
 * 
 * @param data Array of 64 float values
 * @return Average value
 */
float calculateAverage(float data[]){
  float avg = 0.0;
  for (int i = 0; i < 64; i++) {
    avg += data[i];
  }
  return avg/64.0;
}

/**
 * @brief Calculates variance the 64 temperature values
 * 
 * @param data Array of 64 float values
 * @return Variance
 */
float calculateVariance(float data[]){
    float variance = 0.0;
    float avg = calculateAverage(data);
    for (int i = 0; i < 64; i++) {
      float temp = data[i] - avg;
      variance += temp*temp;
    }
    variance /= 64.0;
    return variance;
}

/**
 * @brief Sets AMG power mode
 * Arguments:
 *    AMG_PCTL_NORMAL - Normal operating mode
 *    AMG_PCTL_SLEEP - Sleep mode
 *    AMG_PCTL_STANDBY10 - Stand-by (10 sec intermittence)
 *    AMG_PCTL_STANDBY60 - Stand-by (60 sec intermittence)
 * 
 * @param powerMode Mode that the AMG will enter (AMG_PCTL_NORMAL, AMG_PCTL_SLEEP, AMG_PCTL_STANDBY10 or AMG_PCTL_STANDBY60)
 */
void setPowerMode(uint8_t powerMode){
  Wire.beginTransmission(amg_addr);
  Wire.write(AMG_REGISTER_PCTL);
  Wire.write(powerMode);
  Wire.endTransmission();
}

/**
 * @brief Puts AMG88XX into sleep mode
 */
void AMG88XX_sleep(){
  setPowerMode(AMG_PCTL_SLEEP);
}

/**
 * @brief Puts AMG88XX into 10s standby mode
 */
inline void standBy10(){
  setPowerMode(AMG_PCTL_STANDBY10);
}

/**
 * @brief Wakes up AMG88XX from sleep or standby mode
 */
void AMG88XX_wakeUp(){
  setPowerMode(AMG_PCTL_NORMAL);
  // TODO check if this is needed
  delay(100); // give module time to setup (min 50ms)
}

/**
 * @brief Sets the framerate of the AMG88XX
 * Arguments:
 *    AMG_FPSC_FRAMERATE_1  -  1 FPS
 *    AMG_FPSC_FRAMERATE_10 - 10 FPS
 * 
 * @param frameRate Either AMG_FPSC_FRAMERATE_1 or AMG_FPSC_FRAMERATE_10
 */
void setFrameRate(uint8_t frameRate){
  Wire.beginTransmission(amg_addr);
  Wire.write(AMG_REGISTER_FPSC);
  Wire.write(frameRate);
  Wire.endTransmission();
}

/**
 * @brief Returns the temperature of the internal thermistor in °C
 * 
 * @return temperature of the internal thermistor in °C
 */
float readThermistor(){
    // Request 2 bytes from 0x0E in order to read the thermistor value
  Wire.beginTransmission(amg_addr);
  Wire.write(AMG_REGISTER_TTHL);
  Wire.endTransmission();

    Wire.requestFrom(amg_addr, 2);
    byte lowerLevel = Wire.read();
    byte upperLevel = Wire.read();

    int raw = (upperLevel << 8) | lowerLevel;
    if (upperLevel & 0b00001000) raw = -(0x7FF & raw);

    return raw * 0.0625;
}

/**
 * @brief Sets twice moving average (true) or no moving average (false)
 * Default at startup is no moving average
 * 
 * @param movAvg true for moving average mode, false for normal mode
 */
void setMovingAverage(bool movAvg){
  // According to the data sheet, for some reason we have to write some values into address 0x1F
  Wire.beginTransmission(amg_addr);
  Wire.write(0x1F);
  Wire.write(0x50);
  Wire.endTransmission();

  Wire.beginTransmission(amg_addr);
  Wire.write(0x1F);
  Wire.write(0x45);
  Wire.endTransmission();

  Wire.beginTransmission(amg_addr);
  Wire.write(0x1F);
  Wire.write(0x57);
  Wire.endTransmission();

  // Write actual register value
  Wire.beginTransmission(amg_addr);
  Wire.write(AMG_REGISTER_AVE);
  Wire.write(movAvg?0x20:0x00);
  Wire.endTransmission();

  Wire.beginTransmission(amg_addr);
  Wire.write(0x1F);
  Wire.write(0x00);
  Wire.endTransmission();
}
