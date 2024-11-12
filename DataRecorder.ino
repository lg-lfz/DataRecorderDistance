/*
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>

#include <LittleFS.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "distance_sensor.h"
#include "applicationstate.h"
#include "data.h"
#include "file.h"
#include "html.h"

constexpr const auto SCL_PIN = D1;
constexpr const auto SDA_PIN = D2;

constexpr const auto DNS_PORT = 53;
constexpr const char *DATA_FILENAME = "data.csv";
constexpr const char *BASE_SSID = "DataRecorder";

constexpr const long interval1 = 500;
constexpr const long interval2 = 1000;

unsigned long previousMillis1 = 0; // Stores the last time an event occurred
unsigned long previousMillis2 = 0; // Stores the last time an event occurred


DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

File file;
String index_html = "";
ApplicationState appState;

void setup()
{
  appState.dataRecording = false;
  pinMode(LED_BUILTIN, OUTPUT);
  setlocale(LC_ALL, "de_DE.UTF-8");
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(2000);
  Serial.println("Hello console...");

  initFileSystem();
  listDir("/", 0);

  initAccessPoint(IPAddress(10, 10, 10, 10), IPAddress(10, 10, 10, 10), IPAddress(255, 255, 255, 0), BASE_SSID);
  initWebServerWithSocket();

  initDistanceSensor(sensor);
}

void loop()
{
  unsigned long currentMillis = millis();
  SensorData data;
  // Task 1 -> every 500ms
  if (currentMillis - previousMillis1 >= interval1)
  {
    // Save the last time task 1 was executed
    previousMillis1 = currentMillis;

    // Perform task 1
    auto data = collectData();
    sendDataToClient(data);
  }

  // Task 2 -> every 1000ms
  if (currentMillis - previousMillis2 >= interval2)
  {
    // Save the last time task 2 was executed
    previousMillis2 = currentMillis;

    // Perform task 2
    if (appState.dataRecording)
    {
      auto file_data = storeData(data, appState);
      sendDataToClient(file_data);
    }
  }
}


SensorData collectData()
{
  SensorData data = {};
  data.temperature = getDistanceMillimeters(sensor);
  return data;
}

void print2digits(int number)
{
  if (number >= 0 && number < 10)
  {
    Serial.write('0');
  }
  Serial.print(number);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    DynamicJsonDocument doc(len);
    DeserializationError error = deserializeJson(doc, (char *)data);
    if (doc.containsKey("action") && doc["action"].as<String>().equals("toggle"))
    {
      appState.dataRecording = !appState.dataRecording;

      if (!appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stop\"}");
      }
      else
      {
        // Always create new file...
        writeFile(DATA_FILENAME, getFirstCSVLine().c_str());
        ws.textAll("{\"state\":\"start\"}");
      }
    }
    else if (doc.containsKey("action") && doc["action"].as<String>().equals("request"))
    {
      if (!appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stopped\"}");
      }
      else
      {
        ws.textAll("{\"state\":\"started\"}");
      }
    }
    else if (doc.containsKey("action") && doc["action"].as<String>().equals("format"))
    {
      // stop recording
      if (appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stop\"}");
      }
      // format LittleFS Filesystem !Reboot!
      formatFileSystem();
    }
    else if (doc.containsKey("date_time"))
    {
      String iso8601DateTime = doc["date_time"].as<String>();
      //setDateTimeFromISO8601(iso8601DateTime.c_str(), rtc);
      ws.textAll("{\"date_time\":\"success\"}");
    }
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    Serial.printf("Data received: %s\n", data);
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    Serial.printf("WebSocket Error: %s\n", (char *)arg);
    client->close();
    break;
  }
}

void initWebServerWithSocket()
{
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", htmlContent); });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, DATA_FILENAME, String(), true); });
  server.begin();
}

template <typename T>
void sendDataToClient(const T &data)
{
  ws.printfAll(getJSON(data).c_str());
}

FileData storeData(const SensorData &data, const ApplicationState &appState)
{
  FileData file_data;
  file_data.filename = DATA_FILENAME;
  digitalWrite(LED_BUILTIN, HIGH);
  char buffer[255];
  // Format the date and time into ISO 8601 format
  snprintf(buffer, sizeof(buffer), "%0.2f;%0.2f;%0.2f;%0.2f;%s;", data.altitude, data.pressure, data.temperature, data.gas, formatISO8601(data.year, data.month, data.day, data.hour, data.minute, data.second).c_str());
  file_data.filesize = appendFile(DATA_FILENAME, buffer);
  file_data.free_space = getAvalibleDiskSpace();
  // readFile(DATA_FILENAME); //just for debug purpose...
  digitalWrite(LED_BUILTIN, LOW);
  return file_data;
}

void initAccessPoint(const IPAddress &localIP, const IPAddress &gatewayIP, const IPAddress &netmask, const String &ssid)
{
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", localIP);
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAPConfig(localIP, gatewayIP, netmask))
  {
    Serial.println("Wifi configuration not successful.");
  }
  auto ip = WiFi.softAPIP();
  String mac = WiFi.macAddress();
  Serial.printf("Mac: %s, IP: %s\n", mac.c_str(), ip.toString().c_str());
  mac.replace(":", "");
  auto custom_ssid = ssid + "-" + mac;
  Serial.printf("SSID: %s\n", custom_ssid.c_str());
  WiFi.softAP(custom_ssid);
}
*/
/* This example shows how to get single-shot range
 measurements from the VL53L0X. The sensor can optionally be
 configured with different ranging profiles, as described in
 the VL53L0X API user manual, to get better performance for
 a certain application. This code is based on the four
 "SingleRanging" examples in the VL53L0X API.

 The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;


// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

#define HIGH_SPEED
//#define HIGH_ACCURACY


void setup()
{
  Serial.begin(115200);
  Wire.begin();

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif
}

void loop()
{
  Serial.print(sensor.readRangeSingleMillimeters());
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  Serial.println();
}