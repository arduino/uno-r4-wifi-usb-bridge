

#include "at_handler.h"
#include "commands.h"
#include "cmds_esp_generic.h"


#include "cmds_wifi_station.h"
#include "cmds_wifi_softAP.h"
#include "cmds_wifi_netif.h"
#include "cmds_preferences.h"
#include "cmds_wifi_SSL.h"
#include "cmds_wifi_udp.h"
#include "cmds_ble_bridge.h"
#include "cmds_ota.h"
#include "cmds_se.h"

using namespace SudoMaker;

uint8_t CAtHandler::wifi_status = WIFI_ST_IDLE_STATUS;

/* -------------------------------------------------------------------------- */
CClientWrapper CAtHandler::getClient(int sock) {
/* -------------------------------------------------------------------------- */
  CClientWrapper rv;

  bool is_server = false;
  bool is_sslclienet = false;

  int internal_sock = -1;

  if(sock >= START_SSL_CLIENT_SOCK) {
    internal_sock = sock - START_SSL_CLIENT_SOCK;
    is_sslclienet = true;
  } else
  if(sock >= START_CLIENT_SERVER_SOCK) {
    internal_sock = sock - START_CLIENT_SERVER_SOCK;
    is_server = true;
  }
  else {
    internal_sock = sock;
  }

  if(internal_sock < 0 || internal_sock >= MAX_CLIENT_AVAILABLE) {
    rv.client = nullptr;
    rv.sslclient = nullptr;
    rv.can_delete = -1;
    return rv;
  }

  if (is_sslclienet) {
  rv.sslclient = sslclients[internal_sock];
  rv.can_delete = internal_sock;
  }
  else if(is_server) {
    rv.client = &serverClients[internal_sock].client;
    rv.can_delete = -1;
  }
  else {
    rv.client = clients[internal_sock];
    rv.can_delete = internal_sock;
  }
  return rv;
}

static int baudrates[] = {
  9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600,
  1000000, 1500000, 2000000
};
int i = 0;

/* -------------------------------------------------------------------------- */
void CAtHandler::run() {
/* -------------------------------------------------------------------------- */
  switch(state) {
  case Running:
    at_srv.run();
    break;
  case BaudRateDetection:
    if(serial->available()) {
      log_e("1available %d", serial->available());
      uint8_t a = serial->read();
      log_e("read 0x%X", a);

      serial->end();

      while(serial->available() > 0) {
        log_i("%02X", serial->read());
      }

      if (a == 0x55) {
        state = Running;

        log_e("Baudrate detection finished %d", baudrates[i]);
        serial->flush();
        serial->begin(baudrates[i], SERIAL_8N1, 6, 5);
        serial->write(0x55); // send the confirmation the baudrate was detected
        break;
      }

      i = (i+1) % (sizeof(baudrates)/sizeof(baudrates[0]));
      log_e("trying %d %d", i, baudrates[i]);

      serial->begin(baudrates[i], SERIAL_8N1, 6, 5);
      serial->updateBaudRate(baudrates[i]);
    }
    break;
  }
  vTaskDelay(1);
}



/* -------------------------------------------------------------------------- */
CAtHandler::CAtHandler(HardwareSerial *s) : last_server_client_sock(0), state(BaudRateDetection) {
/* -------------------------------------------------------------------------- */

  for(int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
    clients[i] = nullptr;
  }

  for(int i = 0; i < MAX_SERVER_AVAILABLE; i++) {
    serverWiFi[i] = nullptr;
  }

  for(int i = 0; i < MAX_UDP_AVAILABLE; i++) {
    udps[i] = nullptr;
  }

  for(int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
    sslclients[i] = nullptr;
    clients_ca[i].clear();
    clients_cert_pem[i].clear();
    clients_key_pem[i].clear();
  }

  /* set up serial */
  serial = s;

  /* set up chatAt server callbacks */
  at_srv.set_io_callback({
    .callback_io_read = [this](auto buf, auto len) {
      if (!serial->available()) {
        yield();
        return (unsigned int)0;
      }
      return serial->read(buf, min((unsigned int)serial->available(), len));
    },
    .callback_io_write = [this](auto buf, auto len) {
      return serial->write(buf, len);
    },
  });

  at_srv.set_command_callback([this](chAT::Server & srv, const std::string & command) {
    auto it = command_table.find(command);

    if (it == command_table.end()) {
      return chAT::CommandStatus::ERROR;
    }
    else {
      return it->second(srv, srv.parser());
    }
  });

  /*  SET UP COMMAND TABLE */
  add_cmds_esp_generic();
  add_cmds_wifi_station();
  add_cmds_wifi_softAP();
  add_cmds_wifi_SSL();
  add_cmds_wifi_netif();
  add_cmds_wifi_udp();
  add_cmds_ble_bridge();
  add_cmds_ota();
  add_cmds_preferences();
  add_cmds_se();
}
