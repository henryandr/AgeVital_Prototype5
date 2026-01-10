#ifndef DEVWEBOTA_H
#define DEVWEBOTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>

class DevWebOTA {
    private: 
    WebServer* server;
    Preferences prefs;
    bool initialized;

    const char* apSSID = "ESP-HOTPOT"; 
    const char* apPass = "12345678";



    public:
    
    DevWebOTA(WebServer* srv);

    void begin();
    void handle();
    bool isConfigured();
};

#endif
