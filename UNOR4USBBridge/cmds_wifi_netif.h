#ifndef CMDS_WIFI_NETIF_H
#define CMDS_WIFI_NETIF_H

#include "at_handler.h"

#define INCREMENT_MOD(x,MOD)  x = (++x) % MOD

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

            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            if (the_client.client->connected()) {
               String client_status = the_client.client->remoteIP().toString() + "," + String(the_client.client->remotePort()) + "," + String(the_client.client->localPort()) + "\r\n";
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
            
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
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

            if (!the_client.client->connect(address, atoi(hostport.c_str()))) {
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

            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
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

            if (!the_client.client->connect(hostname.c_str(), atoi(hostport.c_str()))) {
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
               return chAT::CommandStatus::ERROR;
            }

            /* socket */
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());

            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            /* data len */
            auto &size_p = parser.args[1];
            if (size_p.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int data_size = atoi(size_p.c_str());

            if(data_size <= 0) {
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

            srv.continue_read();

            int sent = 0;
            sent += the_client.client->write(data_received.data() + sent, data_received.size() - sent);
            
            if (sent < data_received.size()) {
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

            CClientWrapper the_client = getClient(sock);
   
            if (the_client.client == nullptr) {
            }
            else {
               the_client.client->stop();
               

               if(the_client.can_delete >= 0) {
                  delete clients[the_client.can_delete];
                  clients[the_client.can_delete] = nullptr;
                  clients_num--;
               }
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

            CClientWrapper the_client = getClient(sock);

            if (the_client.client != nullptr && the_client.client->connected()) {
              String client_status = the_client.client->localIP().toString() + "\r\n";
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
   command_table[_SERVERWRITE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            /* the command receive 2 data: the socket and the length of data 
               to be transmitted */

            if (parser.args.size() != 2) {
               return chAT::CommandStatus::ERROR;
            }

            /* socket */
            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());

            if(sock < 0 || sock >= MAX_SERVER_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(serverWiFi[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            /* data len */
            auto &size_p = parser.args[1];
            if (size_p.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int data_size = atoi(size_p.c_str());

            if(data_size <= 0) {
               return chAT::CommandStatus::ERROR;
            }

            /* socket and data received 
               answer back _CLIENTDATA: DATA\r\n 
               so that data transmission can begin */
            
            
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

            srv.continue_read();

            for(int i = 0;i<MAX_CLIENT_AVAILABLE;i++) {
               if(serverClients[i].server == sock) {
                  if(serverClients[i].client) {
                     serverClients[i].client.write(data_received.data(), data_received.size());
                  }
               }
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
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &srv_port = parser.args[0];
            if (srv_port.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            int port = atoi(srv_port.c_str());

            if (servers_num < MAX_SERVER_AVAILABLE) {
               for (int i = 0; i < MAX_SERVER_AVAILABLE; i++) {
                  if (serverWiFi[i] == nullptr) {
                     serverWiFi[i] = new WiFiServer();
                     if(serverWiFi[i] == nullptr) {
                        return chAT::CommandStatus::ERROR;
                     }
                     servers_num++;
                     serverWiFi[i]->begin(port);
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
               }
            }
            return chAT::CommandStatus::ERROR;
         }
         case chAT::CommandMode::Run: {
            return chAT::CommandStatus::ERROR;
         }
         default:
           return chAT::CommandStatus::ERROR;
      }
   };


   /* ....................................................................... */
   command_table[_SERVERAVAILABLE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &socket = parser.args[0];
            if (socket.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket.c_str());

            if(sock < 0 || sock >= MAX_SERVER_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(serverWiFi[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            /* accept */  
            for(int i = 0;i<MAX_CLIENT_AVAILABLE;i++) {
               if(!serverClients[i].client) {
                  serverClients[i].client = serverWiFi[sock]->available();
                  serverClients[i].server = sock;
                  break;
               }
            }

            int client_sock = -1;
            
            int end = last_server_client_sock;
               
            do {
               if(serverClients[last_server_client_sock].client) {
                  client_sock = last_server_client_sock;
                  break;         
               }
               INCREMENT_MOD(last_server_client_sock,MAX_CLIENT_AVAILABLE);
            } while(end != last_server_client_sock);
            
            int sock_to_send = -1;
            if(client_sock == -1) {
               last_server_client_sock = 0;
            }
            else {
               last_server_client_sock = client_sock;
               sock_to_send = START_CLIENT_SERVER_SOCK + client_sock;
            }
           


            srv.write_response_prompt();
            srv.write_str((const char *) String(sock_to_send).c_str());
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Run: {
            return chAT::CommandStatus::ERROR;
         }
         default:
           return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SERVEREND] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &socket = parser.args[0];
            if (socket.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            int sock = atoi(socket.c_str());

            if(sock < 0 || sock >= MAX_SERVER_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(serverWiFi[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            serverWiFi[sock]->end();
            delete serverWiFi[sock];
            serverWiFi[sock] = nullptr;

            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Run: {
            return chAT::CommandStatus::ERROR;
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
            
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String con(the_client.client->connected());
            srv.write_str((const char *)(con.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
            
         }
         case chAT::CommandMode::Run: { // run to do the pop of the client list
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &size = parser.args[1];
            if (size.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int data_wanted = atoi(size.c_str());
            if(data_wanted <= 0) {
               return chAT::CommandStatus::ERROR;
            } 

            int data_available = the_client.client->available();
            data_wanted = (data_wanted < data_available) ? data_wanted : data_available;

            std::vector<uint8_t> data_received;
            data_received.resize(data_wanted);

            int res = the_client.client->read(data_received.data(), data_wanted);
            String results = String(data_received.size()) + "|";
            
            srv.write_response_prompt();
            srv.write_str((const char *)(results.c_str()));
            srv.write_vec8(data_received);
            

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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            String av(the_client.client->available());
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            the_client.client->flush();
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            IPAddress ip = the_client.client->remoteIP();
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            String port(the_client.client->remotePort());
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
            CClientWrapper the_client = getClient(sock);

            if (the_client.client == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            String p(the_client.client->peek());
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