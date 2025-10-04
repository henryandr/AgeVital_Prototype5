#include "Settings.h"

Settings::Settings()
{
}

void Settings::load()
{
  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);
  urlnLen = EEPROM.read(urlnLenAdr);

  ssid = "";
  password = "";
  urln = "";
  for (int i = 0; i < ssidLen; i++)
    ssid += (char)EEPROM.read(ssidAdr + i);
  for (int i = 0; i < passwordLen; i++)
    password += (char)EEPROM.read(passwordAdr + i);
  for (int i = 0; i < urlnLen; i++)
    urln += (char)EEPROM.read(urlnAdr + i);
}

void Settings::save()
{
  ssidLen = ssid.length();
  passwordLen = password.length();
  urlnLen = urln.length();

  EEPROM.write(ssidLenAdr, ssidLen);
  EEPROM.write(passwordLenAdr, passwordLen);
  EEPROM.write(urlnLenAdr, urlnLen);
  for (int i = 0; i < ssidLen; i++)
    EEPROM.write(ssidAdr + i, ssid[i]);
  for (int i = 0; i < passwordLen; i++)
    EEPROM.write(passwordAdr + i, password[i]);
  for (int i = 0; i < urlnLen; i++)
    EEPROM.write(urlnAdr + i, urln[i]);

  EEPROM.commit();

  info();
  Serial.println("settings saved");

}

void Settings::info()
{
  Serial.println("settings:");
  Serial.println("SSID: " + ssid);
  Serial.println("SSID length: " + (String)ssidLen);
  Serial.println("password: " + password);
  Serial.println("password length: " + (String)passwordLen);
  Serial.println("URL: " + urln);
  Serial.println("URL length: " + (String)urlnLen);
}

void Settings::reset()
{
  Serial.print("reset settings...");

  ssid = "dummy";
  password = "dummy";
  urln = "dummy";
  ssidHidden = false;
  apChannel = 1;

  ssidLen = ssid.length();
  passwordLen = password.length();
  urlnLen = urln.length();

  Serial.println("done");

  save();
}