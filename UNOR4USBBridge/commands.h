#ifndef WIFI_COMMANDS_H
#define WIFI_COMMANDS_H

enum {
   IP_ADDR = 0,
   GATEWAY_ADDR,
   NETMASK_ADDR,
   DNS1_ADDR,
   DNS2_ADDR
} ip_type;

enum {
   WIFI_FILE_DELETE = 0,
   WIFI_FILE_WRITE,
   WIFI_FILE_READ,
   WIFI_FILE_APPEND
} file_op;

#define _AT                      "AT"
#define _ENDL                    "\r\n" 
#define _WIFISCAN                "+WIFISCAN"

#define _RESET                   "+RESET"
#define _RESTART_BOOTLOADER      "+RESTARTBOOTLOADER"
#define _GMR                     "+GMR"
#define _GENERIC_CMD             "+CMD"
#define _FILESYSTEM              "+FS"
#define _MOUNTFS                 "+MOUNTFS"
#define _EXIT                    "+EXIT"
#define _MODE                    "+WIFIMODE"
#define _BEGINSTA                "+BEGINSTA"
#define _GETSTATUS               "+GETSTATUS"
#define _RECONNECT               "+RECONNECT"
#define _DISCONNECT              "+DISCONNECT"
#define _BEGINSOFTAP             "+BEGINSOFTAP"
#define _MACSTA                  "+MACSTA"
#define _MACSOFTAP               "+MACSOFTAP" 
#define _DISCONNECTSOFTAP        "+DISCONNECTSOFTAP"
#define _AUTOCONNECT             "+AUTOCONNECT"
#define _IPSTA                   "+IPSTA"
#define _IPSOFTAP                "+IPSOFTAP"
#define _IPV6                    "+IPV6"
#define _GETRSSI                 "+GETRSSI"
#define _GETSSID                 "+GETSSID"
#define _GETBSSID                "+GETBSSID"
#define _GETSOFTAPSSID           "+GETSOFTAPSSID"
#define _HOSTNAME                "+HOSTNAME"
#define _BEGINCLIENT             "+BEGINCLIENT"
#define _CLIENTSTATE             "+CLIENTSTATE"
#define _CLIENTCONNECTIP         "+CLIENTCONNECTIP"
#define _CLIENTCONNECTNAME       "+CLIENTCONNECTNAME"
#define _CLIENTSEND              "+CLIENTSEND"
#define _CLIENTRECEIVE           "+CLIENTRECEIVE"
#define _CLIENTCLOSE             "+CLIENTCLOSE"
#define _IPCLIENT                "+IPCLIENT"
#define _BEGINSERVER             "+BEGINSERVER"
#define _CLIENTCONNECTED         "+CLIENTCONNECTED"
#define _SSLBEGINCLIENT          "+SSLBEGINCLIENT"
#define _SETCAROOT               "+SETCAROOT"
#define _SSLCLIENTSTATE          "+SSLCLIENTSTATE"
#define _SSLCLIENTCONNECTNAME    "+SSLCLIENTCONNECTNAME"
#define _SETIP                   "+SETIP"
#define _GETHOSTBYNAME           "+HOSTBYNAME"
#define _AVAILABLE               "+AVAILABLE"
#define _PEEK                    "+PEEK"
#define _CLIENTFLUSH             "+FLUSH"
#define _REMOTEIP                "+REMOTEIP"
#define _REMOTEPORT              "+REMOTEPORT"
#define _CLIENTSTATUS            "+CLIENTSTATUS"
#define _SOFTRESETWIFI           "+SOFTRESETWIFI"
#define _SSLCLIENTCONNECTIP      "+SSLCLIENTCONNECTIP"
#define _SSLCLIENTSEND           "+SSLCLIENTSEND"
#define _SSLCLIENTCLOSE          "+SSLCLIENTCLOSE"
#define _SSLIPCLIENT             "+SSLIPCLIENT"
#define _SSLCLIENTCONNECTED      "+SSLCLIENTCONNECTED"
#define _SSLCLIENTRECEIVE        "+SSLCLIENTRECEIVE"
#define _SSLAVAILABLE            "+SSLAVAILABLE"
#define _SSLCLIENTSTATUS         "+SSLCLIENTSTATUS"
#define _SSLCLIENTFLUSH          "+SSLCLIENTFLUSH"
#define _SSLREMOTEIP             "+SSLREMOTEIP"
#define _SSLREMOTEPORT           "+SSLREMOTEPORT"
#define _SSLPEEK                 "+SSLPEEK"
#define _SERVERAVAILABLE         "+SERVERAVAILABLE"
#define _SERVEREND               "+SERVEREND"

#define _UDPBEGIN             "+UDPBEGIN"
#define _UDPBEGINMULTI        "+UDPBEGINMULTI"
#define _UDPBEGINPACKET       "+UDPBEGINPACKET"
#define _UDPBEGINPACKETMULTI  "+BEGINPACKETMULTI"
#define _UDPBEGINPACKETNAME   "+UDPBEGINPACKETADD"
#define _UDPBEGINPACKETIP     "+UDPBEGINPACKETIP"
#define _UDPENDPACKET         "+UDPENDPACKET"
#define _UDPWRITE             "+UDPWRITE"
#define _UDPPARSE             "+UDPPARSE"
#define _UDPAVAILABLE         "+UDPAVAILABLE"
#define _UDPREAD              "+UDPREAD"
#define _UDPPEEK              "+UDPPEEK"
#define _UDPFLUSH             "+UDPFLUSH"
#define _UDPREMOTEIP          "+UDPREMOTEIP"
#define _UDPREMOTEPORT        "+UDPREMOTEPORT"
#define _UDPSTOP              "+UDPSTOP"

#define _FWVERSION            "+FWVERSION"

#define _SOFTAPCONFIG         "+SOFTAPCONFIG"
#define _SERVERWRITE          "+SERVERWRITE"

#define _HCI_BEGIN            "+HCIBEGIN"
#define _HCI_END              "+HCIEND"
#define _HCI_WAIT             "+HCIWAIT"
#define _HCI_READ             "+HCIREAD"
#define _HCI_WRITE            "+HCIWRITE"
#define _HCI_AVAILABLE        "+HCIAVAILABLE"

#define _OTA_SETCAROOT        "+OTASETCAROOT"
#define _OTA_BEGIN            "+OTABEGIN"
#define _OTA_DOWNLOAD         "+OTADOWNLOAD"
#define _OTA_VERIFY           "+OTAVERIFY"
#define _OTA_UPDATE           "+OTAUPDATE"
#define _OTA_RESET            "+OTARESET"

#define _PREF_BEGIN           "+PREFBEGIN"
#define _PREF_END             "+PREFEND"
#define _PREF_CLEAR           "+PREFCLEAR"
#define _PREF_REMOVE          "+PREFREMOVE"
#define _PREF_PUT             "+PREFPUT"
#define _PREF_GET             "+PREFGET"
#define _PREF_LEN             "+PREFLEN"
#define _PREF_STAT            "+PREFSTAT"

#define _SOFTSE_BEGIN         "+SOFTSEBEGIN"
#define _SOFTSE_END           "+SOFTSEEND"
#define _SOFTSE_SERIAL        "+SOFTSE_SERIAL"
#define _SOFTSE_RND           "+SOFTSE_RND"
#define _SOFTSE_PRI_KEY       "+SOFTSE_PRI_KEY"
#define _SOFTSE_PUB_KEY       "+SOFTSE_PUB_KEY"
#define _SOFTSE_WRITE_SLOT    "+SOFTSE_WRITE_SLOT"
#define _SOFTSE_READ_SLOT     "+SOFTSE_READ_SLOT"
#define _SOFTSE_S_V_BUF_SET   "+SOFTSE_S_V_BUF_SET"
#define _SOFTSE_SIGN_GET      "+SOFTSE_SIGN_GET"
#define _SOFTSE_VERIFY_GET    "+SOFTSE_VERIFY_GET"
#define _SOFTSE_SHA256_GET    "+SOFTSE_SHA256_GET"



#define CMD(x)             _AT x _ENDL
#define PROMPT(x)          x ":"
#define CMD_WRITE(x)       _AT x "="
#define CMD_READ(x)        _AT x "?" _ENDL

#define START_CLIENT_SERVER_SOCK 1000
#define START_SSL_CLIENT_SOCK    2000

#endif
