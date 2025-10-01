#include <DFRobot_B_LUX_V30B.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ClosedCube_HDC1080.h"

// --- Pantalla OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Utiliza la dirección I2C 0x3C para la mayoría de las pantallas OLED.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Sensores y Pines ---
#define SoundSensorPin 35 // Pin para el sensor de ruido, restaurado a 35.
#define VREF 3.3
#define BUTTON_PIN 27 // Pin para el botón, ahora usando interrupción.

// --- Sensor HDC1080 (Temperatura y Humedad) ---
ClosedCube_HDC1080 hdc1080;

// --- Sensor Lux (Luz) ---
// El pin de habilitación (cEN) del sensor de luz DEBE ser diferente a los pines I2C (21 y 22).
// He configurado el GPIO 5 como ejemplo. Por favor, conecta el pin EN a este pin.
DFRobot_B_LUX_V30B luxSensor(&Wire, /*cEN=*/5);

// --- Variables de estado del programa ---
volatile int screenMode = 0; // 0: Todos, 1: Temp/Hum, 2: Luz, 3: Ruido
unsigned long previousUpdateMillis = 0;
const long updateInterval = 2000; // Intervalo de 2 segundos entre lecturas

// --- Variables para el manejo de la interrupción del botón ---
volatile unsigned long lastButtonPressTime = 0;
const unsigned long debounceTime = 300; // Tiempo de espera para evitar rebote, ahora un poco más corto.
volatile bool needsUpdate = false;

// Variables globales para los valores de los sensores
float temp, hum, lux, dbValue;

// --- Variables para el filtro de datos (lux) ---
const int LUX_HISTORY_SIZE = 15;
float luxHistory[LUX_HISTORY_SIZE];
int luxHistoryIndex = 0;
bool luxHistoryFull = false;

// --- Variables para el temporizador de inactividad ---
volatile unsigned long lastInteractionTime = 0;
const long INACTIVITY_TIMEOUT = 10000; // 10 segundos en milisegundos.
bool isDisplayOn = true;

// --- Factores de calibracion de sensores ---
// Ajusta estos valores segun tus necesidades para calibrar los sensores.
const float NOISE_MULTIPLIER = 0.025; 
const float NOISE_OFFSET = 50.0;
// Calibración de temperatura: ajustado para que la lectura de 21.7°C sea 22.6°C
const float TEMP_OFFSET = -3;   
// Calibración de humedad: ajustado para que la lectura de 67% sea 60%
const float HUM_OFFSET = 7.0; 
const float LUX_CALIBRATION_FACTOR = 0.613; // Para ajustar la lectura del sensor de luz

// Declaración de las funciones para que sean reconocidas en todo el código.
void readSensors();
void updateDisplay();
void drawAllSensors();

// Función de interrupción para manejar la pulsación del botón.
// Se ejecuta cada vez que el pin del botón cambia de estado.
void IRAM_ATTR handleButtonPress() {
  unsigned long now = millis();
  // Simple debounce: ignora las pulsaciones que ocurren muy cerca una de la otra.
  if (now - lastButtonPressTime > debounceTime) {
    lastButtonPressTime = now;
    lastInteractionTime = now; // Reinicia el temporizador de inactividad con cada pulsación.

    if (!isDisplayOn) {
      // Si la pantalla está apagada, la encendemos sin cambiar el modo.
      isDisplayOn = true;
    } else {
      // Si la pantalla está encendida, cambiamos al siguiente modo.
      screenMode++;
      if (screenMode > 3) {
        screenMode = 0; // Vuelve al primer modo.
      }
    }
    needsUpdate = true; // Establece la bandera para actualizar la pantalla de inmediato.
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("--- Inciando sistema ---");

  // Iniciar la comunicación I2C una sola vez para todos los sensores.
  Wire.begin(21, 22);

  // Iniciar sensor HDC1080.
  hdc1080.begin(0x40);
  Serial.println("HDC1080 listo.");
  
  // Iniciar sensor Lux.
  luxSensor.begin();

  // Iniciar la pantalla OLED.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("No se encontro OLED.");
    while(1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  // Configurar la resolución ADC para el sensor de ruido.
  analogReadResolution(12);

  // Configurar el pin del botón con resistencia pull-up interna y adjuntar la interrupción.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

  // Inicializar el array del historial de lux con un valor inicial.
  lux = luxSensor.lightStrengthLux();
  if (lux < 0) lux = 0; // Asegurarse de que el valor inicial no sea negativo
  for (int i = 0; i < LUX_HISTORY_SIZE; i++) {
    luxHistory[i] = lux;
  }

  // Realizar la primera lectura y actualización de la pantalla inmediatamente.
  lastInteractionTime = millis();
  readSensors();
  updateDisplay();
}

void loop() {
  // Actualizar la pantalla y el Serial Monitor periódicamente sin usar delay().
  unsigned long currentMillis = millis();
  if (currentMillis - previousUpdateMillis >= updateInterval) {
    previousUpdateMillis = currentMillis;
    readSensors();
    if (isDisplayOn) {
      updateDisplay();
    }
  }

  // --- Lógica del temporizador de inactividad ---
  if (isDisplayOn && (millis() - lastInteractionTime > INACTIVITY_TIMEOUT)) {
    display.clearDisplay();
    display.display();
    isDisplayOn = false; // Cambia el estado para que no se actualice la pantalla.
  }

  // Si la interrupción del botón ha ocurrido, actualizar la pantalla inmediatamente.
  if (needsUpdate && isDisplayOn) {
    updateDisplay();
    needsUpdate = false;
  }
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
  Serial.print("Temp: "); Serial.print(temp, 1); Serial.print(" C | ");
  Serial.print("Hum: "); Serial.print(hum, 1); Serial.print(" % | ");
  Serial.print("Lux: "); Serial.print(lux, 1); Serial.print(" | Ruido: ");
  Serial.print(dbValue, 1); Serial.println(" dBA");
}

// Función para dibujar los valores de todos los sensores a la vez.
void drawAllSensors() {
  display.setTextSize(1);
  display.setCursor(10, 0); // Posición centrada
  display.println("TODOS LOS SENSORES:");

  display.setCursor(0, 20);
  display.print("Temp:      ");
  display.print(temp, 1);
  display.println(" C");

  display.setCursor(0, 30);
  display.print("Hum:       ");
  display.print(hum, 1);
  display.println(" %");

  display.setCursor(0, 40);
  display.print("Lux:       ");
  if (lux < 0) {
    display.println("Error");
  } else {
    display.print(lux, 1);
    display.println(" lux");
  }
  
  display.setCursor(0, 50);
  display.print("Ruido:     ");
  display.print(dbValue, 1);
  display.println(" dBA");
}


// Función para dibujar los valores actuales de los sensores en la pantalla.
void updateDisplay() {
  if (!isDisplayOn) {
    return; // No hace nada si la pantalla está apagada.
  }

  // Limpiar la pantalla para dibujar el nuevo contenido.
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE); // El color blanco se traduce en amarillo o azul dependiendo de la zona.

  switch (screenMode) {
    case 0: // Ahora el modo 0 es el de todos los sensores.
      drawAllSensors();
      break;
    case 1: // El modo 1 es ahora para temperatura y humedad.
      // Título en la zona amarilla, centrado.
      display.setTextSize(2);
      display.setCursor(16, 0);
      display.println("TEMP/HUM:");
      
      // Valores en la zona azul.
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.print(temp, 1);
      display.println(" C TEMP");
      
      display.setCursor(0, 42);
      display.setTextSize(1);
      display.setCursor(0, 42);
      display.print(hum, 1);
      display.println(" % HUM");
      break;

    case 2: // El modo 2 es para luz.
      // Título en la zona amarilla, centrado.
      display.setTextSize(2);
      display.setCursor(40, 0);
      display.println("LUZ:");
      
      // Valor en la zona azul.
      display.setTextSize(2); // Aumentado el tamaño de la fuente.
      display.setCursor(0, 20); // Ajustada la posición del cursor.
      if (lux < 0) {
        display.println("Error");
      } else {
        display.print(lux, 1);
        display.println(" lux");
      }
      break;

    case 3: // El modo 3 es para ruido.
      // Título en la zona amarilla, centrado.
      display.setTextSize(2);
      display.setCursor(28, 0);
      display.println("RUIDO:");
      
      // Valor en la zona azul.
      display.setTextSize(2); // Aumentado el tamaño de la fuente.
      display.setCursor(0, 20); // Ajustada la posición del cursor.
      display.print(dbValue, 1);
      display.println(" dBA");
      break;
  }
  
  display.display();
}
