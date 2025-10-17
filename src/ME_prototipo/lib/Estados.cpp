#ifndef ESTADOS_CPP
#define ESTADOS_CPP

#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>

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
  void onEnter() override {
    Serial.println("===Entrando en Estado INICIO===");
    statemachine->flags.inicio = true;
    firstRun = true;
  }

  void execute() override;  // ← Declaración (implementación más abajo)

  void onExit() override {
    Serial.println("===Saliendo de Estado INICIO===");
    statemachine->flags.inicio = false;
    firstRun = false;
  }

  const char* getName() override { return "INICIO"; }
};

// ========================================
// ESTADO LECTURA
// ========================================
class EstadoLECTURA : public State {
 public:
  void onEnter() override {
    Serial.println("===Entrando en Estado LECTURA===");
    statemachine->flags.lectura = true;
    statemachine->flags.inicio = false;
    statemachine->flags.envio = false;
  }

  void execute() override;  // ← Declaración (implementación más abajo)

  void onExit() override {
    Serial.println("===Saliendo de Estado LECTURA===");
    statemachine->flags.lectura = false;
  }

  const char* getName() override { return "LECTURA"; }
};

// ========================================
// ESTADO ENVIO
// ========================================
class EstadoENVIO : public State {
 private:
  const char* ServerName = "http://10.38.32.137:1026/v2/entities/AmbientMonitor_001/attrs";

 public:
  void onEnter() override {
    Serial.println("=== ENTRANDO A ESTADO: ENVIO ===");
    statemachine->flags.envio = true;
    statemachine->flags.inicio = false;
    statemachine->flags.lectura = false;

    display.clearDisplay();
    displayStateInfo("ENVIO");
    display.display();
  }

  void execute() override;  // ← Declaración (implementación más abajo)

  void onExit() override {
    Serial.println("=== SALIENDO DE ESTADO: ENVIO ===");
    statemachine->flags.envio = false;
  }

  const char* getName() override { return "ENVIO"; }
};

// ========================================
// ESTADO DESARROLLADOR
// ========================================
class EstadoDESARROLLADOR : public State {
 private:
  bool primera_vez = true;

 public:
  void onEnter() override {
    Serial.println("=== ENTRANDO A ESTADO: DESARROLLADOR ===");
    statemachine->flags.dev = true;
    statemachine->flags.inicio = false;
    statemachine->flags.lectura = false;
    primera_vez = true;

    display.clearDisplay();
    displayStateInfo("DESARROLLADOR");
    display.display();
  }

  void execute() override;  // ← Declaración (implementación más abajo)

  void onExit() override {
    Serial.println("=== SALIENDO DE ESTADO: DESARROLLADOR ===");
    statemachine->flags.dev = false;
    primera_vez = true;
  }

  const char* getName() override { return "DESARROLLADOR"; }
};

// ========================================
// IMPLEMENTACIONES (ahora todas las clases están definidas)
// ========================================

void EstadoINICIO::execute() {
  if (!firstRun) return;

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde INICIO");
    statemachine->ChangeState(new EstadoDESARROLLADOR());  // ✅ Ahora sí está definida
    return;
  }

  unsigned long now = millis();
  Serial.println("Estado: INICIO");
  statemachine->flags.inicio = false;
  statemachine->clocks.proximo_envio = now + statemachine->settings.INTERVALO_ENVIO;
  statemachine->flags.envio_programado = true;

  statemachine->ChangeState(new EstadoLECTURA());  // ✅ Ahora sí está definida
  firstRun = false;
}

void EstadoLECTURA::execute() {
  unsigned long now = millis();

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde LECTURA");
    statemachine->ChangeState(new EstadoDESARROLLADOR());  // ✅ Ahora sí está definida
    return;
  }

  if (statemachine->isDisplayOn && (now - statemachine->clocks.ultima_interaccion > statemachine->settings.TIEMPO_INACTIVIDAD)) {
    display.clearDisplay();
    display.display();
    statemachine->isDisplayOn = false;
    Serial.println("Pantalla apagada por inactividad");
  }

  if (statemachine->needsUpdate && statemachine->isDisplayOn) {
    updateDisplay();
    statemachine->needsUpdate = false;
  }

  if (now - statemachine->clocks.tiempo_lectura >= statemachine->settings.INTERVALO_LECTURA) {
    Serial.println("Estado: LECTURA");
    readSensors();
    if (statemachine->isDisplayOn) {
      updateDisplay();
    }
    statemachine->clocks.tiempo_lectura = now;
  }

  if (statemachine->flags.envio_programado && (long)(now - statemachine->clocks.proximo_envio) >= 0) {
    statemachine->ChangeState(new EstadoENVIO());  // ✅ Ahora sí está definida
  }
}

void EstadoENVIO::execute() {
  unsigned long now = statemachine->clocks.tiempo_actual;

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde ENVIO");
    statemachine->ChangeState(new EstadoDESARROLLADOR());  // ✅ Ahora sí está definida
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin conexión WiFi, posponiendo envío");
    statemachine->clocks.proximo_envio = now + statemachine->settings.INTERVALO_REINTENTO;
    statemachine->flags.envio_programado = true;

    statemachine->ChangeState(new EstadoLECTURA());  // ✅ Ahora sí está definida
    return;
  }

  Serial.println("Estado: ENVIO");
  HTTPClient http;
  http.begin(ServerName);
  http.addHeader("Content-Type", "application/json");

  String payload = construirJson(statemachine->sensors.temp, statemachine->sensors.hum, statemachine->sensors.lux, statemachine->sensors.dbValue);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.printf("✓ Envío exitoso, código: %d\n", httpResponseCode);
  } else {
    Serial.printf("✗ Error en envío: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();

  statemachine->clocks.proximo_envio = now + statemachine->settings.INTERVALO_ENVIO;
  statemachine->flags.envio_programado = true;

  statemachine->ChangeState(new EstadoLECTURA());  // ✅ Ahora sí está definida
}

void EstadoDESARROLLADOR::execute() {
  if (primera_vez) {
    startAPorSTA(settings);
    primera_vez = false;
  }
  server.handleClient();
  displayDeveloperInfo();

  if (digitalRead(DEV_PIN) == HIGH) {
    Serial.println("Saliendo del modo DESARROLLADOR, volviendo a INICIO");
    statemachine->flags.dev = false;
    primera_vez = true;

    statemachine->ChangeState(new EstadoINICIO());  // ✅ Ahora sí está definida
    return;
  }
  delay(100);
}

#endif