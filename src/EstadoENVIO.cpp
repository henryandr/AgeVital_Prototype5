
#ifndef ESTADO_ENVIO_CPP
#define ESTADO_ENVIO_CPP

#include <HTTPClient.h>
#include <WiFi.h>

#include "State.h"
#include "StateMachine.h"

// Declaraciones adelantadas
class EstadoLECTURA;
class EstadoDESARROLLADOR;
// Declaraciones de funciones externas
extern String construirJson(float temperatura, float humedad, float luz, float ruido);
extern void displayStateInfo(const char* estado);
extern Adafruit_SSD1306 display;

class EstadoENVIO : public State {
  // Por ahora hasta implementar el AP con configuración de WiFi y servidor
 private:
  const char* ServerName = "http://10.38.32.137:1026/v2/entities/AmbientMonitor_001/attrs";

 public:
  // Tiempo actual

  void onEnter() override {
    Serial.println("=== ENTRANDO A ESTADO: ENVIO ===");
    statemachine->flags.envio = true;
    statemachine->flags.inicio = false;
    statemachine->flags.lectura = false;

    display.clearDisplay();
    displayStateInfo("ENVIO");
    display.display();
  }

  // Verificar si se debe cambiar al estado DESARROLLADOR
  void execute() override {
    unsigned long now = millis();

    if (statemachine->flags.dev) {
      Serial.println("Cambiando a Estado DESARROLLADOR desde ENVIO");
      statemachine->ChangeState(new EstadoDESARROLLADOR());
      return;
    }

    // Si no hay conexión WiFi, posponer el envío y volver a LECTURA
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Sin conexión WiFi, posponiendo envío---DEVOLVIENDO A LECTURA");
      statemachine->clocks.proximo_envio = now + statemachine->config.INTERVALO_REINTENTO;
      statemachine->flags.envio_programado = true;

      // Volver a lectura
      statemachine->ChangeState(new EstadoLECTURA());
      return;
    }

    // Si hay internet, proceder con el envío
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
    statemachine->ChangeState(new EstadoLECTURA());
  }

  void onExit() override {
    Serial.println("=== SALIENDO DE ESTADO: ENVIO ===");
    statemachine->flags.envio = false;
  }

  const char* getName() override { return "ENVIO"; }
};

#endif