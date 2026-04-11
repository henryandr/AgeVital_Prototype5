#ifndef PAYLOADBUILDER_H
#define PAYLOADBUILDER_H

#include <Arduino.h>

// Payload para Orion Context Broker (PATCH /v2/entities/.../attrs)
String construirPayload(float temperatura, float humedad, float luz, float ruido);

// Payload para el agente Flask
// NOTA: actualmente EstadoENVIO llama construirPayload() también en el path del agente.
// Revisar si los campos deben diferir (illuminance vs light) y unificar o separar.
String construirPayloadAgente(float temperatura, float humedad, float luz, float ruido);

#endif
