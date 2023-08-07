/*
  OTA.h
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
#ifndef __OTA__
#define __OTA__

#include <Arduino_ESP32_OTA.h>

#define GPIO_BOOT   9
#define GPIO_RST    4

class COtaHandler
{
public:
  enum class Error:  int {
    None                 =  0,
    NoOtaStorage         = -2,
    OtaStorageInit       = -3,
    OtaStorageEnd        = -4,
    UrlParseError        = -5,
    ServerConnectError   = -6,
    HttpHeaderError      = -7,
    ParseHttpHeader      = -8,
    OtaHeaderLength      = -9,
    OtaHeaderCrc         = -10,
    OtaHeaderMagicNumber = -11,
    OtaDownload          = -12,
    OtaRunning           = -13,
    NoWiFi               = -14,
    OtaSpiffs            = -15,
    OtaFlashing          = -16
  };

  COtaHandler();

  void run();
  void setCACert (const char *rootCA);
  int start(const String& url);
  bool isRunning();
  COtaHandler::Error getError();

private:
  enum class OtaState : unsigned int {
    Idle,
    WiFiCheck,
    StorageInit,
    Download,
    Verify,
    BootMode,
    Flash,
    Restart,
    Error,
    Retry,
    Cleanup
  };

  OtaState _state = OtaState::Idle;
  String _url;
  COtaHandler::Error _ota_err;
  uint32_t _retry_count;
  uint32_t _next_retry_tick;
  Arduino_ESP32_OTA _ota;

  static constexpr int OTA_HANDLER_MAX_RETRY = 3;
  static constexpr int OTA_HANDLER_RETRY_TIME_ms = 15000;
  static constexpr auto OTA_HANDLER_UPDATE_FULL_PATH = "/spiffs/update.bin";
  static constexpr auto OTA_HANDLER_UPDATE_FILENAME = "/update.bin";

  OtaState handleIdle();
  OtaState handleWiFiCheck();
  OtaState handleStorageInit();
  OtaState handleDownload();
  OtaState handleVerify();
  OtaState handleBootMode();
  OtaState handleFlash();
  OtaState handleRestart();
  OtaState handleError();
  OtaState handleRetry();
  OtaState handleCleanup();
};

extern COtaHandler OtaHandler;

#endif
