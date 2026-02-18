#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <Arduino.h>
#include <Preferences.h>

class AppConfig {
 private:
  Preferences prefs;

 public:
  // ===== VARIABLES CONFIGURABLES & DEFAULT =====
  String serverUrl = "http://localhost:3001/oauth2/token";
  unsigned long intervaloEnvio = 15000;
  unsigned long intervaloLectura = 2000;
  unsigned long intervaloReintento = 20000;

  void begin() {
    prefs.begin("appconfig", false);
    serverUrl = prefs.getString("serverUrl", serverUrl.c_str());
    intervaloEnvio = prefs.getULong("intervaloEnvio", intervaloEnvio);
    intervaloLectura = prefs.getULong("intervaloLectura", intervaloLectura);
    intervaloReintento = prefs.getULong("intervaloReintento", intervaloReintento);
    prefs.end();

    Serial.println("[AppConfig] Configuracion cargada:");
    Serial.printf("serverUrl:          %s\n", serverUrl.c_str());
    Serial.printf("intervaloEnvio:     %lu ms\n", intervaloEnvio);
    Serial.printf("intervaloLectura:   %lu ms\n", intervaloLectura);
    Serial.printf("intervaloReintento: %lu ms\n", intervaloReintento);
  }

  void save() {
    prefs.begin("appconfig", false);
    prefs.putString("serverUrl", serverUrl.c_str());
    prefs.putULong("intervaloEnvio", intervaloEnvio);
    prefs.putULong("intervaloLectura", intervaloLectura);
    prefs.putULong("intervaloReintento", intervaloReintento);
    prefs.end();
    Serial.println("[AppConfig] Configuracion guardada en NVS");
  }

  void reset() {
    prefs.begin("appconfig", false);
    prefs.clear();
    prefs.end();
    serverUrl = "http://localhost:3001/oauth2/token";
    intervaloEnvio = 15000;
    intervaloLectura = 2000;
    intervaloReintento = 20000;
    Serial.println("[AppConfig] Configuracion restaurada a defaults");
  }
};

extern AppConfig appConfig;

#endif