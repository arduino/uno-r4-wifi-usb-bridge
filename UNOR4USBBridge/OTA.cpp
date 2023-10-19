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

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Arduino_ESP32_OTA::Error Arduino_UNOWIFIR4_OTA::begin(const char* file_path, uint32_t magic)
{
  /* initialize private variables */
  otaInit();

  /* ... initialize CRC ... */
  crc32Init();

  /* ... configure board Magic number */
  setMagic(magic);

  if(!SPIFFS.begin()) {
    DEBUG_ERROR("%s: failed to initialize SPIFFS", __FUNCTION__);
    return Error::OtaStorageInit;
  }

  if(SPIFFS.exists(file_path)) {
    SPIFFS.remove(file_path);
  }

  _spiffs = true;

  SPIFFS.end();
  return Error::None;
}

void Arduino_UNOWIFIR4_OTA::write_byte_to_flash(uint8_t data)
{
  if(_spiffs) {
    int ret = fwrite(&data, sizeof(data), 1, _file);
  } else {
    Arduino_ESP32_OTA::write_byte_to_flash(data);
  }
}

int Arduino_UNOWIFIR4_OTA::download(const char * ota_url, const char* file_path)
{
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

  /* Download and decode OTA file */
  size_t size = download(ota_url);

  fclose(_file);
  _file = nullptr;
  SPIFFS.end();
  return size;
}

Arduino_ESP32_OTA::Error Arduino_UNOWIFIR4_OTA::verify()
{
  /* ... then finalize ... */
  crc32Finalize();

  if(!crc32Verify()) {
    DEBUG_ERROR("%s: CRC32 mismatch", __FUNCTION__);
    return Error::OtaHeaderCrc;
  }
  return Error::None;
}

Arduino_ESP32_OTA::Error Arduino_UNOWIFIR4_OTA::update()
{
  if (!Update.end(true)) {
    DEBUG_ERROR("%s: Failure to apply OTA update. Error: %s", __FUNCTION__, Update.errorString());
    return Error::OtaStorageEnd;
  }
  return Error::None;
}
