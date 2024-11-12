#pragma once

#include <Arduino.h>

struct SensorData
{
    float pressure;
    float altitude;
    float temperature;
    float gas;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

struct FileData
{
    size_t filesize;
    String filename;
    size_t free_space;
};

struct TimeDateData
{
    uint16_t Year;
    uint8_t Day;
    uint8_t Month;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
};

String formatISO8601(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour, const uint8_t minute, const uint8_t second);
String formatISO8601(const TimeDateData & time_date_data);
String getJSON(const SensorData & data);
String getJSON(const FileData &data);
String getFirstCSVLine();