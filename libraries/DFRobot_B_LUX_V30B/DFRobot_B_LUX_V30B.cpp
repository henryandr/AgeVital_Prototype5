/*!
 * @file DFRobot_B_LUX_V30B.cpp
 * @brief Implementaciones de los métodos de la clase DFRobot_B_LUX_V30B
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence    The MIT License (MIT)
 * @author [Fary](Fary_young@outlook.com)
 * @version  V1.0
 * @date  2020-12-03
 * @https://github.com/DFRobot/DFRobot_B_LUX_V30B
 *
 * MODIFICACIONES PARA ESP32 Y BUS I2C COMPARTIDO:
 * Se reemplazó el "bit-banging" por llamadas a la librería Wire.h para
 * compatibilidad con otros dispositivos I2C en el mismo bus.
 * Se añadió un robusto manejo de errores de comunicación.
 */

#include <DFRobot_B_LUX_V30B.h>
#include <Wire.h>

DFRobot_B_LUX_V30B::DFRobot_B_LUX_V30B(TwoWire *pWire, uint8_t cEN)
{
  _deviceAddr = DFRobot_B_LUX_V30_IIC_Addr;
  _pWire = pWire; 
  _cEN = cEN;
}

void DFRobot_B_LUX_V30B::begin()
{
  // Controlar el pin de habilitación para encender el sensor.
  pinMode(_cEN, OUTPUT);
  digitalWrite(_cEN, LOW);
  delay(1000);
  digitalWrite(_cEN, HIGH);
  delay(100);

  // Intentar la primera lectura para confirmar que el sensor está listo.
  float luxValue = lightStrengthLux();
  if (luxValue < 0) {
    DBG("Error de lectura inicial. El sensor de luz puede no estar conectado.");
  }
}

uint8_t DFRobot_B_LUX_V30B::readMode(void)
{
  uint8_t mode = 0;
  if (readData(DFRobot_B_LUX_V30_ConfigReg, &mode, 1)) {
    return mode;
  }
  return 0xFF; // Retorna 0xFF en caso de error
}

uint8_t DFRobot_B_LUX_V30B::setMode(uint8_t isManualMode, uint8_t isCDR, uint8_t isTime)
{
  uint8_t mode = isManualMode + isCDR + isTime;
  if (writeData(DFRobot_B_LUX_V30_ConfigReg, mode)) {
    return 1;
  }
  return 0;
}

bool DFRobot_B_LUX_V30B::readData(uint8_t reg, uint8_t* data, uint8_t size)
{
  // Paso 1: Transmitir la dirección del registro que queremos leer
  _pWire->beginTransmission(_deviceAddr);
  _pWire->write(reg);
  uint8_t result = _pWire->endTransmission();
  if (result != 0) {
    DBG("Error al escribir el registro. Código:");
    DBG(result);
    return false;
  }

  // Paso 2: Solicitar y leer los datos
  uint8_t received = _pWire->requestFrom(_deviceAddr, size);
  if (received != size) {
    DBG("Error de requestFrom. Recibidos:");
    DBG(received);
    return false;
  }

  for (uint8_t i = 0; i < size; i++) {
    data[i] = _pWire->read();
  }
  return true;
}

bool DFRobot_B_LUX_V30B::writeData(uint8_t reg, uint8_t data)
{
  _pWire->beginTransmission(_deviceAddr);
  _pWire->write(reg);
  _pWire->write(data);
  uint8_t result = _pWire->endTransmission();
  
  if (result != 0) {
    DBG("Error al escribir los datos. Código:");
    DBG(result);
    return false;
  }
  return true;
}

float DFRobot_B_LUX_V30B::lightStrengthLux()
{
  uint32_t value = 0;
  uint8_t data[4];
  
  if (readData(DFRobot_B_LUX_V30_DataReg, data, 4)) {
    value = data[3];
    value = (value << 8) | data[2];
    value = (value << 8) | data[1];
    value = (value << 8) | data[0];
    return ((float)value * 1.4) / 1000.0;
  }
  return -1.0; // Retorna -1.0 en caso de error
}