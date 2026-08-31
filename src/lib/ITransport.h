#ifndef ITRANSPORT_H
#define ITRANSPORT_H

#include <Arduino.h>

// Resultado de un intento de envío. EstadoENVIO decide el siguiente
// "proximo_envio" a partir de esto, sin conocer detalles de HTTP/LoRa.
enum class TransportResult {
  OK,           // Envío confirmado por el servidor / gateway
  NO_LINK,      // Sin conexión física (WiFi caído / LoRa sin gateway visible)
  AUTH_FAILED,  // Token rechazado o no se pudo renovar (aplica a transportes con auth)
  SEND_FAILED   // Error de protocolo/transporte al enviar
};

// Contrato mínimo que EstadoENVIO necesita de cualquier medio de transmisión.
// WiFiTransport y (a futuro) LoRaTransport implementan esto; EstadoENVIO no
// debe incluir HTTPClient, WiFiClientSecure, ni nada de radios LoRa.
class ITransport {
 public:
  virtual ~ITransport() = default;

  // Se llama una vez desde setup(). Para WiFi no hace mucho (la conexión la
  // maneja WiFiManager); para LoRa aquí iría el init del radio SX1276.
  virtual void begin() = 0;

  // true si el medio está listo para intentar un envío AHORA MISMO
  // (WiFi conectado + token válido / LoRa con duty-cycle disponible, etc).
  // EstadoENVIO usa esto para decidir si vale la pena construir el payload
  // y consumir el acumulador de sensores, o posponer sin tocarlo.
  virtual bool isReady() = 0;

  // Envía el payload ya serializado. Solo se llama si isReady() fue true.
  virtual TransportResult send(const String &payload) = 0;

  // Nombre corto para logs ("WiFi/HTTP", "LoRaWAN", etc).
  virtual const char *getName() = 0;
};

#endif
