#include "pin.h"
#include "profiles.h"

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

void loop()
{
    uint8_t speed_idx = get_speed_idx();
    uint8_t profile_idx = get_profile_idx();
    uint8_t speed = get_speed(profile_idx, speed_idx);
    set_speed(speed);
}