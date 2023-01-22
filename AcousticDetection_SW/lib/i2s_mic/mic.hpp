#ifndef LIB_I2S_MIC_HPP
#define LIB_I2S_MIC_HPP


#include <functional>
#include <Arduino.h>
#include <driver/i2s.h>
#include <soc/i2s_reg.h>
#include "../../include/types.hpp"
#include "config.hpp"


class DCfilter{
	audio_sample_t x1_prev = 0;
	audio_sample_t y1_prev = 0;

   public:
 	void filter(audio_sample_t* x1){
		// source: https://ccrma.stanford.edu/~jos/filters/DC_Blocker_Software_Implementations.html
		y1_prev = *x1 - x1_prev + 0.995*y1_prev;    // = y1_new
		x1_prev = *x1;
		*x1 = y1_prev;
	}
};

class LRMics {
	i2s_port_t i2s_port;
	i2s_pin_config_t i2s_pins;
	
	// The 4 high bits are the channel, and the data is inverted
	size_t bytes_read;
	i2s_sample_t raw_samples_buffer[I2S_DMA_BUF_LEN] = {0};
	
	DCfilter dc_filt_1;
	DCfilter dc_filt_2;


   public:
	//audio_sample_t left_channel_data[I2S_DMA_BUF_LEN] = {0};
	//audio_sample_t right_channel_data[I2S_DMA_BUF_LEN] = {0};
	audio_sample_t data[I2S_DMA_BUF_LEN] = {0};

	LRMics (
		i2s_port_t i2s_port = I2S_NUM_0,
		i2s_pin_config_t i2s_pins = i2s_pin_config_t{
    		.bck_io_num = GPIO_NUM_32,   // BCK = Bit clock line = BCLK
		    .ws_io_num = GPIO_NUM_33,   // WS (Word Select) = LRCL (Left-Right CLock)
    		.data_out_num = I2S_PIN_NO_CHANGE,
    		.data_in_num = GPIO_NUM_35   // DOUT
		}
		) : i2s_port(i2s_port), i2s_pins(i2s_pins) {
		if (i2s_port != I2S_NUM_0 && i2s_port != I2S_NUM_1) return;
	}

	void init() {
		i2s_config_t i2s_config = {
			.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
			.sample_rate = I2S_SAMPLE_RATE,				   // The format of the signal using ADC_BUILT_IN
			.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // 32bit sample
			.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
			.communication_format = I2S_COMM_FORMAT_I2S,
			.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
			.dma_buf_count = I2S_DMA_BUF_COUNT,   // number of DMA buffers
			.dma_buf_len = I2S_DMA_BUF_LEN,   // samples
			.use_apll = false,
			.tx_desc_auto_clear = false,
			.fixed_mclk = 0
		};
		Serial.printf("Attempting to setup I2S with sampling frequency %d Hz\n", I2S_SAMPLE_RATE);
		if (i2s_driver_install(this->i2s_port, &i2s_config, 0, NULL) != ESP_OK) {
			Serial.println("Error installing I2S. Halt!");
			while (1);
		}

		// FIXES for SPH0645
        REG_SET_BIT(I2S_TIMING_REG(this->i2s_port), BIT(9));
        REG_SET_BIT(I2S_CONF_REG(this->i2s_port), I2S_RX_MSB_SHIFT);


		if(i2s_set_pin(this->i2s_port, &this->i2s_pins) != ESP_OK){
			Serial.println("Error setting i2s pins");
			while(1);
		}
		
		Serial.printf("I2S setup ok\n");
	}

	void stop(){
		if (ESP_OK != i2s_driver_uninstall(this->i2s_port)) {
			Serial.println("Error uninstalling I2S. Halt!");
			while (1);
		}
	}

	
	audio_sample_t parse_value(i2s_sample_t sample){
		//Serial.print(sample);
		sample = (sample & 0xFFFFFFF0) >> 11;

		audio_sample_t low = sample&0xFFFF;

		if(sample > 0 && low < 0) return INT16_MAX;
		else if(sample < 0 && low > 0) return INT16_MIN;
		// Serial.print(",");
		// Serial.print(sample);
		// Serial.print(",");
		// Serial.println((int16_t)sample);
		// result value is 18bit but audio_sample_t is 16bit so we have to limit the value to avoid overflow
		// if(sample > INT16_MAX) return INT16_MAX;
		// if(sample < INT16_MIN) return INT16_MIN;
		return low;
	}

	void dc_filter(audio_sample_t* x1, audio_sample_t* x2){
		dc_filt_1.filter(x1);
		dc_filt_2.filter(x2);
	}

	void read_and_print() {
		i2s_read(this->i2s_port, &raw_samples_buffer, sizeof(raw_samples_buffer), &bytes_read, portMAX_DELAY);
		//Serial.printf("read %d Bytes\n", bytes_read);

		int samples_read = bytes_read/sizeof(i2s_sample_t);
		for (int i = 0; i < samples_read; i ++) {
			
			// Serial.printf("[%d] = %d\n", i, raw_samples_buffer[i] & 0x0FFF); // Print with indexes
			audio_sample_t left = parse_value(raw_samples_buffer[i++]);
			audio_sample_t right = parse_value(raw_samples_buffer[i]);
			//Serial.println(raw_samples_buffer[i], BIN);
			//Serial.println(raw_samples_buffer[i]);
			this->dc_filter(&left, &right);
			Serial.printf("%d,%d\n", left, right);  // Print compatible with Arduino Plotter
			//Serial.printf("%d ", (raw_samples_buffer[i] & 0xFFFFFFF0) >> 11);
		}
		//Serial.printf("\n");
	}


	int read(bool dc_filter=true) { //std::function<void(audio_sample_t,audio_sample_t)> callback) {
		/*
			returns: number of samples (count/2 = number of samples for one channeů)
		*/
		i2s_read(this->i2s_port, &raw_samples_buffer, sizeof(raw_samples_buffer), &bytes_read, portMAX_DELAY);
		//Serial.printf("read %d Bytes\n", bytes_read);

		int samples_read = bytes_read/sizeof(i2s_sample_t);
		int count = 0;
		for (int i = 0; i < samples_read; i ++) {
			audio_sample_t left = parse_value(raw_samples_buffer[i++]);
			audio_sample_t right = parse_value(raw_samples_buffer[i]);

			if(dc_filter) this->dc_filter(&left, &right);
			/*Serial.println("--------------------------");
			Serial.println("raw:");
			Serial.println(raw_samples_buffer[i]);
			Serial.println(raw_samples_buffer[i], BIN);
			Serial.println("converted:");
			Serial.println(right);
			Serial.println(right, BIN);
			Serial.println("new conv:");
			Serial.println((raw_samples_buffer[i] << 4) >> 10);
			Serial.println((raw_samples_buffer[i] << 4) >> 10, BIN);
			Serial.println("--------------------------");*/
			
			//callback(left, right);
			// left_channel_data[count] = left;
			// right_channel_data[count] = right;
			data[count++] = left;
			data[count++] = right;
			//count ++;
		}
		return count;
	}
};

#endif