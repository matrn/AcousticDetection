#include <Arduino.h>

#include "../include/circular_buffer.hpp"
#include "../lib/i2s_mic/mic.hpp"


// ----------SETTINGS----------
const double sound_speed = 343;   // [m/s]
const double mics_distance = 0.145;   // [m]  = distance between microphones in meters
// ----------SETTINGS----------


const double Ts = 1./I2S_SAMPLE_RATE;   // sample time

const double max_angle_time = mics_distance/sound_speed;   // how long does it take for sound wave to travel from one microphone to the other

const double correlation_window_time = max_angle_time*1.50;   // 140% of maximum time difference between audio signals
const int correlation_window_samples_num = correlation_window_time/Ts+0.5;   // how many samples should we take for cross correlation computation



// #define ADC_INPUT1 ADC1_CHANNEL_4	 // pin 32
// #define ADC_INPUT2 ADC1_CHANNEL_5	 // pin 33

void correlation(CircularBuffer<audio_sample_t ,correlation_window_samples_num> x1, CircularBuffer<audio_sample_t ,correlation_window_samples_num> x2, int N) {
	/*
	Rx[k] = 1/N * sum from n=0 to N-1-k of x[n]*x[n+k]
	*/
	double max_Rx = 20000;
	int max_Rx_pos = 0;
	for (int k = 0; k < N; k++) {
		int Rx_sum = 0;
		for (int n = 0; n < N - k; n++) {
			Rx_sum += x1[n] * x2[n + k];
		}
		double Rx_k = Rx_sum / (double)N;
		if(Rx_k > max_Rx){
			max_Rx = Rx_k;
			max_Rx_pos = k;
		}
		//Serial.println(Rx_k);
	}
	if(max_Rx_pos != 0) Serial.printf("%d: %f\n", max_Rx_pos, max_Rx);
}

//#define SAMPLES_NUM 512


CircularBuffer<audio_sample_t, correlation_window_samples_num> x1;
CircularBuffer<audio_sample_t, correlation_window_samples_num> x2;

// ADC adc1(ADC_INPUT1, I2S_NUM_0);
// ADC adc2(ADC_INPUT2, I2S_NUM_1);

LRMics mics(I2S_NUM_0);


void setup() {
	Serial.begin(115200);
	Serial.println("ESP32 Acoustic Detection");
	Serial.printf("xcorr window size: %d\n", correlation_window_samples_num);
	
	mics.init();
	//adc2.init();

	// auto start = micros();
	// for(int i = 0; i < 100; i ++){
	// 	correlation(x1, x2, correlation_window_samples_num);
	// }
	// auto end = micros();
	// Serial.printf("Correlation time: %f us\n", (end-start)/100.);
	// delay(10000);
	// while(1);
	Serial.println("Starting...");
}

void loop() {
	//mics.read_and_print();
	mics.read([](audio_sample_t left, audio_sample_t right){
		x1.push(left);
		x2.push(right);
		correlation(x1, x2, correlation_window_samples_num);
	});
}