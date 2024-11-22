#pragma once

#include <stdint.h>

enum class ping_status: int {
    ERROR = -4,
    DNS_RESOLUTION_ERROR = -3,
    TIMEOUT = -2,
    RUNNING = 0,
    SUCCESS = 1
};

struct ping_statistics {
    uint8_t success_count;
    uint8_t timedout_count;
    float averagertt;         // measured in ms
    volatile ping_status status;
};

ping_statistics execute_ping(const char* address, uint8_t ttl, uint8_t count);
