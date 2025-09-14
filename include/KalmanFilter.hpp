#pragma once

class KalmanFilter{
private:
    float past_state;
    float estimate_error;
    float measurement_error;
    float kalman_gain;
    float process_noise;

public:
    // @param initial_estimate_error: Initial error in estimate
    // @param measurement_error: How much error we expect from the measurements
    // @param process_noise: How much error is added per ms (drift)
    KalmanFilter(float initial_estimate_error = 1.0f, float measurement_error = 1.0f, float process_noise = 0.01f)
        : past_state(0.0f), estimate_error(initial_estimate_error),
          measurement_error(measurement_error), kalman_gain(0.0f),
          process_noise(process_noise)
    {}

    // @param value: New measurement to update the filter with
    // @param ms_elapsed: Time in milliseconds since the last update
    void update(float value, int ms_elapsed)
    {
        // Prediction update (no control input, so just propagate error)
        estimate_error += process_noise * ms_elapsed;

        // Measurement update
        kalman_gain = estimate_error / (estimate_error + measurement_error);
        past_state += kalman_gain * (value - past_state);
        estimate_error = (1 - kalman_gain) * estimate_error;
    }

    // @return The current filtered value
    float getFilteredValue()
    {
        return past_state;
    }
};