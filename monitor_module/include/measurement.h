#pragma once
#include "global_vars.h"
#include <Wire.h>
#include <time.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>

#define I2C_SCL 26
#define I2C_SDA 25

void Measurement_Init(void);
void Measure(void);