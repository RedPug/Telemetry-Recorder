#include <inttypes.h>
#include <MPU6050.h>
#include "KalmanFilter.hpp"

class AccelerometerProvider {
private:
    static KalmanFilter kalmanX, kalmanY, kalmanZ;
    static MPU6050 mpu;

public:
    static bool is_active;
    
    static void init();

    static void update(uint32_t dt);

    static void getAcceleration(float &ax, float &ay, float &az);
};