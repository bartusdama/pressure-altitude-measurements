#include "LPS25HB.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <HTTPClient.h>


void LPS25HB_init() {
    // Initialise I2C communication as MASTER 
    Wire.begin();
    Serial.begin(115200);

    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select control register 1
    Wire.write(0x20);
    // Set active mode, continuous Update
    Wire.write(0xC0);
    // Stop I2C Transmission
    Wire.endTransmission();

    Wire.beginTransmission(Addr);
    Wire.write(LPS25HB_WHO_AM_I);
    Wire.endTransmission();
    Wire.requestFrom(Addr, 1);
    if (Wire.available() == 1)
    {
      if (Wire.read() == LPS25HB_OK)
        Serial.print("Found LPS25HB\n");
    }
}

float ReadTemp()
{
  int8_t temp[2];

  Wire.beginTransmission(Addr);
  Wire.write(0x2B | 0x80);
  Wire.endTransmission();

  Wire.requestFrom(Addr, 2);

  if (Wire.available() == 2)
  {
    temp[0] = Wire.read();
    temp[1] = Wire.read();
  }

  return 42.5 + ((temp[1] * 256) + temp[0]) / 480.0;
}

float ReadPressure()
{
  int8_t press[3];

  Wire.beginTransmission(Addr);
  Wire.write(0x28 | 0x80);
  Wire.endTransmission();
  
  Wire.requestFrom(Addr, 3);

  if(Wire.available() == 3)
  {
    press[0] = Wire.read();
    press[1] = Wire.read();
    press[2] = Wire.read();
  }

  return ((press[2] * 65536) + (press[1] * 256) + press[0]) / 4096.0;
}

void PrintData(float pressure, float temperature, float height)
{
  Serial.print("Pressure is : ");
  Serial.print(pressure);
  Serial.println(" hPa\n");
  Serial.print("Temperature is : ");
  Serial.print(temperature);
  Serial.println(" *C\n");
  Serial.print("Height is : ");
  Serial.print(height);
  Serial.println(" m\n");
}

float HeightCalc(float pressure, float temperature, float callibPressure)
{
  float tempK = temperature + 273.15;
  return -29.271769 * tempK * log(pressure / callibPressure);
}

void PS25HB_set_calib(uint16_t value)
{
  Wire.beginTransmission(LPS25HB_RPDS_L);
  Wire.write(value);
  Wire.endTransmission();

  Wire.beginTransmission(LPS25HB_RPDS_H);
  Wire.write(value >> 8);
  Wire.endTransmission();
}

float parseJsonData(const char * jsonString)
{
  DynamicJsonDocument jsonBuffer(10000);
  auto error = deserializeJson(jsonBuffer, jsonString);
  if (error) {
    Serial.println("Deserialize error");
    Serial.println(error.c_str());
    return -1;
  }
  else
  {
    Serial.println("Deserialize OK!");
  }
  size_t len = measureJson(jsonBuffer);
  Serial.println(len);
  JsonArray list = jsonBuffer["current"];
  String fromDateTime = jsonBuffer["current"]["fromDateTime"];
  String tillDateTime = jsonBuffer["current"]["tillDateTime"];
  //----------------------------------------------------------------
  String namePressure = jsonBuffer["current"]["values"][3]["name"];
  String valuePressure = jsonBuffer["current"]["values"][3]["value"];
  Serial.println("----------------------------");
  /*Current fromDateTime display */
  Serial.println("fromDateTime: " + fromDateTime);    
  /*Current tillDateTime display */  
  Serial.println("tillDateTime: " + tillDateTime);
  /*Current values display */
  Serial.println("*****************");
  Serial.println(namePressure + " - " + valuePressure);
  float pressure = valuePressure.toFloat();

  return pressure;
}

float downloadPressure()
{
  float callibPressure;
  const String ENDPOINT_STRING = "endpoint_string";
  const String API_KEY = "api_key";
  String jsonStringText;

      if ((WiFi.status() == WL_CONNECTED))
    {
    HTTPClient http;
    http.begin(ENDPOINT_STRING + API_KEY); //Specify the URL
    int httpResponseCode = http.GET();  //Make the request
    if (httpResponseCode > 0)
    {
        jsonStringText = http.getString();
        Serial.println(httpResponseCode);
        callibPressure = parseJsonData(jsonStringText.c_str());
    }
    else
    {
        Serial.println("Http Request Error");
    }
    http.end();
    }

    return callibPressure;
}
