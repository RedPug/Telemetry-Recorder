#pragma once

#include <WiFi.h>

namespace TelemetryHandler {
    extern bool is_logging;

    void init();
    void loop();
}