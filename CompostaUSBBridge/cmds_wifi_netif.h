#ifndef CMDS_WIFI_NETIF_H
#define CMDS_WIFI_NETIF_H

#include "at_handler.h"

/* -------------------------------------------------------------------------- */
void CAtHandler::add_cmds_wifi_netif() {
/* -------------------------------------------------------------------------- */   
   
   /* ....................................................................... */
   command_table[_BEGINCLIENT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
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
   };
   
   
   /* ....................................................................... */
   command_table[_CLIENTSTATE] = [this](auto & srv, auto & parser) {
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
   };

   /* ....................................................................... */
   command_table[_CLIENTCONNECT] = [this](auto & srv, auto & parser) {
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
   };
   
   /* ....................................................................... */
   command_table[_CLIENTSEND] = [this](auto & srv, auto & parser) {
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
   };

   /* ....................................................................... */
   command_table[_CLIENTCLOSE] = [this](auto & srv, auto & parser) {
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
            clients[sock]->stop();
            clients_num--;

            delete clients[sock];
            clients[sock] = nullptr;
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_IPCLIENT] = [this](auto & srv, auto & parser) {
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
   };

   /* ....................................................................... */
   command_table[_BEGINSERVER] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
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
   };

   /* ....................................................................... */
   command_table[_CLIENTCONNECTED] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
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
   };

   /* ....................................................................... */
   command_table[_CLIENTRECEIVE] = [this](auto & srv, auto & parser) {
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
   };
}

#endif