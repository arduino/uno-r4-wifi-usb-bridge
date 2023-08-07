/*
  OTA.cpp
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
#include <FS.h>
#include <SPIFFS.h>
#include "OTA.h"
#include <BossaArduino.h>

COtaHandler::COtaHandler()
: _state{OtaState::Idle}
, _url("")
, _ota_err {Arduino_ESP32_OTA::Error::None}
, _retry_count(0)
, _next_retry_tick(0)
{

}

void COtaHandler::run()
{
  OtaState next_state = _state;
  switch (_state)
  {
  case OtaState::Idle:                next_state = handleIdle();        break;
  case OtaState::WiFiCheck:           next_state = handleWiFiCheck();   break;
  case OtaState::StorageInit:         next_state = handleStorageInit(); break;
  case OtaState::Download:            next_state = handleDownload();    break;
  case OtaState::Verify:              next_state = handleVerify();      break;
  case OtaState::BootMode:            next_state = handleBootMode();    break;
  case OtaState::Flash:               next_state = handleFlash();       break;
  case OtaState::Restart:             next_state = handleRestart();     break;
  case OtaState::Error:               next_state = handleError();       break;
  case OtaState::Retry:               next_state = handleRetry();       break;
  case OtaState::Cleanup:             next_state = handleCleanup();     break;
  }
  _state = next_state;

  vTaskDelay(10);
}

int COtaHandler::start(const String& url) {
  if(isRunning()) {
    DEBUG_ERROR("OtaHandler::%s url: %s. Cannot start, OTA is running.", __FUNCTION__, _url.c_str());
    _ota_err = Error::OtaRunning;
    return 0;
  }
  _url = url;
  DEBUG_DEBUG("OtaHandler::%s url: %s", __FUNCTION__, _url.c_str());
  return 1;
}

bool COtaHandler::isRunning() {
  if(_state != OtaState::Idle || _retry_count > 0) {
    return true;
  }
  return false;
}

COtaHandler::Error COtaHandler::getError() {
  return _ota_err;
}

void COtaHandler::setCACert(const char *rootCA) {
  _ota.setCACert(rootCA);
}

COtaHandler::OtaState COtaHandler::handleIdle() {
  if(_url.length() > 0) {
    bool const is_retry_attempt = (_retry_count > 0);
    if(!is_retry_attempt || is_retry_attempt && (millis() > _next_retry_tick)) {
      DEBUG_DEBUG("OtaHandler::%s OTA try: %d", __FUNCTION__, _retry_count);
      return OtaState::WiFiCheck;
    }
  }
  return OtaState::Idle;
}

COtaHandler::OtaState COtaHandler::handleWiFiCheck() {
  if(WiFi.status() != WL_CONNECTED) {
    DEBUG_ERROR("OtaHandler::%s failed to start OTA: WiFi is not connected.", __FUNCTION__);
    _ota_err = Error::NoWiFi;
    return OtaState::Error;
  }
  return OtaState::StorageInit;
}

COtaHandler::OtaState COtaHandler::handleStorageInit() {
  if ((_ota_err = static_cast<COtaHandler::Error>(_ota.begin(OTA_HANDLER_UPDATE_FULL_PATH))) != Error::None)
  {
    DEBUG_ERROR("OtaHandler::%s failed with error code %d", __FUNCTION__, _ota_err);
    return OtaState::Error;
  }
  return OtaState::Download;
}

COtaHandler::OtaState COtaHandler::handleDownload() {
  DEBUG_DEBUG("OtaHandler::%s start downloading to flash ...", __FUNCTION__);
  int const ota_download = _ota.download(_url.c_str());
  if (ota_download <= 0)
  {
    DEBUG_DEBUG("OtaHandler::%s failed with error code %d", __FUNCTION__, ota_download);
    _ota_err = static_cast<COtaHandler::Error>(ota_download);
    return OtaState::Retry;
  }
  DEBUG_DEBUG("OtaHandler::%s %d bytes stored.", __FUNCTION__, ota_download);
  return OtaState::Verify;
}

COtaHandler::OtaState COtaHandler::handleVerify() {
  if ((_ota_err = static_cast<COtaHandler::Error>(_ota.update())) != Error::None)
  {
    DEBUG_ERROR("OtaHandler::%s failed with error code %d ", __FUNCTION__, _ota_err);
    return OtaState::Retry;
  }
  return OtaState::BootMode;
}

COtaHandler::OtaState COtaHandler::handleBootMode() {
  digitalWrite(GPIO_BOOT, HIGH);
  digitalWrite(GPIO_RST, LOW);
  delay(100);
  digitalWrite(GPIO_RST, HIGH);
  delay(100);
  digitalWrite(GPIO_RST, LOW);
  delay(100);
  digitalWrite(GPIO_RST, HIGH);
  return OtaState::Flash;
}

COtaHandler::OtaState COtaHandler::handleFlash() {
  if(!SPIFFS.begin()) {
    DEBUG_ERROR("OtaHandler::%s failed mounting SPIFFS", __FUNCTION__);
    _ota_err = Error::OtaSpiffs;
    return OtaState::Retry;
  }

  uint32_t baud = Serial.baudRate();
  if(!BOSSA::flash(OTA_HANDLER_UPDATE_FULL_PATH, Serial)) {
    DEBUG_ERROR("OtaHandler::%s failed flashing new firmware", __FUNCTION__);
    _ota_err = Error::OtaFlashing;
    Serial.begin(baud);
    return OtaState::Retry;
  }
  Serial.begin(baud);
  return OtaState::Restart;
}

COtaHandler::OtaState COtaHandler::handleRestart() {
  digitalWrite(GPIO_RST, LOW);
  delay(100);
  digitalWrite(GPIO_RST, HIGH);
  _url = String("");
  return OtaState::Cleanup;
}

COtaHandler::OtaState COtaHandler::handleError() {
  DEBUG_ERROR("OtaHandler::%s cannot perform OTA", __FUNCTION__);
  _url = String("");
  _retry_count = 0;
  return OtaState::Cleanup;
}

COtaHandler::OtaState COtaHandler::handleRetry() {
  if(_retry_count < OTA_HANDLER_MAX_RETRY) {
    _retry_count ++;
    _next_retry_tick = millis() + OTA_HANDLER_RETRY_TIME_ms;
    DEBUG_DEBUG("OtaHandler::%s retry: %d at tick %d", __FUNCTION__, _retry_count, _next_retry_tick);
    return OtaState::Cleanup;
  } else {
    return OtaState::Error;
  }
}

COtaHandler::OtaState COtaHandler::handleCleanup() {
  if(SPIFFS.exists(OTA_HANDLER_UPDATE_FILENAME)) {
    SPIFFS.remove(OTA_HANDLER_UPDATE_FILENAME);
  }
  SPIFFS.end();
  return OtaState::Idle;
}

COtaHandler OtaHandler;
