#include <inttypes.h>
#include <MPU6050.h>
#include "AccelerometerProvider.hpp"
#include "KalmanFilter.hpp"

using AP = AccelerometerProvider;

KalmanFilter AP::kalmanX = KalmanFilter(10, 500, 1);
KalmanFilter AP::kalmanY = KalmanFilter(10, 500, 1);
KalmanFilter AP::kalmanZ = KalmanFilter(10, 500, 1);
MPU6050 AP::mpu;

void AP::init(){
    mpu.initialize();
    if (!mpu.testConnection()){
        Serial.println("MPU6050 connection failed");
        while (1){
        }
    }
    else{
        Serial.println("MPU6050 ready");
    }

    //+-2g detection range
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);

    //change these based on the particular sensor.
    mpu.setXAccelOffset(-2280);
    mpu.setYAccelOffset(1200);
    mpu.setZAccelOffset(1300);
}

void AP::update(uint32_t dt) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    kalmanX.update(ax, dt);
    kalmanY.update(ay, dt);
    kalmanZ.update(az, dt);
}

void AP::getAcceleration(int16_t &ax, int16_t &ay, int16_t &az){
    ax = (int16_t)kalmanX.getFilteredValue();
    ay = (int16_t)kalmanY.getFilteredValue();
    az = (int16_t)kalmanZ.getFilteredValue();
}
