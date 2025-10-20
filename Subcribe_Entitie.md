# Crear suscripción en Orion Context Broker

**Método:** POST  
**URL:** `http://localhost:1026/v2/subscriptions`

```json
{
    "description": "Subscripción para datos ambientales",
    "subject": {
        "entities": [
            {
                "idPattern": "TARS_1",
                "type": "Confort_Data"
            }
        ],
        "condition": {
            "attrs": [
                "noise",
                "temperature_digital",
                "humidity",
                "illuminance",
                "presence"
            ]
        }
    },
    "notification": {
        "attrs": [
            "noise",
            "temperature_digital",
            "humidity",
            "illuminance",
            "presence"
        ],
        "http": {
            "url": "http://quantumleap:8668/v2/notify"
        },
        "metadata": [
            "dateCreated",
            "dateModified"
        ]
    }
}
```
