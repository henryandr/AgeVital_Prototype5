#include "DevWebOTA.h"

// HTML minimalista (solo WiFi y OTA)
const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>AgeVital Dev</title>
<style>
body{font-family: Arial;margin: 20px;background:#f0f0f0}
.card{background: white;padding:20px;margin:10px 0;border-radius: 10px;box-shadow:0 2px 5px rgba(0,0,0,0.1)}
h1{color:#2196F3;text-align:center}
h2{color:#333;border-bottom:2px solid #2196F3;padding-bottom: 10px}
input,button{width:100%;padding:12px;margin:5px 0;border:1px solid #ddd;border-radius:5px;box-sizing:border-box}
button{background:#2196F3;color: white;border:none;cursor:pointer;font-weight:bold}
button:hover{background:#1976D2}
#status{margin-top:10px;padding:10px;border-radius:5px;display:none}
. success{background:#4CAF50;color: white;display:block}
.error{background:#f44336;color:white;display:block}
.info{background:#e3f2fd;padding:15px;border-radius: 5px;margin-bottom:15px;color:#1976D2}
</style>
</head>
<body>
<h1>🔧 AgeVital TARS</h1>
<div class="card info">
<strong>Modo Desarrollador</strong><br>
Configura WiFi o actualiza el firmware
</div>

<div class="card">
<h2>⚙️ Configuración WiFi</h2>
<form action="/wifi" method="POST">
<input type="text" name="ssid" placeholder="Nombre de Red (SSID)" required>
<input type="password" name="pass" placeholder="Contraseña WiFi" required>
<button type="submit">💾 Guardar y Reiniciar</button>
</form>
</div>

<div class="card">
<h2>🚀 Actualización OTA</h2>
<input type="file" id="file" accept=".bin">
<button onclick="upload()">📤 Subir Firmware (. bin)</button>
<div id="status"></div>
</div>

<script>
function upload(){
const f=document.getElementById('file').files[0];
const s=document.getElementById('status');
if(!f){s.className='error';s.textContent='❌ Selecciona un archivo . bin';s.style.display='block';return;}
const fd=new FormData();
fd.append('file',f);
s.textContent='⏳ Subiendo firmware...';
s.style.display='block';
s.className='';
fetch('/ota',{method:'POST',body:fd})
.then(r=>{
if(r.ok){s.className='success';s.textContent='✅ Actualización exitosa!  Reiniciando.. .';}
else{s.className='error';s.textContent='❌ Error en la actualización';}
})
.catch(e=>{s.className='error';s.textContent='❌ Error de conexión';});
}
</script>
</body>
</html>
)=====";

DevWebOTA:: DevWebOTA(WebServer* srv) : server(srv), initialized(false) {}

void DevWebOTA:: begin() {
    if (initialized) return;
    
    prefs.begin("agevital", false);
    
    Serial.println("\n=== MODO DESARROLLADOR ACTIVADO ===\n");
    
    // Intentar conectar a WiFi guardado
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    
    if (ssid.length() > 0) {
        Serial.printf("Intentando conectar a:  %s\n", ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\n WiFi conectado!\n");
            Serial.printf("   IP: %s\n\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.println("\n No se pudo conectar al WiFi");
            ssid = "";  // Forzar creación de AP
        }
    }
    
    // Si no hay WiFi configurado o falló la conexión, crear AP
    if (ssid.length() == 0 || WiFi.status() != WL_CONNECTED) {
        Serial.println("Creando Access Point...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPass);
        Serial.printf(" Access Point creado\n");
        Serial.printf("   SSID: %s\n", apSSID);
        Serial.printf("   Password: %s\n", apPass);
        Serial.printf("   IP: %s\n\n", WiFi.softAPIP().toString().c_str());
        Serial.println(" Conecta tu teléfono y abre: http://192.168.4.1\n");
    }
    
    // ===== CONFIGURAR RUTAS DEL SERVIDOR =====
    
    // Ruta principal - Página HTML
    server->on("/", HTTP_GET, [this]() {
        Serial.println("RESPUESTA GET / EXITOSA");
        server->send_P(200, "text/html", HTML_PAGE);
    });
    
    // Ruta para guardar configuración WiFi
    server->on("/wifi", HTTP_POST, [this]() {
    String newssid = server->arg("ssid");
    String newpass = server->arg("pass");
    
    if (newssid.length() > 0) {
        prefs.putString("ssid", newssid);
        prefs.putString("pass", newpass);
        Serial.printf(" WiFi guardado: %s\n", newssid.c_str());
        
        // ===== HTML CON UTF-8 CORRECTO =====
        server->send(200, "text/html", 
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<meta charset='UTF-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "</head>"
            "<body style='font-family:Arial;text-align:center;padding:50px;background:#f0f0f0'>"
            "<div style='background: white;padding:40px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)'>"
            "<h1 style='color:#4CAF50'>✅ Configuración Guardada</h1>"
            "<p style='font-size:18px;color:#666'>El dispositivo se reiniciará en 3 segundos...</p>"
            "<div style='margin-top:20px;color:#999'>Reconecta al WiFi configurado</div>"
            "</div>"
            "</body>"
            "</html>");
        
        delay(3000);
        ESP.restart();
    } else {
        server->send(400, "text/plain", "SSID vacío");
    }
});
    
    // Ruta para actualización OTA
    server->on("/ota", HTTP_POST,
        [this]() {
            // Respuesta después de la actualización
            server->sendHeader("Connection", "close");
            server->send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
            delay(1000);
            ESP.restart();
        },
        [this]() {
            // Manejo de la subida del archivo
            HTTPUpload& upload = server->upload();
            
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("Iniciando OTA:  %s\n", upload.filename.c_str());
                if (! Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update. write(upload.buf, upload. currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                } else {
                    int progress = (Update.progress() * 100) / Update.size();
                    Serial.printf("  Progreso: %d%%\r", progress);
                }
            }
            else if (upload. status == UPLOAD_FILE_END) {
                if (Update. end(true)) {
                    Serial.printf("\nOTA completado:  %u bytes\n", upload. totalSize);
                } else {
                    Serial.println("\nError en OTA");
                    Update.printError(Serial);
                }
            }
        }
    );
    
    // Iniciar servidor
    server->begin();
    Serial.println("Servidor web iniciado en puerto 80\n");
    
    initialized = true;
}

void DevWebOTA::handle() {
    server->handleClient();
}

bool DevWebOTA::isConfigured() {
    return prefs.getString("ssid", "").length() > 0;
}
