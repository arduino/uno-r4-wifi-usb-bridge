#ifndef CMDS_PREFERENCES_H
#define CMDS_PREFERENCES_H

#include "at_handler.h"
#include <Preferences.h>

Preferences pref;

void CAtHandler::add_cmds_preferences() {
   log_e("add_cmds_preferences");
   /* ....................................................................... */
   command_table[_PREF_BEGIN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
   log_e("_PREF_BEGIN");
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &name = parser.args[0];
            if (name.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            bool readOnly = strtol(parser.args[1].c_str(), NULL, 10) != 0;
            auto &partition = parser.args[2];

            String error = String();
            if (partition.empty()) {
               error = String(pref.begin(name.c_str(), readOnly)) + "\r\n";
               log_e("pref.begin: %s", error.c_str());
            } else {
               error = String(pref.begin(name.c_str(), readOnly, partition.c_str())) + "\r\n";
               log_e("pref.begin: %s", error.c_str());
            }

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
   command_table[_PREF_END] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            pref.end();
            srv.write_response_prompt();
            srv.write_str("0");
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_PREF_CLEAR] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            String error = String(pref.clear()) + "\r\n";
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
   command_table[_PREF_REMOVE] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            if (key.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            String error = String(pref.remove(key.c_str())) + "\r\n";
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
   command_table[_PREF_PUT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      log_e("_PREF_PUT: %d", parser.args.size());
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            if (key.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            int type = strtol(parser.args[1].c_str(), NULL, 10);

            String error = String();
            switch (type) {
               case PreferenceType::PT_I8: {
                  int8_t value;
                  sscanf(parser.args[2].c_str(), "%hhd", &value);
                  error = String(pref.putChar(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U8: {
                  uint8_t value;
                  sscanf(parser.args[2].c_str(), "%hhu", &value);
                  error = String(pref.putUChar(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_I16: {
                  int16_t value;
                  sscanf(parser.args[2].c_str(), "%hd", &value);
                  error = String(pref.putShort(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U16: {
                  uint16_t value;
                  sscanf(parser.args[2].c_str(), "%hu", &value);
                  error = String(pref.putUShort(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_I32: {
                  int32_t value;
                  sscanf(parser.args[2].c_str(), "%d", &value);
                  error = String(pref.putInt(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U32: {
                  uint32_t value;
                  sscanf(parser.args[2].c_str(), "%u", &value);
                  error = String(pref.putUInt(key.c_str(), value)) + "\r\n";
                  log_v("pref.putUInt v : %u", value);
                  log_v("pref.putUInt e : %s", error.c_str());
               }
               break;
               case PreferenceType::PT_I64: {
                  int64_t value;
                  sscanf(parser.args[2].c_str(), "%lld", &value);
                  error = String(pref.putLong64(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U64: {
                  uint64_t value;
                  sscanf(parser.args[2].c_str(), "%llu", &value);
                  error = String(pref.putULong64(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_STR: {
                  auto value = parser.args[2];
                  error = String(pref.putString(key.c_str(), value.c_str())) + "\r\n";
                  log_v("pref.putUInt v : %s", value.c_str());
                  log_v("pref.putUInt e : %s", error.c_str());
               }
               break;
               case PreferenceType::PT_BLOB: {
                  int value = atoi(parser.args[2].c_str());
                  cert_buf = srv.inhibit_read(value);
                  size_t offset = cert_buf.size();
                  if(offset < value) {
                     cert_buf.resize(value);
                     do {
                        offset += serial->read(cert_buf.data() + offset, value - offset);
                     } while (offset < value);
                  }
                  srv.continue_read();
                  log_v("pref.putBytes start");
                  error = String(pref.putBytes(key.c_str(), cert_buf.data(), value)) + "\r\n";
                  log_v("pref.putBytes end");
               }
               break;
               default:
               case PreferenceType::PT_INVALID: {
                  error = "1\r\n";
               }
               break;
            }
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
   command_table[_PREF_GET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
   log_e("_PREF_GET: %d", parser.args.size());
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            log_e("_PREF_GET Write: %d", parser.args.size());
            if (parser.args.size() < 2) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            if (key.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            int type = strtol(parser.args[1].c_str(), NULL, 10);

            String error = String();
            switch (type) {
               case PreferenceType::PT_I8: {
                  int8_t value;
                  sscanf(parser.args[2].c_str(), "%hhd", &value);
                  error = String(pref.getChar(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U8: {
                  uint8_t value;
                  sscanf(parser.args[2].c_str(), "%hhu", &value);
                  error = String(pref.getUChar(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_I16: {
                  int16_t value;
                  sscanf(parser.args[2].c_str(), "%hd", &value);
                  error = String(pref.getShort(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U16: {
                  uint16_t value;
                  sscanf(parser.args[2].c_str(), "%hu", &value);
                  error = String(pref.getUShort(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_I32: {
                  int32_t value;
                  sscanf(parser.args[2].c_str(), "%d", &value);
                  error = String(pref.getInt(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_U32: {
                  uint32_t value;
                  sscanf(parser.args[2].c_str(), "%u", &value);
                  error = String(pref.getUInt(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_I64: {
                  int64_t value;
                  sscanf(parser.args[2].c_str(), "%lld", &value);
                  error = String(pref.getLong64(key.c_str(), value)) + "\r\n";
               }
               case PreferenceType::PT_U64: {
                  uint64_t value;
                  sscanf(parser.args[2].c_str(), "%llu", &value);
                  error = String(pref.getULong64(key.c_str(), value)) + "\r\n";
               }
               break;
               case PreferenceType::PT_STR: {
                  auto value = parser.args[2];
                  error = String(pref.getString(key.c_str(), value.c_str())) + "\r\n";
                  log_v("pref.getString v : %s", value.c_str());
                  log_v("pref.getString e : %s", error.c_str());
               }
               break;
               case PreferenceType::PT_BLOB: {
                  std::vector<uint8_t> data;
                  int len = pref.getBytesLength(key.c_str());
                  data.resize(len);
                  log_v("pref.getBytes start");
                  pref.getBytes(key.c_str(), data.data(), len) + "\r\n";
                  log_v("pref.getBytes end");

                  for (int x=0; x<len; x++) log_v("%d",data.data()[x]);

                  srv.write_response_prompt();
                  srv.write_str(String(len).c_str());
                  srv.write_str("|");
                  srv.write_vec8(data);
                  srv.write_line_end();
               }
               break;
               default:
               case PreferenceType::PT_INVALID: {
                  error = "1\r\n";
               }
               break;
            }


            if (type != PreferenceType::PT_BLOB) {
               srv.write_response_prompt();
               srv.write_str((const char *)(error.c_str()));
               srv.write_line_end();
            }
            return chAT::CommandStatus::OK;

         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_PREF_LEN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      log_e("_PREF_LEN: %d", parser.args.size());
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
      log_e("_PREF_LEN Write: %d", parser.args.size());
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            if (key.empty()) {
               return chAT::CommandStatus::ERROR;
            }
            log_v("pref.getBytesLength start");
            String error = String(pref.getBytesLength(key.c_str())) + "\r\n";
            log_v("pref.getBytesLength end");
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
   command_table[_PREF_STAT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            String error = String(pref.freeEntries()) + "\r\n";
            srv.write_response_prompt();
            srv.write_str((const char *)(error.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
