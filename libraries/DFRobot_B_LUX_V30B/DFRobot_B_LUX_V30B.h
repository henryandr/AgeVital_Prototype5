/*
 * @file DFRobot_B_LUX_V30B.h
 * @brief Define una clase para el sensor de luz DFRobot_B_LUX_V30B.
 * @n Esta versión ha sido modificada para usar la librería estándar Wire.h.
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [Fary](Fary_young@outlook.com)
 * @version  V1.0
 * @date  2020-12-03
 * @https://github.com/DFRobot/DFRobot_B_LUX_V30B
 *
 * MODIFICACIONES PARA ESP32 Y BUS I2C COMPARTIDO:
 * 1. Se cambió la implementación de I2C a la librería estándar Wire.h.
 * 2. Se modificó el constructor para aceptar un puntero al objeto TwoWire.
 * 3. Se agregaron métodos para manejar transacciones I2C de forma robusta.
 * 4. Se utiliza la dirección I2C de 7 bits (0x4A) en lugar de 8 bits (0x94).
 */
#ifndef  _DFROBOT_B_LUX_V30B_H_
#define  _DFROBOT_B_LUX_V30B_H_

#include <Arduino.h>
#include <Wire.h> 

// Habilita la depuración en el monitor serial
#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...)  { Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__); }
#else
#define DBG(...)
#endif

// Dirección de 7 bits del sensor de luz
#define DFRobot_B_LUX_V30_IIC_Addr 0x4A   

#define DFRobot_B_LUX_V30_DataReg    0x00    // Registro de datos (4 bytes)
#define DFRobot_B_LUX_V30_ConfigReg  0x04    // Registro de configuración

typedef unsigned int    uint;
class DFRobot_B_LUX_V30B{
public:
  #define ACK      0x00
  #define NACK     0x01
  
  // Nuevo constructor que usa el bus I2C de hardware
  DFRobot_B_LUX_V30B(TwoWire *pWire, uint8_t cEN);

  ~DFRobot_B_LUX_V30B(){
  };
  
  void begin(void);
  float lightStrengthLux(void);
  uint8_t setMode(uint8_t isManualMode=0,uint8_t isCDR=0,uint8_t isTime=0);
  uint8_t readMode(void);
  
private:
  uint8_t _cEN;
  uint8_t _deviceAddr;
  TwoWire *_pWire;

  bool readData(uint8_t reg, uint8_t* data, uint8_t size);
  bool writeData(uint8_t reg, uint8_t data);
};
#endif
