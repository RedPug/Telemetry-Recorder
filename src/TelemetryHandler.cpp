#include <TelemetryHandler.h>
#include <inttypes.h>
#include <Arduino.h>
#include <WiFi.h>
#include <MPU6050.h>

class KalmanFilter {
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
        {}

    void update(float value, int ms_elapsed) {
        // Prediction update (no control input, so just propagate error)
        estimate_error += process_noise * ms_elapsed; // process noise, tune as needed

        // Measurement update
        kalman_gain = estimate_error / (estimate_error + measurement_error);
        past_state += kalman_gain * (value - past_state);
        estimate_error = (1 - kalman_gain) * estimate_error;
    }

    float getFilteredValue() {
        return past_state;
    }
};

KalmanFilter kalmanX = KalmanFilter(10, 500, 1);
KalmanFilter kalmanY = KalmanFilter(10, 500, 1);
KalmanFilter kalmanZ = KalmanFilter(10, 500, 1);

void sendData(WiFiClient client){
    struct __attribute__((packed)) TelemetryPacket{
        uint8_t signal;
        uint32_t time;
        uint16_t ax, ay, az;
    };
    
    uint8_t signal = millis()%1000 < 500 ? LOW : HIGH;
    uint32_t time = millis();

    TelemetryPacket packet;
    packet.signal = signal;
    packet.time = time;
    packet.ax = kalmanX.getFilteredValue();
    packet.ay = kalmanY.getFilteredValue();
    packet.az = kalmanZ.getFilteredValue();

    uint8_t* buffer = (uint8_t*)&packet;

    client.write(buffer, sizeof(packet));
}

MPU6050 mpu;

void recordData(int dt){
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    kalmanX.update(ax, dt);
    kalmanY.update(ay, dt);
    kalmanZ.update(az, dt);
}

namespace TelemetryHandler {
    

    bool is_logging = false;

    void init(){

        Wire.begin();
        mpu.initialize();
        if (!mpu.testConnection()) {
            Serial.println("MPU6050 connection failed");
            while (1) {}
        }else{
            Serial.println("MPU6050 ready");
        }

        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        // mpu.CalibrateAccel(10);
        // mpu.setZAccelOffset(mpu.getZAccelOffset() - 1800);

        mpu.setXAccelOffset(-2280);
        mpu.setYAccelOffset(1200);
        mpu.setZAccelOffset(1300);

        // Serial.print("\t");
        // Serial.print(mpu.getXAccelOffset());
        // Serial.print("\t");
        // Serial.print(mpu.getYAccelOffset());
        // Serial.print("\t");
        // Serial.print(mpu.getZAccelOffset());
        // Serial.print("\t");
        // Serial.print(mpu.getXGyroOffset());
        // Serial.print("\t");
        // Serial.print(mpu.getYGyroOffset());
        // Serial.print("\t");
        // Serial.print(mpu.getZGyroOffset());
        // Serial.print("\n");
    }

    void loop(WiFiClient client){
        static uint8_t timer = 0;
        static uint8_t t0 = 0;
        uint8_t t1 = millis();
        const int dt = t1-t0;

        timer += dt;

        if(dt >= 1){
            recordData(dt);
        }

        if(timer >= 100){
            timer = 0;
            sendData(client);
        }

        t0 = t1;
    }
}