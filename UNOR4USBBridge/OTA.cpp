/*
   This file is part of UNOR4USBBridge_OTA.

   Copyright 2023 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/
#include <Update.h>
#include <SPIFFS.h>
#include <Arduino_ESP32_OTA.h>
#include "OTA.h"
#include "BossaUnoR4WiFi.h"

#include "FS.h"
#include "SPIFFS.h"
#include "at_handler.h"

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Arduino_UNOWIFIR4_OTA::Arduino_UNOWIFIR4_OTA()
: _updating_renesas(true) {

}

Arduino_UNOWIFIR4_OTA::~Arduino_UNOWIFIR4_OTA() {
  closeStorage();
}

Arduino_ESP32_OTA::Error Arduino_UNOWIFIR4_OTA::begin(const char* file_path, uint32_t magic)
{
  /* ... configure board Magic number */
  setMagic(magic);

  if(!SPIFFS.begin()) {
    DEBUG_ERROR("%s: failed to initialize SPIFFS", __FUNCTION__);
    return Error::OtaStorageInit;
  }

  if(SPIFFS.exists(file_path)) {
    SPIFFS.remove(file_path);
  }

  _updating_renesas = true;

  SPIFFS.end();
  return Error::None;
}

void Arduino_UNOWIFIR4_OTA::write_byte_to_flash(uint8_t data)
{
  if(_updating_renesas) {
    int ret = fwrite(&data, sizeof(data), 1, _file);
  } else {
    Arduino_ESP32_OTA::write_byte_to_flash(data);
  }
}

int Arduino_UNOWIFIR4_OTA::initStorage(const char* file_path) {
  if(!SPIFFS.begin()) {
    DEBUG_ERROR("%s: failed to initialize SPIFFS", __FUNCTION__);
    return static_cast<int>(Error::OtaStorageInit);
  }

  String spiffs_path = String("/spiffs") + String(file_path);
  _file = fopen(spiffs_path.c_str(), "wb");

  if(!_file) {
    DEBUG_ERROR("%s: failed to write SPIFFS", __FUNCTION__);
    return static_cast<int>(Error::OtaStorageInit);
  }
  return static_cast<int>(Error::None);
}

int Arduino_UNOWIFIR4_OTA::closeStorage() {
  int res = 0;
  if(_file != nullptr) {
    res = fclose(_file);
    _file = nullptr;
  }

  SPIFFS.end();
  return res;
}

int Arduino_UNOWIFIR4_OTA::download(const char * ota_url, const char* file_path)
{
  int res = initStorage(file_path);

  if(res < 0) {
    return res;
  }

  /* Download and decode OTA file */
  res = download(ota_url);

  closeStorage();

  return res;
}

int Arduino_UNOWIFIR4_OTA::startDownload(const char * ota_url, const char* file_path)
{
  int res = initStorage(file_path);

  if(res < 0) {
    return res;
  }

  /* Download and decode OTA file */
  res = startDownload(ota_url);

  if(res < 0) {
    closeStorage();
  }

  return res;
}

int Arduino_UNOWIFIR4_OTA::downloadPoll()
{
  auto res = Arduino_ESP32_OTA::downloadPoll();

  if(_updating_renesas && res != 0) {
    closeStorage();
  }

  return res;
}

int Arduino_UNOWIFIR4_OTA::update(const char* file_path)
{
  return BOSSA.program(file_path, Serial, GPIO_BOOT, GPIO_RST);
}
