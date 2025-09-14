#include <inttypes.h>
#include <MPU6050.h>
#include "KalmanFilter.hpp"

class AccelerometerProvider {
private:
    static KalmanFilter kalmanX, kalmanY, kalmanZ;
    static MPU6050 mpu;

public:
    static void init();

    static void update(uint32_t dt);

    static void getAcceleration(int16_t &ax, int16_t &ay, int16_t &az);
};