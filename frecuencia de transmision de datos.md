Documentación Detallada de la Estrategia de Ahorro de Energía

1. Variables de Control de Ciclo (Memoria RTC)
   Estas variables se declaran con el atributo RTC_DATA_ATTR en el código, asegurando que sus valores persistan incluso después de que el ESP32 se reinicia desde el modo Deep Sleep.

Variable Ciclo de Uso Propósito
RTC_DATA_ATTR int contador_5min; Ciclo de 5 minutos Cuenta las mediciones (1 a 5) para T°, Humedad, y Ruido.
RTC_DATA_ATTR int contador_10min; Ciclo de 10 minutos Cuenta las mediciones (1 a 10) para el sensor de Luz.
RTC_DATA_ATTR int transmision_flag; Control General Indica si en este despertar (minuto 5 o 10) se debe transmitir.

2. Sensores Ambientales (Transmisión cada 5 Minutos)
   Esta categoría incluye los sensores de Temperatura, Humedad y Ruido. El ciclo de medición es de 1 vez por minuto (total 5 mediciones por ciclo).

A. Sensor de Temperatura y Humedad (DHT22 / HDC1080)
Frecuencia de Medición: 1 vez por minuto.

Frecuencia de Transmisión: Cada 5 minutos.

Lógica de Datos:

Acumulación (Minutos 1 a 4): Se toma la lectura y se suma al acumulador en la Memoria RTC (temp_suma += lectura).

Transmisión (Minuto 5): Se toma la última lectura. Se calcula el Promedio (temp_suma / 5.0) y se envía junto con el Último Valor (ultimo_temp).

B. Sensor de Ruido (MAX4466)
Frecuencia de Medición: 1 vez por minuto (en ráfaga corta).

Frecuencia de Transmisión: Cada 5 minutos.

Optimización de Hardware: El pin VCC del módulo MAX4466 se debe conectar a un pin GPIO del ESP32. El sensor solo se enciende (digitalWrite(PIN_VCC_RUÍDO, HIGH)) durante la ráfaga de medición (aprox. 100ms) y luego se apaga (digitalWrite(PIN_VCC_RUÍDO, LOW)), garantizando que no consuma energía constante.

Lógica de Datos:

Acumulación (Minutos 1 a 4): Se enciende el sensor, se toma una ráfaga de muestras, se registra el valor de pico de ruido, se suma al acumulador (ruido_suma += pico) y se apaga el sensor.

Transmisión (Minuto 5): Se calcula el Promedio de Picos (ruido_suma / 5.0) y se envía junto con el Último Valor de Pico registrado.

3. Sensor de Luz (SEN0390) - Transmisión Lenta
   Este sensor utiliza un ciclo más largo, reflejando el cambio típicamente más lento de la iluminación ambiental a lo largo del tiempo.

Sensor de Luz (SEN0390 o Similar)
Frecuencia de Medición: 1 vez por minuto.

Frecuencia de Transmisión: Cada 10 minutos.

Lógica de Control: Depende del contador_10min. El dispositivo solo transmitirá datos de luz (y reiniciará el acumulador de luz) cuando el contador llegue a 10.

Lógica de Datos:

Acumulación (Minutos 1 a 9): Se toma la lectura y se suma al acumulador (luz_suma += lectura).

Transmisión (Minuto 10): Se toma la última lectura. Se calcula el Promedio (luz_suma / 10.0) y se envía junto con el Último Valor (ultimo_luz).

4. Sensor de Presencia (PIR HC-SR501) - Transmisión por Evento
   Este sensor prioriza la detección instantánea y el ahorro de energía mediante la interrupción externa.

Sensor de Presencia (PIR HC-SR501)
Frecuencia de Medición: Continuo (en modo de bajo consumo).

Frecuencia de Transmisión: Inmediata (solo al detectar movimiento).

Lógica de Despertar: El pin de salida del PIR se conecta a un pin de interrupción RTC GPIO del ESP32.

El ESP32 se programa para despertar (esp_sleep_enable_ext0_wakeup) si el pin del PIR pasa a HIGH.

Si el reinicio es causado por el PIR, el dispositivo ignora los contadores de ciclos.

Acción de Transmisión: Se activa inmediatamente la radio Wi-Fi y se envía una alerta al servidor (ej., "ALERTA: Movimiento detectado").

Retorno al Ahorro: Después de la transmisión, el dispositivo vuelve a su ciclo normal de Deep Sleep.

5. Resumen del Flujo de Transmisión (Transmisión de Lote)
   Cuando la transmision_flag se activa (en el minuto 5 o 10):

Activación de Radio: Se enciende la radio Wi-Fi (el momento de mayor consumo de energía).

Cálculo: Se ejecutan los cálculos:

Promedio= suma acumulada/numero de muestras
​

Construcción del Paquete: Se construye un único mensaje MQTT (o JSON) que incluye todos los datos calculados:

temp_promedio, temp_ultimo

hum_promedio, hum_ultimo

ruido_promedio, ruido_ultimo

luz_promedio, luz_ultimo (solo si el contador_10min llega a 10)

Envío y Desconexión: Se envía el paquete de datos masivo. La radio Wi-Fi se apaga inmediatamente para conservar energía.

Limpieza: Se reinician todas las variables \_suma, contador_5min y/o contador_10min a cero para el siguiente ciclo.
