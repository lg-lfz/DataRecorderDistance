#pragma once

#include <Arduino.h>
#include <VL53L0X.h>

void initDistanceSensor(VL53L0X& sensor);
int getDistanceMillimeters(VL53L0X& sensor);
