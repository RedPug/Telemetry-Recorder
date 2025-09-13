#include <TelemetryHandler.h>
#include <inttypes.h>
#include <Arduino.h>
#include <WiFi.h>
#include <MPU6050.h>
#include <queue>
#include "WifiHandler.h"

class KalmanFilter{
private:
    float past_state;
    float estimate_error;
    float measurement_error;
    float kalman_gain;
    float process_noise;

public:
    KalmanFilter(float initial_estimate_error = 1.0f, float measurement_error = 1.0f, float process_noise = 0.01f)
        : past_state(0.0f), estimate_error(initial_estimate_error),
          measurement_error(measurement_error), kalman_gain(0.0f),
          process_noise(process_noise)
    {
    }

    void update(float value, int ms_elapsed)
    {
        // Prediction update (no control input, so just propagate error)
        estimate_error += process_noise * ms_elapsed; // process noise, tune as needed

        // Measurement update
        kalman_gain = estimate_error / (estimate_error + measurement_error);
        past_state += kalman_gain * (value - past_state);
        estimate_error = (1 - kalman_gain) * estimate_error;
    }

    float getFilteredValue()
    {
        return past_state;
    }
};

struct __attribute__((packed)) TelemetryPacket{
    uint8_t signal;
    uint32_t time;
    uint16_t ax, ay, az;
    int64_t gps_long, gps_lat;
};

KalmanFilter kalmanX = KalmanFilter(10, 500, 1);
KalmanFilter kalmanY = KalmanFilter(10, 500, 1);
KalmanFilter kalmanZ = KalmanFilter(10, 500, 1);

#define TELEMETRY_BUFFER_CAPACITY 1200 // max number of packets to buffer. 1200 packets at 10Hz = 2 minutes of data.
std::queue<TelemetryPacket> telemetryBuffer;
MPU6050 mpu;

uint32_t TelemetryHandler::last_ack_ms = 0;
uint32_t last_packet_ms = 0;

//sends the queued telemetry to the client
void sendData(){

    while(WifiHandler::client.connected() && !telemetryBuffer.empty()){
        //check if we have received an ACK within a second of the last packet, verifying the connection is still alive.
        if(!(int32_t(TelemetryHandler::last_ack_ms) - millis() > -1500)){
            Serial.println("No ACK received within 1.5 seconds...");
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
        last_packet_ms = millis();
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
    packet.ax = kalmanX.getFilteredValue();
    packet.ay = kalmanY.getFilteredValue();
    packet.az = kalmanZ.getFilteredValue();
    packet.gps_long = sin(millis() / 1000.0) * 100; // dummy GPS data
    packet.gps_lat = cos(millis() / 1000.0) * 100; // dummy GPS data

    while(telemetryBuffer.size() >= TELEMETRY_BUFFER_CAPACITY){
        telemetryBuffer.pop(); // remove the oldest packet if buffer is full
    }
    telemetryBuffer.push(packet);
}

// samples the sensors and updates the Kalman filters, but does not queue it to be sent
void sampleSensors(int dt){
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    kalmanX.update(ax, dt);
    kalmanY.update(ay, dt);
    kalmanZ.update(az, dt);
}

namespace TelemetryHandler{
    bool is_logging = false;

    void init(){

        Wire.begin();
        mpu.initialize();
        if (!mpu.testConnection()){
            Serial.println("MPU6050 connection failed");
            while (1){
            }
        }
        else{
            Serial.println("MPU6050 ready");
        }

        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        // mpu.CalibrateAccel(10);
        // mpu.setZAccelOffset(mpu.getZAccelOffset() - 1800);


        //change these based on the particular sensor.
        mpu.setXAccelOffset(-2280);
        mpu.setYAccelOffset(1200);
        mpu.setZAccelOffset(1300);
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

        if (timer >= 500){
            timer = 0;
            
            if(is_logging) queueTelemetry();
        }

        sendData(); // always try to send data, if it is queued up.

        t0 = t1;
    }
}