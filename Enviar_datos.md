# Enviar datos a Orion Context Broker

**Método:** POST  
**URL:** `http://localhost:1026/v2/entities/`

```json
{
    "id": "TARS_1",
    "type": "Confort_Data",
    "dateObserved": {
        "type": "DateTime",
        "value": "2025-10-03T19:59:27Z"
    },
    "noise": {
        "type": "Number",
        "value": 45.6,
        "metadata": {
            "unitCode": {
                "type": "Text",
                "value": "DB"
            }
        }
    },
    "temperature_digital": {
        "type": "Number",
        "value": 24.5,
        "metadata": {
            "unitCode": {
                "type": "Text",
                "value": "CEL"
            }
        }
    },
    "humidity": {
        "type": "Number",
        "value": 58.3,
        "metadata": {
            "unitCode": {
                "type": "Text",
                "value": "P1"
            }
        }
    },
    "illuminance": {
        "type": "Number",
        "value": 320,
        "metadata": {
            "unitCode": {
                "type": "Text",
                "value": "LUX"
            }
        }
    },
    "presence": {
        "type": "Boolean",
        "value": true
    }
}
```
