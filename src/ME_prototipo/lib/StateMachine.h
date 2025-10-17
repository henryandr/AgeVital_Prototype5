#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>

#include "State.h"

// Estructura de banderas
struct Flags {
  bool inicio = false;
  bool lectura = false;
  bool envio = false;
  bool dev = false;
  bool envio_programado = false;
  volatile bool boton_presionado = false;
};

// Estructura de tiempos
struct Clocks {
  unsigned long tiempo_lectura = 0;
  unsigned long tiempo_actual = 0;
  unsigned long ultima_interaccion = 0;
  unsigned long ultimo_debounce = 0;
  unsigned long proximo_envio = 0;
  volatile unsigned long lastButtonPressTime = 0;
};

// Estructura de datos de sensores
struct SensorData {
  float temp = 0.0;
  float hum = 0.0;
  float lux = 0.0;
  float dbValue = 0.0;
};

// ===== NUEVA: Estructura de configuración =====
struct Config {
  unsigned long INTERVALO_LECTURA = 2000;     // 2 segundos
  unsigned long INTERVALO_ENVIO = 15000;      // 15 segundos
  unsigned long TIEMPO_INACTIVIDAD = 10000;   // 10 segundos
  unsigned long DEBOUNCE_TIME = 300;          // 300 ms
  unsigned long INTERVALO_REINTENTO = 20000;  // 20 segundos
};

class StateMachine {
 private:
  State* currentState;
  State* previousState;

 public:
  // Definimos las estructuras globales
  Flags flags;
  Clocks clocks;
  SensorData sensors;
  Config settings;

  volatile int screenMode = 0;
  bool isDisplayOn = true;
  bool needsUpdate = false;

  // Constructor de la máquina de estados
  StateMachine();
  ~StateMachine();

  // Metodos principales
  void begin(State* initialState);
  void update();
  void ChangeState(State* newState);

  void setSettings(unsigned long lectura, unsigned long envio) {
    settings.INTERVALO_LECTURA = lectura;
    settings.INTERVALO_ENVIO = envio;
  }

  // Getters de los estados actual y previo
  State* getCurrentState() const { return currentState; }
  State* getPreviousState() const { return previousState; }
  const char* getCurrentStateName();
};

#endif
