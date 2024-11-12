#include "data.h"

// Get ISO8601 Time Format: 2024-07-12T13:00:42Z https://en.wikipedia.org/wiki/ISO_8601
String formatISO8601(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour, const uint8_t minute, const uint8_t second)
{
    char buffer[20];
    // Format the date and time into ISO 8601 format
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d",
             year, month, day, hour, minute, second);
    return String(buffer);
}

// Get ISO8601 Time Format: 2024-07-12T13:00:42Z https://en.wikipedia.org/wiki/ISO_8601
String formatISO8601(const TimeDateData &time_date_data)
{
    return formatISO8601(time_date_data.Year, time_date_data.Month, time_date_data.Day, time_date_data.Hour, time_date_data.Minute, time_date_data.Second);
}

String getJSON(const SensorData &data)
{
    char buffer[255];
    // Format the date and time into ISO 8601 format
    snprintf(buffer, sizeof(buffer), "{\"altitude\": %0.2f, \"pressure\": %0.2f, \"temperature\": %0.2f, \"gas\": %0.2f, \"date_time_ISO8601\": \"%s\"}", data.altitude, data.pressure, data.temperature, data.gas, formatISO8601(data.year, data.month, data.day, data.hour, data.minute, data.second).c_str());
    return String(buffer);
}

String getJSON(const FileData &data)
{
    char buffer[255];
    // Format the file data
    snprintf(buffer, sizeof(buffer), "{\"freespace\": %d, \"filesize\": %d, \"filename\": \"%s\"}", data.free_space, data.filesize, data.filename.c_str());
    return String(buffer);
}

String getFirstCSVLine()
{
    return "altitude;pressure;temperature;gas;date_time";
}