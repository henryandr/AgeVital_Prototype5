#ifndef DEVWEBOTA_H
#define DEVWEBOTA_H

#include <Arduino.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>

class DevWebOTA {
 private:
  WebServer* server;
  Preferences prefs;
  bool initialized;

  const char* apSSID = "ESP-HOTSPOT";
  const char* apPass = "12345678";

  const char* defaultSSID = "Claro_2C06BE";
  const char* defaultPass = "16652524";

 public:
  DevWebOTA(WebServer* srv);

  void begin();
  void handle();
  bool isConfigured();
};

#endif
