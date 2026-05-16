#ifndef PAYLOADBUILDER_H
#define PAYLOADBUILDER_H

#include <Arduino.h>

// Payload para Orion Context Broker y Agente Flask
String construirPayload(float temperatura, float humedad, float luz, float ruido);

#endif
