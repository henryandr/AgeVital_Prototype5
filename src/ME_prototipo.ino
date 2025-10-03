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
#include "ESPaccesspoint.h"
#include "Settings.h"

// --- Definiciones de hardware ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SoundSensorPin 35
#define VREF 3.3
#define BUTTON_PIN 27
#define DEV_PIN 26 // Pin para modo desarrollador

// --- Factores de calibración ---
const float NOISE_MULTIPLIER = 0.025;
const float NOISE_OFFSET = 50.0;
const float TEMP_OFFSET = -3;
const float HUM_OFFSET = 7.0;
const float LUX_CALIBRATION_FACTOR = 0.613;

// --- Objetos ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ClosedCube_HDC1080 hdc1080;
DFRobot_B_LUX_V30B luxSensor(&Wire, /*cEN=*/5);

// --- Estados del sistema ---
enum class Estado : uint8_t
{
    INICIO,
    LECTURA,
    ENVIO,
    DESARROLLADOR
};

// --- Estructura de banderas ---
struct Flag
{
    bool inicio = false;
    bool lectura = false;
    bool envio = false;
    bool dev = false;
    bool envio_programado = false;
    volatile bool boton_presionado = false;
} flags;

// --- Estructura de tiempos ---
struct Clock
{
    unsigned long tiempo_lectura = 0;
    unsigned long tiempo_actual = 0;
    unsigned long ultima_interaccion = 0;
    unsigned long ultimo_debounce = 0;
    unsigned long proximo_envio = 0;
} clocks;

// --- Variables globales ---
Estado estado_actual = Estado::INICIO;
volatile int screenMode = 0;
const unsigned long INTERVALO_ENVIO = 15000;     // 15 segundos entre envíos
const unsigned long INTERVALO_LECTURA = 2000;    // 2 segundos entre lecturas
const unsigned long TIEMPO_INACTIVIDAD = 10000;  // 10 segundos para apagar pantalla
const unsigned long DEBOUNCE_TIME = 300;         // 300 ms para debounce
const unsigned long INTERVALO_REINTENTO = 60000; // 60 segundos para reintento
const char *ServerName = "http://10.38.32.137:1026/v2/entities/AmbientMonitor_001/attrs";
bool isDisplayOn = true;
bool needsUpdate = false;

// --- Variables para el filtro de datos (lux) ---
const int LUX_HISTORY_SIZE = 15;
float luxHistory[LUX_HISTORY_SIZE];
int luxHistoryIndex = 0;
bool luxHistoryFull = false;

// --- Variables de sensores ---
float temp = 0, hum = 0, lux = 0, dbValue = 0;

// --- Función de interrupción del botón ---
void IRAM_ATTR handleButtonPress()
{
    unsigned long now = millis();
    if (now - clocks.ultimo_debounce > DEBOUNCE_TIME)
    {
        flags.boton_presionado = true;
        clocks.ultimo_debounce = now;
        clocks.ultima_interaccion = now;
    }
}

// Access point
WebServer server(80);
Settings settings;

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(4096);
    settings.load(); // Cargar configuración de WiFi desde EEPROM
    settings.info(); // Mostrar información de configuración
    Wire.begin(21, 22);

    // Inicialización de sensores
    hdc1080.begin(0x40);
    luxSensor.begin();
    analogReadResolution(12); // Configurar la resolución ADC para el sensor de ruido

    // Configuración de la pantalla
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("Error OLED");
        while (1)
            ;
    }

    // Configuración de pines
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(DEV_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

    // Inicializar filtro de lux
    lux = luxSensor.lightStrengthLux();
    if (lux < 0)
        lux = 0;
    for (int i = 0; i < LUX_HISTORY_SIZE; i++)
    {
        luxHistory[i] = lux;
    }

    // Configuración inicial
    if (digitalRead(DEV_PIN) == LOW)
    {
        estado_actual = Estado::DESARROLLADOR;
        flags.dev = true;
    }
    else
    {
        estado_actual = Estado::INICIO;
        flags.inicio = true;
    }
}

void loop()
{
    clocks.tiempo_actual = millis();

    switch (estado_actual)
    {
    case Estado::INICIO:
    {
        if (flags.dev)
        {
            Serial.println("Pasando al estado de DESARROLLADOR...");
            estado_actual = Estado::DESARROLLADOR;
            break;
        }

        Serial.println("Estado: INICIO");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        displayStateInfo("INICIO");
        display.setCursor(0, 25);
        display.println("Iniciando...");
        display.display();

        estado_actual = Estado::LECTURA;
        flags.lectura = true;
        flags.inicio = false;

        // Programar primer envío
        clocks.proximo_envio = clocks.tiempo_actual + INTERVALO_ENVIO;
        flags.envio_programado = true;
    }
    break;

    case Estado::LECTURA:
    {
        if (flags.dev)
        {
            estado_actual = Estado::DESARROLLADOR;
            break;
        }

        // Control de inactividad
        if (isDisplayOn && (clocks.tiempo_actual - clocks.ultima_interaccion > TIEMPO_INACTIVIDAD))
        {
            display.clearDisplay();
            display.display();
            isDisplayOn = false;
        }

        // Manejo del botón
        if (flags.boton_presionado)
        {
            if (!isDisplayOn)
            {
                isDisplayOn = true;
                needsUpdate = true;
            }
            else
            {
                screenMode = (screenMode + 1) % 4;
                needsUpdate = true;
            }
            flags.boton_presionado = false;
            clocks.ultima_interaccion = clocks.tiempo_actual;
        }

        // Actualización periódica de sensores
        if (clocks.tiempo_actual - clocks.tiempo_lectura >= INTERVALO_LECTURA)
        {
            Serial.println("Estado: LECTURA");
            readSensors();
            if (isDisplayOn)
            {
                updateDisplay();
            }
            clocks.tiempo_lectura = clocks.tiempo_actual;
        }

        // Verificar si es tiempo de envío
        if (flags.envio_programado && (long)(clocks.tiempo_actual - clocks.proximo_envio) >= INTERVALO_ENVIO)
        {
            estado_actual = Estado::ENVIO;
            flags.envio = true;
            Serial.println("Pasando al estado de ENVIO...");
        }
    }
    break;

    case Estado::ENVIO:
    {
        if (flags.dev)
        {
            estado_actual = Estado::DESARROLLADOR;
            break;
        }

        Serial.println("Estado: ENVIO");

        if (WiFi.status() == WL_CONNECTED)
        {
            // Intentar envío
            HTTPClient http;
            http.begin(ServerName);
            http.addHeader("Content-Type", "application/json");

            String payload = construirJson(temp, hum, lux, dbValue);
            int httpResponseCode = http.POST(payload);

            if (httpResponseCode > 0)
            {
                Serial.printf("Envío exitoso, código: %d\n", httpResponseCode);
                // Programar siguiente envío normal
                clocks.proximo_envio = clocks.tiempo_actual + INTERVALO_ENVIO;
            }
            else
            {
                Serial.printf("Error en envío: %s\n", http.errorToString(httpResponseCode).c_str());
            }

            http.end();

            clocks.proximo_envio = clocks.tiempo_actual + INTERVALO_ENVIO;
        }
        else
        {
            Serial.println("Sin conexión WiFi, posponiendo envío");
            // Programar siguiente intento en más tiempo
            clocks.proximo_envio = clocks.tiempo_actual + INTERVALO_REINTENTO;
        }

        flags.envio_programado = true;
        flags.envio = false;
        flags.lectura = true;
        estado_actual = Estado::LECTURA;
    }
    break;

    case Estado::DESARROLLADOR:
    {

        static bool primera_vez = true; // Vamos a asegurar StartAPorSTA solo una vez

        if (primera_vez)
        {
            startAPorSTA(settings); // Solo se ejecuta una vez para evitar que se inicie por iteracion
            primera_vez = false;
        }

        server.handleClient(); // Manejar las solicitudes del cliente
        displayDeveloperInfo();

        if (!digitalRead(DEV_PIN))
        {
            flags.dev = false;
            primera_vez = true; // Resetear para la próxima vez
            estado_actual = Estado::INICIO;
            break;
        }
        delay(100); // Pequeño delay para hacer parecer que piensa
    }
    break;

    default:
        estado_actual = Estado::INICIO;
        break;
    }
}

// Función para leer todos los sensores y guardar los valores en variables globales.
void readSensors()
{
    temp = hdc1080.readTemperature() + TEMP_OFFSET;
    hum = hdc1080.readHumidity() - HUM_OFFSET;

    float currentLux = luxSensor.lightStrengthLux();

    // --- Implementación del filtro de datos para el sensor de luz ---
    // Si la nueva lectura es anómala (negativa o un salto demasiado grande)
    const float EXTREME_LUX_THRESHOLD = 300000.0;
    if (currentLux < 0 || currentLux > EXTREME_LUX_THRESHOLD)
    {
        Serial.print("Lectura de lux anómala (");
        Serial.print(currentLux);
        Serial.println(") detectada y filtrada. Se usa el último valor válido.");
        lux = luxHistory[(luxHistoryIndex - 1 + LUX_HISTORY_SIZE) % LUX_HISTORY_SIZE];
    }
    else
    {
        // Si la lectura es válida, la añadimos al historial.
        lux = currentLux;
        luxHistory[luxHistoryIndex] = lux;
        luxHistoryIndex = (luxHistoryIndex + 1) % LUX_HISTORY_SIZE;
    }

    int rawADC = analogRead(SoundSensorPin);
    float voltageValue = rawADC * (VREF / 4095.0);
    dbValue = voltageValue * 50.0;

    // Imprimir en el Serial Monitor para depuración.
    Serial.print("Temp: ");
    Serial.print(temp, 1);
    Serial.print(" C | ");
    Serial.print("Hum: ");
    Serial.print(hum, 1);
    Serial.print(" % | ");
    Serial.print("Lux: ");
    Serial.print(lux, 1);
    Serial.print(" | Ruido: ");
    Serial.print(dbValue, 1);
    Serial.println(" dBA");
}

// Función para dibujar los valores de todos los sensores a la vez.
void drawAllSensors()
{
    display.setTextSize(1);
    display.setCursor(20, 23); // Posición centrada
    display.println("TODOS LOS SENSORES:");

    display.setCursor(0, 33);
    display.print("Temp:      ");
    display.print(temp, 1);
    display.println(" C");

    display.setCursor(0, 42);
    display.print("Hum:       ");
    display.print(hum, 1);
    display.println(" %");

    display.setCursor(0, 51);
    display.print("Lux:       ");
    if (lux < 0)
    {
        display.println("Error");
    }
    else
    {
        display.print(lux, 1);
        display.println(" lux");
    }

    display.setCursor(0, 60);
    display.print("Ruido:     ");
    display.print(dbValue, 1);
    display.println(" dBA");
}

// Función para dibujar los valores actuales de los sensores en la pantalla.
void updateDisplay()
{
    if (!isDisplayOn)
    {
        return; // No hace nada si la pantalla está apagada.
    }

    // Limpiar la pantalla para dibujar el nuevo contenido.
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // El color blanco se traduce en amarillo o azul dependiendo de la zona.

    displayStateInfo("LECTURA"); // Va hasta Y=23

    switch (screenMode)
    {
    case 0: // Ahora el modo 0 es el de todos los sensores.
        drawAllSensors();
        break;
    case 1: // El modo 1 es ahora para temperatura y humedad.
        // Título en la zona amarilla, centrado.
        display.setTextSize(2);
        display.setCursor(0, 25);
        display.println("TEMP/HUM:");

        // Valores en la zona azul.
        display.setTextSize(1);
        display.setCursor(0, 45);
        display.print(temp, 1);
        display.println(" C TEMP");

        display.setTextSize(1);
        display.setCursor(0, 55);
        display.print(hum, 1);
        display.println(" % HUM");
        break;

    case 2: // El modo 2 es para luz.
        // Título en la zona amarilla, centrado.
        display.setTextSize(2);
        display.setCursor(0, 25);
        display.println("LUZ:");
        display.setTextSize(1); // Aumentado el tamaño de la fuente.
        if (lux < 0)
        {
            display.println("Error");
        }
        else
        {
            display.print(lux, 1);
            display.println(" lux");
        }
        break;

    case 3: // El modo 3 es para ruido.
        // Título en la zona amarilla, centrado.
        display.setTextSize(2);
        display.setCursor(0, 25);
        display.println("RUIDO:");

        // Valor en la zona azul.
        display.setTextSize(1);   // Aumentado el tamaño de la fuente.
        display.setCursor(0, 45); // Ajustada la posición del cursor.
        display.print(dbValue, 1);
        display.println(" dBA");
        break;
    }

    display.display();
}

// Función para construir el JSON con los datos de los sensores
String construirJson(float temperatura, float humedad, float luz, float ruido)
{
    StaticJsonDocument<512> doc;

    // Temperatura
    JsonObject temp = doc.createNestedObject("temperature");
    temp["value"] = temperatura;
    temp["type"] = "number";

    // Humedad
    JsonObject hum = doc.createNestedObject("humidity");
    hum["value"] = humedad;
    hum["type"] = "number";

    // Luz
    JsonObject light = doc.createNestedObject("light");
    light["value"] = luz;
    light["type"] = "number";

    // Ruido
    JsonObject noise = doc.createNestedObject("noise");
    noise["value"] = ruido;
    noise["type"] = "number";

    // Timestamp
    JsonObject timestamp = doc.createNestedObject("timestamp");
    timestamp["value"] = millis();
    timestamp["type"] = "number";

    String out;
    serializeJson(doc, out);
    return out;
}

void displayDeveloperInfo()
{
    display.clearDisplay();
    display.setTextSize(1);

    // Encabezado
    display.setCursor(0, 0);
    display.println("Modo Desarrollador");
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

    // WiFi Status (16 píxeles desde arriba)
    display.setCursor(0, 12);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.print("WiFi: ");
        display.println(settings.ssid.substring(0, 10)); // Limitar longitud
        display.setCursor(0, 21);
        String ip = WiFi.localIP().toString();
        display.println(ip.substring(0, 16)); // Limitar longitud
    }
    else if (WiFi.getMode() == WIFI_AP)
    {
        display.println("AP: ESP-HOTSPOT");
        display.setCursor(0, 21);
        display.println(WiFi.softAPIP().toString());
    }

    // Línea separadora
    display.drawLine(0, 30, 128, 30, SSD1306_WHITE);

    // Datos de sensores (2 columnas)
    display.setCursor(0, 33);
    display.print("T:");
    display.print(temp, 1);

    display.setCursor(64, 33);
    display.print("H:");
    display.println(hum, 1);

    display.setCursor(0, 42);
    display.print("L:");
    display.print(lux, 1);

    display.setCursor(64, 42);
    display.print("R:");
    display.println(dbValue, 1);

    display.display();
}

// Agregar después de displayDeveloperInfo()
void displayStateInfo(const char *estado)
{
    // Dibujar estado y WiFi en la parte superior
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Estado: ");
    display.println(estado);

    // Dibujar línea separadora
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

    // Mostrar estado WiFi
    display.setCursor(0, 11);
    if (WiFi.status() == WL_CONNECTED)
    {
        String ssid = settings.ssid;
        if (ssid.length() > 10)
            ssid = ssid.substring(0, 10) + "...";
        display.println(ssid);
    }
    else
    {
        display.println("Sin conexion");
    }

    // Línea separadora final
    display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
}