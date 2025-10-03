#ifndef ESPaccesspoint_h
#define ESPaccesspoint_h

#include <WiFi.h>
#include <WebServer.h>
#include "data.h"
#include "Settings.h"

// Declaraciones de las funciones
void startAPorSTA(Settings &settings);
bool is_STA_mode();
void handleServer(WebServer &server, Settings &settings);
void STA_mode_onRst();
void AP_mode_onRst();

#endif
