#include "DevWebOTA.h"

#include "AppConfig.h"
#include "TokenManager.h"
#include "WiFiManager.h"

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
</div>

<div class="card">
<h2>🔑 Credenciales Keyrock</h2>
<form action="/keyrock" method="POST">
<label>URL del Token</label>
<input type="text" name="tokenUrl" placeholder="http://servidor:3001/oauth2/token">
<label>Client ID</label>
<input type="text" name="clientId" placeholder="client_id">
<label>Client Secret</label>
<input type="text" name="clientSecret" placeholder="client_secret">
<label>Usuario Keyrock (email)</label>
<input type="text" name="keyrockUser" placeholder="usuario@email.com">
<label>Password Keyrock</label>
<input type="password" name="keyrockPass" placeholder="password">
<button type="submit">💾 Guardar Credenciales</button>
</form>
</div>

<div class="card">
<h2>⚠️ Restaurar Valores a Quemados</h2>
<p style="color:#666;font-size:14px">Esto restaurara <strong>toda</strong> la configuracion a valores por defecto: WiFi, configuracion del sistema y credenciales de Keyrock.</p>
<form action="/config/reset" method="POST">
<button type="submit" class="btn-danger">🔄 Restaurar Todo a Defaults</button>
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

  if (!wifiManager.connect()) {
    wifiManager.createAP();
  }

  // ===== RUTAS =====

  server->on("/", HTTP_GET, [this]() {
    Serial.println("RESPUESTA GET / EXITOSA");
    server->send_P(200, "text/html", HTML_PAGE);
  });

  server->on("/wifi", HTTP_POST, [this]() {
    String newssid = server->arg("ssid");
    String newpass = server->arg("pass");

    if (newssid.length() > 0) {
      wifiManager.saveCredentials(newssid, newpass);
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

    Serial.printf("[Config] serverUrl: %s\n", appConfig.serverUrl.c_str());
    Serial.printf("[Config] intervaloEnvio: %lu\n", appConfig.intervaloEnvio);
    Serial.printf("[Config] intervaloLectura: %lu\n", appConfig.intervaloLectura);
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

  server->on("/config/reset", HTTP_POST, [this]() {
    appConfig.reset();
    tokenManager.clear();
    wifiManager.reset();

    Serial.printf("[Config] serverUrl: %s\n", appConfig.serverUrl.c_str());
    Serial.printf("[Config] intervaloEnvio: %lu\n", appConfig.intervaloEnvio);
    Serial.printf("[Config] intervaloLectura: %lu\n", appConfig.intervaloLectura);
    Serial.printf("[Config] intervaloReintento: %lu\n", appConfig.intervaloReintento);
    Serial.printf("[Keyrock] tokenUrl: %s\n", appConfig.tokenUrl.c_str());
    Serial.printf("[Keyrock] clientId: %s\n", appConfig.clientId.c_str());
    Serial.printf("[Keyrock] keyrockUser: %s\n", appConfig.keyrockUser.c_str());
    server->send(200, "text/html",
                 "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                 "<meta http-equiv='refresh' content='2;url=/'></head>"
                 "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
                 "<div style='background:white;padding:40px;border-radius:10px'>"
                 "<h1 style='color:#FF9800'>Defaults Restaurados</h1>"
                 "<p>Volviendo al panel...</p>"
                 "</div></body></html>");
  });

  // ===== Ruta Keyrock =====
  server->on("/keyrock", HTTP_POST, [this]() {
    String newTokenUrl = server->arg("tokenUrl");
    String newClientId = server->arg("clientId");
    String newClientSecret = server->arg("clientSecret");
    String newKeyrockUser = server->arg("keyrockUser");
    String newKeyrockPass = server->arg("keyrockPass");

    if (newTokenUrl.length() > 0) appConfig.tokenUrl = newTokenUrl;
    if (newClientId.length() > 0) appConfig.clientId = newClientId;
    if (newClientSecret.length() > 0) appConfig.clientSecret = newClientSecret;
    if (newKeyrockUser.length() > 0) appConfig.keyrockUser = newKeyrockUser;
    if (newKeyrockPass.length() > 0) appConfig.keyrockPass = newKeyrockPass;

    appConfig.save();
    tokenManager.clear();

    Serial.printf("[Keyrock] tokenUrl: %s\n", appConfig.tokenUrl.c_str());
    Serial.printf("[Keyrock] clientId: %s\n", appConfig.clientId.c_str());
    Serial.printf("[Keyrock] keyrockUser: %s\n", appConfig.keyrockUser.c_str());

    server->send(200, "text/html",
                 "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                 "<meta http-equiv='refresh' content='2;url=/'></head>"
                 "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
                 "<div style='background:white;padding:40px;border-radius:10px'>"
                 "<h1 style='color:#4CAF50'>Credenciales Keyrock Guardadas</h1>"
                 "<p>Volviendo al panel...</p>"
                 "</div></body></html>");
  });

  // Ruta para OTA -- Esto no se toca a menos que quieras personalizar el proceso de actualización
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