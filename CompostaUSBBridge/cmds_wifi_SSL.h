#ifndef CMDS_WIFI_SSL_H
#define CMDS_WIFI_SSL_H


#include "at_handler.h"

void CAtHandler::add_cmds_wifi_SSL() {
   /* ....................................................................... */
   command_table[_SSLBEGINCLIENT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            if (clients_num < MAX_CLIENT_AVAILABLE) {
               for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                  if (clients[i] == nullptr) {
                     clients[i] = new WiFiClientSecure();
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
   };
   

   /* ....................................................................... */
   command_table[_SSLSETINSERCURE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               srv.write_response_prompt();
               srv.write_str("sock num empty");
               srv.write_line_end();
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (clients[sock] == nullptr) {
               srv.write_response_prompt();
               srv.write_str("null pointer");
               srv.write_line_end();
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            tmp_clt->setInsecure();
            srv.write_response_prompt();
            srv.write_str("setted insecure");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SETCAROOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 2) {
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

            auto &ca_root = parser.args[1];
            if (ca_root.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            tmp_clt->setCACert((const char *)ca_root.c_str());

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
   /* ....................................................................... */
   command_table[_SSLCLIENTSTATE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
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
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            if (tmp_clt->connected()) {
               String client_status = tmp_clt->remoteIP().toString() + "," + String(tmp_clt->remotePort()) + "," + String(tmp_clt->localPort()) + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(client_status.c_str()));
               srv.write_line_end();
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
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

            auto &host = parser.args[1];
            if (host.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port = parser.args[2];
            if (port.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            if (!tmp_clt->connect(host.c_str(), atoi(port.c_str()))) {
               return chAT::CommandStatus::ERROR;
            }

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   #if 0
   /* ....................................................................... */
   command_table[ ] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      
   };

   /* ....................................................................... */
   command_table[ ] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      
   };
   #endif
}

#endif
