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

using namespace SudoMaker;

class CClientWrapper {
public:
   WiFiClient *client;
   int can_delete = -1;
};


class CAtHandler {
private:
   
   static void onWiFiEvent(WiFiEvent_t event);
   WiFiServer * serverWiFi[MAX_SERVER_AVAILABLE];
   WiFiClient * clients[MAX_CLIENT_AVAILABLE];
   WiFiClient  serverClients[MAX_CLIENT_AVAILABLE];
   int servers_num = 0;
   int clientsToServer_num = 0;
   int clients_num = 0;
   std::unordered_map<std::string, std::function<chAT::CommandStatus(chAT::Server&, chAT::ATParser&)>> command_table;
   chAT::Server at_srv;
   HardwareSerial *serial;

   CClientWrapper getClient(int sock);

   void add_cmds_esp_generic();
   void add_cmds_wifi_station(); 
   void add_cmds_wifi_softAP(); 
   void add_cmds_wifi_SSL();
   void add_cmds_wifi_netif();
public:
   CAtHandler(HardwareSerial *s);
   void run();
};


#endif