# 🌿 AgeVital Prototype 5 (TARS 1)

**Sistema de Monitoreo Ambiental IoT con Máquina de Estados**

---

## 📋 Tabla de Contenidos

- [Descripción General](#-descripción-general)
- [Características](#-características)
- [Arquitectura del Sistema](#-arquitectura-del-sistema)
- [Requisitos](#-requisitos)
- [Estructura del Proyecto](#-estructura-del-proyecto)
- [Instalación desde Cero](#-instalación-desde-cero)
- [Configuración de Hardware](#-configuración-de-hardware)
- [Funcionamiento de la Máquina de Estados](#-funcionamiento-de-la-máquina-de-estados)
- [Diagrama de Clases](#-diagrama-de-clases)
- [Guía de Uso](#-guía-de-uso)

---

## 🌟 Descripción General

**AgeVital Prototype 5** es un sistema embebido de monitoreo ambiental basado en ESP32 que mide **5 variables ambientales críticas** y las envía a un servidor mediante HTTP. El proyecto implementa el **Patrón de Diseño State** para una gestión eficiente y modular de los diferentes modos de operación.

### Variables Monitoreadas:

| Variable | Sensor | Unidad | Frecuencia |
|----------|--------|--------|------------|
| 🌡️ **Temperatura** | HDC1080 | °C | 2 segundos |
| 💧 **Humedad** | HDC1080 | % | 2 segundos |
| ☀️ **Luz** | DFRobot B-LUX V30B | lux | 2 segundos |
| 🔊 **Ruido** | Sensor Analógico | dBA | 2 segundos |
| ⏱️ **Timestamp** | millis() | ms | En cada envío |

### Envío de Datos:

Los datos se envían cada **15 segundos** en formato JSON a un servidor **Orion Context Broker** mediante HTTP POST.

---

## ✨ Características

- ✅ **Arquitectura Modular:** Implementación del Patrón State para separación de responsabilidades
- ✅ **Gestión Automática de Estados:** Transiciones inteligentes entre modos de operación
- ✅ **Pantalla OLED Interactiva:** 4 modos de visualización con apagado automático
- ✅ **Modo Desarrollador:** Servidor web integrado para configuración avanzada
- ✅ **Configuración WiFi Persistente:** Almacenamiento en EEPROM
- ✅ **Filtrado de Datos:** Detección y corrección de lecturas anómalas
- ✅ **Reconexión Automática:** Manejo inteligente de pérdida de WiFi
- ✅ **Bajo Consumo:** Optimización de recursos del ESP32

---

## 🏗️ Arquitectura del Sistema

### Diagrama de Arquitectura General

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 (Main Controller)                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌───────────────────────────────────────────────────┐      │
│  │         StateMachine (Motor Principal)            │      │
│  │  • Gestiona transiciones entre estados           │       │
│  │  • Mantiene datos compartidos (flags, clocks)    │       │
│  │  • Coordina el flujo del programa                │       │
│  └───────────────────────────────────────────────────┘      │
│         │                                                   │
│         │ Controla                                          │
│         ↓                                                   │
│  ┌──────────────────────────────────────────────────┐       │
│  │              Estados Concretos                   │       │
│  ├──────────────────────────────────────────────────┤       │
│  │  🔵 EstadoINICIO        (Inicialización)         │       │
│  │  🟢 EstadoLECTURA       (Lectura de sensores)    │       │
│  │  🟡 EstadoENVIO         (Envío HTTP)             │       │
│  │  🔴 EstadoDESARROLLADOR (Configuración web)      │       │
│  └──────────────────────────────────────────────────┘       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
         │                    │                    │
         ↓                    ↓                    ↓
    ┌────────┐          ┌─────────┐          ┌────────┐
    │Sensores│          │  WiFi   │          │ OLED   │
    │I2C/ADC │          │  HTTP   │          │Display │
    └────────┘          └─────────┘          └────────┘
```

---

## 📦 Requisitos

### Hardware

| Componente | Especificación | Cantidad |
|------------|----------------|----------|
| Microcontrolador | ESP32 Dev Module | 1 |
| Sensor Temp/Hum | HDC1080 (I2C) | 1 |
| Sensor de Luz | DFRobot B-LUX V30B (I2C) | 1 |
| Sensor de Sonido | Micrófono analógico | 1 |
| Pantalla | OLED SSD1306 128x64 (I2C) | 1 |
| Botón | Pulsador (modo pantalla) | 1 |
| Botón | Pulsador (modo desarrollador) | 1 |

### Software

- **Arduino IDE:** 1.8.19 o superior (o Arduino IDE 2.x)
- **Plataforma ESP32:** Instalada en Arduino IDE
- **Librerías Arduino:**
  - Adafruit GFX Library
  - Adafruit SSD1306
  - ArduinoJson (v6.x)
  - DFRobot_B_LUX_V30B
  - HTTPClient (incluida en ESP32)
  - WiFi (incluida en ESP32)
  - WebServer (incluida en ESP32)

### Librerías Personalizadas (incluidas en el proyecto)

- **ClosedCube_HDC1080:** Driver para sensor de temperatura/humedad
- **ESPaccesspoint:** Gestión de Access Point y configuración WiFi
- **Settings:** Almacenamiento persistente en EEPROM

---

## 📁 Estructura del Proyecto

```
AgeVital_Prototype5/
│
├── libraries/                          📚 Librerías Arduino personalizadas
│   ├── ClosedCube_HDC1080/            → Driver HDC1080 (I2C)
│   ├── DFRobot_B_LUX_V30B/            → Driver sensor de luz
│   └── ESPaccesspoint/                → Gestión de Access Point y WiFi
│       ├── ESPaccesspoint.h
│       ├── ESPaccesspoint.cpp
│       ├── Settings.h
│       └── Settings.cpp
│
└── src/
    └── ME_prototipo/                   💻 Código fuente principal
        ├── ME_prototipo.ino           ← Archivo principal (setup/loop)
        │
        └── lib/                        🤖 Máquina de Estados
            ├── State.h                ← Clase base abstracta
            ├── StateMachine.h         ← Definición de la máquina
            ├── StateMachine.cpp       ← Implementación de la máquina
            └── Estados.cpp            ← Todos los estados concretos
```

### Descripción de Carpetas

#### 📚 `libraries/`

Contiene las **librerías personalizadas** del proyecto que deben instalarse en Arduino IDE:

- **ClosedCube_HDC1080:** Librería modificada para el sensor de temperatura y humedad
- **DFRobot_B_LUX_V30B:** Librería del sensor de luz
- **ESPaccesspoint:** Librería custom que incluye:
  - Configuración de Access Point WiFi
  - Gestión de credenciales WiFi
  - Almacenamiento persistente en EEPROM
  - **Configuración de URL del servidor** para envío de datos

> ⚠️ **Importante:** Estas librerías están incluidas en el repositorio porque contienen **modificaciones específicas** para este proyecto (como la configuración del servidor Orion Context Broker).

#### 💻 `src/ME_prototipo/`

Contiene el código fuente principal del proyecto:

- **`ME_prototipo.ino`:** Archivo principal de Arduino con `setup()` y `loop()`
- **`lib/`:** Implementación de la **Máquina de Estados**
  - `State.h` - Interfaz base para todos los estados
  - `StateMachine.h/cpp` - Motor de la máquina de estados
  - `Estados.cpp` - Implementación de los 4 estados concretos

---

## 🚀 Instalación desde Cero

Sigue esta guía paso a paso si **nunca has trabajado con ESP32 o Arduino**.

### Paso 1️⃣: Instalar Arduino IDE

1. Descarga **Arduino IDE** desde [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)
2. Instala la aplicación en tu sistema operativo
3. Abre Arduino IDE

### Paso 2️⃣: Configurar Soporte para ESP32

1. Abre **Arduino IDE**
2. Ve a `Archivo` → `Preferencias`
3. En **"Gestor de URLs adicionales de tarjetas"** pega esta URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click en **OK**
5. Ve a `Herramientas` → `Placa` → `Gestor de tarjetas...`
6. Busca **"esp32"**
7. Instala **"esp32 by Espressif Systems"** (versión 2.0.x o superior)
8. Espera a que termine la instalación

### Paso 3️⃣: Instalar Librerías Estándar

1. Ve a `Herramientas` → `Administrar Bibliotecas...`
2. Instala las siguientes librerías **una por una**:

| Librería | Versión | Autor |
|----------|---------|-------|
| Adafruit GFX Library | Última | Adafruit |
| Adafruit SSD1306 | Última | Adafruit |
| ArduinoJson | 6.x | Benoit Blanchon |

Para instalar:
- Escribe el nombre en la barra de búsqueda
- Click en **Instalar**
- Espera a que termine

### Paso 4️⃣: Clonar el Repositorio

**Opción A: Con Git (recomendado)**

```bash
cd ~/Documentos
git clone https://github.com/henryandr/AgeVital_Prototype5.git
cd AgeVital_Prototype5
```

**Opción B: Descarga directa**

1. Ve a [https://github.com/henryandr/AgeVital_Prototype5](https://github.com/henryandr/AgeVital_Prototype5)
2. Click en el botón verde **"Code"** → **"Download ZIP"**
3. Descomprime el archivo en `Documentos/AgeVital_Prototype5`

### Paso 5️⃣: Instalar Librerías Personalizadas

Las librerías personalizadas deben copiarse manualmente a la carpeta de Arduino:

#### En Windows:

```
Origen: AgeVital_Prototype5/libraries/*
Destino: C:\Users\TuUsuario\Documents\Arduino\libraries\
```

#### En macOS:

```
Origen: AgeVital_Prototype5/libraries/*
Destino: /Users/TuUsuario/Documents/Arduino/libraries/
```

#### En Linux:

```
Origen: AgeVital_Prototype5/libraries/*
Destino: ~/Arduino/libraries/
```

**Pasos detallados:**

1. Abre la carpeta `AgeVital_Prototype5/libraries/`
2. Copia **todas las carpetas** que encuentres:
   - `ClosedCube_HDC1080`
   - `DFRobot_B_LUX_V30B`
   - `ESPaccesspoint`
3. Pega estas carpetas en la carpeta `Arduino/libraries/` de tu usuario
4. **Cierra y vuelve a abrir Arduino IDE** para que reconozca las nuevas librerías

### Paso 6️⃣: Abrir el Proyecto

1. En Arduino IDE, ve a `Archivo` → `Abrir`
2. Navega a:
   ```
   AgeVital_Prototype5/src/ME_prototipo/ME_prototipo.ino
   ```
3. Click en **Abrir**

Deberías ver varias pestañas en la parte superior:

```
[ME_prototipo] [StateMachine.cpp] [StateMachine.h] [Estados.cpp] [State.h]
```

### Paso 7️⃣: Configurar la Placa ESP32

1. Ve a `Herramientas` → `Placa` → `ESP32 Arduino` → **ESP32 Dev Module**
2. Configura los siguientes parámetros:

```
Herramientas →
  ├─ Upload Speed: 921600
  ├─ CPU Frequency: 240MHz (WiFi/BT)
  ├─ Flash Frequency: 80MHz
  ├─ Flash Mode: QIO
  ├─ Flash Size: 4MB (32Mb)
  ├─ Partition Scheme: Default 4MB with spiffs
  ├─ Core Debug Level: None
  └─ PSRAM: Disabled
```

### Paso 8️⃣: Conectar el ESP32

1. Conecta el ESP32 a tu computadora mediante cable USB
2. Ve a `Herramientas` → `Puerto`
3. Selecciona el puerto COM donde está el ESP32:
   - **Windows:** `COM3`, `COM4`, etc.
   - **macOS:** `/dev/cu.usbserial-XXXX`
   - **Linux:** `/dev/ttyUSB0`

> 💡 **Tip:** Si no aparece ningún puerto, instala el driver CH340 o CP2102 según tu placa.

### Paso 9️⃣: Compilar y Subir

1. Click en el botón **✓ Verificar** (esquina superior izquierda)
2. Espera a que compile sin errores
3. Si compila correctamente, click en **→ Subir**
4. Espera a que termine la carga

**Salida esperada:**

```
Sketch uses XXXXX bytes (XX%) of program storage space.
Global variables use XXXX bytes (XX%) of dynamic memory.
Hard resetting via RTS pin...
```

### Paso 🔟: Configurar WiFi (Primera Vez)

Al iniciar por primera vez, el ESP32 creará un **Access Point**:

1. Conéctate a la red WiFi: **`ESP-HOTSPOT`**
2. Abre un navegador y ve a: `http://192.168.4.1`
3. Ingresa:
   - **SSID:** Nombre de tu red WiFi
   - **Contraseña:** Contraseña de tu red
   - **URL del servidor:** `http://TU_SERVIDOR:1026/v2/entities/AmbientMonitor_001/attrs`
4. Click en **Guardar**
5. El ESP32 se reiniciará y se conectará a tu WiFi

---

## 🔌 Configuración de Hardware

### Diagrama de Conexiones

```
                    ┌─────────────────────┐
                    │       ESP32         │
                    │   Dev Module        │
                    ├─────────────────────┤
                    │                     │
      HDC1080       │ GPIO 21 (SDA) ──────┼────── I2C SDA (HDC1080, B-LUX, OLED)
      B-LUX V30B    │ GPIO 22 (SCL) ──────┼────── I2C SCL (HDC1080, B-LUX, OLED)
      OLED SSD1306  │                     │
                    │ GPIO 27 ────────────┼────── Botón Pantalla (Pull-up interno)
                    │ GPIO 26 ────────────┼────── Botón Desarrollador (Pull-up interno)
                    │ GPIO 35 (ADC) ──────┼────── Sensor de Sonido
                    │                     │
                    │ 3.3V ───────────────┼────── VCC (todos los sensores)
                    │ GND ────────────────┼────── GND (todos los sensores)
                    └─────────────────────┘
```

### Tabla de Conexiones Detalladas

| ESP32 Pin | Función | Dispositivo | Pin Dispositivo |
|-----------|---------|-------------|-----------------|
| **GPIO 21** | SDA (I2C) | HDC1080 | SDA |
| **GPIO 21** | SDA (I2C) | B-LUX V30B | SDA |
| **GPIO 21** | SDA (I2C) | OLED | SDA |
| **GPIO 22** | SCL (I2C) | HDC1080 | SCL |
| **GPIO 22** | SCL (I2C) | B-LUX V30B | SCL |
| **GPIO 22** | SCL (I2C) | OLED | SCL |
| **GPIO 27** | Entrada Digital | Botón Pantalla | Terminal 1 |
| **GPIO 26** | Entrada Digital | Botón Desarrollador | Terminal 1 |
| **GPIO 35** | ADC | Sensor Sonido | OUT |
| **3.3V** | Alimentación | Todos | VCC |
| **GND** | Tierra | Todos | GND |
| **GND** | Tierra | Botón Pantalla | Terminal 2 |
| **GND** | Tierra | Botón Desarrollador | Terminal 2 |

### Direcciones I2C

| Dispositivo | Dirección I2C | Modificable |
|-------------|---------------|-------------|
| HDC1080 | `0x40` | No |
| OLED SSD1306 | `0x3C` | Sí (jumper) |
| B-LUX V30B | Por defecto | Depende del modelo |

### Esquema de Botones

```
     ┌───┐
  ───┤   ├───  GPIO 27 (Pull-up interno activado)
     └───┘
       │
      GND

     ┌───┐
  ───┤   ├───  GPIO 26 (Pull-up interno activado)
     └───┘
       │
      GND
```

**Configuración:**
- Resistencias pull-up **internas** activadas en el código
- Botón presionado = `LOW`
- Botón suelto = `HIGH`

---

## 🤖 Funcionamiento de la Máquina de Estados

La máquina de estados gestiona el flujo del programa de forma modular y eficiente.

### Estados del Sistema

| Estado | Símbolo | Responsabilidad | Duración |
|--------|---------|-----------------|----------|
| **INICIO** | 🔵 | Inicialización del sistema | ~100ms |
| **LECTURA** | 🟢 | Lectura de sensores y actualización de pantalla | Continuo |
| **ENVIO** | 🟡 | Envío de datos al servidor HTTP | ~2 segundos |
| **DESARROLLADOR** | 🔴 | Configuración web y diagnósticos | Hasta salir manualmente |

### Diagrama de Transición de Estados

```
                        [POWER ON]
                            │
                            ↓
                    ┌───────────────┐
                    │  🔵 INICIO    │
                    │ (First Scan)  │
                    └───────┬───────┘
                            │
                            │ Automático
                            │ (~100ms)
                            ↓
        ┌───────────────────────────────────────┐
        │          🟢 LECTURA                   │
        │  • Lee sensores cada 2s               │
        │  • Actualiza pantalla                 │
        │  • Verifica tiempo de envío           │
        └───────┬───────────────────────────────┘
                │           ↑
                │           │
                │ Timer     │ Automático
                │ 15s       │ tras envío
                │           │
                ↓           │
        ┌───────────────────┴───────────────┐
        │          🟡 ENVIO                 │
        │  • Verifica WiFi                  │
        │  • Envía datos por HTTP           │
        │  • Programa siguiente envío       │
        └───────────────────────────────────┘


        [Desde cualquier estado]
                │
                │ Botón DEV presionado
                │ (GPIO 26 LOW)
                ↓
        ┌───────────────────────────────────┐
        │    🔴 DESARROLLADOR                │
        │  • Inicia servidor web             │
        │  • Muestra información técnica     │
        │  • Permite configuración           │
        └───────┬───────────────────────────┘
                │
                │ Botón DEV soltado
                │ (GPIO 26 HIGH)
                ↓
        Regresa a 🔵 INICIO
```

### Condiciones de Transición

| Estado Origen | Estado Destino | Condición | Descripción |
|---------------|----------------|-----------|-------------|
| 🔵 INICIO | 🟢 LECTURA | Automático | Tras inicialización exitosa |
| 🔵 INICIO | 🔴 DESARROLLADOR | `flags.dev == true` | Si se detectó botón presionado al inicio |
| 🟢 LECTURA | 🟡 ENVIO | `millis() >= proximo_envio` | Cada 15 segundos |
| 🟢 LECTURA | 🔴 DESARROLLADOR | `flags.dev == true` | Presión del botón DEV |
| 🟡 ENVIO | 🟢 LECTURA | Automático | Tras envío (exitoso o fallido) |
| 🟡 ENVIO | 🔴 DESARROLLADOR | `flags.dev == true` | Presión del botón DEV |
| 🔴 DESARROLLADOR | 🔵 INICIO | `GPIO 26 == HIGH` | Al soltar el botón DEV |

---

## 📊 Diagrama de Clases

### UML de la Arquitectura

```
┌──────────────────────────────────────────────────┐
│              <<abstract>>                        │
│                 State                            │
├──────────────────────────────────────────────────┤
│ # statemachine: StateMachine*                    │
├──────────────────────────────────────────────────┤
│ + setStateMachine(StateMachine*): void           │
│ + onEnter(): void                                │
│ + execute(): void                                │
│ + onExit(): void                                 │
│ + getName(): const char*                         │
└──────────────────────────────────────────────────┘
                        △
                        │
                        │ hereda
        ┌───────────────┼───────────────┬──────────────┐
        │               │               │              │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐ ┌────▼──────────┐
│EstadoINICIO  │ │EstadoLECTURA│ │EstadoENVIO │ │EstadoDESARRO- │
│              │ │             │ │            │ │  LLADOR       │
├──────────────┤ ├─────────────┤ ├────────────┤ ├───────────────┤
│-firstRun:bool│ │             │ │-ServerName │ │-primera_vez   │
├──────────────┤ ├─────────────┤ ├────────────┤ ├───────────────┤
│+onEnter()    │ │+onEnter()   │ │+onEnter()  │ │+onEnter()     │
│+execute()    │ │+execute()   │ │+execute()  │ │+execute()     │
│+onExit()     │ │+onExit()    │ │+onExit()   │ │+onExit()      │
│+getName()    │ │+getName()   │ │+getName()  │ │+getName()     │
└──────────────┘ └─────────────┘ └────────────┘ └───────────────┘

┌──────────────────────────────────────────────────┐
│              StateMachine                        │
├──────────────────────────────────────────────────┤
│ - currentState: State*                           │
│ - previousState: State*                          │
│ + flags: Flags                                   │
│ + clocks: Clocks                                 │
│ + sensors: SensorData                            │
│ + settings: Config                               │
│ + screenMode: int                                │
│ + isDisplayOn: bool                              │
│ + needsUpdate: bool                              │
├──────────────────────────────────────────────────┤
│ + StateMachine()                                 │
│ + ~StateMachine()                                │
│ + begin(State*): void                            │
│ + update(): void                                 │
│ + ChangeState(State*): void                      │
│ + setSettings(ulong, ulong): void                │
│ + getCurrentState(): State*                      │
│ + getCurrentStateName(): const char*             │
└──────────────────────────────────────────────────┘
         │
         │ usa
         ↓
┌──────────────────────────────────────────────────┐
│              Estructuras de Datos                │
├──────────────────────────────────────────────────┤
│ Flags         { inicio, lectura, envio, dev }    │
│ Clocks        { tiempo_actual, proximo_envio }   │
│ SensorData    { temp, hum, lux, dbValue }        │
│ Config        { INTERVALO_LECTURA, ENVIO }       │
└──────────────────────────────────────────────────┘
```

---

## 📖 Guía de Uso

### Iniciar el Sistema

1. **Alimenta el ESP32** mediante USB o fuente externa (5V)
2. La pantalla OLED mostrará:
   ```
   Estado: INICIO
   Iniciando...
   ```
3. Tras ~100ms pasará automáticamente a:
   ```
   Estado: LECTURA
   Temp: 24.5 C
   Hum: 60.2 %
   Lux: 450.0 lux
   Ruido: 45.3 dBA
   ```

### Cambiar Modo de Pantalla

Presiona el **botón de pantalla** (GPIO 27) para alternar entre 4 modos:

1. **Modo 0 - Todos los Sensores**
2. **Modo 1 - Temperatura y Humedad**
3. **Modo 2 - Luz**
4. **Modo 3 - Ruido**

### Entrar a Modo Desarrollador

1. **Mantén presionado** el botón de desarrollador (GPIO 26)
2. En tu navegador, ve a la IP mostrada en pantalla
3. Para salir: Suelta el botón de desarrollador (sigue en desarrollo -- puede cambiar)

---
