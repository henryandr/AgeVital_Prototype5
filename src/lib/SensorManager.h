#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <Arduino.h>

#include "StateMachine.h"

// HARDWARE DE SENSORES
#define SOUND_SENSOR_PIN 35
#define VREF 3.7f

class SensorManager {
 private:
  static const int LUX_HISTORY_SIZE = 15;
  float luxHistory[LUX_HISTORY_SIZE];
  int luxHistoryIndex = 0;

  // Calibración temperatura y humedad
  static constexpr float TEMP_OFFSET = -3.0f;
  static constexpr float HUM_OFFSET = 7.0f;

  // Calibración lux (regresión lineal sobre mediciones reales)
  static constexpr float LUX_CALIBRATION_OFFSET = 6.1551f;
  static constexpr float LUX_CALIBRATION_SLOPE = 1.3788f;
  static constexpr float EXTREME_LUX_THRESHOLD = 300000.0f;

  // Calibración ruido (pendiente e intercepto, reservados para uso futuro)
  static constexpr float NOISE_CALIBRATION_SLOPE = 1.618f;
  static constexpr float NOISE_CALIBRATION_OFFSET = 14.282f;

  // Rangos válidos HDC1080 — fuera de estos la lectura es basura por error I2C
  static constexpr float HDC_TEMP_MIN = -20.0f;
  static constexpr float HDC_TEMP_MAX = 85.0f;
  static constexpr float HDC_HUM_MIN = 0.0f;
  static constexpr float HDC_HUM_MAX = 100.0f;

  // Contador de errores I2C consecutivos del HDC1080
  int hdc1080ErrorCount = 0;

  // ===== ACUMULADOR PARA PROMEDIADO =====
  float accTemp = 0.0f;
  float accHum = 0.0f;
  float accLux = 0.0f;
  float accNoise = 0.0f;
  int sampleCount = 0;

 public:
  // Inicializa el buffer del filtro de lux con la primera lectura real
  void begin();

  // Lee todos los sensores, actualiza stateMachine.sensors y acumula para promedio
  void read();

  // Devuelve el promedio de las muestras acumuladas desde el último reset.
  // Fallback: si sampleCount == 0 retorna stateMachine.sensors (último valor válido)
  SensorData getAverages();

  // Resetea el acumulador — llamar justo después de getAverages() en EstadoENVIO
  void resetAccumulator();

  // Cuántas muestras hay acumuladas actualmente
  int getSampleCount() const { return sampleCount; }
};

extern SensorManager sensorManager;

#endif
