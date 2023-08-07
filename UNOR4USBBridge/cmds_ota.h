#ifndef CMDS_OTA_H
#define CMDS_OTA_H

#include "at_handler.h"
#include "OTA.h"

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
               
            cert_buf = srv.inhibit_read(ca_root_size);
            size_t offset = cert_buf.size();
               
            if(offset < ca_root_size) {
               cert_buf.resize(ca_root_size);
               do {
                  offset += serial->read(cert_buf.data() + offset, ca_root_size - offset);
               } while (offset < ca_root_size);
            }
            OtaHandler.setCACert((const char *)cert_buf.data());
            srv.continue_read();
            return chAT::CommandStatus::OK;
         }            
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_OTA_RUN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }
            auto &url = parser.args[0];
            if(OtaHandler.start(String(url.c_str()))) {
               return chAT::CommandStatus::OK;
            }
            return chAT::CommandStatus::ERROR;
         }
         case chAT::CommandMode::Read: {
            String running = String(OtaHandler.isRunning()) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(running.c_str()));
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_OTA_ERROR] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
            String error = String((int)OtaHandler.getError()) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
