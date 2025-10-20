#include "Settings.h"

Settings::Settings()
{
}

void Settings::load()
{
  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);

  ssid = "";
  password = "";
  for (int i = 0; i < ssidLen; i++)
    ssid += (char)EEPROM.read(ssidAdr + i);
  for (int i = 0; i < passwordLen; i++)
    password += (char)EEPROM.read(passwordAdr + i);

  // Si no hay configuración, asignar valores predeterminados
  if (ssid.length() == 0)
    ssid = "defaultSSID";
  if (password.length() == 0)
    password = "12345678";
}

void Settings::save()
{
  if (ssid.length() > 31 || password.length() > 31)
  {
    Serial.println("Error: SSID o contraseña demasiado largos");
    return;
  }

  EEPROM.write(ssidLenAdr, ssid.length());
  EEPROM.write(passwordLenAdr, password.length());

  for (int i = 0; i < ssid.length(); i++)
    EEPROM.write(ssidAdr + i, ssid[i]);
  for (int i = 0; i < password.length(); i++)
    EEPROM.write(passwordAdr + i, password[i]);

  EEPROM.commit();
  Serial.println("Configuración WiFi guardada");
  info();
}

void Settings::info()
{
  Serial.println("Configuración:");
  Serial.println("SSID: " + ssid);
  Serial.println("Contraseña: " + password);
}

void Settings::reset()
{
  Serial.println("Reseteando configuración WiFi...");
  ssid = "defaultSSID";
  password = "12345678";
  save();
}