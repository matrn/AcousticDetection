#ifndef DSP_LIB_HPP
#define DSP_LIB_HPP

#include <utility>

#include "../include/circular_buffer.hpp"
#include "../lib/i2s_mic/mic.hpp"


#define XCORR_MOVING_AVG_SIZE 10


class DSP {
	CircularBuffer<audio_sample_t, XCORR_MOVING_AVG_SIZE> xcorr_mov_avg_buf;
   public:
	DSP() {
	}

	template <size_t sig_N>
	std::pair<int, int> correlation(const CircularBuffer<audio_sample_t, sig_N> x1, const CircularBuffer<audio_sample_t, sig_N> x2, const int N, int max_k = -1) {
		/*
		Rx[k] = 1/N * sum from n=0 to N-1-k of x[n]*x[n+k]
		*/
		if (max_k == -1) max_k = N;

		double max_Rx = 0;
		int max_Rx_pos = -1;
		for (int k = 0; k < max_k; k++) {
			int Rx_sum = 0;
			for (int n = 0; n < N - k; n++) {
				Rx_sum += x1[n] * x2[n + k];
			}
			double Rx_k = Rx_sum / (double)(N - k);
			if (Rx_k > max_Rx) {
				max_Rx = Rx_k;
				max_Rx_pos = k;
			}
			// Serial.println(Rx_k);
		}
		
		auto avg = xcorr_mov_avg_buf.avg();
		if(abs(max_Rx) <= abs(avg*1.5)) max_Rx_pos = -1;
		xcorr_mov_avg_buf.push(max_Rx);
		//Serial.printf("%f,%f,%d\n", avg, max_Rx, (max_Rx_pos != -1)*50000);

		//if (max_Rx_pos != -1) Serial.printf("%d: %f\n", max_Rx_pos, max_Rx);
		return std::make_pair(max_Rx_pos, max_Rx);
	}

	static double rad2deg(double rad_angle){
		return rad_angle*(180./3.141);
	}
};

#endif