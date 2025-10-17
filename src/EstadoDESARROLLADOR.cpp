#ifndef ESTADO_DESARROLLADOR_CPP
#define ESTADO_DESARROLLADOR_CPP

#include "ESPaccesspoint.h"
#include "Settings.h"
#include "State.h"
#include "StateMachine.h"

class EstadoINICIO;

extern Adafruit_SSD1306 display;
extern void displayStateInfo(const char* estado);
extern void displayDeveloperInfo();
extern void startAPorSTA(Settings& settings);
extern Webserver server;
extern Settings settings;

#define DEV_PIN 26  // Pin para modo desarrollador

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

  void execute() override {
    if (primera_vez) {
      startAPorSTA(settings);
      primera_vez = false;
    }
    server.handleClient();  // Manejar las solicitudes del cliente
    displayDeveloperInfo();

    if (digitalRead(DEV_PIN) == HIGH) {
      Serial.println("Saliendo del modo DESARROLLADOR, volviendo a INICIO");
      statemachine->flags.dev = false;
      primera_vez = true;
      statemachine->ChangeState(new EstadoINICIO());
      return;
    }
  }

  void onExit() override {
    Serial.println("=== SALIENDO DE ESTADO: DESARROLLADOR ===");
    statemachine->flags.dev = false;
    primera_vez = true;
  }

  const char* getName() override { return "DESARROLLADOR"; }
};

#endif