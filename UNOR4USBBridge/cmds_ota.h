#ifndef CMDS_OTA_H
#define CMDS_OTA_H

#include "at_handler.h"
#include "OTA.h"
#include "BossaUnoR4WiFi.h"

Arduino_UNOWIFIR4_OTA OTA;

void CAtHandler::add_cmds_ota() {
   /* ....................................................................... */
   command_table[_OTA_SETCAROOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            int ca_root_size = 0;
            auto &ca_root_size_str = parser.args[0];
            if (ca_root_size_str.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            ca_root_size = atoi(ca_root_size_str.c_str());
            if(!ca_root_size) {
               return chAT::CommandStatus::ERROR;
            }
               
            ota_cert_buf = srv.inhibit_read(ca_root_size);
            size_t offset = ota_cert_buf.size();
            if(offset < ca_root_size) {
               ota_cert_buf.resize(ca_root_size);
               do {
                  offset += serial->read(ota_cert_buf.data() + offset, ca_root_size - offset);
               } while (offset < ca_root_size);
            }
            OTA.setCACert((const char *)ota_cert_buf.data());
            srv.continue_read();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;            
      }
   };

   /* ....................................................................... */
   command_table[_OTA_BEGIN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            Arduino_ESP32_OTA::Error ota_error = OTA.begin();
            String error = String((int)ota_error) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }
               
            auto &path = parser.args[0];
            if (path.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            Arduino_ESP32_OTA::Error ota_error = OTA.begin(path.c_str());
            String error = String((int)ota_error) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;

         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_OTA_DOWNLOAD] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() == 1) {
               auto &url = parser.args[0];
               if (url.empty()) {
                  return chAT::CommandStatus::ERROR;
               }
  
               int ota_error = OTA.download(url.c_str());
               String error = String((int)ota_error) + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(error.c_str()));
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            } else if(parser.args.size() == 2) {
               auto &url = parser.args[0];
               if (url.empty()) {
                  return chAT::CommandStatus::ERROR;
               }

               auto &path = parser.args[1];
               if (path.empty()) {
                  return chAT::CommandStatus::ERROR;
               }

               int ota_error = OTA.download(url.c_str(), path.c_str());
               String error = String((int)ota_error) + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(error.c_str()));
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            } else {
               return chAT::CommandStatus::ERROR;
            }

         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_OTA_VERIFY] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            Arduino_ESP32_OTA::Error ota_error = OTA.verify();
            String error = String((int)ota_error) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_OTA_UPDATE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            Arduino_ESP32_OTA::Error ota_error = OTA.update();
            String error = String((int)ota_error) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }
               
            auto &path = parser.args[0];
            if (path.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int flash_error = BOSSA.program(path.c_str(), Serial, GPIO_BOOT, GPIO_RST);
            String error = String(flash_error) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_OTA_RESET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            OTA.reset();
            srv.write_response_prompt();
            srv.write_str("0");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
