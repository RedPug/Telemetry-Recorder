#pragma once

#include <WiFi.h>

namespace TelemetryHandler {
    extern bool is_logging;
    extern uint32_t last_ack_ms;

    void init();
    void loop();
}