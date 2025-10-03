# Proyecto ESP32 Access Point & Station Mode

Este proyecto permite configurar un ESP32 para que funcione como un punto de acceso (Access Point) o como cliente (Station Mode) para conectarse a una red Wi-Fi. El dispositivo actúa como un servidor web, donde se puede configurar el Wi-Fi mediante una página web.

## Requisitos

Antes de comenzar, debes asegurarte de tener las siguientes librerías en tu proyecto:

1. **data.h**: Contiene los datos HTML y JavaScript necesarios para las páginas web del servidor.
2. **settings.h**: Maneja la configuración del SSID y la contraseña almacenada en la EEPROM.
3. **ESPaccesspoint.h**: Define las funciones para manejar los modos AP y Station.

## Instrucciones

1. Clona o descarga este repositorio.
2. Copia las librerías `data.h`, `settings.h`, y `ESPaccesspoint.h` en tu proyecto.
3. Asegúrate de incluir las librerías requeridas en tu archivo principal.
4. Copia el siguiente código en tu archivo principal de Arduino para configurar el ESP32.

## Código de Ejemplo (main.ino)

```cpp

/* INCLUYE LAS LIBRERIAS LAS YA MENCIONADAS Y ESTAS LIBRERIAS GLOBALES ADICIONALES PARA EL FUNCIONAMIENTO DEL ESP COMO AP Y STA SEA CORRECTO */
#include <Arduino.h>
#include <WebServer.h>
#include <EEPROM.h>

#include "ESPaccesspoint.h"
#include "Settings.h"

/* Declaración de instancias necesarias para el servidor web y la configuración */
WebServer server(80);
Settings settings;

/* IMPORTANTE: Copia este codigo antes de lo que necesites hacer para que el esp se conecte a una red wifi */

// Código esencial para que el ESP se conecte al WiFi
void setup() {
    Serial.begin(115200);
    EEPROM.begin(4096);
    settings.load();  // Cargar configuración de WiFi desde EEPROM
    settings.info();  // Mostrar información de configuración
    startAPorSTA(settings);  // Iniciar como AP o conectar a WiFi
}

void loop() {
    
server.handleClient();  // Maneja solicitudes HTTP del HTTP quemado
    

// Aqui va el codigo del sensor 
}
```
