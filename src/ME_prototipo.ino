#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <DFRobot_B_LUX_V30B.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

#include "ClosedCube_HDC1080.h"
#include "ESPaccesspoint.h"
#include "Settings.h"

// ===== INCLUIR LA LIBRERÍA DE MÁQUINA DE ESTADOS =====
#include "EstadoDESARROLLADOR.cpp"
#include "EstadoENVIO.cpp"
#include "EstadoINICIO.cpp"
#include "EstadoLECTURA.cpp"
#include "StateMachine.h"

// --- Definiciones de hardware ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SoundSensorPin 35
#define VREF 3.3
#define BUTTON_PIN 27
#define DEV_PIN 26

// --- Factores de calibración ---
const float NOISE_MULTIPLIER = 0.025;
const float NOISE_OFFSET = 50.0;
const float TEMP_OFFSET = -3;
const float HUM_OFFSET = 7.0;
const float LUX_CALIBRATION_FACTOR = 0.613;

// --- Objetos ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ClosedCube_HDC1080 hdc1080;
DFRobot_B_LUX_V30B luxSensor(&Wire, 5);
WebServer server(80);
Settings settings;

// ===== MÁQUINA DE ESTADOS =====
StateMachine stateMachine;

// --- Variables para filtro de lux ---
const int LUX_HISTORY_SIZE = 15;
float luxHistory[LUX_HISTORY_SIZE];
int luxHistoryIndex = 0;
bool luxHistoryFull = false;

// --- Interrupción del botón ---
void IRAM_ATTR handleButtonPress() {
  unsigned long now = millis();
  const unsigned long DEBOUNCE_TIME = 300;

  if (now - stateMachine.clocks.lastButtonPressTime > DEBOUNCE_TIME) {
    stateMachine.clocks.lastButtonPressTime = now;
    stateMachine.clocks.ultima_interaccion = now;

    if (!stateMachine.isDisplayOn) {
      stateMachine.isDisplayOn = true;
    } else {
      stateMachine.screenMode++;
      if (stateMachine.screenMode > 3) {
        stateMachine.screenMode = 0;
      }
    }
    stateMachine.needsUpdate = true;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== TARS1 ===");
  Serial.println("Con Máquina de Estados Modular\n");

  EEPROM.begin(4096);
  settings.load();
  settings.info();
  Wire.begin(21, 22);

  // Inicialización de sensores
  hdc1080.begin(0x40);
  luxSensor.begin();
  analogReadResolution(12);

  // Configuración de la pantalla
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error OLED");
    while (1);
  }

  // Configuración de pines
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(DEV_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  // Inicializar filtro de lux
  float initialLux = luxSensor.lightStrengthLux();
  if (initialLux < 0) initialLux = 0;
  for (int i = 0; i < LUX_HISTORY_SIZE; i++) {
    luxHistory[i] = initialLux;
  }

  // Configuracion de intervalos de la máquina de estados para pruebas rapidas
  // stateMachine.setSettings(2000, 15000);  // 2 seg lectura, 15 seg envío

  // Iniciamos la maquina de estados
  if (digitalRead(DEV_PIN) == LOW) {
    stateMachine.flags.dev = true;
    stateMachine.begin(new EstadoDESARROLLADOR());
  } else {
    stateMachine.begin(new EstadoINICIO());
  }
}

void loop() {
  // Actualizamos la máquina de estados
  stateMachine.update();
}

// Función para leer todos los sensores y guardar los valores en variables globales.
void readSensors() {
  temp = hdc1080.readTemperature() + TEMP_OFFSET;
  hum = hdc1080.readHumidity() - HUM_OFFSET;

  float currentLux = luxSensor.lightStrengthLux();

  // --- Implementación del filtro de datos para el sensor de luz ---
  // Si la nueva lectura es anómala (negativa o un salto demasiado grande)
  const float EXTREME_LUX_THRESHOLD = 300000.0;
  if (currentLux < 0 || currentLux > EXTREME_LUX_THRESHOLD) {
    Serial.print("Lectura de lux anómala (");
    Serial.print(currentLux);
    Serial.println(") detectada y filtrada. Se usa el último valor válido.");
    lux = luxHistory[(luxHistoryIndex - 1 + LUX_HISTORY_SIZE) % LUX_HISTORY_SIZE];
  } else {
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
void drawAllSensors() {
  display.setTextSize(1);
  display.setCursor(20, 23);  // Posición centrada
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
  if (lux < 0) {
    display.println("Error");
  } else {
    display.print(lux, 1);
    display.println(" lux");
  }

  display.setCursor(0, 60);
  display.print("Ruido:     ");
  display.print(dbValue, 1);
  display.println(" dBA");
}

// Función para dibujar los valores actuales de los sensores en la pantalla.
void updateDisplay() {
  if (!isDisplayOn) {
    return;  // No hace nada si la pantalla está apagada.
  }

  // Limpiar la pantalla para dibujar el nuevo contenido.
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);  // El color blanco se traduce en amarillo o azul dependiendo de la zona.

  displayStateInfo("LECTURA");  // Va hasta Y=23

  switch (screenMode) {
    case 0:  // Ahora el modo 0 es el de todos los sensores.
      drawAllSensors();
      break;
    case 1:  // El modo 1 es ahora para temperatura y humedad.
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

    case 2:  // El modo 2 es para luz.
      // Título en la zona amarilla, centrado.
      display.setTextSize(2);
      display.setCursor(0, 25);
      display.println("LUZ:");
      display.setTextSize(1);  // Aumentado el tamaño de la fuente.
      if (lux < 0) {
        display.println("Error");
      } else {
        display.print(lux, 1);
        display.println(" lux");
      }
      break;

    case 3:  // El modo 3 es para ruido.
      // Título en la zona amarilla, centrado.
      display.setTextSize(2);
      display.setCursor(0, 25);
      display.println("RUIDO:");

      // Valor en la zona azul.
      display.setTextSize(1);    // Aumentado el tamaño de la fuente.
      display.setCursor(0, 45);  // Ajustada la posición del cursor.
      display.print(dbValue, 1);
      display.println(" dBA");
      break;
  }

  display.display();
}

// Función para construir el JSON con los datos de los sensores
String construirJson(float temperatura, float humedad, float luz, float ruido) {
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

void displayDeveloperInfo() {
  display.clearDisplay();
  display.setTextSize(1);

  // Encabezado
  display.setCursor(0, 0);
  display.println("Modo Desarrollador");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // WiFi Status (16 píxeles desde arriba)
  display.setCursor(0, 12);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: ");
    display.println(settings.ssid.substring(0, 10));  // Limitar longitud
    display.setCursor(0, 21);
    String ip = WiFi.localIP().toString();
    display.println(ip.substring(0, 16));  // Limitar longitud
  } else if (WiFi.getMode() == WIFI_AP) {
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
void displayStateInfo(const char* estado) {
  // Dibujar estado y WiFi en la parte superior
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Estado: ");
  display.println(estado);

  // Dibujar línea separadora
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Mostrar estado WiFi
  display.setCursor(0, 11);
  if (WiFi.status() == WL_CONNECTED) {
    String ssid = settings.ssid;
    if (ssid.length() > 10) ssid = ssid.substring(0, 10) + "...";
    display.println(ssid);
  } else {
    display.println("Sin conexion");
  }

  // Línea separadora final
  display.drawLine(0, 20, 128, 20, SSD1306_WHITE);
}