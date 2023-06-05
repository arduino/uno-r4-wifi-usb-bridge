#ifndef CMDS_WIFI_UDP_H
#define CMDS_WIFI_UDP_H

#include "at_handler.h"

/* -------------------------------------------------------------------------- */
void CAtHandler::add_cmds_wifi_udp() {
/* -------------------------------------------------------------------------- */   
   
   /* ....................................................................... */
   command_table[_UDPBEGIN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() < 1 || parser.args.size() > 2) {
               
               return chAT::CommandStatus::ERROR;
            }

            int arg_num = parser.args.size();

            auto &port_num = parser.args[0];
            if (port_num.empty()) {
               
              return chAT::CommandStatus::ERROR;
            }
            int port = atoi(port_num.c_str());
            IPAddress address;
            if(arg_num == 2) {
               auto &ip = parser.args[1];
               if (ip.empty()) {
                  
                  return chAT::CommandStatus::ERROR;
               }

               if(!address.fromString(ip.c_str())) {
                  
                  return chAT::CommandStatus::ERROR;
               }
            }

            if (udps_num < MAX_UDP_AVAILABLE) {
               int i = 0;
               for (; i < MAX_UDP_AVAILABLE; i++) {
                  if (udps[i] == nullptr) {
                     udps[i] = new WiFiUDP();
                     if(udps[i] == nullptr) {
                        
                        return chAT::CommandStatus::ERROR;
                     }

                     if(arg_num == 2) {
                        if(udps[i]->begin(address,port)) {
                           udps_num++;
                        }
                        else {
                           delete udps[i];
                           
                           chAT::CommandStatus::ERROR;
                        }
                     }
                     else {
                        if(udps[i]->begin(port)) {
                           udps_num++;
                        }
                        else {
                           delete udps[i];
                           
                           chAT::CommandStatus::ERROR;
                        }
                     }

                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
               }
            }
            
            return chAT::CommandStatus::ERROR;
         }
         case chAT::CommandMode::Run: 
            
            return chAT::CommandStatus::ERROR;
         
         default:
            
            return chAT::CommandStatus::ERROR;
      }
   };


   /* ....................................................................... */
   command_table[_UDPBEGINMULTI] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 2) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port_num = parser.args[0];
            if (port_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int port = atoi(port_num.c_str());
            IPAddress address;
            
            auto &ip = parser.args[1];
            if (ip.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            if(!address.fromString(ip.c_str())) {
               return chAT::CommandStatus::ERROR;
            }
            
            if (udps_num < MAX_UDP_AVAILABLE) {
               for (int i = 0; i < MAX_UDP_AVAILABLE; i++) {
                  if (udps[i] == nullptr) {
                     udps[i] = new WiFiUDP();
                     if(udps[i] == nullptr) {
                        return chAT::CommandStatus::ERROR;
                     }

                     if(udps[i]->beginMulticast(address,port)) {
                        udps_num++;
                     }
                     else {
                        delete udps[i];
                        chAT::CommandStatus::ERROR;
                     }
                    
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
               }
            }
            return chAT::CommandStatus::ERROR;
         }
         case chAT::CommandMode::Run: 
            
            return chAT::CommandStatus::ERROR;
         
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
   
   
   /* ....................................................................... */
   command_table[_UDPBEGINPACKET] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            
            if(udps[sock]->beginPacket()) {
               srv.write_response_prompt();
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }

            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPBEGINPACKETMULTI] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            
            if(udps[sock]->beginMulticastPacket()) {
               srv.write_response_prompt();
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }

            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPBEGINPACKETIP] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port_num = parser.args[1];
            if (port_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int port = atoi(port_num.c_str());
            
            IPAddress address;
            
            auto &ip = parser.args[2];
            if (ip.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            if(!address.fromString(ip.c_str())) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock]->beginPacket(address,port)) {
               srv.write_response_prompt();
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }
            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPBEGINPACKETNAME] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port_num = parser.args[1];
            if (port_num.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int port = atoi(port_num.c_str());
                     
            auto &ip = parser.args[2];
            if (ip.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock]->beginPacket(ip.c_str(),port)) {
               srv.write_response_prompt();
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }

            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPENDPACKET] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            
            if(udps[sock]->endPacket()) {
               srv.write_response_prompt();
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }

            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   
   /* ....................................................................... */
   command_table[_UDPWRITE] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
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

            unsigned long start_time = millis();
            int sent = 0;
            while(millis() - start_time < 5000 && sent < data_received.size()){
               sent += udps[sock]->write(data_received.data() + sent, data_received.size() - sent);
               if(sent < data_received.size())
                  delay(100);
            }

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
   command_table[_UDPPARSE] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }
            
            int res = udps[sock]->parsePacket();
            String results = String(res);
         
            srv.write_response_prompt();
            srv.write_str((const char *)results.c_str());
            srv.write_line_end();
            return chAT::CommandStatus::OK;
            

            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPAVAILABLE] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String av(udps[sock]->available());
            srv.write_str((const char *)(av.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPREAD] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
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

            int data_available = udps[sock]->available();
            data_wanted = (data_wanted < data_available) ? data_wanted : data_available;

            std::vector<uint8_t> data_received;
            data_received.resize(data_wanted);

            int res = udps[sock]->read(data_received.data(), data_wanted);
            String results = String(data_received.size()) + "|";
            
            srv.write_response_prompt();
            srv.write_str((const char *)(results.c_str()));
            srv.write_vec8(data_received);
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPPEEK] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String p(udps[sock]->peek());
            srv.write_str((const char *)(p.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPFLUSH] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            udps[sock]->flush();
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_UDPREMOTEIP] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            } 

            IPAddress ip = udps[sock]->remoteIP();
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
   command_table[_UDPREMOTEPORT] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }

            if(udps[sock] == nullptr) {
               return chAT::CommandStatus::ERROR;
            } 

            String port(udps[sock]->remotePort());
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
   command_table[_UDPSTOP] = [this](auto & srv, auto & parser) {
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

            if(sock < 0 || sock >= MAX_UDP_AVAILABLE) {
               return chAT::CommandStatus::ERROR;
            }
   
            if (udps[sock] == nullptr) {
               
            }
            else {
               udps[sock]->stop();
               
               delete udps[sock];
               udps[sock] = nullptr;
               udps_num--;
               
            }
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   
   

   

   


   

   
   

  
}

#endif