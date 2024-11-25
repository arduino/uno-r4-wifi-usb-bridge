#ifndef CMDS_ESP_GENERIC_H
#define CMDS_ESP_GENERIC_H

#include "at_handler.h"
#include "ping.h"

extern "C" {
   #include "esp32-hal-tinyusb.h"
}

static const uint8_t version[4] = {
   (FIRMWARE_MAJOR & 0xff),
   (FIRMWARE_MINOR & 0xff),
   (FIRMWARE_PATCH & 0xff),
   0
};

void CAtHandler::add_cmds_esp_generic() {

   /* ....................................................................... */
   command_table[_RESET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
               ESP.restart();
               return chAT::CommandStatus::OK;
            }
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_RESTART_BOOTLOADER] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
               usb_persist_restart(RESTART_BOOTLOADER);
               return chAT::CommandStatus::OK;
            }
      }
      return chAT::CommandStatus::ERROR;
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
   command_table[_FWVERSION_U32] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            srv.write_response_prompt();



            log_e("version 0x%X, 0x%X", *((uint32_t*)version), &version);

            srv.write_data(version, sizeof(version));
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

   /* ....................................................................... */
   command_table[_GETTIME] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */

      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            char epoch[12];                                // gettime
            constexpr uint32_t SECS_YR_2000 = 946684800UL; // the time at the start of y2k
            time_t now = time(nullptr);

            if (now < SECS_YR_2000) {
               now = 0;
            }

            srv.write_response_prompt();
            sprintf(epoch,"%d", (unsigned long) now);
            srv.write_str((const char *) epoch);
            srv.write_line_end();

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

/* ....................................................................... */
   command_table[_PING] = [this](auto & srv, auto & parser) {
/* ....................................................................... */
      switch (parser.cmd_mode) {
      case chAT::CommandMode::Write: {
         if (parser.args.size() != 4) {
            return chAT::CommandStatus::ERROR;
         }

         // get IP
         auto &target = parser.args[1];
         if (target.empty()) {
            return chAT::CommandStatus::ERROR;
         }

         // get ttl
         auto &ttl = parser.args[2];
         if (ttl.empty()) {
            return chAT::CommandStatus::ERROR;
         }

         // get count
         auto &cnt = parser.args[3];
         if (cnt.empty()) {
            return chAT::CommandStatus::ERROR;
         }

         auto ping_res = execute_ping(target.c_str(), atoi(ttl.c_str()), atoi(cnt.c_str()));
         char rsl[8];
         if (ping_res.status == ping_status::SUCCESS) {
           sprintf(rsl,"%.0f", ping_res.averagertt);
         } else {
           sprintf(rsl,"%d", ping_res.status);
         }

         srv.write_response_prompt();
         srv.write_str((const char *) rsl);
         srv.write_line_end();
         return chAT::CommandStatus::OK;

      }
      default:
         return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
