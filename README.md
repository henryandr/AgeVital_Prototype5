# 🌿 AgeVital Prototype 5 (TARS 1)

**Sistema de Monitoreo Ambiental IoT con Máquina de Estados — ESP32**

---

## 📋 Tabla de Contenidos

- [Descripción General](#-descripción-general)
- [Características](#-características)
- [Estructura del Proyecto](#-estructura-del-proyecto)
- [Instalación y Compilación](#-instalación-y-compilación)
- [Arquitectura del Software](#-arquitectura-del-software)
- [Máquina de Estados (StateMachine)](#-máquina-de-estados-statemachine)
- [Estados Concretos](#-estados-concretos)
- [ButtonHandler](#-buttonhandler)
- [WiFiManager](#-wifimanager)
- [AppConfig](#-appconfig)
- [TokenManager](#-tokenmanager)
- [DevWebOTA](#-devwebota)
- [Flujo del Programa (TARS.ino)](#-flujo-del-programa-tarsino)
- [Diagrama de Clases](#-diagrama-de-clases)
- [Pruebas sin Hardware](#-pruebas-sin-hardware)
- [Mejoras Pendientes](#-mejoras-pendientes)

---

## 🌟 Descripción General

**AgeVital Prototype 5 (TARS 1)** es un firmware para ESP32 que implementa un sistema de monitoreo ambiental usando el **Patrón de Diseño State**. Lee sensores, muestra datos en pantalla OLED, y envía información a un servidor **Orion Context Broker** (FIWARE) mediante HTTP POST.

### Datos que maneja

| Variable | Unidad | Frecuencia de Lectura | Frecuencia de Envío |
|----------|--------|-----------------------|---------------------|
| Temperatura | °C | Cada 2 segundos | Cada 15 segundos |
| Humedad | % | Cada 2 segundos | Cada 15 segundos |
| Luz | lux | Cada 2 segundos | Cada 15 segundos |
| Ruido | dBA | Cada 2 segundos | Cada 15 segundos |

---

## ✨ Características

- ✅ **Patrón State:** 4 estados concretos con transiciones claras y ciclo de vida definido (`onEnter`, `execute`, `onExit`)
- ✅ **ButtonHandler por Polling:** Detección de short press y long press sin interrupciones (IRAM_ATTR)
- ✅ **Pantalla OLED:** 4 modos de visualización con apagado automático por inactividad
- ✅ **Modo Desarrollador:** Servidor web con OTA, salida por botón o Serial
- ✅ **WiFiManager:** Conexión WiFi con portal cautivo (Access Point) para configuración inicial
- ✅ **AppConfig:** Persistencia de configuración en EEPROM
- ✅ **TokenManager:** Gestión de autenticación con el servidor
- ✅ **Filtrado de Datos:** Historial de 15 muestras de lux para corregir lecturas anómalas
- ✅ **Calibración:** Offsets configurables para temperatura, humedad y luz

---

## 📁 Estructura del Proyecto

```
AgeVital_Prototype5/
│
├── README.md
├── frecuencia de transmision de datos.md
├── .gitignore
│
├── library/                          📚 Librerías de terceros (copiar a Arduino/libraries/)
│   ├── Adafruit_GFX_Library/
│   ├── Adafruit_SSD1306/
│   ├── ClosedCube_HDC1080/
│   └── DFRobot_B_LUX_V30B/
│
└── src/
    └── TARS/                         💻 Carpeta del sketch (todo junto para compilar)
        ├── TARS.ino                 ← Punto de entrada: setup() y loop()
        ├── State.h                  ← Clase base abstracta
        ├── StateMachine.h           ← Definición de la máquina de estados
        ├── StateMachine.cpp         ← Implementación de la máquina
        ├── Estados.h                ← Declaración de los 4 estados
        ├── Estados.cpp              ← Implementación de los 4 estados
        ├── ButtonHandler.h          ← Detección de botón por polling
        ├── WiFiManager.h            ← Gestión de conexión WiFi y AP
        ├── AppConfig.h              ← Configuración persistente (EEPROM)
        ├── TokenManager.h           ← Autenticación con el servidor
        └── DevWebOTA.h              ← Servidor web + actualización OTA
```

> **¿Por qué todo en una carpeta?** Arduino IDE requiere que el `.ino` y todos los archivos `.h` / `.cpp` propios estén en la misma carpeta para compilar correctamente.

---

## 🚀 Instalación y Compilación

### 1. Instalar Arduino IDE

Descargar desde [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

### 2. Configurar soporte ESP32

1. `Archivo` → `Preferencias` → en **URLs adicionales** pegar:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
2. `Herramientas` → `Placa` → `Gestor de tarjetas` → buscar **esp32** → instalar

### 3. Instalar librerías desde el Library Manager

`Herramientas` → `Administrar Bibliotecas` → instalar:

| Librería | Autor |
|----------|-------|
| Adafruit GFX Library | Adafruit |
| Adafruit SSD1306 | Adafruit |
| ArduinoJson (v6.x) | Benoit Blanchon |

### 4. Instalar librerías del proyecto

Copiar las carpetas de `library/` a la carpeta de librerías de Arduino:

| SO | Ruta |
|----|------|
| Windows | `C:\Users\TuUsuario\Documents\Arduino\libraries\` |
| macOS | `/Users/TuUsuario/Documents/Arduino/libraries/` |
| Linux | `~/Arduino/libraries/` |

### 5. Compilar y subir

1. Abrir `src/TARS/TARS.ino` en Arduino IDE
2. `Herramientas` → `Placa` → **ESP32 Dev Module**
3. Seleccionar puerto COM
4. Click en **→ Subir**

---

## 🏗️ Arquitectura del Software

```
┌─────────────────────────────────────────────────────────────┐
│                     TARS.ino (loop)                         │
│  ┌──────────────────┐    ┌──────────────────────────┐       │
│  │  ButtonHandler    │    │  StateMachine.update()   │       │
│  │  .update()        │    │  → ejecuta estado actual │       │
│  │  → pone banderas  │    │  → estado lee banderas   │       │
│  └──────────────────┘    └──────────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
     ┌────────▼──┐    ┌──────▼─────┐   ┌─────▼──────────┐
     │ INICIO    │    │  LECTURA   │   │ DESARROLLADOR  │
     │ WiFi      │    │  Sensores  │   │ Web + OTA      │
     │ → LECTURA │    │  Pantalla  │   │ Serial "exit"  │
     └───────────┘    │  → ENVIO   │   └────────────────┘
                      └──────┬─────┘
                             │
                      ┌──────▼─────┐
                      │   ENVIO    │
                      │  HTTP POST │
                      │  → LECTURA │
                      └────────────┘
```

### Principio clave

> El `loop()` solo **pone banderas**. Los estados **reaccionan** a esas banderas en su `execute()`.

---

## 🤖 Máquina de Estados (StateMachine)

### Archivos: `StateMachine.h` / `StateMachine.cpp`

Gestiona el estado actual, las transiciones y los datos compartidos entre estados.

### Métodos

| Método | Descripción |
|--------|-------------|
| `begin(State* initial)` | Establece el estado inicial y llama su `onEnter()` |
| `update()` | Actualiza `tiempo_actual` y llama `execute()` del estado activo |
| `ChangeState(State* newState)` | Llama `onExit()` del actual, `onEnter()` del nuevo, y cambia |
| `getCurrentStateName()` | Retorna el nombre del estado activo |

### Estructuras de datos compartidas

```cpp
struct Flags {
  bool inicio;
  bool lectura;
  bool envio;
  bool dev;           // true = entrar a DESARROLLADOR
};

struct Clocks {
  unsigned long tiempo_actual;
  unsigned long tiempo_lectura;
  unsigned long ultima_interaccion;
  unsigned long proximo_envio;
};

struct SensorData {
  float temp;
  float hum;
  float lux;
  float voltage;
  float dbValue;
};

struct Config {
  unsigned long TIEMPO_INACTIVIDAD;  // ms para apagar pantalla
};
```

### Variables públicas

| Variable | Tipo | Descripción |
|----------|------|-------------|
| `flags` | `Flags` | Banderas de control de estados |
| `clocks` | `Clocks` | Tiempos y relojes del sistema |
| `sensors` | `SensorData` | Últimas lecturas de sensores |
| `settings` | `Config` | Configuración de tiempos |
| `screenMode` | `int` | Modo de pantalla actual (0-3) |
| `isDisplayOn` | `bool` | Si la pantalla está encendida |
| `needsUpdate` | `bool` | Si la pantalla necesita redibujarse |

---

## 📦 Estados Concretos

### Archivo: `State.h` (Clase base abstracta)

Define la interfaz que todos los estados deben implementar:

| Método virtual | Cuándo se llama | Propósito |
|----------------|-----------------|-----------|
| `onEnter()` | Al entrar al estado (`ChangeState`) | Inicialización |
| `execute()` | En cada `loop()` vía `update()` | Lógica principal |
| `onExit()` | Al salir del estado (`ChangeState`) | Limpieza |
| `getName()` | Cuando se necesita el nombre | Retorna `const char*` |

---

### 🔵 EstadoINICIO

**Responsabilidad:** Conectar WiFi e inicializar el sistema.

| Paso | Acción |
|------|--------|
| `onEnter()` | Log de entrada |
| `execute()` | Intenta conectar WiFi. Si hay credenciales guardadas → STA. Si no → crea AP (`ESP-HOTSPOT`). Cuando termina → `ChangeState(EstadoLECTURA)`. Si `flags.dev == true` → `ChangeState(EstadoDESARROLLADOR)` |
| `onExit()` | Log de salida |

---

### 🟢 EstadoLECTURA

**Responsabilidad:** Leer sensores, actualizar pantalla, verificar si toca enviar.

| Paso | Acción |
|------|--------|
| `onEnter()` | Programa siguiente lectura |
| `execute()` | Si `flags.dev == true` → cambia a DESARROLLADOR. Si `millis() >= tiempo_lectura` → llama `readSensors()`. Si `needsUpdate` o nueva lectura → llama `updateDisplay()`. Si pantalla inactiva > `TIEMPO_INACTIVIDAD` → apaga. Si `millis() >= proximo_envio` → cambia a ENVIO |
| `onExit()` | Log de salida |

**Modos de pantalla (`screenMode`):**

| Modo | Muestra |
|------|---------|
| 0 | Todos los sensores |
| 1 | Temperatura y Humedad |
| 2 | Luz |
| 3 | Ruido |

---

### 🟡 EstadoENVIO

**Responsabilidad:** Construir JSON y enviar por HTTP POST.

| Paso | Acción |
|------|--------|
| `onEnter()` | Log de entrada |
| `execute()` | Si `flags.dev == true` → cambia a DESARROLLADOR. Construye payload JSON con `construirPayload()`. Envía HTTP POST al servidor. Programa `proximo_envio`. Cambia a LECTURA |
| `onExit()` | Log de salida |

**Formato del JSON enviado:**

```json
{
  "temperature": { "type": "Number", "value": "24.5" },
  "humidity": { "type": "Number", "value": "60.2" },
  "illuminance": { "type": "Number", "value": "450.0" },
  "noise": { "type": "Number", "value": "45.3" }
}
```

---

### 🔴 EstadoDESARROLLADOR

**Responsabilidad:** Levantar servidor web con OTA, permitir configuración.

| Paso | Acción |
|------|--------|
| `onEnter()` | Log de entrada |
| `execute()` | Si `primera_vez` → crea `DevWebOTA` y llama `begin()`. Llama `devWeb->handle()`. Si `flags.dev == false` → `ChangeState(EstadoINICIO)`. Si Serial recibe `"exit"` → `flags.dev = false`, `ChangeState(EstadoINICIO)` |
| `onExit()` | `flags.dev = false`, `delete devWeb`, `primera_vez = true` |

**Dos formas de salir:**

| Método | Para quién |
|--------|------------|
| Long press (5s) | ESPs con botón |
| Serial: `exit` | ESPs sin botón |

---

## 🔘 ButtonHandler

### Archivo: `ButtonHandler.h`

Detecta eventos de botón por **polling** (sin interrupciones). Se llama en cada ciclo del `loop()`.

### Clase

```cpp
class ButtonHandler {
 public:
  enum Event { NONE, SHORT_PRESS, LONG_PRESS };

  ButtonHandler(uint8_t pin);
  void begin();       // Configura el pin como INPUT_PULLUP
  Event update();     // Revisa el botón y retorna el evento
};
```

### Constantes

| Constante | Valor | Función |
|-----------|-------|---------|
| `DEBOUNCE_TIME` | 50ms | Tiempo mínimo para confirmar que no es ruido eléctrico |
| `LONG_PRESS_TIME` | 5000ms | Tiempo para detectar long press |

### Lógica del `update()`

```
Lee el pin
│
├─ ¿Cambió de estado? → Reiniciar debounce timer
│
├─ ¿Está presionado y pasaron 5s? → Retorna LONG_PRESS (una sola vez)
│
└─ ¿Soltó después del debounce y no fue long press? → Retorna SHORT_PRESS
```

### Variables internas

| Variable | Tipo | Función |
|----------|------|---------|
| `pin` | `uint8_t` | GPIO del botón |
| `lastState` | `bool` | Estado anterior del pin |
| `currentState` | `bool` | Estado actual del pin |
| `pressStart` | `unsigned long` | Momento en que se presionó |
| `lastDebounce` | `unsigned long` | Último cambio de estado |
| `longPressTriggered` | `bool` | Evita disparar long press más de una vez |

---

## 📡 WiFiManager

### Archivo: `WiFiManager.h`

Gestiona la conexión WiFi en dos modos:

| Modo | Cuándo | Qué hace |
|------|--------|----------|
| **STA** | Hay credenciales en EEPROM | Se conecta a la red guardada |
| **AP** | No hay credenciales o fallo | Crea red `ESP-HOTSPOT` con portal de configuración |

### Métodos principales

| Método | Descripción |
|--------|-------------|
| `connect()` | Intenta conexión STA con credenciales guardadas |
| `startAP()` | Crea Access Point para configuración |
| `isConnected()` | Retorna `true` si está conectado a WiFi |
| `getIP()` | Retorna la IP actual (STA o AP) |

---

## 💾 AppConfig

### Archivo: `AppConfig.h`

Lee y escribe configuración persistente en EEPROM.

### Datos almacenados

| Campo | Descripción |
|-------|-------------|
| SSID | Nombre de la red WiFi |
| Password | Contraseña de la red |
| Server URL | URL del Orion Context Broker |

### Métodos principales

| Método | Descripción |
|--------|-------------|
| `begin()` | Inicializa EEPROM y carga configuración |
| `save()` | Guarda configuración actual en EEPROM |
| `getSSID()` | Retorna el SSID guardado |
| `getPassword()` | Retorna la contraseña guardada |
| `getServerURL()` | Retorna la URL del servidor |

---

## 🔑 TokenManager

### Archivo: `TokenManager.h`

Gestiona tokens de autenticación para las peticiones HTTP al servidor.

### Métodos principales

| Método | Descripción |
|--------|-------------|
| `getToken()` | Obtiene o renueva el token de autenticación |
| `isValid()` | Verifica si el token actual es válido |

---

## 🌐 DevWebOTA

### Archivo: `DevWebOTA.h`

Servidor web que se levanta en modo desarrollador. Permite ver información del dispositivo y actualizar el firmware por OTA.

### Métodos principales

| Método | Descripción |
|--------|-------------|
| `begin()` | Inicia el servidor web y configura rutas |
| `handle()` | Procesa peticiones HTTP (llamar en cada `execute()`) |

### Ciclo de vida

```
EstadoDESARROLLADOR::execute()
│
├─ primera_vez == true
│   ├─ devWeb = new DevWebOTA(&server)
│   ├─ devWeb->begin()
│   └─ primera_vez = false
│
├─ devWeb->handle()  ← Cada ciclo
│
└─ Al salir (onExit)
    ├─ delete devWeb
    └─ devWeb = nullptr
```

---

## 🔄 Flujo del Programa (TARS.ino)

### `setup()`

```
1. Serial.begin(115200)
2. Wire.begin(21, 22)           → Inicializa I2C
3. Inicializar sensores          → HDC1080, B-LUX, ADC
4. Inicializar OLED              → SSD1306
5. buttonHandler.begin()         → Configura GPIO como INPUT_PULLUP
6. appConfig.begin()             → Carga configuración de EEPROM
7. stateMachine.begin(INICIO)    → Arranca la máquina de estados
```

### `loop()`

```
loop() se ejecuta en cada ciclo
│
├─ 1. buttonHandler.update()        → Revisa botón
│      │
│      ├─ SHORT_PRESS
│      │   ├─ Actualizar ultima_interaccion
│      │   ├─ Pantalla apagada? → encender
│      │   └─ Pantalla encendida? → screenMode++ (0→1→2→3→0)
│      │   └─ needsUpdate = true
│      │
│      ├─ LONG_PRESS
│      │   └─ flags.dev = !flags.dev  (toggle)
│      │
│      └─ NONE → nada
│
└─ 2. stateMachine.update()         → Ejecuta estado actual
       └─ currentState->execute()
           └─ El estado lee flags y reacciona
```

### Funciones globales en TARS.ino

Funciones que los estados llaman vía `extern`:

| Función | Qué hace |
|---------|----------|
| `readSensors()` | Lee HDC1080 (temp, hum), B-LUX (lux), ADC (ruido). Aplica calibración y filtro |
| `updateDisplay()` | Dibuja en OLED según `screenMode` |
| `displayStateInfo(estado)` | Dibuja header con nombre del estado |
| `displayDeveloperInfo()` | Pantalla del modo desarrollador (IP, sensores) |
| `construirPayload(t, h, l, r)` | Retorna `String` con JSON para HTTP POST |

### Calibración de sensores en `readSensors()`

| Sensor | Ajuste | Valor por defecto |
|--------|--------|-------------------|
| Temperatura | Offset | -3.0 °C |
| Humedad | Offset | +7.0 % |
| Lux | Lineal | `(raw - 6.1551) / 1.3788` |
| Lux | Filtro | Historial de 15 muestras, descarta valores negativos y extremos (>300000) |
| Ruido | Conversión | `ADC * (3.7 / 4096) * 50.0` → dBA |

---

## 📊 Diagrama de Clases

```
┌───────────────────────────────────────────────────────┐
│                  <<abstract>>                         │
│                     State                             │
├───────────────────────────────────────────────────────┤
│ # statemachine: StateMachine*                         │
├───────────────────────────────────────────────────────┤
│ + setStateMachine(StateMachine*): void                │
│ + onEnter(): void        {virtual}                    │
│ + execute(): void        {virtual}                    │
│ + onExit(): void         {virtual}                    │
│ + getName(): const char* {virtual}                    │
└────────────────────────────��──────────────────────────┘
                        △
                        │ hereda
        ┌───────────────┼───────────────┬───────────────┐
        │               │               │               │
┌───────▼──────┐ ┌──────▼──────┐ ┌─────▼──────┐ ┌──────▼─────────┐
│EstadoINICIO  │ │EstadoLECTURA│ │EstadoENVIO │ │EstadoDESARRO-  │
├──────────────┤ ├─────────────┤ ├────────────┤ │  LLADOR        │
│-firstRun     │ │             │ │-ServerName │ ├────────────────┤
├──────────────┤ ├─────────────┤ ├────────────┤ │-primera_vez    │
│+onEnter()    │ │+onEnter()   │ │+onEnter()  │ │-devWeb*        │
│+execute()    │ │+execute()   │ │+execute()  │ ├────────────────┤
│+onExit()     �� │+onExit()    │ │+onExit()   │ │+onEnter()      │
│+getName()    │ │+getName()   │ │+getName()  │ │+execute()      │
└──────────────┘ └─────────────┘ └────────────┘ │+onExit()       │
                                                │+getName()      │
                                                └────────────────┘

┌───────────────────────────────────────────────────────┐
│                  StateMachine                         │
├───────────────────────────────────────────────────────┤
│ - currentState: State*                                │
│ - previousState: State*                               │
│ + flags: Flags                                        │
│ + clocks: Clocks                                      │
│ + sensors: SensorData                                 │
│ + settings: Config                                    │
│ + screenMode: int                                     │
│ + isDisplayOn: bool                                   │
│ + needsUpdate: bool                                   │
├───────────────────────────────────────────────────────┤
│ + begin(State*): void                                 │
│ + update(): void                                      │
│ + ChangeState(State*): void                           │
│ + getCurrentStateName(): const char*                  │
└─────────��─────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────┐
│                  ButtonHandler                        │
├───────────────────────────────────────────────────────┤
│ - pin: uint8_t                                        │
│ - lastState / currentState: bool                      │
│ - pressStart / lastDebounce: unsigned long            │
│ - longPressTriggered: bool                            │
├───────────────────────────────────────────────────────┤
│ + enum Event { NONE, SHORT_PRESS, LONG_PRESS }        │
│ + ButtonHandler(uint8_t pin)                          │
│ + begin(): void                                       │
│ + update(): Event                                     │
└───────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────┐
│              Estructuras de Datos                     │
├───────────────────────────────────────────────────────┤
│ Flags      { inicio, lectura, envio, dev }            │
│ Clocks     { tiempo_actual, tiempo_lectura,           │
│              ultima_interaccion, proximo_envio }       │
│ SensorData { temp, hum, lux, voltage, dbValue }       │
│ Config     { TIEMPO_INACTIVIDAD }                     │
└───────────────────────────────────────────────────────┘
```

---

## 🧪 Pruebas sin Hardware

### Sin sensores conectados

Simular valores para evitar crash por I2C:

```cpp
void readSensors() {
  stateMachine.sensors.temp = 25.0;
  stateMachine.sensors.hum = 50.0;
  stateMachine.sensors.lux = 100.0;
  stateMachine.sensors.voltage = 1.5;
  stateMachine.sensors.dbValue = 45.0;
  Serial.println("[TEST] Sensores simulados");
}
```

### Sin botón físico

Usar el botón BOOT del ESP32:

```cpp
#define BUTTON_PIN 0  // GPIO0 = botón BOOT
```

> ⚠️ No mantener presionado BOOT al resetear (entra en modo descarga).

### Forzar modo desarrollador sin botón

```cpp
// En setup(), después de stateMachine.begin():
stateMachine.flags.dev = true;
```

---

## 📝 Mejoras Pendientes

| Mejora | Prioridad | Descripción |
|--------|-----------|-------------|
| Adelgazar `TARS.ino` | Baja | Extraer funciones a clases: `SensorReader.h`, `DisplayManager.h`, `PayloadBuilder.h` |
| Protección I2C | Media | Verificar conexión de sensores antes de leer para evitar crash del core |
| Persistencia de calibración | Baja | Guardar offsets de calibración en EEPROM en vez de hardcodeados |

---
