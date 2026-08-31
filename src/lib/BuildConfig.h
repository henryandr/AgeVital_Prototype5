#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

// Selección de transporte en tiempo de compilación.
// Definir UNO solo de estos, vía build_flags de PlatformIO, p. ej.:
//
//   [env:tars-wifi]
//   build_flags = -D BUILD_WIFI
//
//   [env:tars-lora]
//   build_flags = -D BUILD_LORA
//
// Si no se define ninguno (p. ej. compilando desde el Arduino IDE),
// se usa WiFi por defecto mientras LoRa sigue en desarrollo.

#if !defined(BUILD_WIFI) && !defined(BUILD_LORA)
#define BUILD_WIFI
#endif

#if defined(BUILD_WIFI) && defined(BUILD_LORA)
#error "Definir solo BUILD_WIFI o BUILD_LORA en build_flags, no ambos"
#endif

#endif
