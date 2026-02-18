#include "DevWebOTA.h"

#include "AppConfig.h"

const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>AgeVital Dev</title>
<style>
body{font-family:Arial;margin:20px;background:#f0f0f0}
.card{background:white;padding:20px;margin:10px 0;border-radius:10px;box-shadow:0 2px 5px rgba(0,0,0,0.1)}
h1{color:#2196F3;text-align:center}
h2{color:#333;border-bottom:2px solid #2196F3;padding-bottom:10px}
label{display:block;margin-top:10px;font-weight:bold;color:#555;font-size:14px}
input,button{width:100%;padding:12px;margin:5px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box}
button{background:#2196F3;color:white;border:none;cursor:pointer;font-weight:bold}
button:hover{background:#1976D2}
.btn-danger{background:#f44336}
.btn-danger:hover{background:#d32f2f}
#status{margin-top:10px;padding:10px;border-radius:5px;display:none}
.success{background:#4CAF50;color:white;display:block}
.error{background:#f44336;color:white;display:block}
.info{background:#e3f2fd;padding:15px;border-radius:5px;margin-bottom:15px;color:#1976D2}
</style>
</head>
<body>
<h1>🔧 AgeVital TARS</h1>
<div class="card info">
<strong>Modo Desarrollador</strong><br>
Configura WiFi, parametros del sistema o actualiza el firmware
</div>

<div class="card">
<h2>⚙️ Configuracion WiFi</h2>
<form action="/wifi" method="POST">
<label>Nombre de Red (SSID)</label>
<input type="text" name="ssid" placeholder="Nombre de Red (SSID)" required>
<label>Contrasena WiFi</label>
<input type="password" name="pass" placeholder="Contrasena WiFi" required>
<button type="submit">💾 Guardar y Reiniciar</button>
</form>
</div>

<div class="card">
<h2>🌐 Configuracion del Sistema</h2>
<form action="/config" method="POST">
<label>URL del Servidor</label>
<input type="text" name="serverUrl" placeholder="http://servidor/ruta">
<label>Intervalo de Envio (ms)</label>
<input type="number" name="intervaloEnvio" placeholder="15000" min="1000">
<label>Intervalo de Lectura (ms)</label>
<input type="number" name="intervaloLectura" placeholder="2000" min="500">
<label>Intervalo de Reintento (ms)</label>
<input type="number" name="intervaloReintento" placeholder="20000" min="1000">
<button type="submit">💾 Guardar Configuracion</button>
</form>
<form action="/config/reset" method="POST" style="margin-top:10px">
<button type="submit" class="btn-danger">🔄 Restaurar Defaults</button>
</form>
</div>

<div class="card">
<h2>🚀 Actualizacion OTA</h2>
<label>Selecciona el archivo .bin</label>
<input type="file" id="file" accept=".bin">
<button onclick="upload()">📤 Subir Firmware</button>
<div id="status"></div>
</div>

<script>
function upload(){
const f=document.getElementById('file').files[0];
const s=document.getElementById('status');
if(!f){s.className='error';s.textContent='Selecciona un archivo .bin';s.style.display='block';return;}
const fd=new FormData();
fd.append('file',f);
s.textContent='Subiendo firmware...';
s.style.display='block';
s.className='';
fetch('/ota',{method:'POST',body:fd})
.then(r=>{
if(r.ok){s.className='success';s.textContent='Actualizacion exitosa! Reiniciando...';}
else{s.className='error';s.textContent='Error en la actualizacion';}
})
.catch(()=>{s.className='error';s.textContent='Error de conexion';});
}
</script>
</body>
</html>
)=====";

DevWebOTA::DevWebOTA(WebServer* srv) : server(srv), initialized(false) {}

void DevWebOTA::begin() {
  if (initialized) return;
  Serial.println("\n=== MODO DESARROLLADOR ACTIVADO ===\n");

  prefs.begin("agevital", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");

  if (ssid.length() > 0) {
    Serial.printf("Intentando conectar a: %s\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\nWiFi conectado!\n");
      Serial.printf("  IP: %s\n\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("\nNo se pudo conectar al WiFi");
      ssid = "";
    }
  }

  if (ssid.length() == 0 || WiFi.status() != WL_CONNECTED) {
    Serial.println("Creando Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPass);
    Serial.printf("Access Point creado\n");
    Serial.printf("SSID: %s\n", apSSID);
    Serial.printf("Password: %s\n", apPass);
    Serial.printf("IP: %s\n\n", WiFi.softAPIP().toString().c_str());
  }

  // ===== RUTAS =====

  // ===== Ruta WiFi =====

  server->on("/", HTTP_GET, [this]() {
    Serial.println("RESPUESTA GET / EXITOSA");
    server->send_P(200, "text/html", HTML_PAGE);
  });

  server->on("/wifi", HTTP_POST, [this]() {
    String newssid = server->arg("ssid");
    String newpass = server->arg("pass");

    if (newssid.length() > 0) {
      prefs.putString("ssid", newssid);
      prefs.putString("pass", newpass);
      Serial.printf("WiFi guardado: %s\n", newssid.c_str());

      server->send(200, "text/html",
                   "<!DOCTYPE html><html><head><meta charset='UTF-8'></head>"
                   "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
                   "<div style='background:white;padding:40px;border-radius:10px'>"
                   "<h1 style='color:#4CAF50'>Configuracion Guardada</h1>"
                   "<p>El dispositivo se reiniciara en 3 segundos...</p>"
                   "</div></body></html>");
      delay(3000);
      ESP.restart();
    } else {
      server->send(400, "text/plain", "SSID vacio");
    }
  });

  // ===== Ruta Config =====
  // Guardar configuración del sistema
  server->on("/config", HTTP_POST, [this]() {
    String newUrl = server->arg("serverUrl");
    String newEnvio = server->arg("intervaloEnvio");
    String newLectura = server->arg("intervaloLectura");
    String newReintento = server->arg("intervaloReintento");

    if (newUrl.length() > 0) appConfig.serverUrl = newUrl;
    if (newEnvio.length() > 0) appConfig.intervaloEnvio = newEnvio.toInt();
    if (newLectura.length() > 0) appConfig.intervaloLectura = newLectura.toInt();
    if (newReintento.length() > 0) appConfig.intervaloReintento = newReintento.toInt();

    appConfig.save();

    Serial.printf("[Config] serverUrl:          %s\n", appConfig.serverUrl.c_str());
    Serial.printf("[Config] intervaloEnvio:     %lu\n", appConfig.intervaloEnvio);
    Serial.printf("[Config] intervaloLectura:   %lu\n", appConfig.intervaloLectura);
    Serial.printf("[Config] intervaloReintento: %lu\n", appConfig.intervaloReintento);

    server->send(200, "text/html",
                 "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                 "<meta http-equiv='refresh' content='2;url=/'></head>"
                 "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
                 "<div style='background:white;padding:40px;border-radius:10px'>"
                 "<h1 style='color:#4CAF50'>Configuracion Guardada</h1>"
                 "<p>Volviendo al panel...</p>"
                 "</div></body></html>");
  });

  // ===== Ruta Config Reset =====
  // Reset a defaults
  server->on("/config/reset", HTTP_POST, [this]() {
    appConfig.reset();
    server->send(200, "text/html",
                 "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                 "<meta http-equiv='refresh' content='2;url=/'></head>"
                 "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
                 "<div style='background:white;padding:40px;border-radius:10px'>"
                 "<h1 style='color:#FF9800'>Defaults Restaurados</h1>"
                 "<p>Volviendo al panel...</p>"
                 "</div></body></html>");
  });

  // Ruta OTA — sin cambios tal cual
  server->on(
      "/ota", HTTP_POST,
      [this]() {
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
        delay(1000);
        ESP.restart();
      },
      [this]() {
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
          Serial.printf("Iniciando OTA: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
          }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
          } else {
            Serial.printf("  Progreso: %d%%\r", (Update.progress() * 100) / Update.size());
          }
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.end(true)) {
            Serial.printf("\nOTA completado: %u bytes\n", upload.totalSize);
          } else {
            Serial.println("\nError en OTA");
            Update.printError(Serial);
          }
        }
      });

  server->begin();
  Serial.println("Servidor web iniciado en puerto 80\n");
  initialized = true;
}

void DevWebOTA::handle() { server->handleClient(); }

bool DevWebOTA::isConfigured() { return prefs.getString("ssid", "").length() > 0; }