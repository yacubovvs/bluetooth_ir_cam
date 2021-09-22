
#include "Arduino.h"
#include "AMG88XX.h"

char addr = 0x69;

void setup() {
  Wire.setClock(800000);
  Wire.begin();

  // Initialize UART with 115200 baud
  Serial.begin(115200);

  AMG88XX_reset(); 
  setFrameRate(AMG_FPSC_FRAMERATE_10);

  //pinMode(13, OUTPUT);
  AMG88XX_wakeUp();
  //delay(5000);

}

void loop() {
  //while(Serial.read()==-1); // Wait for input

  //AMG88XX_wakeUp();
  //delay(15000); // AMG needs ~15 seconds to stabilize after waking up

  float data[64];

  readValues(data);

  //float variance = calculateVariance(data);
  for (int pixel = 0; pixel < 64; pixel++) {
    Serial.print(data[pixel]);
    Serial.print(",");
  }
  Serial.print(";");
  Serial.print(readThermistor());
  Serial.println();

  //digitalWrite(13, variance>0.8);

  //AMG88XX_sleep();

  //standBy10();
  //delay(1000);
}
