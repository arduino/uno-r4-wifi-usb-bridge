#include "at_handler.h"
#include "commands.h"
#include "cmds_esp_generic.h"

using namespace SudoMaker;

static WiFiServer   serverWiFi(80);

void CAtHandler::run() {
   at_srv.run();
}


void CAtHandler::onWiFiEvent(WiFiEvent_t event) {
   switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      //SERIAL_DEBUG.println("Station Mode Started");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //SERIAL_DEBUG.println("Connected to :" + String(WiFi.SSID()));
      //SERIAL_DEBUG.print("Got IP: ");
      //SERIAL_DEBUG.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //SERIAL_DEBUG.println("Disconnected from station, attempting reconnection");
      //WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      //SERIAL_DEBUG.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      //wpsStop();
      delay(10);
      //SERIAL_DEBUG.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      //SERIAL_DEBUG.println("WPS Failed, retrying");
      //wpsStop();
      //wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      //SERIAL_DEBUG.println("WPS Timedout, retrying");
      //wpsStop();
      //wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      //Serial.println("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
      break;
    default:
      break;
  }

}

/* -------------------------------------------------------------------------- */
CAtHandler::CAtHandler(HardwareSerial *s)  {
/* -------------------------------------------------------------------------- */   
   
   /* Set up wifi event */
   WiFi.onEvent(onWiFiEvent);
   
   /* set up serial */
   serial = s;

   /* set up chatAt server callbacks */
   at_srv.set_io_callback({
      .callback_io_read = [this](auto buf, auto len) {
         if (!serial->available()) {
            yield();
            return (unsigned int)0;
         }
         return serial->read(buf, len);
      },
      .callback_io_write = [this](auto buf, auto len) {
         return serial->write(buf, len);
      },
   });

   at_srv.set_command_callback([this](chAT::Server & srv, const std::string & command) {
      auto it = command_table.find(command);

      if (it == command_table.end()) {
         return chAT::CommandStatus::ERROR;
      } else {
         return it->second(srv, srv.parser());
      }
   });

   /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */ 
   /*                          SET UP COMMAND TABLE                           */
   /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
   command_table = {
         {  "", [](auto & srv, auto & parser) {
            return chAT::CommandStatus::OK;
         }
      },
      
      
     
     // Object WiFi
     { "+WIFIMODE",  [&](auto & srv, auto & parser) {
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
       }
     },
     { "+WIFIBEGIN", [&](auto & srv, auto & parser) {
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
               WiFi.begin(ssid.c_str(), password.c_str());
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFIRECONNCFG",  [&](auto & srv, auto & parser) {
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
       }
     },
     #ifdef PIPPO
     { _WIFISCAN,  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Run: {
               int n = WiFi.scanNetworks();
               if (n == 0) {
               } else {
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

               WiFi.scanDelete();
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     #endif

     { "+WIFICWQAP",  [&](auto & srv, auto & parser) {
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
       }
     },
     { "+WIFICWSAP",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() <= 0 || parser.args.size() > 5) {
                 return chAT::CommandStatus::ERROR;
               }
               const char * ssid = NULL;
               const char * passphrase = NULL;
               int ch  = 1;
               bool ssid_hidden = false;
               int max_connection = 5;
               switch (parser.args.size()) {
                 case 5: {
                     auto &_max_connection = parser.args[4];
                     if (_max_connection.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     max_connection = atoi(_max_connection.c_str());
                   }
                 case 4: {
                     auto &_ssid_hidden = parser.args[3];
                     if (_ssid_hidden.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     ssid_hidden = atoi(_ssid_hidden.c_str());
                   }
                 case 3: {
                     auto &_ch = parser.args[2];
                     if (_ch.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     ch = atoi(_ch.c_str());
                   }
                 case 2: {
                     auto &_passphrase = parser.args[1];
                     if (_passphrase.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     passphrase = _passphrase.c_str();
                   }
                 case 1: {
                     auto &_ssid = parser.args[0];
                     if (_ssid.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     ssid = _ssid.c_str();
                     break;
                   }
                 default: {
                     return chAT::CommandStatus::ERROR;
                   }
               }

               WiFi.softAP(ssid, passphrase, ch, ssid_hidden, max_connection);
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICWQIF",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Run: {
               WiFi.softAPdisconnect();
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
               WiFi.softAPdisconnect(atoi(wifi_off.c_str()));
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICWAUTOCONN",  [&](auto & srv, auto & parser) {
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
       }
     },
     { "+WIFICIPSTAMAC",  [&](auto & srv, auto & parser) {
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
       }
     },
     { "+WIFICIPAPMAC",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Read: {
               srv.write_response_prompt();
               String ap_mac = WiFi.softAPmacAddress();
               srv.write_str((const char *)(ap_mac.c_str()));

               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICIPAP",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Run: {
               srv.write_response_prompt();
               String ap_ip = WiFi.softAPIP().toString() + "\r\n";
               srv.write_str((const char *)(ap_ip.c_str()));
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISTA", [&](auto & srv, auto & parser) {
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
       }
     },
     { "+WIFIHOSTNAME", [&](auto & srv, auto & parser) {
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
       }
     },// new from here
     { "+WIFICIPV6", [&](auto & srv, auto & parser) {
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
       }
     },  // Object Client
     { "+WIFICLIENTSTART", [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Run: {
               if (clients_num < MAX_CLIENT_AVAILABLE) {
                 for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                   if (clients[i] == nullptr) {
                     clients[i] = new WiFiClient();
                     clients_num++;
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                   }
                 }

               }
               return chAT::CommandStatus::ERROR;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICIPSTATE",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (clients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               if (clients[sock]->connected()) {
                 String client_status = clients[sock]->remoteIP().toString() + "," + String(clients[sock]->remotePort()) + "," + String(clients[sock]->localPort()) + "\r\n";
                 srv.write_response_prompt();
                 srv.write_str((const char *)(client_status.c_str()));
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },

     { "+WIFISTART",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 3) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (clients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &endpoint_ip = parser.args[1];
               if (endpoint_ip.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &endpoint_port = parser.args[2];
               if (endpoint_port.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               if (!clients[sock]->connect(endpoint_ip.c_str(), atoi(endpoint_port.c_str()))) {
                 return chAT::CommandStatus::ERROR;
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISEND",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 3) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (clients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &size_p = parser.args[1];
               if (size_p.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int data_size = atoi(size_p.c_str());

               std::vector<uint8_t> data_p;

               data_p = srv.inhibit_read(data_size);
               auto data_p_pos = data_p.size();
               data_p.resize(data_size);
               srv.write_str(String(data_p_pos).c_str());
               
               uint8_t buf[data_size];
               size_t buffered_len = 0;
               do {
                 buffered_len += serial->read(buf + buffered_len, data_size - buffered_len);
                 //uart_get_buffered_data_len(UART_NUM_0, &buffered_len);
               } while (buffered_len < data_size);

               
               //uart_read_bytes(UART_NUM_0, buf, data_size, portMAX_DELAY);
               srv.write_cstr((char*)buf, data_size);

               auto ok = clients[sock]->write(buf, data_size);
               srv.continue_read();
               if (!ok) {
                 return chAT::CommandStatus::ERROR;
               }

               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICLOSE",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(sock_num.c_str());
               if (clients[sock] == nullptr) {
                 srv.write_response_prompt();
                 srv.write_str("socket already closed");
                 return chAT::CommandStatus::OK;
               }
               clients[sock]->stop();
               clients_num--;

               delete clients[sock];
               clients[sock] = nullptr;
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICIFSR",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Read: {
               for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                 if (clients[i] != nullptr) {
                   if (clients[i]->connected()) {
                     String client_status = clients[i]->localIP().toString() + "\r\n";
                     srv.write_response_prompt();
                     srv.write_str((const char *)(client_status.c_str()));
                   }
                 }
               }
               return chAT::CommandStatus::OK;
             }
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(sock_num.c_str());

               if (clients[sock] != nullptr && clients[sock]->connected()) {
                 String client_status = clients[sock]->localIP().toString() + "\r\n";
                 srv.write_response_prompt();
                 srv.write_str((const char *)(client_status.c_str()));
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     // object: Server
     { "+WIFICIPSERVER",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               switch (parser.args.size()) {
                 case 2: {
                     auto &srv_port = parser.args[0];
                     if (srv_port.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }

                     auto &reuse_enable = parser.args[1];
                     if (reuse_enable.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     reuse_enable = reuse_enable.c_str();
                     int port = atoi(srv_port.c_str());
                     //WiFiServer serverTMP(port);

                     //serverWiFi = &serverTMP;
                     serverWiFi.begin(port, atoi(reuse_enable.c_str()));

                     return chAT::CommandStatus::OK;
                   }
                 case 1: {
                     auto &srv_port = parser.args[0];
                     if (srv_port.empty()) {
                       return chAT::CommandStatus::ERROR;
                     }
                     int port = atoi(srv_port.c_str());
                     //                  WiFiServer serverTMP(port);
                     //                  serverWiFi = &serverTMP;
                     serverWiFi.begin(port);
                     return chAT::CommandStatus::OK;
                   }
                 default: {
                     return chAT::CommandStatus::ERROR;
                   }
               }
             }
           case chAT::CommandMode::Run: {
               //WiFiServer serverTMP(80);

               //serverWiFi = &serverTMP;
               serverWiFi.begin();
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFICWLIF",  [&](auto & srv, auto & parser) { // use this to manage the clients connected to the server
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {// write to do the read of a specific list
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &socket_num = parser.args[0];
               if (socket_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(socket_num.c_str());
               if (clients[sock] != nullptr)
               {
                 return chAT::CommandStatus::OK;
               }

               return chAT::CommandStatus::ERROR;
             }
           case chAT::CommandMode::Run: { // run to do the pop of the client list
               Serial.println(WiFi.localIP());
               WiFiClient client = serverWiFi.available();
               if (client && (clients_num < MAX_CLIENT_AVAILABLE))
               {
                 Serial.println("Server?");
                 Serial.println("yes");
                 for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                   if (clients[i] == nullptr) {
                     clients[i] = &client;
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     clients_num++;
                     return chAT::CommandStatus::OK;
                   }
                 }
               }
               Serial.println("no");
               return chAT::CommandStatus::ERROR;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     // object: client
     { "+WIFIRECV",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Read: {
               String res = "";
               while (clients[0]->available()) {
                 res += clients[0]->readStringUntil('\r');
               }
               srv.write_response_prompt();
               srv.write_str((const char *)(res.c_str()));
               srv.write_line_end();

               return chAT::CommandStatus::OK;
             }
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &socket_num = parser.args[0];
               if (socket_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(socket_num.c_str());

               String res = "";
               while (clients[sock]->available()) {
                 res += clients[sock]->readStringUntil('\r');
               }
               srv.write_response_prompt();
               srv.write_str((const char *)(res.c_str()));
               srv.write_line_end();

               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },  // Object sslClients
     { "+WIFISETINSERCURE", [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (sslClients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }
               sslClients[sock]->setInsecure();
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLCLIENTSTART", [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Run: {
               if (clients_num < MAX_CLIENT_AVAILABLE) {
                 for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                   if (sslClients[i] == nullptr) {
                     sslClients[i] = new WiFiClientSecure();
                     clients_num++;
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                   }
                 }

               }
               return chAT::CommandStatus::ERROR;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLCIPSTATE",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (sslClients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               if (sslClients[sock]->connected()) {
                 String client_status = sslClients[sock]->remoteIP().toString() + "," + String(sslClients[sock]->remotePort()) + "," + String(sslClients[sock]->localPort()) + "\r\n";
                 srv.write_response_prompt();
                 srv.write_str((const char *)(client_status.c_str()));
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLSTART",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 3) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (sslClients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &endpoint_ip = parser.args[1];
               if (endpoint_ip.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &endpoint_port = parser.args[2];
               if (endpoint_port.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               if (!sslClients[sock]->connect(endpoint_ip.c_str(), atoi(endpoint_port.c_str()))) {
                 return chAT::CommandStatus::ERROR;
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLSEND",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 3) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               int sock = atoi(sock_num.c_str());
               if (sslClients[sock] == nullptr) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &size_p = parser.args[1];
               if (size_p.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               auto &data_p = parser.args[2];
               if (data_p.empty()) {
                 return chAT::CommandStatus::ERROR;
               }

               //            if (!(sslClients[sock]->write(data_p.c_str(), atoi(size_p.c_str())))) {
               //              return chAT::CommandStatus::ERROR;
               //            }

               if (!(sslClients[sock]->print(data_p.c_str()))) {
                 return chAT::CommandStatus::ERROR;
               }
               /*
                 if (!(sslClients[sock]->println(data_p.c_str()))){//, atoi(size_p.c_str())))) {
                 return chAT::CommandStatus::ERROR;
                 }
               */

               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLCLOSE",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(sock_num.c_str());
               if (sslClients[sock] == nullptr) {
                 srv.write_response_prompt();
                 srv.write_str("socket already closed");
                 return chAT::CommandStatus::OK;
               }
               sslClients[sock]->stop();
               clients_num--;

               free(sslClients[sock]);// = nullptr;
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLCIFSR",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Read: {
               for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                 if (sslClients[i] != nullptr) {
                   if (sslClients[i]->connected()) {
                     String client_status = sslClients[i]->localIP().toString() + "\r\n";
                     srv.write_response_prompt();
                     srv.write_str((const char *)(client_status.c_str()));
                   }
                 }
               }
               return chAT::CommandStatus::OK;
             }
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &sock_num = parser.args[0];
               if (sock_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(sock_num.c_str());

               if (sslClients[sock] != nullptr && sslClients[sock]->connected()) {
                 String client_status = sslClients[sock]->localIP().toString() + "\r\n";
                 srv.write_response_prompt();
                 srv.write_str((const char *)(client_status.c_str()));
               }
               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     },
     { "+WIFISSLRECV",  [&](auto & srv, auto & parser) {
         switch (parser.cmd_mode) {
           case chAT::CommandMode::Read: {
               String res = "";
               while (sslClients[0]->available()) {
                 res += sslClients[0]->readStringUntil('\r');
               }
               srv.write_response_prompt();
               srv.write_str((const char *)(res.c_str()));
               srv.write_line_end();

               return chAT::CommandStatus::OK;
             }
           case chAT::CommandMode::Write: {
               if (parser.args.size() != 1) {
                 return chAT::CommandStatus::ERROR;
               }
               auto &socket_num = parser.args[0];
               if (socket_num.empty()) {
                 return chAT::CommandStatus::ERROR;
               }
               int sock = atoi(socket_num.c_str());

               String res = "";
               while (sslClients[sock]->available()) {
                 res += sslClients[sock]->readStringUntil('\r');
               }
               srv.write_response_prompt();
               srv.write_str((const char *)(res.c_str()));
               srv.write_line_end();

               return chAT::CommandStatus::OK;
             }
           default:
             return chAT::CommandStatus::ERROR;
         }
       }
     }//, add here your new command
   };

   add_cmds_esp_generic();
   add_cmds_wifi_station();
}