#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H
#include <Arduino.h>

void drawAllSensors();
void updateDisplay();
void displayStateInfo(const char* estado);
void displayDeveloperInfo();

// Nuevas funciones para el menú de WiFi interactivo
void displayWiFiList(int selected, int offset, int total);
void displayAPAlert();

// Pantalla de arranque, se muestra en setup() antes de intentar conectar
// WiFi para no dejar el OLED con contenido basura durante esa espera.
void displaySplashScreen();

#endif
