#pragma once

#include <stdint.h>

enum ping_error: uint8_t {
    RUNNING,
    SUCCESS,
    ERROR,
    DNS_RESOLUTION_ERROR,
};

struct ping_statistics {
    uint8_t success_count;
    uint8_t timedout_count;
    float averagertt;         // measured in ms
    volatile ping_error error;
};

ping_statistics execute_ping(const char* address, uint8_t count=3);
