#include "ESPaccesspoint.h"

void handleServer(WebServer &server, Settings &settings)
{
    server.on("/", [&server]()
              { server.send(200, "text/html", data_getIndexHTML()); });
    server.on("/functions.js", [&server]()
              { server.send(200, "text/javascript", data_getFunctionsJS()); });
    server.onNotFound([&server]()
                      { server.send(404, "text/html", data_get404()); });

    // Guardar configuración de WiFi
    server.on("/settingsSave.json", [&server, &settings]()
              {
        settings.ssid = server.arg("ssid");
        settings.password = server.arg("password");
        settings.save();
        server.send(200, "text/json", "true");
        STA_mode_onRst(); });

    server.begin();
}

void startAPorSTA(Settings &settings)
{
    WiFi.disconnect();
    if (is_STA_mode())
    {
        Serial.println("Intentando conectar a WiFi...");
        WiFi.begin(settings.ssid.c_str(), settings.password.c_str());

        int intentos = 0;
        while (WiFi.status() != WL_CONNECTED && intentos < 20)
        {
            delay(500);
            Serial.print(".");
            intentos++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConectado a WiFi. IP: " + WiFi.localIP().toString());
            extern WebServer server;
            handleServer(server, settings);
            return;
        }
    }

    // Si falla la conexión, inicia el AP con contraseña FIJA "12345678"
    Serial.println("\nNo se pudo conectar. Iniciando AP...");
    WiFi.softAP("ESP-HOTSPOT", "12345678");

    Serial.println("AP iniciado. SSID: ESP-HOTSPOT | Contraseña: 12345678");
    Serial.println("IP del AP: " + WiFi.softAPIP().toString());

    extern WebServer server;
    handleServer(server, settings);
}

bool is_STA_mode()
{
    return EEPROM.read(flagAdr) == 1;
}

void STA_mode_onRst()
{
    EEPROM.write(flagAdr, 1);
    EEPROM.commit();
    ESP.restart();
}

void AP_mode_onRst()
{
    EEPROM.write(flagAdr, 0);
    EEPROM.commit();
    ESP.restart();
}
