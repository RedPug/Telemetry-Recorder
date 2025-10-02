#include <inttypes.h>
#include <MPU6050.h>
#include "AccelerometerProvider.hpp"
#include "KalmanFilter.hpp"

using AP = AccelerometerProvider;

KalmanFilter AP::kalmanX = KalmanFilter(10, 500, 1);
KalmanFilter AP::kalmanY = KalmanFilter(10, 500, 1);
KalmanFilter AP::kalmanZ = KalmanFilter(10, 500, 1);
MPU6050 AP::mpu;
bool AP::is_active = false;

void AP::init(){
    mpu.initialize();
    if (!mpu.testConnection()){
        Serial.println("MPU6050 connection failed");
        AP::is_active = false;
        return;
    }
    else{
        Serial.println("MPU6050 ready");
        AP::is_active = true;
    }

    //+-4g detection range
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
    
    //change these based on the particular sensor.
    mpu.setXAccelOffset(-2280);
    mpu.setYAccelOffset(1200);
    mpu.setZAccelOffset(1300);
}

void AP::update(uint32_t dt) {
    if(!AP::is_active) return;

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float ax1, ay1, az1;
    ax1 = ax * mpu.get_acce_resolution();
    ay1 = ay * mpu.get_acce_resolution();
    az1 = az * mpu.get_acce_resolution();

    kalmanX.update(ax1, dt);
    kalmanY.update(ay1, dt);
    kalmanZ.update(az1, dt);
}

void AP::getAcceleration(float &ax, float &ay, float &az){
    if(!AP::is_active) {
        ax = 0;
        ay = 0;
        az = 0;
        return;
    }
    ax = kalmanX.getFilteredValue();
    ay = kalmanY.getFilteredValue();
    az = kalmanZ.getFilteredValue();
}
