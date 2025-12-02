#ifndef ESTADOS_H
#define ESTADOS_H

#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_SSD1306.h>

#include "ESPaccesspoint.h"
#include "Settings.h"
#include "State.h"
#include "StateMachine.h"

// ===== DECLARACIONES EXTERNAS =====
extern void readSensors();
extern void updateDisplay();
extern void displayStateInfo(const char* estado);
extern void displayDeveloperInfo();
extern void startAPorSTA(Settings& settings);
extern Adafruit_SSD1306 display;
extern WebServer server;
extern Settings settings;
extern String construirJson(float temperatura, float humedad, float luz, float ruido);

#define DEV_PIN 26

// ========================================
// ESTADO INICIO
// ========================================
class EstadoINICIO : public State {
 private:
  bool firstRun = true;

 public:
  void onEnter() override;
  void execute() override;
  void onExit() override;
  const char* getName() override;
};

// ========================================
// ESTADO LECTURA
// ========================================
class EstadoLECTURA : public State {
 public:
  void onEnter() override;
  void execute() override;
  void onExit() override;
  const char* getName() override;
};

// ========================================
// ESTADO ENVIO
// ========================================
class EstadoENVIO : public State {
 private:
  const char* ServerName = "http://10.38.32.137:1026/v2/entities/AmbientMonitor_001/attrs";

 public:
  void onEnter() override;
  void execute() override;
  void onExit() override;
  const char* getName() override;
};

// ========================================
// ESTADO DESARROLLADOR
// ========================================
class EstadoDESARROLLADOR : public State {
 private:
  bool primera_vez = true;

 public:
  void onEnter() override;
  void execute() override;
  void onExit() override;
  const char* getName() override;
};

#endif