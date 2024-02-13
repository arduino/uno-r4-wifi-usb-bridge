#ifndef ARDUINO_AT_HANDLER_H
#define ARDUINO_AT_HANDLER_H

#include "chAT.hpp"
#include "WiFi.h"
#include "Server.h"

#include "WiFiClient.h"
#include <WiFiClientSecure.h>
#include "FS.h"
#include "SPIFFS.h"

#define MAX_CLIENT_AVAILABLE   8
#define MAX_SERVER_AVAILABLE   4
#define MAX_UDP_AVAILABLE      4

#define ESP_FW_VERSION    "0.4.0"
#define FIRMWARE_MAYOR    0
#define FIRMWARE_MINOR    4
#define FIRMWARE_PATCH    0

#define GPIO_BOOT   9
#define GPIO_RST    4

using namespace SudoMaker;

#define WIFI_ST_NO_SHIELD            255
#define WIFI_ST_NO_MODULE            255
#define WIFI_ST_IDLE_STATUS           0
#define WIFI_ST_NO_SSID_AVAIL         1
#define WIFI_ST_SCAN_COMPLETED        2
#define WIFI_ST_CONNECTED             3
#define WIFI_ST_CONNECT_FAILED        4
#define WIFI_ST_CONNECTION_LOST       5
#define WIFI_ST_DISCONNECTED          6
#define WIFI_ST_AP_LISTENING          7
#define WIFI_ST_AP_CONNECTED          8
#define WIFI_ST_AP_FAILED             9


class CClientWrapper {
public:
   WiFiClient *client;
   WiFiClientSecure *sslclient;
   int can_delete = -1;
};

class CServerClient {
public:
   WiFiClient client;
   int server = -1;
   bool accepted = false;
};

class CAtHandler {
private:
   static uint8_t wifi_status; 

   int last_server_client_sock;
   
   WiFiUDP    * udps[MAX_UDP_AVAILABLE];
   WiFiServer * serverWiFi[MAX_SERVER_AVAILABLE];
   WiFiClient * clients[MAX_CLIENT_AVAILABLE];
   CServerClient  serverClients[MAX_CLIENT_AVAILABLE];
   WiFiClientSecure * sslclients[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_ca[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_cert_pem[MAX_CLIENT_AVAILABLE];
   std::vector<std::uint8_t> clients_key_pem[MAX_CLIENT_AVAILABLE];
   int udps_num = 0;
   int servers_num = 0;
   int clientsToServer_num = 0;
   int clients_num = 0;
   int sslclients_num = 0;
   std::unordered_map<std::string, std::function<chAT::CommandStatus(chAT::Server&, chAT::ATParser&)>> command_table;
   chAT::Server at_srv;
   HardwareSerial *serial;

   CClientWrapper getClient(int sock);

   void add_cmds_esp_generic();
   void add_cmds_wifi_station(); 
   void add_cmds_wifi_softAP(); 
   void add_cmds_wifi_SSL();
   void add_cmds_wifi_netif();
   void add_cmds_wifi_udp();
   void add_cmds_ble_bridge();
   void add_cmds_ota();
   void add_cmds_preferences();
   void add_cmds_se();
public:
   /* Used by cmds_se */
   std::vector<std::uint8_t> se_buf;

   /* Used by cmds_ota */
   std::vector<std::uint8_t> ota_cert_buf;

   /* Used by cmds_preferences */
   std::vector<std::uint8_t> pref_buf;

   CAtHandler(HardwareSerial *s);
   CAtHandler() = delete ;
   static void onWiFiEvent(WiFiEvent_t event);
   void run();
};


#endif
