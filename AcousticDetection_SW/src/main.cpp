#include <Arduino.h>

#include "../include/circular_buffer.hpp"





typedef int16_t adc_sample_t;

#define MIC1_PIN 12
#define MIC2_PIN 14

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

adc_sample_t x1[SAMPLES_NUM];
adc_sample_t x2[SAMPLES_NUM];

void setup() {
	Serial.begin(115200);
	pinMode(MIC1_PIN, INPUT);
	pinMode(MIC2_PIN, INPUT);
}

void loop() {
}