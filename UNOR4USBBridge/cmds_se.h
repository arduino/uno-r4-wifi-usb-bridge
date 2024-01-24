#ifndef CMDS_SE_H
#define CMDS_SE_H

#include "at_handler.h"
#include "SSE.h"


void CAtHandler::add_cmds_se() {

   /* ....................................................................... */
   command_table[_SOFTSE_BEGIN] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &name = parser.args[0];
            if (name.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            /* Allow to call begin multiple times */
            sse.end();

            bool readOnly = strtol(parser.args[1].c_str(), NULL, 10) != 0;
            auto &partition = parser.args[2];
            String error = String();
            if (partition.empty()) {
               error = String(sse.begin(name.c_str(), readOnly)) + "\r\n";
            } else {
               error = String(sse.begin(name.c_str(), readOnly, partition.c_str())) + "\r\n";
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
   command_table[_SOFTSE_END] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            sse.end();
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
   command_table[_SOFTSE_SERIAL] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            std::vector<uint8_t> data;
            data.resize(8);
            esp_efuse_mac_get_default(data.data());
            srv.write_response_prompt();
            srv.write_str(String(8).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_RND] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            int len = strtol(parser.args[0].c_str(), NULL, 10);

            std::vector<uint8_t> data;
            data.resize(len);
            esp_fill_random(data.data(), len);
            srv.write_response_prompt();
            srv.write_str(String(len).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_PRI_KEY] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            int len = 0;
            int ret = 0;

            std::vector<unsigned char> der;
            der.resize(SSE_EC256_DER_PRI_KEY_LENGTH);
            if ((len = Arduino_UNOWIFIR4_SSE::generateECKeyPair(der.data(), static_cast<int>(der.size()))) < 0) {
               DEBUG_ERROR(" failed\n  !  generateECKeyPair returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }
            der.resize(len);
            if ((ret = sse.putBytes(key.c_str(), der.data(), len)) != len) {
              DEBUG_ERROR(" failed\n  !  sse.putBytes returned -0x%04x", (unsigned int) -ret);
              return chAT::CommandStatus::ERROR;
            }
#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_PRI_KEY: generated EC keypair");
            log_buf_v((const uint8_t *)der.data(), len);
#endif

            std::vector<uint8_t> data;
            data.resize(SSE_EC256_PUB_KEY_LENGTH);
            if ((len = Arduino_UNOWIFIR4_SSE::exportECKeyXY(der.data(), static_cast<int>(der.size()), data.data())) != SSE_EC256_PUB_KEY_LENGTH) {
               DEBUG_ERROR(" failed\n  !  exportECKeyXY returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }
            data.resize(len);
            srv.write_response_prompt();
            srv.write_str(String(len).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_PUB_KEY] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            int ret = 0;
            int len = 0;

            std::vector<unsigned char> der;
            len = sse.getBytesLength(key.c_str());
            der.resize(len);
            if ((ret = sse.getBytes(key.c_str(), der.data(), len)) < len) {
               DEBUG_ERROR(" failed\n  !  sse.getBytes returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }
#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_PUB_KEY: stored EC keypair");
            log_buf_v((const uint8_t *)der.data(), len);
#endif
            std::vector<uint8_t> data;
            data.resize(SSE_EC256_PUB_KEY_LENGTH);
            if ((len = Arduino_UNOWIFIR4_SSE::exportECKeyXY(der.data(), static_cast<int>(der.size()), data.data())) != SSE_EC256_PUB_KEY_LENGTH) {
               DEBUG_ERROR(" failed\n  !  exportECKeyXY returned -0x%04x", (unsigned int) -len);
               return chAT::CommandStatus::ERROR;
            }

            data.resize(len);
            srv.write_response_prompt();
            srv.write_str(String(len).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_S_V_BUF_SET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            int value = atoi(parser.args[0].c_str());

            se_buf = srv.inhibit_read(value);
            size_t offset = se_buf.size();
            if(offset < value) {
               se_buf.resize(value);
               do {
                  offset += serial->read(se_buf.data() + offset, value - offset);
               } while (offset < value);
            }
            srv.continue_read();

            /* Only stores message in se_buffer */
#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_S_V_BUF_SET: message buffer");
            log_buf_v((const uint8_t *)se_buf.data(), value);
#endif

            srv.write_response_prompt();
            srv.write_str((const char *)(parser.args[0].c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_SIGN_GET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            int len = 0;
            int ret = 0;

            /* Read private key from non volatile storage */
            std::vector<unsigned char> der;
            len = sse.getBytesLength(key.c_str());
            der.resize(len);
            if ((ret = sse.getBytes(key.c_str(), der.data(), len)) < len) {
               DEBUG_ERROR(" failed\n  !  sse.getBytes returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_SIGN_GET: message to sign:");
            log_buf_v((const uint8_t *)se_buf.data(), se_buf.size());

            log_v("_SOFTSE_SIGN_GET: key to use:");
            log_buf_v((const uint8_t *)der.data(), len);
#endif

            /* sign message/digest/sha256 stored in se_buffer */
            std::vector<uint8_t> data;
            data.resize(SSE_EC256_SIGNATURE_LENGTH);
            if ((ret = Arduino_UNOWIFIR4_SSE::sign(der.data(), len, se_buf.data(), data.data())) != SSE_EC256_SIGNATURE_LENGTH) {
               DEBUG_ERROR(" failed\n  !  sign returned -0x%04x", (unsigned int) -ret);
            }

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_SIGN_GET: {r,s} array");
            log_buf_v((const uint8_t *)data.data(), ret);
#endif

            srv.write_response_prompt();
            srv.write_str(String(ret).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_VERIFY_GET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            int ret = 0;

            /* verify data stored in se_buffer
             *
             * [  0 - 31 ] SHA526
             * [ 32 - 96 ] {r,s} signature values
             * [ 96 - 160] EC XY values
             */

            /* Import public key from buffer */
            std::vector<unsigned char> pub;
            pub.resize(SSE_EC256_DER_PUB_KEY_LENGTH);
            if ((ret = Arduino_UNOWIFIR4_SSE::importECKeyXY(&se_buf.data()[SSE_SHA256_LENGTH + SSE_EC256_SIGNATURE_LENGTH], pub.data(), SSE_EC256_DER_PUB_KEY_LENGTH)) < 0) {
               DEBUG_ERROR(" failed\n  !  importECKeyXY returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }
            pub.resize(ret);

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_VERIFY_GET: input buffer", se_buf.size());
            log_buf_v((const uint8_t *)se_buf.data(), se_buf.size());

            log_v("_SOFTSE_VERIFY_GET: EC key XY values");
            log_buf_v((const uint8_t *)pub.data(), ret);
#endif

            /* Verify data */
            if ((ret = Arduino_UNOWIFIR4_SSE::verify(pub.data(), ret, &se_buf.data()[0], &se_buf.data()[SSE_SHA256_LENGTH])) != 0) {
               DEBUG_ERROR(" failed\n  !  verify returned -0x%04x", (unsigned int) -ret);
            }
            srv.write_response_prompt();
            srv.write_str((const char *)String(ret).c_str());
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_SHA256_GET] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            int ret = 0;

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_SHA256_GET: message to hash");
            log_buf_v((const uint8_t *)se_buf.data(), se_buf.size());
#endif

            /* sign message/digest/sha256 stored in se_buffer */
            std::vector<unsigned char> data;
            data.resize(SSE_SHA256_LENGTH);
            if ((ret = Arduino_UNOWIFIR4_SSE::sha256(se_buf.data(), se_buf.size(), data.data())) != SSE_SHA256_LENGTH) {
               DEBUG_ERROR(" failed\n  !  sse.getBytes returned -0x%04x", (unsigned int) -ret);
            }

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_SHA256_GET: sha256");
            log_buf_v((const uint8_t *)data.data(), ret);
#endif

            srv.write_response_prompt();
            srv.write_str(String(ret).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SOFTSE_WRITE_SLOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 2) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            int value = atoi(parser.args[1].c_str());

            se_buf = srv.inhibit_read(value);
            size_t offset = se_buf.size();
            if(offset < value) {
               se_buf.resize(value);
               do {
                  offset += serial->read(se_buf.data() + offset, value - offset);
               } while (offset < value);
            }
            srv.continue_read();
            String error = String(sse.putBytes(key.c_str(), se_buf.data(), value)) + "\r\n";

#if SSE_DEBUG_ENABLED
            log_v("_SOFTSE_WRITE_SLOT: input data");
            log_buf_v((const uint8_t *)se_buf.data(), value);
#endif

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
   command_table[_SOFTSE_READ_SLOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &key = parser.args[0];
            std::vector<uint8_t> data;

            int len = sse.getBytesLength(key.c_str());
            data.resize(len);
            sse.getBytes(key.c_str(), data.data(), len);
            srv.write_response_prompt();
            srv.write_str(String(len).c_str());
            srv.write_str("|");
            srv.write_vec8(data);
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
