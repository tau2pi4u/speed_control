#include "pin.h"
#include "profiles.h"

#define SMOOTH_STEP
#define TRANS_TIME 1000
#define MAX_STEP 2
#define LOOP_DELAY 10

#if defined(SMOOTH_STEP)
uint8_t prev_speed = 0;
uint8_t last_target_speed = 0;
uint8_t start_speed = 0;
uint64_t start_time = 0;
#endif

const int profile_select_pins[PROFILE_SELECT_PIN_COUNT] = PROFILE_SELECT_PINS;
const int speed_select_pins[SPEED_SELECT_PIN_COUNT] = SPEED_SELECT_PINS;

void setup_profile_pins()
{
    for(uint8_t i = 0; i < PROFILE_SELECT_PIN_COUNT; i++)
    {
        pinMode(profile_select_pins[i], INPUT);
    }
}

void setup_speed_pins()
{
    for(uint8_t i = 0; i < SPEED_SELECT_PIN_COUNT; i++)
    {
        pinMode(speed_select_pins[i], INPUT);
    }
}

void setup_output_pins()
{
    pinMode(PWM_PIN, OUTPUT);
}

uint8_t get_profile_idx()
{
    uint8_t profile = 0;
    for(uint8_t i = 0; i < PROFILE_SELECT_PIN_COUNT; i++)
    {
        profile |= (digitalRead(profile_select_pins[i]) ? 0x01 : 0x00) << i;
    }
    return profile;
}

uint8_t get_speed_idx()
{
    uint8_t speed = 0;
    for(uint8_t i = 0; i < SPEED_SELECT_PIN_COUNT; i++)
    {
        speed |= (digitalRead(speed_select_pins[i]) ? 0x01 : 0x00) << i;
    }
    return speed;
}

uint8_t * get_profile(uint8_t profile_idx)
{
    if(profile_idx < PROFILE_COUNT)
    {
        return profile_array[profile_idx];
    }
    else
    {
        return nullptr;
    }
}

uint8_t get_speed(uint8_t profile_idx, uint8_t speed_idx)
{
    uint8_t * profile = get_profile(profile_idx);
    if(profile == nullptr) { return 0; }
    if(speed_idx < SPEED_COUNT) { return profile[speed_idx]; }
    return profile[SPEED_COUNT-1];
}

void set_speed(uint8_t speed)
{
    analogWrite(PWM_PIN, speed);
}

void setup()
{
    setup_speed_pins();
    setup_profile_pins();
    setup_output_pins();
}

#if defined(SMOOTH_STEP) && defined(MAX_STEP)
uint8_t get_speed(uint8_t target_speed)
{
    if(target_speed != last_target_speed)
    {
        last_target_speed = target_speed;
        start_time = millis();
        start_speed = prev_speed;
    }

    if(prev_speed != target_speed)
    {
        uint64_t duration = millis() - start_time;
        uint8_t new_speed;
        if(duration >= TRANS_TIME)
        {
            new_speed = target_speed;
        }
        else
        {
            new_speed = map(duration, 0, TRANS_TIME, prev_speed, target_speed);
        }

        int8_t step = (int8_t) ((int16_t) prev_speed - (int16_t) new_speed);
        if(abs(step) > MAX_STEP)
        {
            return step > 0 ? prev_speed + MAX_STEP : prev_speed - MAX_STEP;
        }
        else
        {
            return new_speed;
        }
    }
    return target_speed;
}
#endif

#if defined(SMOOTH_STEP) && !defined(MAX_STEP)
uint8_t get_speed(uint8_t target_speed)
{
    if(target_speed != last_target_speed)
    {
        last_target_speed = target_speed;
        start_time = millis();
        start_speed = prev_speed;
    }

    if(prev_speed != target_speed)
    {
        uint64_t duration = millis() - start_time;
        uint8_t new_speed;
        if(duration >= TRANS_TIME)
        {
            new_speed = target_speed;
        }
        else
        {
            new_speed = map(duration, 0, TRANS_TIME, prev_speed, target_speed);
        }

        return new_speed;
    }
    return target_speed;
}
#endif

void loop()
{
    static uint64_t loop_start = 0;
    uint8_t speed_idx = get_speed_idx();
    uint8_t profile_idx = get_profile_idx();
    uint8_t target_speed = get_speed(profile_idx, speed_idx);

// With smooth step it will interpolate between speeds
#if defined(SMOOTH_STEP)
    prev_speed = get_speed(target_speed);
    set_speed(prev_speed);
// Without smooth step, it immediately impacts the output
#else
    set_speed(target_speed);
#endif

// Set the maximum speed of updates to limit the rate of change
// of the output, e.g. with LOOP_DELAY 100 and MAX_STEP 2, that's
// a max of 20 steps a second which is 392mV/s
#if defined(SMOOTH_STEP) && LOOP_DELAY > 0
    loop_duration = millis() - loop_start;
    if(loop_duration < LOOP_DELAY)
    {
        delay(LOOP_DELAY - loop_duration);
    }
#endif
}