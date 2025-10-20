import requests
from telegram import (
    Update,
    InlineKeyboardMarkup,
    InlineKeyboardButton,
)
from telegram.ext import (
    ApplicationBuilder,
    CommandHandler,
    MessageHandler,
    CallbackQueryHandler,
    ContextTypes,
    filters,
)
from telegram.error import BadRequest
from datetime import datetime

# ==============================
# TOKEN DEL BOT
# ==============================
TOKEN = "8208268564:AAGuSI3Rhfo1Iy_3aYcwuWeOfAQuLXydG8g"

# ==============================
# CONFIGURACIÓN CRATEDB
# ==============================
CRATE_URL = "http://localhost:4200"
CRATE_USER = ""
CRATE_PASS = ""
CRATE_TIMEOUT = 5

# ==============================
# URL de Grafana
# ==============================
GRAFANA_URL = "http://172.29.15.164:3000/?orgId=1&from=now-6h&to=now&timezone=browser"

MENSAJE_PREDETERMINADO = (
    "¡Bienvenido! Soy Agevital 🤖.\n"
    "Gestiono tu confort en casa: clima, luz y más ✨.\n"
    "¿Cómo puedo ayudarte?"
)

# ------------------------------------------------------------------
# Mapeo de código sensor -> columna en CrateDB
# ------------------------------------------------------------------
SENSOR_COLUMN_MAP = {
    "hum": "humidity",
    "temp": "temperature_digital",
    "ruido": "noise",
    "lux": "illuminance",
    "pres": "presence",
}

# ------------------------------------------------------------------
# Validación de URL
# ------------------------------------------------------------------
def url_es_valida(url: str) -> bool:
    if not url:
        return False
    url_l = url.lower()
    if any(b in url_l for b in ["localhost", "127.0.0.1", "0.0.0.0"]):
        return False
    if not (url_l.startswith("http://") or url_l.startswith("https://")):
        return False
    return True

# ------------------------------------------------------------------
# Menús
# ------------------------------------------------------------------
def menu_principal() -> InlineKeyboardMarkup:
    botones = []
    if url_es_valida(GRAFANA_URL):
        botones.append([InlineKeyboardButton("Link de Grafana", url=GRAFANA_URL)])
    else:
        botones.append([InlineKeyboardButton("Mostrar Grafana (texto)", callback_data="mostrar_url")])
    botones.append([InlineKeyboardButton("TARS_1", callback_data="submenu_tars1")])
    return InlineKeyboardMarkup(botones)

def submenu_tars1() -> InlineKeyboardMarkup:
    return InlineKeyboardMarkup([
        [
            InlineKeyboardButton("💧 Humedad", callback_data="sensor_hum"),
            InlineKeyboardButton("🌡 Temperatura", callback_data="sensor_temp"),
        ],
        [
            InlineKeyboardButton("🔊 Ruido", callback_data="sensor_ruido"),
            InlineKeyboardButton("💡 Iluminancia", callback_data="sensor_lux"),
        ],
        [
            InlineKeyboardButton("👤 Presencia", callback_data="sensor_pres"),
        ],
        [
            InlineKeyboardButton("⬅ Volver", callback_data="volver_menu")
        ]
    ])

# ------------------------------------------------------------------
# Consulta SÍNCRONA a CrateDB con requests
# ------------------------------------------------------------------
def query_crate_sync(sql: str) -> tuple:
    """Ejecuta SQL en CrateDB y devuelve (value, timestamp)"""
    url = CRATE_URL.rstrip("/") + "/_sql"
    headers = {"Content-Type": "application/json"}
    payload = {"stmt": sql}
    auth = None
    if CRATE_USER and CRATE_PASS:
        auth = (CRATE_USER, CRATE_PASS)

    try:
        resp = requests.post(url, json=payload, headers=headers, auth=auth, timeout=CRATE_TIMEOUT)
        if resp.status_code != 200:
            print(f"[ERROR] CrateDB retornó status {resp.status_code}: {resp.text}")
            return None, None
        
        data = resp.json()
        print(f"[DEBUG] Respuesta CrateDB: {data}")  # DIAGNÓSTICO
        
        rows = data.get("rows") or []
        
        if not rows:
            print("[DEBUG] No se encontraron filas")
            return None, None
        
        first = rows[0]
        print(f"[DEBUG] Primera fila: {first}")  # DIAGNÓSTICO
        
        value = first[0] if len(first) > 0 else None
        ts = first[1] if len(first) > 1 else None
        return value, ts
        
    except requests.exceptions.Timeout:
        print("[ERROR] Timeout al consultar CrateDB")
        return None, None
    except Exception as e:
        print(f"[ERROR] Excepción al consultar CrateDB: {e}")
        import traceback
        traceback.print_exc()
        return None, None

# ------------------------------------------------------------------
# Formatea timestamp
# ------------------------------------------------------------------
def format_ts(ts):
    if ts is None:
        return ""
    
    if isinstance(ts, (int, float)):
        if ts > 1e12:  # milisegundos
            try:
                return datetime.utcfromtimestamp(ts/1000).strftime("%Y-%m-%d %H:%M:%S UTC")
            except Exception:
                return str(ts)
        else:  # segundos
            try:
                return datetime.utcfromtimestamp(ts).strftime("%Y-%m-%d %H:%M:%S UTC")
            except Exception:
                return str(ts)
    
    return str(ts)

# ------------------------------------------------------------------
# Construye SQL
# ------------------------------------------------------------------
def build_sql_for_column(column_name: str) -> str:
    table = '"doc"."etconfort_data"'
    col = f'"{column_name}"'
    sql = f'SELECT {col}, time_index FROM {table} WHERE {col} IS NOT NULL ORDER BY time_index DESC LIMIT 1'
    return sql

# ------------------------------------------------------------------
# Obtiene valor de sensor
# ------------------------------------------------------------------
async def get_sensor_value_from_crate(sensor_code: str) -> tuple:
    col = SENSOR_COLUMN_MAP.get(sensor_code)
    if not col:
        return "Sensor no mapeado", ""
    
    sql = build_sql_for_column(col)
    print(f"[DEBUG] Ejecutando SQL para {sensor_code}: {sql}")  # DIAGNÓSTICO
    
    value, ts = query_crate_sync(sql)
    
    if value is None:
        return "No hay datos disponibles", ""
    
    ts_str = format_ts(ts)
    
    # Formateo según tipo
    if sensor_code == "hum":
        valor_fmt = f"{value}%"
    elif sensor_code == "temp":
        valor_fmt = f"{value} °C"
    elif sensor_code == "ruido":
        valor_fmt = f"{value} dB"
    elif sensor_code == "lux":
        valor_fmt = f"{value} lx"
    elif sensor_code == "pres":
        if isinstance(value, bool):
            valor_fmt = "Detectada ✅" if value else "No detectada ❌"
        elif isinstance(value, (int, float)):
            valor_fmt = "Detectada ✅" if value > 0 else "No detectada ❌"
        else:
            valor_fmt = str(value)
    else:
        valor_fmt = str(value)
    
    return valor_fmt, ts_str

# ------------------------------------------------------------------
# FUNCIÓN DE DIAGNÓSTICO
# ------------------------------------------------------------------
def test_crate_connection():
    """Prueba la conexión a CrateDB"""
    print("\n" + "="*60)
    print("DIAGNÓSTICO DE CONEXIÓN A CRATEDB")
    print("="*60)
    
    # Test 1: Conectividad básica
    try:
        resp = requests.get(CRATE_URL, timeout=3)
        print(f"✅ CrateDB alcanzable en {CRATE_URL} (status {resp.status_code})")
    except Exception as e:
        print(f"❌ No se puede alcanzar CrateDB: {e}")
        return False
    
    # Test 2: Consulta COUNT
    sql_test = 'SELECT COUNT(*) FROM "doc"."etconfort_data"'
    print(f"\nTest COUNT: {sql_test}")
    result = query_crate_sync(sql_test)
    print(f"Resultado: {result}")
    
    # Test 3: Última fila completa
    sql_full = 'SELECT noise, temperature_digital, humidity, illuminance, presence, time_index FROM "doc"."etconfort_data" ORDER BY time_index DESC LIMIT 1'
    print(f"\nTest última fila: {sql_full}")
    result_full = query_crate_sync(sql_full)
    print(f"Resultado: {result_full}")
    
    # Test 4: Cada sensor individual
    print("\n" + "-"*60)
    print("PRUEBA DE CADA SENSOR:")
    print("-"*60)
    for code, col in SENSOR_COLUMN_MAP.items():
        sql = build_sql_for_column(col)
        print(f"\n[{code} -> {col}]")
        print(f"SQL: {sql}")
        value, ts = query_crate_sync(sql)
        print(f"Valor: {value}, Timestamp: {ts}")
    
    print("\n" + "="*60)
    print("FIN DEL DIAGNÓSTICO")
    print("="*60 + "\n")
    return True

# ------------------------------------------------------------------
# Handlers
# ------------------------------------------------------------------
async def enviar_menu_principal(update: Update, context: ContextTypes.DEFAULT_TYPE):
    try:
        await update.message.reply_text(MENSAJE_PREDETERMINADO, reply_markup=menu_principal())
    except BadRequest as e:
        print(f"[WARN] Error al enviar menú principal: {e}")
        await update.message.reply_text(MENSAJE_PREDETERMINADO + "\n(No se pudo mostrar el menú)")

async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await enviar_menu_principal(update, context)

async def ayuda(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await update.message.reply_text(
        "Comandos disponibles:\n"
        "/start - menú inicial\n"
        "/ayuda - esta ayuda\n"
        "Escribe cualquier cosa para ver el menú principal."
    )

async def texto_generico(update: Update, context: ContextTypes.DEFAULT_TYPE):
    await enviar_menu_principal(update, context)

async def manejar_callback(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()
    data = query.data

    if data == "submenu_tars1":
        await query.edit_message_text(
            text="Submenú TARS_1:\nSelecciona una variable para consultar.",
            reply_markup=submenu_tars1()
        )
        return

    if data == "volver_menu":
        await query.edit_message_text(
            text=MENSAJE_PREDETERMINADO,
            reply_markup=menu_principal()
        )
        return

    if data == "mostrar_url":
        await query.message.reply_text(f"Grafana: {GRAFANA_URL}")
        return

    if data.startswith("sensor_"):
        sensor_map = {
            "sensor_hum": ("Humedad", "hum"),
            "sensor_temp": ("Temperatura", "temp"),
            "sensor_ruido": ("Ruido", "ruido"),
            "sensor_lux": ("Iluminancia", "lux"),
            "sensor_pres": ("Presencia", "pres"),
        }
        nombre, code = sensor_map.get(data, ("Desconocido", ""))
        
        valor, ts_str = await get_sensor_value_from_crate(code)
        
        texto = f"Submenú TARS_1\n{nombre}: {valor}"
        if ts_str:
            texto += f"\nÚltima lectura: {ts_str}"
        texto += "\n\nSelecciona otra variable:"
        
        try:
            await query.edit_message_text(text=texto, reply_markup=submenu_tars1())
        except BadRequest as e:
            print(f"[WARN] No se pudo editar mensaje: {e}")
            await query.message.reply_text(texto)
        return

    await query.message.reply_text("Callback no reconocido.")

# ------------------------------------------------------------------
# MAIN
# ------------------------------------------------------------------
def main():
    # EJECUTAR DIAGNÓSTICO PRIMERO
    test_crate_connection()
    
    app = ApplicationBuilder().token(TOKEN).build()
    app.add_handler(CommandHandler("start", start))
    app.add_handler(CommandHandler("ayuda", ayuda))
    app.add_handler(CallbackQueryHandler(manejar_callback))
    app.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, texto_generico))
    
    print("Bot ejecutándose... Ctrl+C para detener.")
    app.run_polling()

if __name__ == "__main__":
    main()