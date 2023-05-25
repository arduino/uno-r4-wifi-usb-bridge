#ifndef WIFI_COMMANDS_H
#define WIFI_COMMANDS_H

enum{
   IP_ADDR = 0,
   GATEWAY_ADDR,
   NETMASK_ADDR,
   DNS1_ADDR,
   DNS2_ADDR
} ip_type;

#define _AT                "AT"
#define _ENDL              "\r\n" 
#define _WIFISCAN          "+WIFISCAN"

#define _RESET             "+RESET"
#define _GMR               "+GMR"
#define _GENERIC_CMD       "+CMD"
#define _FILESYSTEM        "+FS"
#define _MOUNTFS           "+MOUNTFS"
#define _EXIT              "+EXIT"
#define _MODE              "+WIFIMODE"
#define _BEGINSTA          "+BEGINSTA"
#define _GETSTATUS         "+GETSTATUS"
#define _RECONNECT         "+RECONNECT"
#define _DISCONNECT        "+DISCONNECT"
#define _BEGINSOFTAP       "+BEGINSOFTAP"
#define _MACSTA            "+MACSTA"
#define _MACSOFTAP         "+MACSOFTAP" 
#define _DISCONNECTSOFTAP  "+DISCONNECTSOFTAP"
#define _AUTOCONNECT       "+AUTOCONNECT"
#define _IPSTA             "+IPSTA"
#define _IPSOFTAP          "+IPSOFTAP"
#define _IPV6              "+IPV6"
#define _GETRSSI           "+GETRSSI"
#define _GETSSID           "+GETSSID"
#define _GETBSSID          "+GETBSSID"
#define _GETSOFTAPSSID     "+GETSOFTAPSSID"
#define _HOSTNAME          "+HOSTNAME"
#define _BEGINCLIENT       "+BEGINCLIENT"
#define _CLIENTSTATE       "+CLIENTSTATE"
#define _CLIENTCONNECTIP   "+CLIENTCONNECTIP"
#define _CLIENTCONNECTNAME "+CLIENTCONNECTNAME"
#define _CLIENTSEND        "+CLIENTSEND"
#define _CLIENTRECEIVE     "+CLIENTRECEIVE"
#define _CLIENTCLOSE       "+CLIENTCLOSE"
#define _IPCLIENT          "+IPCLIENT"
#define _BEGINSERVER       "+BEGINSERVER"
#define _CLIENTCONNECTED   "+CLIENTCONNECTED"
#define _SSLBEGINCLIENT    "+SSLBEGINCLIENT"
#define _SSLSETINSERCURE   "+SSLSETINSERCURE"
#define _SSLCLIENTSTATE    "+SSLCLIENTSTATE"
#define _SSLCLIENTCONNECT  "+SSLCLIENTCONNECT"
#define _SSLCLIENTSEND     "+SSLCLIENTSEND"
#define _SSLCLIENTCLOSE    "+SSLCLIENTCLOSE"
#define _IPSSLCLIENT       "+IPSSLCLIENT"
#define _SSLCLIENTRECEIVE  "+SSLCLIENTRECEIVE"
#define _SETIP             "+SETIP"
#define _GETHOSTBYNAME     "+HOSTBYNAME"
#define _AVAILABLE         "+AVAILABLE"
#define _PEEK              "+PEEK"
#define _CLIENTFLUSH       "+FLUSH"
#define _REMOTEIP          "+REMOTEIP"
#define _REMOTEPORT        "+REMOTEPORT"
#define _CLIENTSTATUS      "+CLIENTSTATUS"
#define _SOFTRESETWIFI     "+SOFTRESETWIFI"

#define CMD(x)             _AT x _ENDL
#define PROMPT(x)          x ":"
#define CMD_WRITE(x)       _AT x "="
#define CMD_READ(x)        _AT x "?" _ENDL

#endif
