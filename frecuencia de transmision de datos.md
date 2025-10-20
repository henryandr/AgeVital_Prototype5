# Estrategia de recolección y envío de datos para sensores ambientales

## Objetivo

Diseñar una estrategia de muestreo, agregación y transmisión por Wi‑Fi para sensores de temperatura, humedad, ruido y luminancia que:

- Minimice el consumo energético (reducir tiempo de radio activo, procesador y muestreo innecesario).
- Mantenga información representativa del entorno enviando promedios y/o valores relevantes.
- Sea simple y robusta frente a pérdidas temporales de conectividad.

---

## Resumen de la estrategia (visión general)

- Ventana de transmisión: cada 5 minutos el nodo despierta y transmite los datos agregados.
- Temperatura y Humedad:
  - Frecuencia de muestreo (fm): 0.1 Hz → 1 dato cada 10 s.
  - En 5 min se recolectan 30 muestras (6 por minuto × 5).
  - Se calcula: promedio de las 30 muestras + último valor recolectado.
  - Se envía un único paquete con: promedio (5 min) y último valor.
- Ruido (sonómetro):
  - fm: 1 Hz → 1 dato por segundo.
  - Se toman 60 muestras cada minuto → se promedia por minuto → 1 paquete por minuto (promedio min).
  - Durante 5 min se generan 5 paquetes (uno por minuto).
  - Se recogen 5 paquetes (5 minutos) y se envían esos 5 paquetes (es decir, las 5 medias minuto) como una serie de 5 valores en la transmisión de 5 min.
- Luminancia:
  - fm: 0.1 Hz → 1 dato cada 10 s.
  - En 1 minuto se toman 6 muestras y se promedian → 1 paquete por minuto (media min).
  - Se recogen 5 paquetes (5 minutos) y se envían esos 5 paquetes (es decir, las 5 medias minuto) como una serie de 5 valores en la transmisión de 5 min.

---

## Motivación de la configuración

- Temperatura y humedad cambian lentamente → muestreo bajo (0.1 Hz) y envío de promedio + último para conservar tendencia y valor actual.
- Ruido es variable y con cambios rápidos → muestreo alto (1 Hz). Para ahorrar energía y ancho de banda se realiza agregación jerárquica (promedio por minuto → promedio de 5 minutos).
- Luminancia puede tener cambios a escala de minutos (paso de sombra/luz) → se prefirió enviar las 5 medias minuto (serie breve) para mantener resolución temporal suficiente sin enviar todos los puntos crudos.

---

## Formato de paquete / Payload (propuesto JSON compacto)

En la transmisión cada 5 minutos se enviará un JSON compacto con metadatos y bloques por sensor. Ejemplo:

{
"node_id": "node-01",
"ts_start": "2025-10-20T10:00:00Z",
"ts_end": "2025-10-20T10:05:00Z",
"battery_pct": 87,
"temperature": {
"avg_5min": 23.6,
"last": 23.8,
"samples": 30
},
"humidity": {
"avg_5min": 45.2,
"last": 45.1,
"samples": 30
},
"noise": {
"avg_5min": 56.8,
"per_min_avgs": [57.1, 56.7, 56.5, 56.9, 56.8], // opcional si se quiere detalle
"samples_total": 300
},
"luminance": {
"per_min_avgs": [120.5, 118.3, 115.2, 122.0, 119.6] // 5 valores (cada uno es promedio de 6 muestras)
}
}

Notas:

- El campo `per_min_avgs` puede omitirse para reducir tamaño si no es necesario; mantener solo el valor agregado.
- Incluir siempre timestamps start/end y `battery_pct` para gestión energética y diagnósticos.
- Usar compresión simple (por ejemplo minificar JSON o usar CBOR) si es necesario reducir bytes enviados.

---

## Algoritmo de recolección y agregación (pseudocódigo)

Loop principal (duración 5 minutos):

- Inicializar contadores/acuumuladores:
  - temp_sum = 0; temp_count = 0; temp_last = null
  - hum_sum = 0; hum_count = 0; hum_last = null
  - noise_minute_sum = 0; noise_minute_count = 0; noise_minute_avgs = []
  - luminance_minute_sum = 0; luminance_minute_count = 0; luminance_minute_avgs = []
- Establecer t0 = ahora()
- Mientras ahora() < t0 + 5 minutos:
  - Si es momento de muestrear temperatura/humedad (cada 10 s):
    - leer temp, hum
    - temp_sum += temp; temp_count++
    - temp_last = temp
    - hum_sum += hum; hum_count++
    - hum_last = hum
  - Si es momento de muestrear ruido (cada 1 s):
    - leer ruido
    - noise_minute_sum += ruido; noise_minute_count++
    - Si pasó 1 minuto (noise_minute_count == 60):
      - noise_minute_avgs.append(noise_minute_sum / noise_minute_count)
      - noise_minute_sum = 0; noise_minute_count = 0
  - Si es momento de muestrear luminancia (cada 10 s):
    - leer lux
    - luminance_minute_sum += lux; luminance_minute_count++
    - Si pasó 1 minuto (luminance_minute_count == 6):
      - luminance_minute_avgs.append(luminance_minute_sum / luminance_minute_count)
      - luminance_minute_sum = 0; luminance_minute_count = 0
  - Mantener al MCU en modo bajo consumo entre muestreos (sleep).
- Al finalizar ventana 5 min:
  - temp_avg_5min = temp_sum / temp_count (debería ser 30)
  - hum_avg_5min = hum_sum / hum_count
  - noise_avg_5min = average(noise_minute_avgs) (debería tener 5 valores)
  - luminance_per_min_values = luminance_minute_avgs (5 valores)
- Activar radio, transmitir JSON con estructura definida.
- Volver a modo bajo consumo y repetir.

---

## Gestión energética recomendada (prácticas)

- Duty cycle del radio: mantener radio apagada salvo el momento de transmisión (cada 5 min). Apertura y autenticación Wi‑Fi solo cuando sea necesario.
- Modo bajo consumo del MCU: entre muestreos y mientras hay buffer para llenar, entrar en sleep profundo con RTC despertador para el siguiente muestreo.
- Sincronización de reloj: usar RTC del nodo para programar muestreos y transmisiones regulares; minimizar polling.
- Agrupación de tareas: wake → completar tareas de agregación remanente → encender radio → transmitir → apagar radio → dormir.
- Reintentos controlados: si falla la transmisión, reintentar hasta N veces con backoff exponencial corto; si sigue fallando, guardar último paquete en memoria no volátil y reintentar en la siguiente ventana.
- Monitor de batería y reducción adaptativa: si battery_pct < threshold (ej. 20%), aumentar periodo de transmisión a 10 min o reducir muestreo (ej. bajar ruido a 0.5 Hz o enviar solo promedios).

---

## Manejo de errores y pérdidas de muestra

- Contador de samples: enviar `samples` por sensor para verificar integridad.
- Si falta un minuto de noise (por reinicio), computar promedio con los minutos disponibles (dividir por el número real de minutos).
- Guardar en memoria circular (ring buffer) las últimas K ventanas (ej. K=12 → 1 h) para retransmisión en caso de fallos prolongados.
- Registro de estado: incluir flags de error en el payload (p. ej. "sensor_fault": ["noise"]).

---

## Consideraciones de diseño y opciones de mejora

- Ruido: además de promedio, considerar enviar RMS o percentiles (p95) si se requiere medir picos.
- Compresión: si el ancho de banda o consumo es crítico, usar CBOR o mensaje binario en lugar de JSON.
- Adaptación dinámica: ajuste automático de fm según varianza de los últimos N valores (si muy estable, reducir muestreo).
- Seguridad: usar TLS o DTLS para transmisión de datos. Considerar coste energético del cifrado en MCU.
- Timestamps por muestra: no es necesario guardarlos todos; con promedios basta con ts_start/ts_end y, opcionalmente, el timestamp del último valor.

---

## Ejemplo numérico (confirmando cuentas)

- Temperatura/Humedad: 0.1 Hz → 1 dato / 10 s → 6/min → en 5 min = 30 muestras.
- Ruido: 1 Hz → 60/min → en 5 min = 300 muestras → se reducen a 5 medias minuto → finalmente 1 media 5‑min.
- Luminancia: 0.1 Hz → 6/min → en 1 min se promedian → 1 valor por minuto → en 5 min = 5 valores a enviar.

---

## Parámetros configurables (para archivo de configuración)

- sampling.temp_hum_interval_s = 10
- sampling.noise_interval_s = 1
- sampling.lux_interval_s = 10
- tx.window_minutes = 5
- tx.max_retries = 3
- power.battery_threshold_pct = 20
- buffer.max_windows_saved = 12

---

## Checklist para implementación

- [ ] Implementar temporizadores RTC para muestreos.
- [ ] Implementar buffers y lógica de agregación (sum/count por sensor).
- [ ] Implementar wake→agregación→tx→sleep flow.
- [ ] Definir y validar esquema de payload (JSON/CBOR).
- [ ] Probar pérdida de conectividad y reintentos.
- [ ] Medir consumo energético real y ajustar parámetros si es necesario.
