#include "DisplayManager.h"

#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#include "StateMachine.h"
#include "WiFiManager.h"

extern Adafruit_SSD1306 display;
extern StateMachine stateMachine;
extern WiFiManager wifiManager;

void drawAllSensors() {
  display.setTextSize(0);
  display.setCursor(15, 17);
  display.println("Sensores:");

  display.setCursor(0, 23);
  display.print("Temp:      ");
  display.print(stateMachine.sensors.temp, 1);
  display.println(" C");

  display.setCursor(0, 32);
  display.print("Hum:       ");
  display.print(stateMachine.sensors.hum, 1);
  display.println(" %");

  display.setCursor(0, 41);
  display.print("Lux:       ");
  display.print(stateMachine.sensors.lux, 1);
  display.println(" lux");

  display.setCursor(0, 51);
  display.print("Ruido:     ");
  display.print(stateMachine.sensors.dbValue, 1);
  display.println(" dBA");
}

void updateDisplay() {
  if (!stateMachine.isDisplayOn) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  displayStateInfo("LECTURA");

  switch (stateMachine.screenMode) {
    case 0:
      drawAllSensors();
      break;

    case 1:
      display.setTextSize(1);
      display.setCursor(0, 20);
      display.println("TEMP/HUM:");

      display.setTextSize(1);
      display.setCursor(0, 35);
      display.print(stateMachine.sensors.temp, 1);
      display.println(" C TEMP");

      display.setCursor(0, 45);
      display.print(stateMachine.sensors.hum, 1);
      display.println(" % HUM");
      break;

    case 2:
      display.setTextSize(1);
      display.setCursor(0, 17);
      display.println("LUZ:");
      display.setTextSize(2);
      display.print(stateMachine.sensors.lux, 1);
      display.println(" lux");
      break;

    case 3:
      display.setTextSize(1);
      display.setCursor(0, 17);
      display.println("RUIDO:");

      display.setTextSize(2);
      display.setCursor(0, 27);
      display.print(stateMachine.sensors.dbValue, 1);
      display.println(" dBA");

      display.setTextSize(1);
      display.setCursor(0, 50);
      display.print("V: ");
      display.print(stateMachine.sensors.voltage, 3);
      display.println(" V");
      break;
  }

  display.display();
}

void displayStateInfo(const char* estado) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Estado: ");
  display.println(estado);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 9);
  if (WiFi.status() == WL_CONNECTED) {
    display.println(wifiManager.getIP());
  } else {
    display.println("Sin conexion");
  }
}

void displayDeveloperInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Modo Desarrollador");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 12);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: ");
    display.setCursor(0, 21);
    display.println(WiFi.localIP().toString());
  } else if (WiFi.getMode() == WIFI_AP) {
    display.println("AP: ESP-HOTSPOT");
    display.setCursor(0, 21);
    display.println(WiFi.softAPIP().toString());
  }

  display.drawLine(0, 30, 128, 30, SSD1306_WHITE);
  display.setCursor(0, 33);
  display.print("T:");
  display.print(stateMachine.sensors.temp, 1);
  display.setCursor(64, 33);
  display.print("H:");
  display.println(stateMachine.sensors.hum, 1);
  display.setCursor(0, 42);
  display.print("L:");
  display.print(stateMachine.sensors.lux, 1);
  display.setCursor(64, 42);
  display.print("R:");
  display.println(stateMachine.sensors.dbValue, 1);

  display.display();
}
