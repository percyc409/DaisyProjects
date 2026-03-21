#include "daisy_seed.h"
#include "daisysp.h"
#include "cts_encoder.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;

// SpiHandle object and Spi Configuration object
SpiHandle spi_handle;
SpiHandle::Config spi_conf;
GPIO touch_detect;

//Encoder
constexpr Pin ENC_A_PIN     = seed::D26;
constexpr Pin ENC_B_PIN     = seed::D25;
constexpr Pin ENC_CLICK_PIN = seed::D13;
Cts_Encoder encoder;
volatile int enc_val;
bool enc_button;

uint8_t tx_buffer[3];
uint8_t rx_buffer[3];

uint16_t x_axis, y_axis;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	encoder.Debounce();
	enc_val += encoder.Increment();
	enc_button = encoder.Pressed();

	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = in[0][i];
		out[1][i] = in[1][i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	// Set some configurations
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

	// Initialize the handle using our configuration
	spi_handle.Init(spi_conf);
	touch_detect.Init(seed::D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

	encoder.Init(ENC_A_PIN, ENC_B_PIN, ENC_CLICK_PIN);
	enc_val = 0;

	hw.StartLog();
	
	hw.StartAudio(AudioCallback);

	tx_buffer[1] = 0x0;
	tx_buffer[2] = 0x0; 
	
	while(1) {
		
		if(!touch_detect.Read()) { 
			tx_buffer[0] = 0xd0;
			spi_handle.BlockingTransmitAndReceive(tx_buffer, rx_buffer, 3);
			x_axis = (rx_buffer[1] << 5) + (rx_buffer[2] >> 3);

			tx_buffer[0] = 0x90;
			spi_handle.BlockingTransmitAndReceive(tx_buffer, rx_buffer, 3);
			y_axis = (rx_buffer[1] << 5) + (rx_buffer[2] >> 3);

			hw.PrintLine("X axis: %d", x_axis);
			hw.PrintLine("Y axis: %d", y_axis);
			hw.PrintLine("Encoder Value: %d", enc_val);
			hw.PrintLine("Encoder Button: %d", enc_button);
			hw.PrintLine("* * * * * * * * * * * * * * * *");
		} else {
			hw.PrintLine("No Touch Detected");
			hw.PrintLine("Encoder Value: %d", enc_val);
			hw.PrintLine("Encoder Button: %d", enc_button);
			hw.PrintLine("* * * * * * * * * * * * * * * *");
		}

		System::Delay(2000);
	}
}
