#include <arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <math.h>
#include <HTTPClient.h>

#define Addr                0x5D
#define LPS25HB_WHO_AM_I    0x0F
#define LPS25HB_OK          0xBD

#define LPS25HB_RPDS_L 			0x39
#define LPS25HB_RPDS_H 			0x3A

void LPS25HB_init();
void LPS25HB_set_calib(uint16_t value);

void PrintData(float pressure, float temperature, float height);

float ReadPressure();
float ReadTemp();
float HeightCalc(float pressure, float temperature, float callibPressure);
float parseJsonData(const char * jsonString);
float downloadPressure();