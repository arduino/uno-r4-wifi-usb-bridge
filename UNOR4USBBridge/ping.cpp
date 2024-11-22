#include "ping.h"
#include <string.h>
#include <Arduino.h>
#include <lwip/inet.h>
#include <lwip/ip_addr.h>
#include <lwip/netdb.h>
#include "ping/ping_sock.h"

// we are assuming that ping is a blocking call that returns the results of a ping session
// async operations are not taken into account as of now
// one ping session can be performed

static ping_statistics _stats;

void ping_success(esp_ping_handle_t hdl, void *args) {
    uint32_t elapsed_time;
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    // streaming average on rtt
    _stats.averagertt = _stats.averagertt + ((elapsed_time-_stats.averagertt)/++_stats.success_count);
}

void ping_timeout(esp_ping_handle_t hdl, void *args) {
    _stats.timedout_count++;

}

void ping_end(esp_ping_handle_t hdl, void *args) {
    _stats.error = SUCCESS;
}

ping_statistics execute_ping(const char* address, uint8_t ttl, uint8_t count) {

    ip_addr_t target_addr;
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    memset(&target_addr, 0, sizeof(target_addr));

    log_e("resolving \"%s\"", address);

    if(getaddrinfo(address, NULL, &hint, &res) != 0) {
        log_e("resolution error");
        _stats.error = DNS_RESOLUTION_ERROR;
        return _stats;
    }

    struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
    inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    freeaddrinfo(res);


    //FIXME check that the address is resolved
    esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
    ping_config.target_addr = target_addr;          // target IP address
    ping_config.ttl = ttl;

    // for simplification we are not pinging indefinetly
    ping_config.count = count > 0? count : 10;

    /* set callback functions */
    esp_ping_callbacks_t cbs;
    cbs.on_ping_success = ping_success;
    cbs.on_ping_timeout = ping_timeout;
    cbs.on_ping_end     = ping_end;
    cbs.cb_args         = NULL;

    memset(&_stats, 0, sizeof(_stats));
    _stats.error = RUNNING;

    esp_ping_handle_t ping; // FIXME do I need this?
    esp_ping_new_session(&ping_config, &cbs, &ping);
    esp_ping_start(ping);

    // wait for the end of ping session
    while(_stats.error != RUNNING) {
        delay(10);
    }

    return _stats;
}



