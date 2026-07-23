#include "daisy_pad.h"

using namespace daisy;

#define XY_BOUND 648
#define I2C_ADDRESS 0x70
#define TOUCH_REFRESH_RATE 20

//Pins
constexpr Pin SCREEN_IRQ    = seed::D17;
constexpr Pin I2S_SCL       = seed::D11;
constexpr Pin I2S_SDA       = seed::D12;
constexpr Pin ENC_A_PIN     = seed::D5;
constexpr Pin ENC_B_PIN     = seed::D3;
constexpr Pin ENC_CLICK_PIN = seed::D1;
constexpr Pin SW_1_PIN      = seed::D16;
constexpr Pin SW_2_PIN      = seed::D15;
constexpr Pin KNOB_1_PIN    = seed::D21;
constexpr Pin KNOB_2_PIN    = seed::D20;


void DaisyPad::Init(uint8_t *dma_buff16, bool boost)
{
    // Set Some numbers up for accessors.
    // Initialize the hardware.
    seed.Configure();
    seed.Init(boost);
    SetAudioBlockSize(48);

    //Assign led matrix buffer
    led_matrix_buffer = dma_buff16;
    
    // Touchscreen SPI peripheral Init
    SpiHandle::Config spi_conf;
	spi_conf.periph = SpiHandle::Config::Peripheral::SPI_1;
	spi_conf.mode = SpiHandle::Config::Mode::MASTER;
	spi_conf.direction = SpiHandle::Config::Direction::TWO_LINES;
	spi_conf.nss = SpiHandle::Config::NSS::HARD_OUTPUT;
	spi_conf.datasize = 8;
	spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
	spi_conf.clock_phase = SpiHandle::Config::ClockPhase::TWO_EDGE;
	spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_32;
	spi_conf.pin_config.sclk = Pin(PORTG, 11);
	spi_conf.pin_config.miso = Pin(PORTB, 4);
	spi_conf.pin_config.mosi = Pin(PORTB, 5);
	spi_conf.pin_config.nss = Pin(PORTG, 10);
	touchscreen.Init(spi_conf); // Initialize the handle using our configuration
	touch_detect.Init(SCREEN_IRQ, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    //Led Matrix Init
    LedMatrixInit();
    last_led_update = System::GetNow();
    last_screen_read = System::GetNow();
    led_mode = INIT;

    //Encoder Init
	encoder.Init(ENC_A_PIN, ENC_B_PIN, ENC_CLICK_PIN);

    //Switches Init 
    button1.Init(SW_1_PIN);
    button2.Init(SW_2_PIN);
    buttons[BUTTON_1] = &button1;
    buttons[BUTTON_2] = &button2;

    //Knobs Init 
    // Configure the ADC channels using the desired pin
    AdcChannelConfig knob_init[KNOB_LAST];
    knob_init[KNOB_1].InitSingle(KNOB_1_PIN);
    knob_init[KNOB_2].InitSingle(KNOB_2_PIN);
    // Initialize with the knob init struct w/ 2 members
    // Set Oversampling to 32x
    seed.adc.Init(knob_init, KNOB_LAST);
    // Make an array of pointers to the knobs.
    knobs[KNOB_1] = &knob1;
    knobs[KNOB_2] = &knob2;
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knobs[i]->Init(seed.adc.GetPtr(i), seed.AudioCallbackRate());
    }

    //Midi Init
	MidiUartHandler::Config midi_config;
    midi.Init(midi_config);

}

void DaisyPad::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyPad::StartAudio(AudioHandle::AudioCallback cb)
{
    seed.StartAudio(cb);
}

void DaisyPad::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyPad::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    seed.ChangeAudioCallback(cb);
}

void DaisyPad::StopAudio()
{
    seed.StopAudio();
}

void DaisyPad::SetAudioBlockSize(size_t size)
{
    seed.SetAudioBlockSize(size);
    SetHidUpdateRates();
}

size_t DaisyPad::AudioBlockSize()
{
    return seed.AudioBlockSize();
}

void DaisyPad::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    seed.SetAudioSampleRate(samplerate);
    SetHidUpdateRates();
}

float DaisyPad::AudioSampleRate()
{
    return seed.AudioSampleRate();
}

float DaisyPad::AudioCallbackRate()
{
    return seed.AudioCallbackRate();
}

void DaisyPad::StartAdc()
{
    seed.adc.Start();
}

void DaisyPad::StopAdc()
{
    seed.adc.Stop();
}

void DaisyPad::SetHidUpdateRates()
{
    for(int i = 0; i < KNOB_LAST; i++)
    {
        knobs[i]->SetSampleRate(AudioCallbackRate());
    }
}

float DaisyPad::GetKnobValue(Knob k)
{
    size_t idx;
    idx = k < KNOB_LAST ? k : KNOB_1;
    return knobs[idx]->Value();
}

void DaisyPad::ProcessAnalogControls()
{
    knob1.Process();
    knob2.Process();
}

void DaisyPad::ProcessDigitalControls()
{
    encoder.Debounce();
    button1.Debounce();
    button2.Debounce();
}

void DaisyPad::ProcessPeripherals()
{   
    
    // update no faster than 10Hz
    uint32_t now = System::GetNow();

    if (now - last_screen_read > TOUCH_REFRESH_RATE) { // update period duration has passed 
    
        //Handle SPI Touchscreen
        if(!touch_detect.Read()) { 

            last_screen_read = now;

            uint8_t spi_rx_buffer[3];
            uint8_t spi_tx_buffer[3];

            int x_axis0, y_axis0;

            spi_tx_buffer[1] = 0x0;
            spi_tx_buffer[2] = 0x0; 

            spi_tx_buffer[0] = 0x90;
            touchscreen.BlockingTransmitAndReceive(spi_tx_buffer, spi_rx_buffer, 3);
            x_axis0 = (spi_rx_buffer[1] << 5) + (spi_rx_buffer[2] >> 3);
            //x_axis0 = 4096-x_axis0;

            spi_tx_buffer[0] = 0xd0;
            touchscreen.BlockingTransmitAndReceive(spi_tx_buffer, spi_rx_buffer, 3);
            y_axis0 = (spi_rx_buffer[1] << 5) + (spi_rx_buffer[2] >> 3);

            if (x_axis0 < XY_BOUND) {
                x_axis0 = 0;
            } else if (x_axis0 >= 4096 - XY_BOUND) {
                x_axis0 = 4096 - XY_BOUND - XY_BOUND-1;
            } else {
                x_axis0 -= XY_BOUND;
            }
            if (y_axis0 < XY_BOUND) {
                y_axis0 = 0;
            } else if (y_axis0 >=  4096 - XY_BOUND) {
                y_axis0 = 4096 - XY_BOUND - XY_BOUND-1;
            } else {
                y_axis0 -= XY_BOUND;
            }

            x_axis = x_axis + 0.25f*(x_axis0 - x_axis);
            y_axis = y_axis + 0.25f*(y_axis0 - y_axis);
        }
    }

}

void DaisyPad::SetLedMode(DaisyPad::LedMode mode) {
    led_mode = mode;
    if (mode == CHANGE_STATE) {
        led_state_count = 0;
    }
}

void DaisyPad::LedMatrixInit() {

    // Configure the handle
    I2CHandle::Config i2c1_conf;
    i2c1_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c1_conf.mode   = I2CHandle::Config::Mode::I2C_MASTER; // Use the peripheral as a controller
    i2c1_conf.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c1_conf.pin_config.scl  = I2S_SCL; // Must match pinout for I2C1 SCL
    i2c1_conf.pin_config.sda  = I2S_SDA; // Must match pinout for I2C1 SDA

    // Initialise the handle
    if (led_display.Init(i2c1_conf) != I2CHandle::Result::OK) {
        // Something went wrong! Handle it here.
    }

    uint8_t buffer[4]; // Data to send
    buffer[0] = 0b00100001; // Command: System Setup, Internal Oscillator on
    buffer[1] = 0b10100000; // Command: Set Row Output Pin to row output driver
    //buffer[2] = 0b11101000; // Command: Set Dimming level to 9/16 duty
    buffer[2] = 0b11100010; // Command: Set Dimming level to 3/16 duty
    buffer[3] = 0b10000001; // Command: Display Setup, Blinking off, Display on


    for (int i=0; i < 4; i++) {
        
        led_display.TransmitBlocking( 
            I2C_ADDRESS, // Target address
            &buffer[i], // Pointer to buffer
            1, // Number of bytes to send
            100 // Try for this many milliseconds before failing
        );

    }
    
    for (int i = 0; i < 16; i++) {
        led_matrix_buffer[i]  = 0b00000000; // Clear
    }

}

void DaisyPad::UpdateLed() {

    // update no faster than 20Hz
    uint32_t now = System::GetNow();

    uint32_t update_period; // LED update period in milliseconds

    switch (led_mode) {
        case XY_TRACKER :
            update_period = 40;
            break;
        case CHANGE_STATE :
            update_period = 50;
            break;
        case INIT : 
            update_period = 100;
            break;
        default :
            update_period = 100;
    }

    if (now - last_led_update > update_period) { // update period duration has passed 

        last_led_update = now;

        led_matrix_buffer[0]  = 0b00000000; // Command: Set Write Pointer to first address in display ram
        led_matrix_buffer[7]  = (led_mode==INIT || led_mode==LED_OFF) ? 0b00000000 : number7seg(0xff); // Digit 4 to 7 seg diplay
        led_matrix_buffer[9]  = (led_mode==INIT || led_mode==LED_OFF) ? 0b00000000 : number7seg(0xff); // Digit 3 to 7 seg diplay
        led_matrix_buffer[11] = (led_mode==INIT || led_mode==LED_OFF) ? 0b00000000 : number7seg((display_num/10)%10); // Digit 2 to 7 seg diplay
        led_matrix_buffer[13] = (led_mode==INIT || led_mode==LED_OFF) ? 0b00000000 : number7seg(display_num%10); // Digit 1 to 7 seg diplay
        uint16_t temp;
        
        switch (led_mode) {

            case XY_TRACKER :
                
                temp = y_axis;
                for (int i=0; i<6; i++) {
                    int idx = i + 1;
                    if (temp < 466) {
                        //Y axis found, find X axis
                        uint16_t shift = x_axis/400;
                        led_matrix_buffer[idx] = 0x1 << (6-shift);
                        temp = 5000; 
                    } else {
                        led_matrix_buffer[idx] = 0b00000000;
                        temp-=466;
                    }
                }

                led_state_count = 0;

                break;

            case CHANGE_STATE :

                led_matrix_buffer[1]  = 0b01000001;
                led_matrix_buffer[2]  = 0b00100010;
                led_matrix_buffer[3]  = 0b00010100;
                led_matrix_buffer[4]  = 0b00010100;
                led_matrix_buffer[5]  = 0b00100010;
                led_matrix_buffer[6]  = 0b01000001;
                

                led_state_count++;

                if(led_state_count == 20) {
                    led_mode = XY_TRACKER;
                }

                break;

            case INIT :

                for (int i=5; i>0; i--) {
                    int idx = i + 1;
                    led_matrix_buffer[idx] = led_matrix_buffer[idx-1];
                }

                led_matrix_buffer[1] = led_matrix_buffer[1]<<1;

                if (!(led_matrix_buffer[1] & 0x3) && (led_state_count < 40)) {
                    led_matrix_buffer[1] += 1;
                }

                led_state_count++;

                if (led_state_count == 50) {
                    led_mode = XY_TRACKER;
                }

                break;
            case LED_OFF : 
                led_matrix_buffer[1]  = 0b00000000;
                led_matrix_buffer[2]  = 0b00000000;
                led_matrix_buffer[3]  = 0b00000000;
                led_matrix_buffer[4]  = 0b00000000;
                led_matrix_buffer[5]  = 0b00000000;
                led_matrix_buffer[6]  = 0b00000000;
        }
        
        led_display.TransmitDma( 
            I2C_ADDRESS,       // Target address
            led_matrix_buffer, // Pointer to buffer
            16,                // Number of bytes to send
            nullptr,           // No callback function
            nullptr            // No callback function data
        );

    }
    
}

uint8_t DaisyPad::number7seg (int num) {

    uint8_t segcode;

    switch (num) {
        case 0 : 
            segcode = 0b00111111;
            break;
        case 1 : 
            segcode = 0b00000110;
            break;
        case 2 : 
            segcode = 0b01011011;
            break;
        case 3 : 
            segcode = 0b01001111;
            break;
        case 4 : 
            segcode = 0b01100110;
            break;
        case 5 : 
            segcode = 0b01101101;
            break;
        case 6 : 
            segcode = 0b01111101;
            break;
        case 7 : 
            segcode = 0b00000111;
            break;
        case 8 : 
            segcode = 0b01111111;
            break;
        case 9 : 
            segcode = 0b01101111;
            break;
        default : 
            segcode = 0b00000000;  
    }

    return segcode;
}