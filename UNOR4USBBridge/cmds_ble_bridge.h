#ifndef CMDS_BLE_BRIDGE_H
#define CMDS_BLE_BRIDGE_H

#include "at_handler.h"
#include "ArduinoBLE.h"
#include "utility/HCIVirtualTransport.h"

extern HCIVirtualTransportClass HCIVirtualTransport;

void CAtHandler::add_cmds_ble_bridge() {

   /* ....................................................................... */
   command_table[_HCI_BEGIN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      srv.write_response_prompt();
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
               auto res = HCIVirtualTransport.begin();
               if (res) {
                  return chAT::CommandStatus::OK;
               }
            }
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_HCI_END] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      srv.write_response_prompt();
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
               HCIVirtualTransport.end();
               return chAT::CommandStatus::OK;
            }
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_HCI_WAIT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      srv.write_response_prompt();
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
              return chAT::CommandStatus::ERROR;
            }

            auto &timeout = parser.args[0];
            if (timeout.empty()) {
              return chAT::CommandStatus::ERROR;
            }
            int _timeout = atoi(timeout.c_str());
            HCIVirtualTransport.wait(_timeout);
            return chAT::CommandStatus::OK;
         }
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_HCI_AVAILABLE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Read: {
               srv.write_response_prompt();
               String av(HCIVirtualTransport.available());
               srv.write_str((const char *)(av.c_str()));
               srv.write_line_end();
               return chAT::CommandStatus::OK;
            }
      }
      return chAT::CommandStatus::ERROR;
   };

   /* ....................................................................... */
   command_table[_HCI_READ] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            std::vector<uint8_t> data;
            int i = 0;
            auto howmany = HCIVirtualTransport.available();
            while (i < howmany) {
               data.push_back(HCIVirtualTransport.read());
               i++;
            }
            data.resize(howmany);
            srv.write_response_prompt();
            srv.write_str(String(i).c_str());
            srv.write_str("|");
            srv.write_vec8(data);

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_HCI_WRITE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {

            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            /* data len */
            auto &size_p = parser.args[0];
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
               sent += HCIVirtualTransport.write(data_received.data() + sent, data_received.size() - sent);
            }

            if (sent < data_received.size()) {
              return chAT::CommandStatus::ERROR;
            }

            return chAT::CommandStatus::OK;
         }
      }
      return chAT::CommandStatus::ERROR;
   };
}

#endif