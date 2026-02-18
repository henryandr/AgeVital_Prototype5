#ifndef ESTADOS_H
#define ESTADOS_H

#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>

#include "AppConfig.h"
#include "State.h"
#include "StateMachine.h"

// ===== DECLARACIONES EXTERNAS =====
extern void readSensors();
extern void updateDisplay();
extern void displayStateInfo(const char* estado);
extern void displayDeveloperInfo();
extern Adafruit_SSD1306 display;
extern WebServer server;
extern String construirPayload(float temperatura, float humedad, float luz, float ruido);

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