/*
  BossaUnoR4WiFiArduino.h
  Copyright (c) 2023 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _BOSSAUNOR4_H
#define _BOSSAUNOR4_H

#include <BossaArduino.h>

class BossaUnoR4WiFiObserver : public FlasherObserver
{
public:
  BossaUnoR4WiFiObserver() : _lastTicks(-1) {}
  virtual ~BossaUnoR4WiFiObserver() {}

  virtual void onStatus(const char *message, ...) {};
  virtual void onProgress(int num, int div);

private:
  int _lastTicks;
};

class BossaUnoR4WiFi : public BossaArduino
{
public:
  BossaUnoR4WiFi(FlasherObserver& observer) : BossaArduino(observer) {}
  virtual ~BossaUnoR4WiFi() {}

  int program(const char* filePath, HardwareSerial& serial, int bootPin, int resetPin);

private:
  void setProgramMode(int bootPin, int resetPin);
  void reset(int resetPin);
};

extern BossaUnoR4WiFi BOSSA;

#endif
