#ifndef CMDS_WIFI_STATION_H
#define CMDS_WIFI_STATION_H

#include "at_handler.h"

/* -------------------------------------------------------------------------- */
void CAtHandler::add_cmds_wifi_station() {
/* -------------------------------------------------------------------------- */   
   
   /* ....................................................................... */
   command_table[_WIFISCAN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            int n = WiFi.scanNetworks();
            if (n == 0) {
            } 
            else {
               String scan_results = "";

               srv.write_response_prompt();
               for (int i = 0; i < n; ++i) {
                  uint8_t *mac = WiFi.BSSID(i);
                  String bssid = "";
                  for(int k = 0; k < 6; k++) {
                     if(*(mac + k) < 0x10) {
                        bssid += "0";
                     }
                     bssid += String(*(mac + k),HEX);
                     if(k < 5)
                        bssid += ":";
                  }

                  scan_results += WiFi.SSID(i) + " | " + bssid + " | " + String(WiFi.RSSI(i)) + " | " + String(WiFi.channel(i)) + " | ";
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
               }
               srv.write_str((const char *)(scan_results.c_str()));
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_GETSTATUS] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            srv.write_response_prompt();
            String status(WiFi.status());
            srv.write_str((const char *)(status.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_MODE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() <= 0 || parser.args.size() > 2) {
               return chAT::CommandStatus::ERROR;
            }

            auto &conn_type = parser.args[0];
            if (conn_type.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            switch (atoi(conn_type.c_str())) {
               case 0: {
                  WiFi.mode(WIFI_MODE_NULL);
                  break;
               }
               case 1: {
                  WiFi.mode(WIFI_MODE_STA);
                  break;
               }
               case 2: {
                  WiFi.mode(WIFI_MODE_AP);
                  break;
               }
               case 3: {
                  WiFi.mode(WIFI_MODE_APSTA);
                  break;
               }
               default: {
                  return chAT::CommandStatus::ERROR;
               }
            }

            auto &auto_Conn = parser.args[1];
            if (!auto_Conn.empty()) {
               bool autoconn = atoi(auto_Conn.c_str());
               WiFi.setAutoConnect(autoconn);
            }
            srv.write_response_prompt();
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Read: {
            String mode_read = String(WiFi.getMode()) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(mode_read.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }    
   };

   /* ....................................................................... */
   command_table[_BEGINSTA] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 2) {
              return chAT::CommandStatus::ERROR;
            }
            auto &ssid = parser.args[0];
            if (ssid.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            auto &password = parser.args[1];
            if (password.empty()) {
              return chAT::CommandStatus::ERROR;
            }

            int res = WiFi.begin(ssid.c_str(), password.c_str());
            
            String status = String(res);
            srv.write_response_prompt();
            srv.write_str((const char *)status.c_str());
            srv.write_line_end();
            return chAT::CommandStatus::OK;
            
         }
            
            
            return chAT::CommandStatus::ERROR;
         default:
            return chAT::CommandStatus::ERROR;
      } 
   };

  /* ....................................................................... */
  command_table[_RECONNECT] = [this](auto & srv, auto & parser) {
  /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            String line = String(WiFi.getAutoReconnect());

            srv.write_response_prompt();
            srv.write_str((const char *)(line.c_str()));
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &enable = parser.args[0];
            if (enable.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            WiFi.setAutoReconnect(atoi(enable.c_str()));
               return chAT::CommandStatus::OK;
            }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_DISCONNECT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            WiFi.disconnect();
               return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &wifi_off = parser.args[0];
            if (wifi_off.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            WiFi.disconnect(atoi(wifi_off.c_str()));
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      } 
   };
   
   /* ....................................................................... */
   command_table[_MACSTA] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            String mac = WiFi.macAddress();
            srv.write_str((const char *)(mac.c_str()));

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };


   /* ....................................................................... */
   command_table[_AUTOCONNECT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }
            auto &auto_conn = parser.args[0];
            if (auto_conn.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            bool autoconn = atoi(auto_conn.c_str());
            WiFi.setAutoConnect(autoconn);
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            String ac_state = String(WiFi.getAutoConnect()) + "\r\n";
            srv.write_str((const char *)(ac_state.c_str()));
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_IPSTA] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            String res = "ip:" + WiFi.localIP().toString() + "\r\n";
            srv.write_str((const char *)(res.c_str()));
            srv.write_response_prompt();
            res = "gateway:" + WiFi.gatewayIP().toString() + "\r\n";;
            srv.write_str((const char *)(res.c_str()));
            srv.write_response_prompt();
            res = "netmask:" + WiFi.subnetMask().toString() + "\r\n";;
            srv.write_str((const char *)(res.c_str()));
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
   
   /* ....................................................................... */
   command_table[_HOSTNAME] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            srv.write_str(WiFi.getHostname());
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &host_name = parser.args[0];
            if (host_name.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            WiFi.setHostname(host_name.c_str());
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_IPV6] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            String ip_v6 = WiFi.localIPv6().toString() + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(ip_v6.c_str()));
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Run: {
            WiFi.enableIpV6();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

}

#endif
