#include "pin.h"
#include "profiles.h"

#if defined(PER_PROFILE_TRANSITION)
#define SMOOTH_STEP
#endif

#if !defined(SMOOTH_STEP)
#define SMOOTH_STEP // comment this out AND remove trans_time from profiles.csv to turn off smooth stepping
#endif
#if !defined(TRANS_TIME)
#define TRANS_TIME 1000
#endif
#if !defined(MAX_STEP)
#define MAX_STEP 1 // comment this out AND remove max_step from profiles.csv to turn of max step size
#endif

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

#if defined(PER_PROFILE_TRANSITION)
uint16_t get_trans_time(uint8_t profile_idx)
{
  uint16_t trans_time = get_profile(profile_idx)[SPEED_COUNT+1];
  trans_time = (trans_time << 8) | get_profile(profile_idx)[SPEED_COUNT];
  return trans_time;
}

uint8_t get_max_step(uint8_t profile_idx)
{
  return get_profile(profile_idx)[SPEED_COUNT+2];
}
#else
uint16_t get_trans_time(uint8_t profile_idx) { return TRANS_TIME; }
uint8_t get_max_step(uint8_t profile_idx) { return MAX_STEP; }
#endif

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
uint8_t get_speed(uint8_t target_speed, uint16_t trans_time, uint8_t max_step)
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
        if(duration >= trans_time)
        {
            new_speed = target_speed;
        }
        else if(start_speed < target_speed)
        {
            new_speed = map(duration, 0, trans_time, start_speed, target_speed);
        }
        else
        {
            new_speed = map(trans_time - duration, 0, trans_time, target_speed, start_speed);
        }

        int16_t step = ((int16_t) new_speed) - ((int16_t) prev_speed);
        if(abs(step) > max_step)
        {
            return step > 0 ? prev_speed + max_step : prev_speed - max_step;
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
uint8_t get_speed(uint8_t target_speed, uint16_t trans_time, uint8_t max_step)
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
        if(duration >= trans_time)
        {
            new_speed = target_speed;
        }
        else if(start_speed < target_speed)
        {
            new_speed = map(duration, 0, trans_time, start_speed, target_speed);
        }
        else
        {
            new_speed = map(trans_time - duration, 0, trans_time, target_speed, start_speed);
        }

        return new_speed;
    }
    return target_speed;
}
#endif

void loop()
{
    uint64_t loop_start = millis();
    uint8_t speed_idx = get_speed_idx();
    uint8_t profile_idx = get_profile_idx();
    uint8_t target_speed = get_speed(profile_idx, speed_idx);

// With smooth step it will interpolate between speeds
#if defined(SMOOTH_STEP)
    prev_speed = get_speed(target_speed, get_trans_time(profile_idx), get_max_step(profile_idx));
    set_speed(prev_speed);
// Without smooth step, it immediately impacts the output
#else
    set_speed(target_speed);
#endif

// Set the maximum speed of updates to limit the rate of change
// of the output, e.g. with LOOP_DELAY 100 and MAX_STEP 2, that's
// a max of 20 steps a second which is 392mV/s
#if defined(SMOOTH_STEP) && LOOP_DELAY > 0
    uint16_t loop_duration = millis() - loop_start;
    if(loop_duration < LOOP_DELAY)
    {
        delay(LOOP_DELAY - loop_duration);
    }
#endif
}