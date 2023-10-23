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

#ifndef ARDUINO_UNOWIFIR4_OTA_H_
#define ARDUINO_UNOWIFIR4_OTA_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino_ESP32_OTA.h>

/******************************************************************************
 * DEFINES
 ******************************************************************************/
#define ARDUINO_RA4M1_OTA_MAGIC 0x23411002

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class Arduino_UNOWIFIR4_OTA : public Arduino_ESP32_OTA
{

public:

  enum class UNO_WiFi_R4_Error : int
  {
    StorageConfig = -20,
  };

  using Arduino_ESP32_OTA::begin;
  Arduino_ESP32_OTA::Error begin(const char* file_path, uint32_t magic = ARDUINO_RA4M1_OTA_MAGIC);
  using Arduino_ESP32_OTA::download;
  int download(const char * ota_url, const char* file_path);
  void write_byte_to_flash(uint8_t data);
  Arduino_ESP32_OTA::Error verify();
  Arduino_ESP32_OTA::Error update();

private:

  FILE* _file;
  bool _spiffs;
};

#endif /* ARDUINO_UNOWIFIR4_OTA_H_ */
