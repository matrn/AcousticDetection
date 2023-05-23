#ifndef DSP_LIB_HPP
#define DSP_LIB_HPP

#include <utility>
#include <vector>

#include "../../include/types.hpp"
#include "../../include/circular_buffer.hpp"
#include "../i2s_mic/config.hpp"
#include "config.hpp"
// #include "../lib/i2s_mic/mic.hpp"





const double Ts = 1. / I2S_SAMPLE_RATE;	 // sample time

const double travel_time_for_max_angle = MICS_DISTANCE / SOUND_SPEED;	// how long does it take for sound wave to travel from one microphone to the other

const int maxN = travel_time_for_max_angle*(double)I2S_SAMPLE_RATE;   // int is equal to floor() operation


const double correlation_window_time = travel_time_for_max_angle * 50.0;					// 140% of maximum time difference between audio signals
const int correlation_window_samples_num = 1024; //correlation_window_time / Ts + 0.5;	// how many samples should we take for cross correlation computation

const int max_shift_samples_num = travel_time_for_max_angle/Ts+0.99;   // number of samples needed for maximum time, 0.99 = round up

/* TODO: překryv o 50% */

//#define XCORR_MA_ENABLED
#define XCORR_MOVING_AVG_SIZE 10


#define OD_SUM_THRESHOLD 500
class OnsetDetector {
	double sum = 0;
	double p = 0.2;

   public:
	bool detect(audio_sample_t value){
		/* Rekurentní odhad okamžité střední hodnoty - CZS 12. přednáška, str. 6 */
		sum = (p*abs(value) + (1-p)*sum)/2;
		//Serial.println(sum);
		return sum > OD_SUM_THRESHOLD;
	}
};


class DSP {
	#ifdef XCORR_MA_ENABLED
		CircularBuffer<audio_sample_t, XCORR_MOVING_AVG_SIZE> xcorr_mov_avg_buf;
	#endif
   public:
	DSP() {
	}

	template <typename T>
	std::pair<std::vector<int>, std::vector<double>> xcorr_complete(T x1, T x2, const int N, bool biased = true, bool abs_rx = true){  //int max_k = -1
		std::vector<int> k_vec;
		std::vector<double> Rx_vec;
		//if (max_k == -1) max_k = N;

		//puts(biased?"BIASED":"UNBIASED");

		for (int k = -(N-1); k <= N-1; k++) {
			/* TODO: handle integer overflow */
			int64_t Rx_sum = 0;
			if(k < 0){
				for (int n = 0; n < N - (-k); n++) {
					Rx_sum += x2[n-k] * x1[n];
				}
			} else {
				for (int n = 0; n < N - k; n++) {
					Rx_sum += x1[n+k] * x2[n];
				}
			}

			double Rx_k = Rx_sum / (double)(biased ? N : (N - abs(k)));
			k_vec.push_back(k);
			Rx_vec.push_back(abs_rx ? abs(Rx_k) : Rx_k);	
		}

		return std::make_pair(k_vec, Rx_vec);
	}


	template <typename T>
	double Rx(T & x1, T & x2, const int N, int k, bool biased = true){
		/* TODO: handle integer overflow */
		int64_t Rx_sum = 0;
		if(k < 0){
			for (int n = 0; n < N - (-k); n++) {
				Rx_sum += x2[n-k] * x1[n];
			}
		} else {
			for (int n = 0; n < N - k; n++) {
				Rx_sum += x1[n+k] * x2[n];
			}
		}
		double Rx_k = Rx_sum / (double)(biased ? N : (N - abs(k)));
		return Rx_k;
	}

	template <typename T>
	bool xcorr_max(T & x1, T & x2, xcorr_result_t & out, const int N, int max_k = -1, bool biased = true, bool abs_compare = true, bool quadratic_interpolation = false){
		/*
		
		returns: false if no peak was found
		*/
		double max_Rx = 0;
		int max_Rx_pos = -1;

		if (max_k == -1) max_k = N;

		//puts(biased?"BIASED":"UNBIASED");
		
		for (int k = -(max_k-1); k <= max_k-1; k++) {
			double Rx_k = Rx<T>(x1,x2, N, k, biased);   // calculate cross correlation in point k
			if(abs_compare) Rx_k = abs(Rx_k);
			if (Rx_k > max_Rx) {
				max_Rx = Rx_k;
				max_Rx_pos = k;
			}
		}

		#ifdef XCORR_MA_ENABLED
			double avg = xcorr_mov_avg_buf.avg();
			xcorr_mov_avg_buf.push(max_Rx);
			//Serial.printf("max: %f, avg: %f\n", max_Rx, avg);
			if(abs(max_Rx) <= abs(avg*1.8)) return false;
		#endif

		//Serial.printf("%f,%f,%d\n", avg, max_Rx, (max_Rx_pos != -1)*50000);

		//if (max_Rx_pos != -1) Serial.printf("%d: %f\n", max_Rx_pos, max_Rx);

		if(quadratic_interpolation){
			int k = max_Rx_pos;
			// absolute value should't be necessary but it's to be sure that interpolated parabole in concave (convex should work too)
			double Ra = abs(Rx(x1,x2, N, k-1, biased));   // R_-1
			double Rb = abs(Rx(x1,x2, N, k, biased));     // R_0
			double Rc = abs(Rx(x1,x2, N, k+1, biased));   // R_+1

			double n_new = k+(Ra-Rc)/(2*(Ra-2*Rb+Rc));
			out.interpolated_max_pos = n_new;
		}

		out.max_pos = max_Rx_pos;    // position of maximum
		out.max_Rx = max_Rx;   // Rx value in maximum
		return true;
	}
	
	// template <size_t sig_N>
	// std::pair<int, int> correlation(const CircularBuffer<audio_sample_t, sig_N> x1, const CircularBuffer<audio_sample_t, sig_N> x2, const int N, int max_k = -1) {
	// 	/*
	// 	Rx[k] = 1/N * sum from n=0 to N-1-k of x[n]*x[n+k]
	// 	*/
	// 	if (max_k == -1) max_k = N;

	// 	double max_Rx = 0;
	// 	int max_Rx_pos = -1;
	// 	for (int k = 0; k < max_k; k++) {
	// 		int Rx_sum = 0;
	// 		for (int n = 0; n < N - k; n++) {
	// 			Rx_sum += x1[n] * x2[n + k];
	// 		}
	// 		double Rx_k = Rx_sum / (double)(N - k);
	// 		if (Rx_k > max_Rx) {
	// 			max_Rx = Rx_k;
	// 			max_Rx_pos = k;
	// 		}
	// 		// Serial.println(Rx_k);
	// 	}
		
	// 	auto avg = xcorr_mov_avg_buf.avg();
	// 	if(abs(max_Rx) <= abs(avg*1.5)) max_Rx_pos = -1;
	// 	xcorr_mov_avg_buf.push(max_Rx);
	// 	//Serial.printf("%f,%f,%d\n", avg, max_Rx, (max_Rx_pos != -1)*50000);

	// 	//if (max_Rx_pos != -1) Serial.printf("%d: %f\n", max_Rx_pos, max_Rx);
	// 	return std::make_pair(max_Rx_pos, max_Rx);
	// }

	static double rad2deg(double rad_angle){
		return rad_angle*(180./PI);
	}

	static double theta_from_sample(int n){
		/*
			returns: angle in radians
		*/
		return acos((n*Ts*SOUND_SPEED)/MICS_DISTANCE);
	}

	static double theta_from_tau(double tau){
		/*
			returns: angle in radians
		*/
		return acos((tau*SOUND_SPEED)/MICS_DISTANCE);
	}

	static double theta_error(int n){
		// Thesis chapter: 4.7.2 Chyba odhadu úhlu způsobená vzorkováním
		/*			
			returns: error in radians
		*/
		if(n <= 0) return theta_from_sample(n)-theta_from_sample(n+1);
		else       return theta_from_sample(n-1)-theta_from_sample(n);
	}
};

#endif