#ifndef Settings_h
#define Settings_h

#include <EEPROM.h>
#include <Arduino.h>

extern const bool debug;

#define ssidLenAdr 1024
#define ssidAdr 1025
#define passwordLenAdr 1057
#define passwordAdr 1058
#define urlnLenAdr 1110
#define urlnAdr 1111
#define flagAdr 1080

class Settings
{
public:
  Settings();
  void load();
  void reset();
  void save();
  String get();
  void info();

  int ssidLen;
  String ssid = "";
  bool ssidHidden;
  int passwordLen;
  String password = "";
  int urlnLen;
  String urln = "";
  int apChannel;

private:
};

#endif
