#include "cts_encoder.h"

using namespace daisy;

void Cts_Encoder::Init(dsy_gpio_pin a,
                   dsy_gpio_pin b,
                   dsy_gpio_pin click,
                   float        update_rate)
{
    last_update_ = System::GetNow();
    updated_     = false;

    // Init GPIO for A, and B
    hw_a_.pin  = a;
    hw_a_.mode = DSY_GPIO_MODE_INPUT;
    hw_a_.pull = DSY_GPIO_PULLUP;
    hw_b_.pin  = b;
    hw_b_.mode = DSY_GPIO_MODE_INPUT;
    hw_b_.pull = DSY_GPIO_PULLUP;
    dsy_gpio_init(&hw_a_);
    dsy_gpio_init(&hw_b_);
    // Default Initialization for Switch
    sw_.Init(click);
    // Set initial states, etc.
    inc_ = 0;
    p_state_ = dsy_gpio_read(&hw_a_) | (dsy_gpio_read(&hw_b_) << 1);
}

void Cts_Encoder::Debounce()
{
    // update no faster than 1kHz
    uint32_t now = System::GetNow();
    updated_     = false;

    if(now - last_update_ >= 1)
    {
        last_update_ = now;
        updated_     = true;

        uint8_t state = dsy_gpio_read(&hw_a_) | (dsy_gpio_read(&hw_b_) << 1);

        if (state != p_state_){
            // infer increment direction
            uint8_t state_b   = state ^ (state>>1); //Gray to binary conversion
            uint8_t p_state_b = p_state_ ^ (p_state_>>1); //Gray to binary conversion

            if (state_b == ((p_state_b+1)%4)) {
                inc_ = 1; 
            } else {
                inc_ = -1;
            }

            p_state_ = state;
        } else {
            inc_ = 0;
        }
    }

    // Debounce built-in switch
    sw_.Debounce();
}
