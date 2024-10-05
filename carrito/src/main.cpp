#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <luces.h>
#include <control.h>
#include <motores.h>
#include <OTA.h>
/***************************************************
  This is an example for our Adafruit 16-channel PWM & Servo driver
  Servo test - this will drive 8 servos, one after the other on the
  first 8 pins of the PCA9685

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These drivers use I2C to communicate, 2 pins are required to
  interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "CameraWebServer.h"

// our servo # counter
uint8_t servonum = 0;

#define I2C_DEV_ADDR 0x55
void i2c_scanner();
void handleSerial(void *params);

void setup()
{
  Serial.begin(115200);
  Serial.println("8 channel Servo test!");
  Wire.begin();

  delay(100);
  // xTaskCreatePinnedToCore(luces, "luces", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(control, "control", 4096, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(motores, "motores", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(OTATask, "OTAtask", 4096, NULL, 1, NULL, 1);
  xTaskCreate(handleSerial, "handleSerial", 2048, NULL, 1, NULL);
}

void loop()
{
  // i2c_scanner();
}

void i2c_scanner()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("No I2C devices found\n");
  }
  else
  {
    Serial.println("done\n");
  }
  delay(5000);
}

void handleSerial(void *params)
{
  while (true)
  {
    if (Serial.available() > 0)
    {
      String line = Serial.readStringUntil('\n');
      if (line.startsWith("SSID:"))
      {
        Serial.print(F("SSID:"));
        Wire.beginTransmission(I2C_DEV_ADDR);
        Wire.println(line);
        uint8_t error = Wire.endTransmission(true);
        if (error != 0)
          Serial.printf("endTransmission: %u\n", error);
        Wire.requestFrom(I2C_DEV_ADDR, 64);
        if (Wire.available())
        {
          String response = Wire.readStringUntil('\n');
          Serial.println(response);
        }
      }
      else if (line.startsWith("PASS:"))
      {
        Serial.print(F("PASS:"));
        Wire.beginTransmission(I2C_DEV_ADDR);
        Wire.println(line);
        uint8_t error = Wire.endTransmission(true);
        if (error != 0)
          Serial.printf("endTransmission: %u\n", error);
        Wire.requestFrom(I2C_DEV_ADDR, 64);
        if (Wire.available())
        {
          String response = Wire.readStringUntil('\n');
          Serial.println(response);
        }
      }
      else if (line.equals("RESTART"))
      {
        Wire.beginTransmission(I2C_DEV_ADDR);
        Wire.println("RESTART");
        uint8_t error = Wire.endTransmission(true);
        if (error != 0)
          Serial.printf("endTransmission: %u\n", error);
        Wire.requestFrom(I2C_DEV_ADDR, 4);
        if (Wire.available())
        {
          String response = Wire.readStringUntil('\n');
          Serial.println(response);
        }
      }
      else if (line.equals("IP"))
      {
        Serial.print(F("IP:"));
        Wire.beginTransmission(I2C_DEV_ADDR);
        Wire.println("IP");
        uint8_t error = Wire.endTransmission(true);
        if (error != 0)
          Serial.printf("endTransmission: %u\n", error);
        Wire.requestFrom(I2C_DEV_ADDR, 32);
        if (Wire.available())
        {
          String response = Wire.readStringUntil('\n');
          Serial.println(response);
        }
      }
      else if (line.startsWith("L:"))
      {
        Serial.print(F("L:"));
        Wire.beginTransmission(I2C_DEV_ADDR);
        Wire.write(line.substring(2).equals("1") ? "L:1\n" : "L:0\n");
        uint8_t error = Wire.endTransmission(true);
        if (error != 0)
          Serial.printf("endTransmission: %u\n", error);
        Wire.requestFrom(I2C_DEV_ADDR, 4);
        if (Wire.available())
        {
          String response = Wire.readStringUntil('\n');
          Serial.println(response);
        }
      }
      else
      {
        Serial.print(F("COMANDO_INVALIDO:"));
        Serial.println(line);
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
