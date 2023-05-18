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
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            tmp_clt->setInsecure();

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

   /* ....................................................................... */
   command_table[_SSLCLIENTSEND] = [this](auto & srv, auto & parser) {
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
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];

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

            auto ok = tmp_clt->write(buf, data_size);
            srv.continue_read();
            if (!ok) {
              return chAT::CommandStatus::ERROR;
            }
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCLOSE] = [this](auto & srv, auto & parser) {
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
               srv.write_response_prompt();
               srv.write_str("socket already closed");
               return chAT::CommandStatus::OK;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];
            tmp_clt->stop();
            clients_num--;

            delete clients[sock];;

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_IPSSLCLIENT] = [this](auto & srv, auto & parser) {
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
               srv.write_response_prompt();
               srv.write_str("socket  closed");
               return chAT::CommandStatus::OK;
            }
            WiFiClientSecure* tmp_clt = (WiFiClientSecure*) clients[sock];

            if (tmp_clt->connected()) {
               String client_status = tmp_clt->localIP().toString() + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(client_status.c_str()));
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTRECEIVE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
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
