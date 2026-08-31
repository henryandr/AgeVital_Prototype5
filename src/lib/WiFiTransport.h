#ifndef WIFITRANSPORT_H
#define WIFITRANSPORT_H

#include <WiFiClientSecure.h>

#include "ITransport.h"

// Implementación de ITransport sobre WiFi + HTTPS (moreha.com.co) + token Keyrock.
class WiFiTransport : public ITransport {
 public:
  void begin() override;
  bool isReady() override;
  TransportResult send(const String &payload) override;
  const char *getName() override { return "WiFi/HTTPS"; }

 private:
  // Reutilizado entre envíos: se conecta y se cierra (stop()) en cada ciclo,
  // pero el objeto en sí vive una sola vez (ya no hay new/delete por ciclo).
  WiFiClientSecure secureClient;
};

#endif
