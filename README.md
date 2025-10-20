# AgeVital - Asistente Virtual de TARS_1

## Contexto del Proyecto

Este prototipo forma parte del sistema **TARS_1**, diseñado para medir 5 variables ambientales críticas utilizando una arquitectura basada en FIWARE. La rama `agente` contiene la implementación del asistente virtual **AgeVital** (bot de Telegram), que funciona como interfaz de usuario para consultar datos históricos almacenados en CrateDB.

**TARS_1** es el prototipo físico con sensores ambientales conectados a una ESP32.

**AgeVital** es el bot de Telegram que actúa como asistente virtual para consultar y monitorear los datos de TARS_1.

## 🏗️ Arquitectura del Sistema

```
┌─────────────────────┐
│     TARS_1          │
│  (Prototipo con     │
│    Sensores)        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│      ESP32          │
│  (Microcontrolador) │
│   - Lee sensores    │
│   - HTTP POST       │
└──────────┬──────────┘
           │
           │ HTTP
           ▼
┌─────────────────────┐
│  Orion Context      │
│     Broker          │
│  (FIWARE)           │
│   - Gestión NGSIv2  │
│   - Entidades       │
└──────┬──────────────┘
       │
       │ QuantumLeap
       ▼
┌─────────────────────┐
│     CrateDB         │
│  (Persistencia)     │
│   - Time Series     │
│   - Histórico       │
└──────┬──────┬───────┘
       │      │
       │      │ HTTP Queries
       │      │
       ▼      ▼
┌──────────┐ ┌─────────────────┐
│ Grafana  │ │   AgeVital      │
│(Dashboard)│ │  Bot Telegram   │
│          │ │  (Asistente)    │
└──────────┘ └─────────────────┘
```

## 🔄 Flujo de Datos

1. **TARS_1 → ESP32**: Los sensores del prototipo TARS_1 envían lecturas a la ESP32
2. **ESP32 → Orion**: La ESP32 envía datos vía **HTTP POST/PATCH** al Context Broker
3. **Orion → CrateDB**: QuantumLeap persiste automáticamente los datos en CrateDB
4. **CrateDB → AgeVital**: El bot AgeVital consulta datos históricos desde CrateDB vía **HTTP**
5. **CrateDB → Grafana**: Grafana visualiza series temporales
6. **AgeVital → Usuario**: El bot presenta información en Telegram

## 🏗️ Creación de una Entidad NGSIv2

Una **entidad** en el contexto de TARS_1 es un objeto de datos que sigue el estándar **NGSIv2** de FIWARE y representa las variables ambientales medidas por el prototipo.

### Estructura de una Entidad NGSIv2

```json
{
  "id": "urn:ngsi-ld:TARS1:prototype001",
  "type": "EnvironmentalMonitor",
  "temperatura": {
    "type": "Number",
    "value": 25.5,
    "metadata": {
      "timestamp": {
        "type": "DateTime",
        "value": "2025-10-20T16:03:40.000Z"
      },
      "unitCode": {
        "type": "Text",
        "value": "CEL"
      }
    }
  },
  "humedad": {
    "type": "Number",
    "value": 65.2,
    "metadata": {
      "timestamp": {
        "type": "DateTime",
        "value": "2025-10-20T16:03:40.000Z"
      },
      "unitCode": {
        "type": "Text",
        "value": "P1"
      }
    }
  },
  "presion": {
    "type": "Number",
    "value": 1013.25,
    "metadata": {
      "timestamp": {
        "type": "DateTime",
        "value": "2025-10-20T16:03:40.000Z"
      },
      "unitCode": {
        "type": "Text",
        "value": "HPA"
      }
    }
  },
  "calidadAire": {
    "type": "Number",
    "value": 45,
    "metadata": {
      "timestamp": {
        "type": "DateTime",
        "value": "2025-10-20T16:03:40.000Z"
      },
      "unitCode": {
        "type": "Text",
        "value": "AQI"
      }
    }
  },
  "luminosidad": {
    "type": "Number",
    "value": 350,
    "metadata": {
      "timestamp": {
        "type": "DateTime",
        "value": "2025-10-20T16:03:40.000Z"
      },
      "unitCode": {
        "type": "Text",
        "value": "LUX"
      }
    }
  },
  "location": {
    "type": "geo:json",
    "value": {
      "type": "Point",
      "coordinates": [-74.0817, 4.6097]
    }
  },
  "status": {
    "type": "Text",
    "value": "active"
  }
}
```

### Pasos para Crear una Entidad

#### 1. Definir las variables ambientales

TARS_1 mide 5 variables ambientales:
- **Temperatura**
- **Humedad**
- **Presión atmosférica**
- **Calidad del aire**
- **Luminosidad**

#### 2. Registrar la entidad en Orion Context Broker

Desde la ESP32, se envía una petición HTTP POST:

```cpp
// Código ESP32 - Crear entidad en Orion
#include <HTTPClient.h>
#include <ArduinoJson.h>

void crearEntidad() {
  HTTPClient http;
  
  // URL del Orion Context Broker
  http.begin("http://orion:1026/v2/entities");
  http.addHeader("Content-Type", "application/json");
  
  // JSON de la entidad
  StaticJsonDocument<1024> doc;
  doc["id"] = "urn:ngsi-ld:TARS1:prototype001";
  doc["type"] = "EnvironmentalMonitor";
  
  // Temperatura
  JsonObject temp = doc.createNestedObject("temperatura");
  temp["type"] = "Number";
  temp["value"] = 0.0;
  
  // Humedad
  JsonObject hum = doc.createNestedObject("humedad");
  hum["type"] = "Number";
  hum["value"] = 0.0;
  
  // Presión
  JsonObject pres = doc.createNestedObject("presion");
  pres["type"] = "Number";
  pres["value"] = 0.0;
  
  // Calidad del aire
  JsonObject air = doc.createNestedObject("calidadAire");
  air["type"] = "Number";
  air["value"] = 0.0;
  
  // Luminosidad
  JsonObject lux = doc.createNestedObject("luminosidad");
  lux["type"] = "Number";
  lux["value"] = 0.0;
  
  // Status
  JsonObject status = doc.createNestedObject("status");
  status["type"] = "Text";
  status["value"] = "active";
  
  String json;
  serializeJson(doc, json);
  
  int httpCode = http.POST(json);
  
  if (httpCode == 201) {
    Serial.println("Entidad TARS_1 creada exitosamente en Orion");
  } else {
    Serial.printf("Error: %d\n", httpCode);
  }
  
  http.end();
}
```

#### 3. Actualizar valores de la entidad (HTTP PATCH)

```cpp
void actualizarDatos(float temp, float hum, float pres, float air, float lux) {
  HTTPClient http;
  
  String url = "http://orion:1026/v2/entities/urn:ngsi-ld:TARS1:prototype001/attrs";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<512> doc;
  
  // Temperatura
  JsonObject tempObj = doc.createNestedObject("temperatura");
  tempObj["type"] = "Number";
  tempObj["value"] = temp;
  
  // Humedad
  JsonObject humObj = doc.createNestedObject("humedad");
  humObj["type"] = "Number";
  humObj["value"] = hum;
  
  // Presión
  JsonObject presObj = doc.createNestedObject("presion");
  presObj["type"] = "Number";
  presObj["value"] = pres;
  
  // Calidad del aire
  JsonObject airObj = doc.createNestedObject("calidadAire");
  airObj["type"] = "Number";
  airObj["value"] = air;
  
  // Luminosidad
  JsonObject luxObj = doc.createNestedObject("luminosidad");
  luxObj["type"] = "Number";
  luxObj["value"] = lux;
  
  String json;
  serializeJson(doc, json);
  
  int httpCode = http.PATCH(json);
  
  if (httpCode == 204) {
    Serial.println("Datos actualizados en Orion");
  }
  
  http.end();
}
```

#### 4. Suscripción de la entidad a Orion

La ESP32 se registra y actualiza periódicamente:

```cpp
void setup() {
  Serial.begin(115200);
  
  // Conectar WiFi
  conectarWiFi();
  
  // Crear entidad en Orion (solo primera vez)
  crearEntidad();
}

void loop() {
  // Leer variables ambientales
  float temp = leerTemperatura();
  float hum = leerHumedad();
  float pres = leerPresion();
  float air = leerCalidadAire();
  float lux = leerLuminosidad();
  
  // Actualizar entidad cada 30 segundos
  actualizarDatos(temp, hum, pres, air, lux);
  
  delay(30000);
}
```

## 🤖 Bot de Telegram - AgeVital (Asistente Virtual)

**AgeVital** es un asistente virtual implementado como bot de Telegram que consulta **CrateDB** vía **HTTP** para proporcionar información histórica sobre las mediciones de TARS_1.

### Funcionalidades del Asistente

- **Consultas en tiempo real**: Obtiene los últimos datos almacenados en CrateDB
- **Historial de mediciones**: Consulta rangos de tiempo personalizados
- **Alertas automáticas**: Notificaciones cuando los valores exceden umbrales
- **Estadísticas**: Promedios, máximos y mínimos por período
- **Interfaz conversacional**: Comandos intuitivos en español

### Arquitectura del Bot AgeVital

El bot se conecta exclusivamente a **CrateDB** usando el protocolo **HTTP** para realizar consultas SQL.

```python
# bot_agevital.py - Asistente Virtual
import requests
from telegram import Update
from telegram.ext import Application, CommandHandler, ContextTypes

CRATE_URL = "http://crate:4200"

def consultar_crate(query):
    """Ejecuta una query SQL en CrateDB vía HTTP"""
    response = requests.post(
        f"{CRATE_URL}/_sql",
        json={"stmt": query},
        headers={"Content-Type": "application/json"}
    )
    return response.json()

async def status(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Consultar últimas lecturas de TARS_1 desde CrateDB"""
    query = """
    SELECT 
        temperatura,
        humedad,
        presion,
        calidadaire,
        luminosidad,
        time_index
    FROM etenvironmentalmonitor
    WHERE entity_id = 'urn:ngsi-ld:TARS1:prototype001'
    ORDER BY time_index DESC
    LIMIT 1
    """
    
    resultado = consultar_crate(query)
    
    if resultado['rows']:
        datos = resultado['rows'][0]
        mensaje = f"""📊 *Estado Actual de TARS_1*

🌡️ *Temperatura:* {datos[0]}°C
💧 *Humedad:* {datos[1]}%
🌪️ *Presión:* {datos[2]} hPa
🌫️ *Calidad del Aire:* {datos[3]} AQI
💡 *Luminosidad:* {datos[4]} lux

⏰ *Última actualización:* {datos[5]}
"""
        await update.message.reply_text(mensaje, parse_mode='Markdown')
    else:
        await update.message.reply_text("❌ No hay datos disponibles de TARS_1")

async def temperatura(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Consultar temperatura"""
    query = """
    SELECT temperatura, time_index
    FROM etenvironmentalmonitor
    WHERE entity_id = 'urn:ngsi-ld:TARS1:prototype001'
    ORDER BY time_index DESC
    LIMIT 1
    """
    
    resultado = consultar_crate(query)
    
    if resultado['rows']:
        temp = resultado['rows'][0][0]
        tiempo = resultado['rows'][0][1]
        await update.message.reply_text(
            f"🌡️ *Temperatura actual:* {temp}°C\n"
            f"⏰ *Medido:* {tiempo}",
            parse_mode='Markdown'
        )

async def historial(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Consultar historial de las últimas 24 horas"""
    query = """
    SELECT 
        temperatura,
        humedad,
        presion,
        calidadaire,
        luminosidad,
        time_index
    FROM etenvironmentalmonitor
    WHERE entity_id = 'urn:ngsi-ld:TARS1:prototype001'
    AND time_index >= now() - interval '24 hours'
    ORDER BY time_index DESC
    LIMIT 10
    """
    
    resultado = consultar_crate(query)
    
    if resultado['rows']:
        mensaje = "📈 *Historial últimas 24h (últimas 10 lecturas)*\n\n"
        
        for i, row in enumerate(resultado['rows'], 1):
            mensaje += f"*Lectura {i}:*\n"
            mensaje += f"🌡️ {row[0]}°C | 💧 {row[1]}% | "
            mensaje += f"🌪️ {row[2]}hPa\n"
            mensaje += f"🌫️ {row[3]}AQI | 💡 {row[4]}lux\n"
            mensaje += f"⏰ {row[5]}\n\n"
        
        await update.message.reply_text(mensaje, parse_mode='Markdown')

async def promedio(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Calcular promedios del día"""
    query = """
    SELECT 
        AVG(temperatura) as temp_avg,
        AVG(humedad) as hum_avg,
        AVG(presion) as pres_avg,
        AVG(calidadaire) as air_avg,
        AVG(luminosidad) as lux_avg
    FROM etenvironmentalmonitor
    WHERE entity_id = 'urn:ngsi-ld:TARS1:prototype001'
    AND time_index >= date_trunc('day', now())
    """
    
    resultado = consultar_crate(query)
    
    if resultado['rows']:
        datos = resultado['rows'][0]
        mensaje = f"""📊 *Promedios del día - TARS_1*

🌡️ *Temp. promedio:* {datos[0]:.2f}°C
💧 *Humedad promedio:* {datos[1]:.2f}%
🌪️ *Presión promedio:* {datos[2]:.2f} hPa
🌫️ *Calidad aire promedio:* {datos[3]:.2f} AQI
💡 *Luminosidad promedio:* {datos[4]:.2f} lux
"""
        await update.message.reply_text(mensaje, parse_mode='Markdown')

async def alertas(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Verificar si hay valores fuera de rango"""
    query = """
    SELECT 
        temperatura,
        humedad,
        presion,
        calidadaire
    FROM etenvironmentalmonitor
    WHERE entity_id = 'urn:ngsi-ld:TARS1:prototype001'
    ORDER BY time_index DESC
    LIMIT 1
    """
    
    resultado = consultar_crate(query)
    
    if resultado['rows']:
        temp, hum, pres, air = resultado['rows'][0]
        alertas = []
        
        if temp > 30:
            alertas.append("⚠️ Temperatura alta (>30°C)")
        if temp < 10:
            alertas.append("⚠️ Temperatura baja (<10°C)")
        if hum > 80:
            alertas.append("⚠️ Humedad alta (>80%)")
        if hum < 30:
            alertas.append("⚠️ Humedad baja (<30%)")
        if air > 100:
            alertas.append("⚠️ Calidad del aire deficiente (>100 AQI)")
        
        if alertas:
            mensaje = "🚨 *Alertas Activas*\n\n" + "\n".join(alertas)
        else:
            mensaje = "✅ Todos los parámetros en rango normal"
        
        await update.message.reply_text(mensaje, parse_mode='Markdown')

async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    """Mostrar ayuda"""
    mensaje = """🤖 *AgeVital - Asistente de TARS_1*

*Comandos disponibles:*

/status - Estado actual de todas las variables
/temp - Temperatura actual
/humedad - Humedad actual
/presion - Presión atmosférica
/aire - Calidad del aire
/luz - Luminosidad actual
/historial - Últimas 10 lecturas (24h)
/promedio - Promedios del día
/alertas - Verificar alertas
/grafana - Enlace al dashboard
/help - Mostrar esta ayuda

_Datos obtenidos desde CrateDB vía HTTP_
"""
    await update.message.reply_text(mensaje, parse_mode='Markdown')

def main():
    """Iniciar el bot AgeVital"""
    application = Application.builder().token("TU_TOKEN_AQUI").build()
    
    # Registrar comandos
    application.add_handler(CommandHandler("status", status))
    application.add_handler(CommandHandler("temp", temperatura))
    application.add_handler(CommandHandler("historial", historial))
    application.add_handler(CommandHandler("promedio", promedio))
    application.add_handler(CommandHandler("alertas", alertas))
    application.add_handler(CommandHandler("help", help_command))
    
    # Iniciar bot
    application.run_polling()

if __name__ == '__main__':
    main()
```

### Configuración del Bot AgeVital

1. **Crear bot en Telegram**
   - Hablar con [@BotFather](https://t.me/botfather)
   - Comando: `/newbot`
   - Nombre: AgeVital
   - Username: @agevital_bot (o el que prefieras)
   - Guardar el token de API

2. **Instalar dependencias**
   ```bash
   pip install python-telegram-bot requests
   ```

3. **Configurar variables de entorno**
   ```bash
   export TELEGRAM_TOKEN="tu_token_de_telegram"
   export CRATE_HOST="http://localhost:4200"
   ```

4. **Ejecutar el bot**
   ```bash
   python bot_agevital.py
   ```

## 📊 Variables Ambientales Monitoreadas por TARS_1

| # | Variable | Rango | Unidad |
|---|----------|-------|--------|
| 1 | Temperatura | -20°C a 50°C | °C (Celsius) |
| 2 | Humedad Relativa | 0% a 100% | % (Porcentaje) |
| 3 | Presión Atmosférica | 300 hPa a 1100 hPa | hPa (Hectopascal) |
| 4 | Calidad del Aire | 0-500 | AQI (Air Quality Index) |
| 5 | Luminosidad | 0 a 100,000 | lux |

## 🚀 Comandos del Bot AgeVital

- `/start` - Iniciar interacción con AgeVital
- `/status` - Estado actual de todas las variables de TARS_1
- `/temp` - Consultar temperatura
- `/humedad` - Consultar humedad
- `/presion` - Consultar presión atmosférica
- `/aire` - Consultar calidad del aire
- `/luz` - Consultar luminosidad
- `/historial` - Últimas 10 lecturas (24 horas)
- `/promedio` - Promedios del día actual
- `/alertas` - Verificar si hay valores fuera de rango
- `/grafana` - Obtener enlace al dashboard
- `/help` - Ayuda y lista de comandos

## 🔧 Instalación y Configuración Completa

### Requisitos del Sistema

- **Hardware**: ESP32 (TARS_1)
- **Software**: 
  - Docker & Docker Compose
  - Arduino IDE / PlatformIO
  - Python 3.8+

### Paso 1: Configurar FIWARE Stack

```yaml
# docker-compose.yml
version: '3.8'

services:
  orion:
    image: fiware/orion:latest
    ports:
      - "1026:1026"
    command: -dbhost mongo
    depends_on:
      - mongo

  mongo:
    image: mongo:4.4
    volumes:
      - mongo-data:/data/db

  crate:
    image: crate:latest
    ports:
      - "4200:4200"
      - "4300:4300"
    environment:
      - CRATE_HEAP_SIZE=2g

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    volumes:
      - grafana-data:/var/lib/grafana

  quantumleap:
    image: orchestracities/quantumleap:latest
    ports:
      - "8668:8668"
    environment:
      - CRATE_HOST=crate
    depends_on:
      - crate

volumes:
  mongo-data:
  grafana-data:
```

```bash
docker-compose up -d
```

### Paso 2: Configurar ESP32 (TARS_1)

```cpp
// config.h
#define WIFI_SSID "tu_red_wifi"
#define WIFI_PASSWORD "tu_password"
#define ORION_HOST "192.168.1.100"  // IP de tu servidor
#define ORION_PORT 1026
#define UPDATE_INTERVAL 30000  // 30 segundos
```

### Paso 3: Configurar Bot AgeVital

```python
# config.py
TELEGRAM_TOKEN = "TU_TOKEN_DE_BOTFATHER"
CRATE_URL = "http://localhost:4200"
```

### Paso 4: Crear Suscripción en Orion para QuantumLeap

```bash
curl -iX POST \
  'http://localhost:1026/v2/subscriptions' \
  -H 'Content-Type: application/json' \
  -d '{
  "description": "Notify QuantumLeap of TARS_1 changes",
  "subject": {
    "entities": [
      {
        "id": "urn:ngsi-ld:TARS1:prototype001",
        "type": "EnvironmentalMonitor"
      }
    ]
  },
  "notification": {
    "http": {
      "url": "http://quantumleap:8668/v2/notify"
    },
    "attrs": [
      "temperatura",
      "humedad",
      "presion",
      "calidadAire",
      "luminosidad"
    ]
  }
}'
```

## 📈 Visualización en Grafana

1. Acceder a Grafana: `http://localhost:3000`
2. Agregar CrateDB como datasource
   - Type: PostgreSQL
   - Host: `crate:5432`
   - Database: `doc`
   - User: `crate`
   - SSL Mode: `disable`

3. Crear dashboard con queries:

```sql
-- Query para temperatura
SELECT 
  time_index as time,
  temperatura as value
FROM 
  etenvironmentalmonitor
WHERE 
  entity_id = 'urn:ngsi-ld:TARS1:prototype001'
  AND time_index >= now() - interval '24 hours'
ORDER BY time_index

-- Query para humedad
SELECT 
  time_index as time,
  humedad as value
FROM 
  etenvironmentalmonitor
WHERE 
  entity_id = 'urn:ngsi-ld:TARS1:prototype001'
  AND time_index >= now() - interval '24 hours'
ORDER BY time_index

-- Query para todas las variables
SELECT 
  time_index as time,
  temperatura,
  humedad,
  presion,
  calidadaire,
  luminosidad
FROM 
  etenvironmentalmonitor
WHERE 
  entity_id = 'urn:ngsi-ld:TARS1:prototype001'
  AND time_index >= now() - interval '7 days'
ORDER BY time_index
```

## 📝 Notas Técnicas

- **Prototipo**: TARS_1 (ESP32 + 5 variables ambientales)
- **Asistente**: AgeVital (Bot de Telegram)
- **Protocolo de comunicación**: HTTP
- **ESP32 → Orion**: HTTP POST/PATCH
- **AgeVital → CrateDB**: HTTP (SQL queries)
- **Context Broker**: Orion NGSIv2
- **Persistencia**: CrateDB (Time-Series Database)
- **Bot Framework**: python-telegram-bot
- **Visualización**: Grafana

## 🔐 Seguridad

- Implementar autenticación en Orion Context Broker
- Usar HTTPS para comunicación ESP32-Orion en producción
- Encriptar credenciales WiFi en ESP32
- Validar usuarios autorizados en bot de Telegram
- Configurar firewall para CrateDB (solo acceso interno)

## 🔧 Desarrollo Futuro

- [ ] Predicciones con machine learning
- [ ] Alertas proactivas vía Telegram
- [ ] Soporte para múltiples prototipos TARS
- [ ] Exportación de datos (CSV, JSON, Excel)
- [ ] Dashboard web responsive
- [ ] Integración con servicios meteorológicos
- [ ] Comandos por voz en el bot
- [ ] Reportes automáticos diarios/semanales

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit tus cambios (`git commit -m 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abre un Pull Request

## 📄 Licencia

Este proyecto es un prototipo educativo/de investigación.

## 📚 Referencias

- [FIWARE Documentation](https://fiware.org/developers/)
- [Orion Context Broker](https://fiware-orion.readthedocs.io/)
- [NGSIv2 Specification](https://fiware.github.io/specifications/ngsiv2/stable/)
- [CrateDB Documentation](https://crate.io/docs/)
- [CrateDB SQL Reference](https://crate.io/docs/crate/reference/en/latest/sql/index.html)
- [Telegram Bot API](https://core.telegram.org/bots/api)
- [python-telegram-bot](https://python-telegram-bot.org/)

## 👥 Equipo

**Desarrolladores**: Grupo AgeVital 
**Proyecto**: TARS_1 (Prototipo 5)  
**Asistente**: AgeVital Bot

---

**Versión**: Prototipo 5  
**Última actualización**: 2025-10-20 16:03:40 UTC  
**Arquitectura**: FIWARE + ESP32 + Telegram  
**Protocolo**: HTTP
