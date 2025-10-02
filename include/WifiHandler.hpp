#include <WiFi.h>

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#define WIFI_INACTIVE 0
#define WIFI_WAITING 1
#define WIFI_CONNECTED 2

namespace WifiHandler{
    extern WiFiServer server; // TCP server on port 4210
    extern WiFiClient client;
    extern uint32_t last_heartbeat_ms;

    extern uint8_t connection_status;

    extern void initServer();
    extern void terminateServer();
    extern void checkUpdates();
}

#endif