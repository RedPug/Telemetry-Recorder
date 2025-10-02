#pragma once

#include <inttypes.h>

class GpsProvider {
public:
    static bool is_active;
    static void init();
    static void update(uint32_t dt);
    static int32_t getLatitude();
    static int32_t getLongitude();
    static void getCoordinates(int32_t &latitude, int32_t &longitude);
    static uint8_t getNumSatellites();
};