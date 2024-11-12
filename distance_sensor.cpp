#include "distance_sensor.h"

void initDistanceSensor(VL53L0X& sensor) {
  Wire.begin();
  sensor.init();
  sensor.setMeasurementTimingBudget(200000);
  sensor.setTimeout(500);
  Serial.println("VL53L0X-Sensor bereit!");
  Serial.print("Address: ");
  Serial.println(sensor.getAddress());
}

int getDistanceMillimeters(VL53L0X& sensor) {
  int distance = sensor.readRangeSingleMillimeters();
  if (sensor.timeoutOccurred()) {
    Serial.println("Timeout! ");
  }
  Serial.print("Millimeters: ");
  Serial.println(distance);
  return distance;
}