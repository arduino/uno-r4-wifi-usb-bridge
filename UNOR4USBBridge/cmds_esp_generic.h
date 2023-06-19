#ifndef CMDS_ESP_GENERIC_H
#define CMDS_ESP_GENERIC_H

#include "at_handler.h"

void CAtHandler::add_cmds_esp_generic() {
   
   /* ....................................................................... */
   command_table[_RESET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      ESP.restart();
      return chAT::CommandStatus::OK;
   };

   /* ....................................................................... */
   command_table[_GMR] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      //srv.write_response_prompt();
      srv.write_cstr("<len>"); // TODO: report some useful information
      srv.write_line_end();
      return chAT::CommandStatus::OK;
   };

   /* ....................................................................... */
   command_table[_GENERIC_CMD] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            srv.write_cstr("<test>");     // report some CMDs
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
             return chAT::CommandStatus::ERROR;
      }
   };

    /* ....................................................................... */
   command_table[_FWVERSION] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();
            srv.write_cstr(ESP_FW_VERSION);    
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
             return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_FILESYSTEM] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() < 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &type_str = parser.args[0];
            if (type_str.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int _type = atoi(type_str.c_str());

            auto &operation_str = parser.args[1];
            if (operation_str.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            uint8_t operation = atoi(operation_str.c_str());

            auto &filename = parser.args[2];
            if (filename.empty()) {
              return chAT::CommandStatus::ERROR;
            }

            FILE* f;
            if (_type == 0) {
               switch (operation) {
                  case WIFI_FILE_DELETE: {
                     if(SPIFFS.remove(filename.c_str())){
                        return chAT::CommandStatus::OK;
                     }
                     return chAT::CommandStatus::ERROR;
                  }
                  case WIFI_FILE_WRITE: {
                     uint8_t size = 0;
                     if (parser.args.size() >= 3) {
                        auto &size_str = parser.args[3];
                        if (size_str.empty()) {
                           return chAT::CommandStatus::ERROR;
                        }
                        size = atoi(size_str.c_str());
                     }

                     std::vector<uint8_t> data_received;
                     data_received = srv.inhibit_read(size);
                     size_t offset = data_received.size();
                     
                     if(offset < size) {

                        data_received.resize(size);
                        do {
                           offset += serial->read(data_received.data() + offset, size - offset);
                        } while (offset < size);
                     }

                     File file = SPIFFS.open(filename.c_str(), FILE_WRITE);
                     if(!file){
                        return chAT::CommandStatus::ERROR;
                     }
                     int res = file.write(data_received.data(), data_received.size());
                     if(res == 0){
                       return chAT::CommandStatus::ERROR;
                     }

                     file.close();
                     srv.continue_read();
                     return chAT::CommandStatus::OK;
                  }
                  case WIFI_FILE_READ: {
                     int offset = 0;
                     if (parser.args.size() >= 3) {
                        auto &offset_str = parser.args[3];
                        if (offset_str.empty()) {
                           return chAT::CommandStatus::ERROR;
                        }
                        offset = atoi(offset_str.c_str());
                     }

                     int length = 0;
                     if (parser.args.size() >= 4) {
                        auto &length_str = parser.args[4];
                        if (length_str.empty()) {
                           return chAT::CommandStatus::ERROR;
                        }
                        length = atoi(length_str.c_str());
                     }

                     File file = SPIFFS.open(filename.c_str());
                     if(!file){
                        return chAT::CommandStatus::ERROR;
                     }
                     bool end_of_file = file.seek(offset, SeekCur);
                     if(!end_of_file){
                        file.close();
                        return chAT::CommandStatus::ERROR;
                     }

                     std::vector<uint8_t> buf;
                     length = (length < file.available()) ? length : file.available();
                     buf.resize(length);

                     file.read(buf.data(), length);
                     file.close();

                     String results = String(buf.size()) + "|";

                     srv.write_response_prompt();
                     srv.write_str((const char *)(results.c_str()));
                     srv.write_vec8(buf);
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
                  case WIFI_FILE_APPEND: {
                      uint8_t size = 0;
                     if (parser.args.size() >= 3) {
                        auto &size_str = parser.args[3];
                        if (size_str.empty()) {
                           return chAT::CommandStatus::ERROR;
                        }
                        size = atoi(size_str.c_str());
                     } else {
                        return chAT::CommandStatus::ERROR;
                     }

                     std::vector<uint8_t> data_received;
                     data_received = srv.inhibit_read(size);
                     size_t offset = data_received.size();
                     
                     if(offset < size) {

                        data_received.resize(size);
                        do {
                           offset += serial->read(data_received.data() + offset, size - offset);
                        } while (offset < size);
                     }

                     File file = SPIFFS.open(filename.c_str(), FILE_APPEND);
                     if(!file){
                        return chAT::CommandStatus::ERROR;
                     }

                     int res = file.write(data_received.data(), data_received.size());
                     if(res == 0){
                       return chAT::CommandStatus::ERROR;
                     }

                     file.close();
                     srv.continue_read();
                     
                     return chAT::CommandStatus::OK;
                  }
                  default:
                     return chAT::CommandStatus::ERROR;
               }
            }
            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_MOUNTFS] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
        case chAT::CommandMode::Write: {
          if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &format_on_fault = parser.args[0];
            if (format_on_fault.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int _fof = atoi(format_on_fault.c_str());
            if(!SPIFFS.begin(_fof)){
               return chAT::CommandStatus::ERROR;
            }
            srv.write_response_prompt();
            srv.write_cstr("FS Mounted\r\n");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_EXIT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      return chAT::CommandStatus::OK;
   };


   /* ....................................................................... */
   command_table[_SOFTRESETWIFI] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                  serverClients[i].client.stop();
            }
            
            for (int i = 0; i < MAX_SERVER_AVAILABLE; i++) {
               if (serverWiFi[i] != nullptr) {
                  serverWiFi[i]->end();
                  delete serverWiFi[i];
                  serverWiFi[i] = nullptr;
               }
            }

            servers_num = 0;

            for (int i = 0; i < MAX_UDP_AVAILABLE; i++) {
               if (udps[i] != nullptr) {
                  udps[i]->stop();
                  delete udps[i];
                  udps[i] = nullptr;
               }
            }

            udps_num = 0;

            for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
               if (clients[i] != nullptr) {
                  clients[i]->stop();
                  delete clients[i];
                  clients[i] = nullptr;
               }
            }
            
            clients_num = 0;

            for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
               if (sslclients[i] != nullptr) {
                  sslclients[i]->stop();
                  delete sslclients[i];
                  sslclients[i] = nullptr;
               }
            }
            sslclients_num = 0;

            #define IP0 IPAddress(0,0,0,0)

            WiFi.config(IP0,IP0,IP0);

            WiFi.disconnect();
            WiFi.softAPdisconnect();
                  
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