#ifndef ARDUINO_AT_HANDLER_H
#define ARDUINO_AT_HANDLER_H

#include "chAT.hpp"
#include "WiFi.h"
#include "Server.h"

#include "WiFiClient.h"
#include <WiFiClientSecure.h>

#define MAX_CLIENT_AVAILABLE   16

using namespace SudoMaker;

class CAtHandler {
private:
   
   static void onWiFiEvent(WiFiEvent_t event);
   WiFiClient * clients[MAX_CLIENT_AVAILABLE];
   WiFiClientSecure * sslClients[MAX_CLIENT_AVAILABLE];
   int clients_num = 0;
   std::unordered_map<std::string, std::function<chAT::CommandStatus(chAT::Server&, chAT::ATParser&)>> command_table;
   chAT::Server at_srv;
   HardwareSerial *serial;

   void add_cmds_esp_generic();
   void add_cmds_wifi_station(); 
public:
   CAtHandler(HardwareSerial *s);
   void run();
};


#endif