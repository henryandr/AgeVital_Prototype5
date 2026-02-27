#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

class WiFiManager {
 private:
  Preferences prefs;

  // Default credentials
  const char* defaultSSID = "Claro_2C06BE";
  const char* defaultPass = "16652524";

  // Access Point
  const char* apSSID = "ESP-HOTSPOT";
  const char* apPass = "12345678";

 public:
  bool connect(int maxAttempts = 20) {
    prefs.begin("agevital", true);
    String ssid = prefs.getString("ssid", defaultSSID);
    String pass = prefs.getString("pass", defaultPass);
    prefs.end();

    if (ssid.length() == 0) {
      Serial.println("[WiFiManager] No SSID configurado o no se pudo conectar al SSID guardado");
      return false;
    }

    Serial.printf("[WiFiManager] Conectando a: %s\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n[WiFiManager] Conectado! IP: %s\n", WiFi.localIP().toString().c_str());
      return true;
    }

    Serial.println("\n[WiFiManager] Conexion fallida");
    return false;
  }

  void createAP() {
    Serial.println("[WiFiManager] Creando Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPass);
    Serial.printf("[WiFiManager] SSID: %s\n", apSSID);
    Serial.printf("[WiFiManager] Password: %s\n", apPass);
    Serial.printf("[WiFiManager] IP: %s\n", WiFi.softAPIP().toString().c_str());
  }

  void saveCredentials(const String& newSSID, const String& newPass) {
    prefs.begin("agevital", false);
    prefs.putString("ssid", newSSID);
    prefs.putString("pass", newPass);
    prefs.end();
    Serial.printf("[WiFiManager] Credenciales guardadas: %s\n", newSSID.c_str());
  }

  void reset() {
    prefs.begin("agevital", false);
    prefs.clear();
    prefs.end();
    Serial.println("[WiFiManager] Credenciales restauradas a los valores por defecto");
  }

  bool isConnected() { return WiFi.status() == WL_CONNECTED; }

  String getIP() {
    if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
    if (WiFi.getMode() == WIFI_AP) return WiFi.softAPIP().toString();
    return "No conectado";
  }
};

extern WiFiManager wifiManager;

#endif