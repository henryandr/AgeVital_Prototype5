#include "WiFiTransport.h"

#include <HTTPClient.h>

#include "AppConfig.h"
#include "TokenManager.h"
#include "WiFiManager.h"

void WiFiTransport::begin() {
  // NOTA: en el core arduino-esp32 3.x, WiFiClientSecure (alias de
  // NetworkClientSecure) ya no expone setBufferSizes() -- pertenecía a la
  // API vieja. El tamaño de los buffers de mbedTLS ahora se maneja por
  // configuración del lado de ESP-IDF (normalmente ya usa asignación
  // dinámica en vez de un buffer fijo de 16KB), así que no hay una perilla
  // equivalente disponible desde el sketch.
  secureClient.setInsecure();  // No validamos la cadena de certificados de moreha.com.co
}

bool WiFiTransport::isReady() {
  // Mismo orden que el EstadoENVIO original: primero el enlace físico,
  // luego el token. Si cualquiera falla, EstadoENVIO no debe tocar el
  // acumulador de sensores (las muestras se conservan para el próximo ciclo).
  if (!wifiManager.isConnected()) {
    return false;
  }
  if (appConfig.skipToken) {
    return true;
  }
  return tokenManager.ensureValidToken();
}

TransportResult WiFiTransport::send(const String &payload) {
  Serial.printf("[WiFiTransport] Heap libre antes de enviar: %u bytes\n", ESP.getFreeHeap());

  HTTPClient http;
  http.setConnectTimeout(5000);
  http.setTimeout(8000);

  if (!http.begin(secureClient, appConfig.serverUrl)) {
    Serial.println("[WiFiTransport] http.begin() fallo");
    secureClient.stop();
    return TransportResult::SEND_FAILED;
  }

  http.addHeader("Content-Type", "application/json");
  if (!appConfig.skipToken) {
    http.addHeader("Authorization", "Bearer " + tokenManager.getToken());
  }

  int httpResponseCode = http.PATCH(payload);
  String errMsg = (httpResponseCode < 0) ? http.errorToString(httpResponseCode) : String("");

  http.end();
  secureClient.stop();  // Cierra el socket TLS y libera los recursos de este ciclo

  Serial.printf("[WiFiTransport] Heap libre despues de enviar: %u bytes\n", ESP.getFreeHeap());

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.printf("[WiFiTransport] Envio exitoso, codigo: %d\n", httpResponseCode);
    return TransportResult::OK;
  }

  if (httpResponseCode == 401) {
    Serial.println("[WiFiTransport] Token rechazado (401), se limpia para renovar en el proximo intento");
    tokenManager.clear();
    return TransportResult::AUTH_FAILED;
  }

  Serial.printf("[WiFiTransport] Error en envio, codigo: %d %s\n", httpResponseCode, errMsg.c_str());
  return TransportResult::SEND_FAILED;
}
