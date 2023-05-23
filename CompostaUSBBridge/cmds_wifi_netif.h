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
               for (int i = 5; i < MAX_CLIENT_AVAILABLE; i++) {
                  if (clients[i] == nullptr) {
                     clients[i] = new WiFiClient();
                     if(clients[i] == nullptr) {
                        return chAT::CommandStatus::ERROR;
                     }
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
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            
            int sock = atoi(sock_num.c_str());

            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(clients[sock] != nullptr) {
               delete clients[sock];
               clients[sock] = nullptr;
            }

            clients[sock] = new WiFiClient();

            if(clients[sock] != nullptr) {
               srv.write_response_prompt();
               
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }
            else {
               return chAT::CommandStatus::ERROR;
            }
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
            
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

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
   command_table[_CLIENTCONNECTIP] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }
            
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostip = parser.args[1];
            if (hostip.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostport = parser.args[2];
            if (hostport.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            IPAddress address;
            if(!address.fromString(hostip.c_str())) {
               return chAT::CommandStatus::ERROR;
            }

            if(clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            if (!clients[sock]->connect(address, atoi(hostport.c_str()))) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_CLIENTCONNECTNAME] = [this](auto & srv, auto & parser) {
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
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostname = parser.args[1];
            if (hostname.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &hostport = parser.args[2];
            if (hostport.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            if(clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            if (!clients[sock]->connect(hostname.c_str(), atoi(hostport.c_str()))) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            srv.write_line_end();
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
            /* the command receive 2 data: the socket and the length of data 
               to be transmitted */

            if (parser.args.size() != 2) {
               srv.write_str("A");
               return chAT::CommandStatus::ERROR;
            }

            /* socket */
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               srv.write_str("B");
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               srv.write_str("C");
               return chAT::CommandStatus::ERROR;
            }
            if (clients[sock] == nullptr) {
               srv.write_str("D");
               return chAT::CommandStatus::ERROR;
            }

            /* data len */
            auto &size_p = parser.args[1];
            if (size_p.empty()) {
               srv.write_str("E");
               return chAT::CommandStatus::ERROR;
            }

            int data_size = atoi(size_p.c_str());

            if(data_size <= 0) {
               srv.write_str("F");
               return chAT::CommandStatus::ERROR;
            }

            /* socket and data received 
               answer back _CLIENTDATA: DATA\r\n 
               so that data transmission can begin */
            //srv.write_response_prompt();
            //srv.write_str(" DATA");
            //srv.write_line_end();
            //return chAT::CommandStatus::OK;
            
            /* -----------------------------------
             * BEGIN TRANSPARENT DATA TRANSMISSION
             * ----------------------------------- */
            std::vector<uint8_t> data_received;
            data_received = srv.inhibit_read(data_size);
            size_t offset = data_received.size();
            
            if(offset < data_size) {

               data_received.resize(data_size);
               do {
                  offset += serial->read(data_received.data() + offset, data_size - offset);
               } while (offset < data_size);
            }

            
            auto ok = clients[sock]->write(data_received.data(), data_received.size());

            srv.continue_read();
            if (!ok) {
              return chAT::CommandStatus::ERROR;
            }
            srv.write_str("PIPPO ");
            srv.write_data(data_received.data(), data_received.size());
            srv.write_line_end();
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

            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }
            

            if (clients[sock] == nullptr) {
               
            }
            else {
               clients[sock]->stop();
               clients_num--;

               delete clients[sock];
               clients[sock] = nullptr;
            }
            srv.write_response_prompt();
            srv.write_line_end();
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
            
            if(sock < 0 || sock >= MAX_CLIENT_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if (clients[sock] == nullptr) {
              return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String con(clients[sock]->connected());
            srv.write_str((const char *)(con.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
            
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
            if (parser.args.size() != 2) {
              return chAT::CommandStatus::ERROR;
            }
            auto &socket_num = parser.args[0];
            if (socket_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket_num.c_str());
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &size = parser.args[1];
            if (size.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int to_be_received = atoi(size.c_str());
            if(to_be_received <= 0) {
               return chAT::CommandStatus::ERROR;
            } 

            std::vector<uint8_t> data_received;
            data_received.resize(to_be_received);

            int res = clients[sock]->read(data_received.data(), to_be_received);
            String results = String(res) + " | ";
            
            srv.write_response_prompt();
            srv.write_str((const char *)(results.c_str()));
            srv.write_data(data_received.data(), data_received.size());
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_AVAILABLE] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            String av(clients[sock]->available());
            srv.write_str((const char *)(av.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_CLIENTSTATUS] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            //String st(clients[sock]->status());
            //srv.write_str((const char *)(st.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_CLIENTFLUSH] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            clients[sock]->flush();
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_REMOTEIP] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            IPAddress ip = clients[sock]->remoteIP();
            srv.write_response_prompt();
            srv.write_str((const char *)(ip.toString().c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_REMOTEPORT] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            String port(clients[sock]->remotePort());
            srv.write_response_prompt();
            srv.write_str((const char *)(port.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };


   /* ....................................................................... */
   command_table[_PEEK] = [this](auto & srv, auto & parser) {
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
            if (clients[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            String p(clients[sock]->peek());
            srv.write_str((const char *)(p.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_GETHOSTBYNAME] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }
            
            auto &hostname = parser.args[0];
            if (hostname.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            
            IPAddress address;

            if(!WiFiGenericClass::hostByName(hostname.c_str(), address)) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            srv.write_str((const char *)(address.toString().c_str()));
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif