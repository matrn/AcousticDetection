#include <Arduino.h>

#include "../include/circular_buffer.hpp"
#include "../lib/i2s_mic/mic.hpp"




typedef int16_t adc_sample_t;

// #define ADC_INPUT1 ADC1_CHANNEL_4	 // pin 32
// #define ADC_INPUT2 ADC1_CHANNEL_5	 // pin 33

void correlation(adc_sample_t* x1, adc_sample_t* x2, int N) {
	/*
	Rx[k] = 1/N * sum from n=0 to N-1-k of x[n]*x[n+k]
	*/

	for (int k = 0; k < N; k++) {
		int Rx_sum = 0;
		for (int n = 0; n < N - k; n++) {
			Rx_sum += x1[n] * x2[n + k];
		}
		double Rx_k = Rx_sum / (double)N;
		Serial.println(Rx_k);
	}
}

#define SAMPLES_NUM 512

// adc_sample_t x1[SAMPLES_NUM];
// adc_sample_t x2[SAMPLES_NUM];

// ADC adc1(ADC_INPUT1, I2S_NUM_0);
// ADC adc2(ADC_INPUT2, I2S_NUM_1);

Mic mic(I2S_NUM_0);


void setup() {
	Serial.begin(115200);
	
	mic.init();
	//adc2.init();	
}

void loop() {
	mic.read();
}