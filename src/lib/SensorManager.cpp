#include "SensorManager.h"

#include <DFRobot_B_LUX_V30B.h>

#include "ClosedCube_HDC1080.h"
#include "StateMachine.h"

extern ClosedCube_HDC1080 hdc1080;
extern DFRobot_B_LUX_V30B luxSensor;
extern StateMachine stateMachine;

void SensorManager::begin() {
  float initialLux = luxSensor.lightStrengthLux();
  if (initialLux < 0) initialLux = 0;
  for (int i = 0; i < LUX_HISTORY_SIZE; i++) {
    luxHistory[i] = initialLux;
  }
  Serial.println("[SensorManager] Buffer de lux inicializado");
}

void SensorManager::read() {
  // ===== TEMPERATURA Y HUMEDAD con validación de rango HDC1080 =====
  // Rango físico del sensor: -40°C a 125°C / 0% a 100% HR
  // Si hay error I2C la librería devuelve basura (ej. 122°C), lo descartamos
  // y conservamos el último valor válido en stateMachine.sensors
  float rawTemp = hdc1080.readTemperature();
  float rawHum = hdc1080.readHumidity();

  if (rawTemp > HDC_TEMP_MIN && rawTemp < HDC_TEMP_MAX) {
    stateMachine.sensors.temp = rawTemp + TEMP_OFFSET;
    hdc1080ErrorCount = 0;
  } else {
    hdc1080ErrorCount++;
    Serial.printf("[SensorManager] HDC1080 temp fuera de rango (%.1f °C) — conservando último valor válido (errores consecutivos: %d)\n", rawTemp, hdc1080ErrorCount);
  }

  if (rawHum >= HDC_HUM_MIN && rawHum <= HDC_HUM_MAX) {
    stateMachine.sensors.hum = rawHum - HUM_OFFSET;
  } else {
    Serial.printf("[SensorManager] HDC1080 hum fuera de rango (%.1f %%) — conservando último valor válido\n", rawHum);
  }

  // ===== LUX con filtro de anomalías =====
  float rawLux = luxSensor.lightStrengthLux();

  if (rawLux < 0 || rawLux > EXTREME_LUX_THRESHOLD) {
    // Lectura anómala: usar el último valor válido del buffer
    stateMachine.sensors.lux = luxHistory[(luxHistoryIndex - 1 + LUX_HISTORY_SIZE) % LUX_HISTORY_SIZE];
    Serial.print("[SensorManager] Lux anómalo (");
    Serial.print(rawLux);
    Serial.println(") — usando último valor válido");
  } else {
    float calibratedLux = (rawLux - LUX_CALIBRATION_OFFSET) / LUX_CALIBRATION_SLOPE;
    stateMachine.sensors.lux = calibratedLux;
    luxHistory[luxHistoryIndex] = calibratedLux;
    luxHistoryIndex = (luxHistoryIndex + 1) % LUX_HISTORY_SIZE;
  }

  // ===== RUIDO =====
  int rawADC = analogRead(SOUND_SENSOR_PIN);
  float voltage = rawADC * (VREF / 4096.0f);
  stateMachine.sensors.voltage = voltage;
  stateMachine.sensors.dbValue = voltage * 50.0f;

  Serial.printf("Temp: %.1f C | Hum: %.1f %% | Lux: %.1f | Ruido: %.1f dBA\n", stateMachine.sensors.temp, stateMachine.sensors.hum, stateMachine.sensors.lux,
                stateMachine.sensors.dbValue);
}
