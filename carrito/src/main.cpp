#include <Arduino.h>
#include <SPI.h>
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

void setup()
{
  Serial.begin(115200);
  Serial.println("8 channel Servo test!");

  delay(100);
  // xTaskCreatePinnedToCore(luces, "luces", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(control, "control", 4096, NULL, 1, NULL, 1);
  //xTaskCreatePinnedToCore(motores, "motores", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(OTATask, "OTAtask", 4096, NULL, 1, NULL, 1);
}

void loop()
{
}