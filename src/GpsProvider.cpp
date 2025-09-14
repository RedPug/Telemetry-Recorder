#include <inttypes.h>
#include <Arduino.h>
#include "GpsProvider.hpp"

#define USE_FAKE_GPS

#if defined(USE_FAKE_GPS)

//dummy gps implementation for testing.

void GpsProvider::init(){

}

void GpsProvider::update(uint32_t dt){

}

int32_t GpsProvider::getLatitude(){
    return cos(millis() / 1000.0) * 100; // dummy GPS data
}

int32_t GpsProvider::getLongitude(){
    return sin(millis() / 1000.0) * 100; // dummy GPS data
}

void GpsProvider::getCoordinates(int32_t &latitude, int32_t &longitude){
    latitude = getLatitude();
    longitude = getLongitude();
}

uint8_t GpsProvider::getNumSatellites(){
    return 0;
}

#else // Real GPS implementation would go here

void GpsProvider::init(){

}

void GpsProvider::update(){

}

int32_t GpsProvider::getLatitude(){
    return 0;
}

int32_t GpsProvider::getLongitude(){
    return 0;
}

uint8_t GpsProvider::getNumSatellites(){
    return 0;
}

#endif