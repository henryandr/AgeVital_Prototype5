#include <Arduino.h>
#include <WebServer.h>
#include <EEPROM.h>

#include "ESPaccesspoint.h"
#include "Settings.h"

WebServer server(80);
Settings settings;

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(4096);
    settings.load();
    settings.info();
    startAPorSTA(settings);
}

void loop()
{
    if (!is_STA_mode())
        server.handleClient();
}