#include <inttypes.h>
#include <Arduino.h>
#include <WiFi.h>
#include <MPU6050.h>
#include <queue>
#include "WifiHandler.hpp"
#include "KalmanFilter.hpp"
#include "TelemetryHandler.hpp"
#include "AccelerometerProvider.hpp"
#include "GpsProvider.hpp"

struct __attribute__((packed)) TelemetryPacket{
    uint8_t signal;
    uint32_t time;
    int16_t ax, ay, az;
    int32_t gps_long, gps_lat;
};

#define TELEMETRY_BUFFER_CAPACITY 1200 // max number of packets to buffer. 1200 packets at 10Hz = 2 minutes of data.
std::queue<TelemetryPacket> telemetryBuffer;

//sends the queued telemetry to the client
void sendData(){

    while(WifiHandler::client.connected() && !telemetryBuffer.empty()){
        //check if we have received an heartbeat recently, verifying the connection is still alive.
        if(!(WifiHandler::last_heartbeat_ms > millis() - 1500)){
            return;
        }

        TelemetryPacket packet = telemetryBuffer.front();

        // Serial.print("Sending telemetry packet...");
        uint8_t* buffer = (uint8_t*)&packet;
        size_t num_bytes_sent = WifiHandler::client.write(buffer, sizeof(packet));
        
        if(num_bytes_sent != sizeof(packet)){
            Serial.println("Error sending telemetry. Sent " + String(num_bytes_sent) + " bytes of " + String(sizeof(packet)));
            // Serial.println("Error code: " + String(WifiHandler::client.getWriteError()));
            Serial.println(String(telemetryBuffer.size()) + " packets left in buffer.");
            WifiHandler::client.stop();
            return; //probably a network error, stop trying to send more.
        }
        telemetryBuffer.pop(); //only pop if it was actually sent.
        // Serial.println(" sent!");
    }


}

//records the data and queues it to be sent in sendData()
void queueTelemetry(){
    uint8_t signal = millis() % 1000 < 500 ? LOW : HIGH;
    uint32_t time = millis();

    TelemetryPacket packet;
    packet.signal = signal;
    packet.time = time;

    int16_t ax, ay, az;
    AccelerometerProvider::getAcceleration(ax, ay, az);
    packet.ax = ax;
    packet.ay = ay;
    packet.az = az;

    packet.gps_long = GpsProvider::getLongitude();
    packet.gps_lat = GpsProvider::getLatitude();

    while(telemetryBuffer.size() >= TELEMETRY_BUFFER_CAPACITY){
        telemetryBuffer.pop(); // remove the oldest packet if buffer is full
    }
    telemetryBuffer.push(packet);
}

// samples the sensors and updates the Kalman filters, but does not queue it to be sent
void sampleSensors(int dt){
    AccelerometerProvider::update(dt);
    GpsProvider::update(dt);
}

namespace TelemetryHandler{
    bool is_logging = false;

    void init(){
        Wire.begin();
        AccelerometerProvider::init();
        GpsProvider::init();
    }

    void loop(){

        static uint32_t timer = 0;
        static uint32_t t0 = 0;
        uint32_t t1 = millis();
        const int dt = t1 - t0;

        timer += dt;

        if (is_logging && dt >= 1){ //don't sample faster than 1kHz
            sampleSensors(dt);
        }

        if (timer >= 100){
            timer = 0;
            
            if(is_logging) queueTelemetry();
        }

        sendData(); // always try to send data, if it is queued up.

        t0 = t1;
    }
}