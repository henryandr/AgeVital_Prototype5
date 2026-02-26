#include "Estados.h"

#include "AppConfig.h"
#include "DevWebOTA.h"

DevWebOTA* devWeb = nullptr;

// ========================================
// IMPLEMENTACIÓN ESTADO INICIO
// ========================================

void EstadoINICIO::onEnter() {
  Serial.println("===Entrando en Estado INICIO===");
  statemachine->flags.inicio = true;
  firstRun = true;
}

void EstadoINICIO::execute() {
  if (!firstRun) return;

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde INICIO");
    statemachine->ChangeState(new EstadoDESARROLLADOR());
    return;
  }

  unsigned long now = millis();
  Serial.println("Estado: INICIO");
  statemachine->flags.inicio = false;
  statemachine->clocks.proximo_envio = now + appConfig.intervaloEnvio;
  statemachine->flags.envio_programado = true;

  statemachine->ChangeState(new EstadoLECTURA());
  firstRun = false;
}

void EstadoINICIO::onExit() {
  Serial.println("===Saliendo de Estado INICIO===");
  statemachine->flags.inicio = false;
  firstRun = false;
}

const char* EstadoINICIO::getName() { return "INICIO"; }

// ========================================
// IMPLEMENTACIÓN ESTADO LECTURA
// ========================================

void EstadoLECTURA::onEnter() {
  Serial.println("===Entrando en Estado LECTURA===");
  statemachine->flags.lectura = true;
  statemachine->flags.inicio = false;
  statemachine->flags.envio = false;
}

void EstadoLECTURA::execute() {
  unsigned long now = millis();

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde LECTURA");
    statemachine->ChangeState(new EstadoDESARROLLADOR());
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

  if (now - statemachine->clocks.tiempo_lectura >= appConfig.intervaloLectura) {
    Serial.println("Estado: LECTURA");
    readSensors();
    if (statemachine->isDisplayOn) {
      updateDisplay();
    }
    statemachine->clocks.tiempo_lectura = now;
  }

  if (statemachine->flags.envio_programado && (long)(now - statemachine->clocks.proximo_envio) >= 0) {
    statemachine->ChangeState(new EstadoENVIO());
  }
}

void EstadoLECTURA::onExit() {
  Serial.println("===Saliendo de Estado LECTURA===");
  statemachine->flags.lectura = false;
}

const char* EstadoLECTURA::getName() { return "LECTURA"; }

// ========================================
// IMPLEMENTACIÓN ESTADO ENVIO
// ========================================

void EstadoENVIO::onEnter() {
  Serial.println("=== ENTRANDO A ESTADO: ENVIO ===");
  statemachine->flags.envio = true;
  statemachine->flags.inicio = false;
  statemachine->flags.lectura = false;

  display.clearDisplay();
  displayStateInfo("ENVIO");
  display.display();
}

void EstadoENVIO::execute() {
  unsigned long now = statemachine->clocks.tiempo_actual;

  if (statemachine->flags.dev) {
    Serial.println("Cambiando a Estado DESARROLLADOR desde ENVIO");
    statemachine->ChangeState(new EstadoDESARROLLADOR());
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin conexión WiFi, posponiendo envío");
    statemachine->clocks.proximo_envio = now + appConfig.intervaloReintento;
    statemachine->flags.envio_programado = true;

    statemachine->ChangeState(new EstadoLECTURA());
    return;
  }

  Serial.println("Estado: ENVIO");
  HTTPClient http;
  http.begin(appConfig.serverUrl);
  http.addHeader("Content-Type", "application/json");

  String payload = construirPayload(statemachine->sensors.temp, statemachine->sensors.hum, statemachine->sensors.lux, statemachine->sensors.dbValue);
  Serial.println("[ENVIO] Payload JSON:");
  Serial.println(payload);

  int httpResponseCode = http.PATCH(payload);

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.printf("✓ Envío exitoso, código: %d\n", httpResponseCode);
  } else {
    Serial.printf("✗ Error en envío: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();

  statemachine->clocks.proximo_envio = now + appConfig.intervaloEnvio;
  statemachine->flags.envio_programado = true;

  statemachine->ChangeState(new EstadoLECTURA());
}

void EstadoENVIO::onExit() {
  Serial.println("=== SALIENDO DE ESTADO: ENVIO ===");
  statemachine->flags.envio = false;
}

const char* EstadoENVIO::getName() { return "ENVIO"; }

// ========================================
// IMPLEMENTACIÓN ESTADO DESARROLLADOR
// ========================================

void EstadoDESARROLLADOR::onEnter() {
  Serial.println("=== ENTRANDO A ESTADO: DESARROLLADOR ===");
  statemachine->flags.dev = true;
  statemachine->flags.inicio = false;
  statemachine->flags.lectura = false;
  primera_vez = true;

  display.clearDisplay();
  displayStateInfo("DESARROLLADOR");
  display.display();
}

void EstadoDESARROLLADOR::execute() {
  if (primera_vez) {
    if (!devWeb) {
      devWeb = new DevWebOTA(&server);
    }
    devWeb->begin();
    primera_vez = false;
  }

  devWeb->handle();

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "exit") {
      Serial.println("Saliendo del modo desarrollador...");
      statemachine->flags.dev = false;
      if (devWeb) {
        delete devWeb;
        devWeb = nullptr;
      }
      statemachine->ChangeState(new EstadoINICIO());
      return;
    }
  }
}

void EstadoDESARROLLADOR::onExit() {
  Serial.println("=== SALIENDO DE ESTADO: DESARROLLADOR ===");
  statemachine->flags.dev = false;
  primera_vez = true;
}

const char* EstadoDESARROLLADOR::getName() { return "DESARROLLADOR"; }