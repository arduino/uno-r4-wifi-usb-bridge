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
   command_table[_FILESYSTEM] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */     
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            // <type>,<operation>,<filename>,<offset>,<length>
            if (parser.args.size() < 3) {
              return chAT::CommandStatus::ERROR;
            }

            auto &type_str = parser.args[0];
            if (type_str.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            size_t _type = std::stoi(type_str);

            auto &operation_str = parser.args[1];
            if (operation_str.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            size_t operation = std::stoi(operation_str);

            auto &filename = parser.args[2];
            if (filename.empty()) {
              return chAT::CommandStatus::ERROR;
            }

            size_t offset = 0;
            if (parser.args.size() >= 3) {
              auto &offset_str = parser.args[3];
              if (offset_str.empty()) {
                return chAT::CommandStatus::ERROR;
              }
              offset = std::stoi(offset_str);
            }

            size_t length = 0;
            if (parser.args.size() >= 4) {
              auto &length_str = parser.args[4];
              if (length_str.empty()) {
                return chAT::CommandStatus::ERROR;
              }
              length = std::stoi(length_str);
            }

            FILE* f;
            if (_type == 0) {
               switch (operation) {
                  case 0: //delete
                     remove(filename.c_str());
                     break;
                  case 1: //write
                     f = fopen(filename.c_str(), "w");
                     break;
                  case 2: //read
                     f = fopen(filename.c_str(), "r");
                     fseek(f, offset, 1);
                     uint8_t buf[length];
                     fread(buf, length, 1, f);
                     srv.write_cstr((const char*)buf, length);
                     break;
               }
            }

            srv.write_response_prompt();
            srv.write_cstr("<test>");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
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
            // <type>,<operation>,<filename>,<offset>,<length>
            srv.write_response_prompt();
            srv.write_cstr("<test>");
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
 
}

#endif