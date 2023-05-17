#ifndef CMDS_WIFI_STATION_H
#define CMDS_WIFI_STATION_H


#include "at_handler.h"

void CAtHandler::add_cmds_wifi_station() {
   
   /* ....................................................................... */
   command_table[_WIFISCAN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            int n = WiFi.scanNetworks();
            if (n == 0) {
            } 
            else {
               for (int i = 0; i < n; ++i) {
                  srv.write_response_prompt();
                  String scan_results = WiFi.SSID(i) + " | " + String(WiFi.RSSI(i)) + " | " + String(WiFi.channel(i)) + " | ";
                  switch (WiFi.encryptionType(i)) {
                     case WIFI_AUTH_OPEN:
                       scan_results += "open\r\n";
                       break;
                     case WIFI_AUTH_WEP:
                       scan_results += "WEP\r\n";
                       break;
                     case WIFI_AUTH_WPA_PSK:
                       scan_results += "WPA\r\n";
                       break;
                     case WIFI_AUTH_WPA2_PSK:
                       scan_results += "WPA2\r\n";
                       break;
                     case WIFI_AUTH_WPA_WPA2_PSK:
                       scan_results += "WPA+WPA2\r\n";
                       break;
                     case WIFI_AUTH_WPA2_ENTERPRISE:
                       scan_results += "WPA2-EAP\r\n";
                       break;
                     case WIFI_AUTH_WPA3_PSK:
                       scan_results += "WPA3\r\n";
                       break;
                     case WIFI_AUTH_WPA2_WPA3_PSK:
                       scan_results += "WPA2+WPA3\r\n";
                       break;
                     case WIFI_AUTH_WAPI_PSK:
                       scan_results += "WAPI\r\n";
                       break;
                     default:
                       scan_results += "unknown\r\n";
                  }
                  
                  srv.write_str((const char *)(scan_results.c_str()));
               }
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
 
}

#endif