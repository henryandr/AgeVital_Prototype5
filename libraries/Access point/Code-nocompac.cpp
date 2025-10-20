#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdlib.h>

#include "data.h"
#include "Settings.h"

WebServer server(80);

Settings settings;

void load404();
void loadIndex();
void loadFunctionsJS();
void restartESP();
void saveSettings();
bool is_STA_mode();
void AP_mode_onRst();
void STA_mode_onRst();
void clearEEPROM();
void startAP();
void start_STA_client();

void setup()
{
    Serial.begin(115200);
    delay(2000);

    clearEEPROM();
    Serial.println("EPROM LIMPIADA");

    EEPROM.begin(4096); // Se inicializa la EEPROM con su tamaño max 4KB

    settings.load(); // se carga SSID y PWD guardados en EEPROM
    settings.info(); // ... y se visualizan

    Serial.println("");
    Serial.println("starting...");

    if (is_STA_mode())
    {
        start_STA_client();
    }
    else // Modo Access Point & WebServer
    {
        startAP();

        /* ========== Modo Web Server ========== */

        /* HTML sites */
        server.onNotFound(load404);

        server.on("/", loadIndex);
        server.on("/index.html", loadIndex);
        server.on("/functions.js", loadFunctionsJS);

        /* JSON */
        server.on("/settingsSave.json", saveSettings);
        server.on("/restartESP.json", restartESP);

        server.begin();
        Serial.println("HTTP server started");
    }
}

void loop()
{
    if (!is_STA_mode())
    {
        server.handleClient();
    }
    delay(10);
}

// funciones para responder al cliente desde el webserver
void load404()
{
    server.send(200, "text/html", data_get404());
}

void loadIndex()
{
    server.send(200, "text/html", data_getIndexHTML());
}

void loadFunctionsJS()
{
    server.send(200, "text/javascript", data_getFunctionsJS());
}

void restartESP()
{
    server.send(200, "text/json", "true");
    ESP.restart();
}

void saveSettings()
{
    if (server.hasArg("ssid"))
        settings.ssid = server.arg("ssid");
    if (server.hasArg("password"))
        settings.password = server.arg("password");

    settings.save();
    server.send(200, "text/json", "true");
    STA_mode_onRst();
}

// Rutina para verificar si ya se guardó SSID y PWD del cliente
bool is_STA_mode()
{
    if (EEPROM.read(0))
        return true;
    else
        return false;
}

// Rutina para iniciar en modo AP (Access Point) "Servidor"
void startAP()
{
    WiFi.disconnect();
    delay(19);
    Serial.println("Starting WiFi Access Point (AP)");
    WiFi.softAP("ESP-HOTPOT", "12345678");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
}

// Rutina para iniciar en modo STA (Station) "Cliente"
void start_STA_client()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();
    delay(100);
    Serial.println("Starting WiFi Station Mode");
    WiFi.begin((const char *)settings.ssid.c_str(), (const char *)settings.password.c_str());
    WiFi.mode(WIFI_STA);

    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        if (cnt == 100) // Si después de 100 intentos no se conecta, vuelve a modo AP
            AP_mode_onRst();
        cnt++;
        Serial.println("attempt # " + (String)cnt);
    }

    WiFi.setAutoReconnect(true);
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
}

void AP_mode_onRst()
{
    EEPROM.write(0, 0);
    EEPROM.commit();
    delay(100);
    ESP.restart();
}

void STA_mode_onRst()
{
    EEPROM.write(0, 1);
    EEPROM.commit();
    delay(100);
    ESP.restart();
}

void clearEEPROM()
{
    for (int i = 0; i < 4096; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}
