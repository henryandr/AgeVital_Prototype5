# AgeVital Prototype 5

## Descripción

Sistema de monitoreo ambiental basado en ESP32 que implementa una máquina de estados para la lectura de múltiples sensores ambientales y su visualización en una pantalla OLED. Incluye modo desarrollador con configuración WiFi.

## Características

- Lectura de múltiples sensores ambientales:
  - Temperatura y Humedad (HDC1080)
  - Luz (DFRobot B-LUX-V30B)
  - Ruido (Sensor analógico)
- Pantalla OLED para visualización
- Máquina de estados para control del sistema
- Modo desarrollador con configuración WiFi
- Filtrado de datos para lecturas más estables
- Sistema de ahorro de energía

## Componentes de Hardware

- ESP32
- Sensor HDC1080 (Temperatura y Humedad)
- Sensor DFRobot B-LUX-V30B (Luz)
- Sensor de sonido analógico
- Pantalla OLED SSD1306 128x64
- Botón de control
- Botón de modo desarrollador

## Pines Utilizados

- GPIO21, GPIO22: I2C (SDA, SCL)
- GPIO35: Sensor de sonido
- GPIO27: Botón de control
- GPIO26: Botón de modo desarrollador
- GPIO5: Enable pin para sensor de luz

## Estados del Sistema

1. **INICIO**: Inicialización del sistema
2. **LECTURA**: Lectura periódica de sensores
3. **ENVIO**: Envío de datos (preparado para IoT)
4. **DESARROLLADOR**: Configuración WiFi y diagnóstico

## Modos de Visualización

0. Todos los sensores
1. Temperatura y Humedad
2. Luz
3. Ruido

## Características Técnicas

- Intervalo de lectura: 2 segundos
- Intervalo de envío: 15 segundos
- Timeout de inactividad: 10 segundos
- Debounce de botón: 300ms
- Resolución ADC: 12 bits
- Filtrado de luz: Media móvil de 15 muestras

## Instalación

### Requisitos de Hardware

1. ESP32 DevKit
2. Sensores mencionados
3. Pantalla OLED SSD1306
4. 2 botones pulsadores

### Requisitos de Software

- Arduino IDE
- Bibliotecas necesarias:
  ```cpp
  #include <DFRobot_B_LUX_V30B.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #include <HTTPClient.h>
  #include <ArduinoJson.h>
  #include <WiFi.h>
  #include <EEPROM.h>
  #include <WebServer.h>
  #include "ClosedCube_HDC1080.h"
  ```

### Conexiones

1. I2C: GPIO21 (SDA), GPIO22 (SCL)
2. Sensor de sonido: GPIO35
3. Botón principal: GPIO27
4. Botón desarrollador: GPIO26
5. Sensor de luz EN: GPIO5

## Uso

### Operación Normal

1. El sistema inicia en modo de INICIO para luego pasar a LECTURA
2. Presionar el botón principal para cambiar entre modos de visualización
3. La pantalla se apaga después de 10 segundos de inactividad
4. Cualquier pulsación reactiva la pantalla

### Modo Desarrollador

1. Mantener presionado el botón de desarrollador al inicio si se quiere ingresar a este al iniciar el prototipo
2. Conectarse al AP "ESP-HOTSPOT" (password: 12345678)
3. Configurar WiFi a través de la interfaz web
4. Los datos de conexión se guardan en EEPROM

## Calibración

El sistema incluye factores de calibración para cada sensor:

- Temperatura: -3°C offset
- Humedad: +7% offset
- Ruido: Factor 0.025 y offset 50.0
- Luz: Factor 0.613

## Contribución

Para contribuir al proyecto:

1. Fork del repositorio
2. Crear branch feature/nombre-caracteristica
3. Commit de cambios
4. Push al branch
5. Crear Pull Request

## Estado del Proyecto

En desarrollo - Prototipo 5
