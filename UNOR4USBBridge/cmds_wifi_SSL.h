#ifndef CMDS_WIFI_SSL_H
#define CMDS_WIFI_SSL_H

#ifdef BUNDLED_CA_ROOT_CRT
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(x509_crt_bundle, PATH_CERT_BUNDLE);
#else
#define ROOT_CERTIFICATES_ADDRESS   (0xB000)
#endif

#include "at_handler.h"
#include "mbedtls/pem.h"
#include "SSE.h"

#ifndef WIFI_CLIENT_DEF_CONN_TIMEOUT_MS
#define WIFI_CLIENT_DEF_CONN_TIMEOUT_MS  (3000)
#endif

void CAtHandler::add_cmds_wifi_SSL() {
   /* ....................................................................... */
   command_table[_SSLBEGINCLIENT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Run: {
            if (sslclients_num < MAX_CLIENT_AVAILABLE) {
               for (int i = 0; i < MAX_CLIENT_AVAILABLE; i++) {
                  if (sslclients[i] == nullptr) {
                     sslclients[i] = new WiFiClientSecure();
                     clients_ca[i].clear();
                     clients_cert_pem[i].clear();
                     clients_key_pem[i].clear();
                     if(sslclients[i] == nullptr) {
                        return chAT::CommandStatus::ERROR;
                     }
                     sslclients_num++;
                     srv.write_response_prompt();
                     srv.write_str((const char *) String(i + START_SSL_CLIENT_SOCK).c_str());
                     srv.write_line_end();
                     return chAT::CommandStatus::OK;
                  }
               }
            }
            return chAT::CommandStatus::ERROR;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SETCAROOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() < 1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());

            CClientWrapper the_client = getClient(sock);

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            const int internal_sock = the_client.can_delete;
            if (internal_sock == -1) {
               return chAT::CommandStatus::ERROR;
            }

            bool ca_root_custom = false;
            int ca_root_size = 0;
            if (parser.args.size() >= 2){
               auto &ca_root_size_str = parser.args[1];
               if (ca_root_size_str.empty()) {
                  return chAT::CommandStatus::ERROR;
               }
               ca_root_size = atoi(ca_root_size_str.c_str());
               ca_root_custom = true;
            }

            if(ca_root_custom) {
               clients_ca[internal_sock] = srv.inhibit_read(ca_root_size);
               size_t offset = clients_ca[internal_sock].size();

               if(offset < ca_root_size) {

                  clients_ca[internal_sock].resize(ca_root_size);
                  do {
                     offset += serial->read(clients_ca[internal_sock].data() + offset, ca_root_size - offset);
                  } while (offset < ca_root_size);
               }
               the_client.sslclient->setCACert((const char *)clients_ca[internal_sock].data());
               srv.continue_read();
            } else {
               #ifdef BUNDLED_CA_ROOT_CRT
               the_client.sslclient->setCACertBundle((const uint8_t *)x509_crt_bundle_data);
               #else
               if(cert_in_flash_ptr == nullptr) {
                  const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "cert");
                  if(partition != nullptr) {
                     esp_partition_mmap(partition, 0, partition->size, SPI_FLASH_MMAP_DATA, (const void**)&cert_in_flash_ptr, &cert_in_flash_handle);
                  }
               }

               if(cert_in_flash_ptr != nullptr) {
                 the_client.sslclient->setCACertBundle(cert_in_flash_ptr);
               }
               #endif
               srv.write_response_prompt();
            }

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SETECCSLOT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() != 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            auto &slot_num = parser.args[1];
            auto &cert_len = parser.args[2];
            if (sock_num.empty() || slot_num.empty() || cert_len.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            int size = atoi(cert_len.c_str());

            CClientWrapper the_client = getClient(sock);
            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            const int internal_sock = the_client.can_delete;
            if (internal_sock == -1) {
               return chAT::CommandStatus::ERROR;
            }

            std::vector<unsigned char> client_cert_der;
            client_cert_der = srv.inhibit_read(size);
            size_t offset = client_cert_der.size();

            if(offset < size) {
               client_cert_der.resize(size);
               do {
                  offset += serial->read(client_cert_der.data() + offset, size - offset);
               } while (offset < size);
            }
            srv.continue_read();

#if ECC_DEBUG_ENABLED
            log_v("_SETECCSLOT: input cert");
            log_buf_v((const uint8_t *)client_cert_der.data(), size);
#endif

            /* Convert client certificate DER buffer into PEM */
            clients_cert_pem[internal_sock].resize(1024);
            size_t olen;
            int ret = -1;
            if ((ret = mbedtls_pem_write_buffer("-----BEGIN CERTIFICATE-----\n",
                                                "-----END CERTIFICATE-----\n",
                                                client_cert_der.data(), size,
                                                clients_cert_pem[internal_sock].data(), 1024,
                                                &olen)) != 0)
            {
               log_e(" failed\n  !  mbedtls_pem_write_buffer returned -0x%04x", (unsigned int) -ret);
               clients_cert_pem[internal_sock].clear();
               return chAT::CommandStatus::ERROR;
            }
            clients_cert_pem[internal_sock].resize(olen);

#if ECC_DEBUG_ENABLED
            log_v("_SETECCSLOT: output cert");
            log_v("\n%s", clients_cert_pem[internal_sock].data());
#endif

            /* Set client certificate */
            the_client.sslclient->setCertificate((const char *)clients_cert_pem[internal_sock].data());

            /* Read private key from non volatile storage in DER format */
            std::vector<unsigned char> client_key_der;
            int len = sse.getBytesLength(slot_num.c_str());
            client_key_der.resize(len);
            if ((ret = sse.getBytes(slot_num.c_str(), client_key_der.data(), len)) < len) {
               log_e(" failed\n  !  sse.getBytes returned -0x%04x", (unsigned int) -ret);
               return chAT::CommandStatus::ERROR;
            }

#if ECC_DEBUG_ENABLED
            log_v("_SETECCSLOT: input key");
            log_buf_v((const uint8_t *)client_key_der.data(), ret);
#endif

            /* Convert private key in PEM format */
            clients_key_pem[internal_sock].resize(1024);
            if ((ret = mbedtls_pem_write_buffer("-----BEGIN EC PRIVATE KEY-----\n",
                                                "-----END EC PRIVATE KEY-----\n",
                                                client_key_der.data(), len,
                                                clients_key_pem[internal_sock].data(), 1024,
                                                &olen)) != 0)
            {
               log_e(" failed\n  !  mbedtls_pem_write_buffer returned -0x%04x", (unsigned int) -ret);
               clients_cert_pem[internal_sock].clear();
               return chAT::CommandStatus::ERROR;
            }
            clients_key_pem[internal_sock].resize(olen);

#if ECC_DEBUG_ENABLED
            log_v("_SETECCSLOT: output key");
            log_v("\n%s", clients_key_pem[internal_sock].data());
#endif

            /* Set client key */
            the_client.sslclient->setPrivateKey((const char *)clients_key_pem[internal_sock].data());

            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTSTATE] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            if (the_client.sslclient->connected()) {
               String client_status = the_client.sslclient->remoteIP().toString() + "," + String(the_client.sslclient->remotePort()) + "," + String(the_client.sslclient->localPort()) + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(client_status.c_str()));
               srv.write_line_end();
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECTNAME] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            const int internal_sock = the_client.can_delete;
            if (internal_sock == -1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &host = parser.args[1];
            if (host.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port = parser.args[2];
            if (port.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            /* Set custom root ca */
            if (clients_ca[internal_sock].size()) {
               the_client.sslclient->setCACert((const char *)clients_ca[internal_sock].data());
            }
            /* Default ca bundle is configured automatically on connect by the WiFiSSLClient */

            if (clients_cert_pem[internal_sock].size()) {
              /* Set client certificate */
              the_client.sslclient->setCertificate((const char *)clients_cert_pem[internal_sock].data());
            }
            if (clients_key_pem[internal_sock].size()) {
              /* Set client key */
              the_client.sslclient->setPrivateKey((const char *)clients_key_pem[internal_sock].data());
            }

            if (!the_client.sslclient->connect(host.c_str(), atoi(port.c_str()))) {
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
   command_table[_SSLCLIENTCONNECTIP] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            const int internal_sock = the_client.can_delete;
            if (internal_sock == -1) {
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

            /* Set custom root ca */
            if (clients_ca[internal_sock].size()) {
               the_client.sslclient->setCACert((const char *)clients_ca[internal_sock].data());
            }
            /* Default ca bundle is configured automatically on connect by the WiFiSSLClient */

            if (clients_cert_pem[internal_sock].size()) {
              /* Set client certificate */
              the_client.sslclient->setCertificate((const char *)clients_cert_pem[internal_sock].data());
            }
            if (clients_key_pem[internal_sock].size()) {
              /* Set client key */
              the_client.sslclient->setPrivateKey((const char *)clients_key_pem[internal_sock].data());
            }

            if (!the_client.sslclient->connect(address, atoi(hostport.c_str()))) {
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
   command_table[_SSLCLIENTCONNECT] = [this](auto & srv, auto & parser) {
   /* ....................................................................... */
      switch (parser.cmd_mode) {
         case chAT::CommandMode::Write: {
            if (parser.args.size() < 3) {
               return chAT::CommandStatus::ERROR;
            }

            auto &sock_num = parser.args[0];
            if (sock_num.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int sock = atoi(sock_num.c_str());
            CClientWrapper the_client = getClient(sock);

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            const int internal_sock = the_client.can_delete;
            if (internal_sock == -1) {
               return chAT::CommandStatus::ERROR;
            }

            auto &host = parser.args[1];
            if (host.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            auto &port = parser.args[2];
            if (port.empty()) {
               return chAT::CommandStatus::ERROR;
            }

            int timeout = WIFI_CLIENT_DEF_CONN_TIMEOUT_MS;
            if (parser.args.size() > 3) {
               auto &tmp = parser.args[3];
               if (tmp.empty()) {
                  return chAT::CommandStatus::ERROR;
               }
               int t = atoi(tmp.c_str());
               if (t > 0) {
                  timeout = t;
               }
            }

            /* Set custom root ca */
            if (clients_ca[internal_sock].size()) {
               the_client.sslclient->setCACert((const char *)clients_ca[internal_sock].data());
            }
            /* Default ca bundle is configured automatically on connect by the WiFiSSLClient */

            if (clients_cert_pem[internal_sock].size()) {
              /* Set client certificate */
              the_client.sslclient->setCertificate((const char *)clients_cert_pem[internal_sock].data());
            }
            if (clients_key_pem[internal_sock].size()) {
              /* Set client key */
              the_client.sslclient->setPrivateKey((const char *)clients_key_pem[internal_sock].data());
            }

            if (!the_client.sslclient->connect(host.c_str(), atoi(port.c_str()), timeout)) {
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
   command_table[_SSLCLIENTSEND] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
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
            sent += the_client.sslclient->write(data_received.data() + sent, data_received.size() - sent);


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
   command_table[_SSLCLIENTCLOSE] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
            }
            else {
               the_client.sslclient->stop();

               if(the_client.can_delete >= 0) {
                  delete sslclients[the_client.can_delete];
                  sslclients[the_client.can_delete] = nullptr;
                  clients_ca[the_client.can_delete].clear();
                  clients_cert_pem[the_client.can_delete].clear();
                  clients_key_pem[the_client.can_delete].clear();
                  sslclients_num--;
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
   command_table[_SSLIPCLIENT] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            if (the_client.sslclient->connected()) {
               String client_status = the_client.sslclient->localIP().toString() + "\r\n";
               srv.write_response_prompt();
               srv.write_str((const char *)(client_status.c_str()));
               srv.write_line_end();
            }
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTCONNECTED] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String con(the_client.sslclient->connected());
            srv.write_str((const char *)(con.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;

         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTRECEIVE] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
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

            int data_available = the_client.sslclient->available();
            data_wanted = (data_wanted < data_available) ? data_wanted : data_available;

            std::vector<uint8_t> data_received;
            data_received.resize(data_wanted);

            int res = the_client.sslclient->read(data_received.data(), data_wanted);
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
   command_table[_SSLAVAILABLE] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String av(the_client.sslclient->available());
            srv.write_str((const char *)(av.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTSTATUS] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            //String st(sslclients[sock]->status());
            //srv.write_str((const char *)(st.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLCLIENTFLUSH] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            the_client.sslclient->flush();
            srv.write_response_prompt();
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };

   /* ....................................................................... */
   command_table[_SSLREMOTEIP] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }


            IPAddress ip = the_client.sslclient->remoteIP();
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
   command_table[_SSLREMOTEPORT] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            String port(the_client.sslclient->remotePort());
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
   command_table[_SSLPEEK] = [this](auto & srv, auto & parser) {
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

            if (the_client.sslclient == nullptr) {
               return chAT::CommandStatus::ERROR;
            }

            srv.write_response_prompt();
            String p(the_client.sslclient->peek());
            srv.write_str((const char *)(p.c_str()));
            srv.write_line_end();
            return chAT::CommandStatus::OK;
         }
         default:
            return chAT::CommandStatus::ERROR;
      }
   };
}

#endif
