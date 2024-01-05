/*
  BossaUnoR4WiFi.cpp
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

#include "BossaUnoR4WiFi.h"
#include <Arduino_DebugUtils.h>
#include <SPIFFS.h>

void BossaUnoR4WiFiObserver::onProgress(int num, int div) {   

  int ticks;
  int bars = 30;  
  ticks = num * bars / div;

  if (ticks == _lastTicks) {
    return;
  }

  Debug.newlineOff();
  DEBUG_INFO("\r[");
  while (ticks-- > 0) {
    DEBUG_INFO("=");
    bars--;
  }
  while (bars-- > 0) {
    DEBUG_INFO(" ");
  }
  Debug.newlineOn();

  DEBUG_INFO("] %d%% (%d/%d pages)", num * 100 / div, num, div);

  _lastTicks = 0;
}

int BossaUnoR4WiFi::program(const char* filePath, HardwareSerial& serial, int bootPin, int resetPin) {

  SPIFFS.begin();
  if(!SPIFFS.exists(filePath)) {
    DEBUG_ERROR("Update file do not exist");
    return 0;
  }
    
  String spiffs_path = String("/spiffs") + String(filePath);  
  setProgramMode(bootPin, resetPin);  
  delay(500);
  
  /* Save original baudrate */
  uint32_t baud = serial.baudRate();
  
  DEBUG_INFO("Connecting to SAM-BA ... ");
  if (!connect(serial)) {
    DEBUG_ERROR("SAM-BA can't connect.");
    return 0;
  }
  flash(spiffs_path.c_str());

  /* Restore baudrate */
  serial.begin(baud);

  SPIFFS.remove(filePath);
  SPIFFS.end();

  /* Reset microcontroller*/
  reset(resetPin);

  return 1;
}

void BossaUnoR4WiFi::setProgramMode(int bootPin, int resetPin) {

  digitalWrite(bootPin, HIGH);
  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);
  delay(100);
  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);
}

void BossaUnoR4WiFi::reset(int resetPin) {

  digitalWrite(resetPin, LOW);
  delay(100);
  digitalWrite(resetPin, HIGH);
}

BossaUnoR4WiFiObserver obs;
BossaUnoR4WiFi BOSSA(obs);
