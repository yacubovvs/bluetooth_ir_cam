#ifndef _Arduino_AMG88XX_H_
#define _Arduino_AMG88XX_H_
#include "Arduino.h"
#include "Wire.h"

// Register addresses
#define AMG_REGISTER_PCTL    0x00  // Power control register
#define AMG_REGISTER_RST    0x01  // Reset Register
#define AMG_REGISTER_FPSC   0x02  // Frame rate register
#define AMG_REGISTER_INTC   0x03  // Interrupt control register
#define AMG_REGISTER_AVE    0x07  // Interrupt control register
#define AMG_REGISTER_T01L   0x80  // Pixel 1 Output Value (Lower Level)
#define AMG_REGISTER_T01H   0x81  // Pixel 1 Output Value (Higher Level)
#define AMG_REGISTER_TTHL   0x0E  // Thermistor Output Value (Lower level)
#define AMG_REGISTER_TTHH   0x0F  // Thermistor Output Value (Higher level)

// Power control register values
#define AMG_PCTL_NORMAL     0x00  // Normal operating mode
#define AMG_PCTL_SLEEP      0x10  // Sleep mode
#define AMG_PCTL_STANDBY10    0x21  // Stand-by (10 sec intermittence)
#define AMG_PCTL_STANDBY60    0x20  // Stand-by (60 sec intermittence)

// Reset register values
#define AMG_RST_FLAGRESET   0x30  // Flag Reset
#define AMG_RST_INITIALRESET  0x30  // Flag Reset

// Frame Rate control register values
#define AMG_FPSC_FRAMERATE_10   0x00  // 10 FPS measurement
#define AMG_FPSC_FRAMERATE_1  0x01  // 1 FPS Measurement

// Functions
void AMG88XX_sleep();
void AMG88XX_wakeUp();
void AMG88XX_reset();
void setMovingAverage(bool movAvg);
void setFrameRate(uint8_t frameRate);
inline void standBy10();
void readValues(float result[]);
float readThermistor();
float calculateVariance(float data[]);
float calculateAverage(float data[]);

#endif
