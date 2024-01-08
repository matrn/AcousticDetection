#ifndef DSP_LIB_HPP
#define DSP_LIB_HPP

#include <utility>
#include <vector>

#include "../../include/circular_buffer.hpp"
#include "../../include/types.hpp"
#include "../i2s_mic/config.hpp"
#include "config.hpp"

const double Ts = 1. / I2S_SAMPLE_RATE;	 // sample time

const double travel_time_for_max_angle = MICS_DISTANCE / SOUND_SPEED;	  // how long does it take for sound wave to travel from one microphone to the other
const int maxN = travel_time_for_max_angle * (double)I2S_SAMPLE_RATE;	  // int is equal to floor() operation
const int max_shift_samples_num = travel_time_for_max_angle / Ts + 0.99;  // number of samples needed for maximum time, 0.99 = round up, it can be higher than maxN
const int correlation_window_samples_num = 1024;						  //  how many samples should we take for cross correlation computation

// #define XCORR_MA_ENABLED
#define XCORR_MOVING_AVG_SIZE 10



/* ---------- OnSet detector for acoustic impulse events ---------- */
#define OD_SUM_THRESHOLD_DEFAULT 500
class OnsetDetector {
	double sum = 0;
	double p = 0.2;
	uint16_t OD_SUM_THRESHOLD = OD_SUM_THRESHOLD_DEFAULT;

   public:
	void set_threshold(const uint16_t threshold){
		this->OD_SUM_THRESHOLD = threshold;
	}
	uint16_t get_threshold(){
		return this->OD_SUM_THRESHOLD;
	}
	bool detect(audio_sample_t value) {
		/* Rekurentní odhad okamžité střední hodnoty - CZS 12. přednáška, str. 6 */
		sum = (p * abs(value) + (1 - p) * sum) / 2;
		// Serial.println(sum);
		return sum > this->OD_SUM_THRESHOLD;
	}
};
/* ---------------------------------------------------------------- */

class DSP {
#ifdef XCORR_MA_ENABLED
	CircularBuffer<audio_sample_t, XCORR_MOVING_AVG_SIZE> xcorr_mov_avg_buf;
#endif
   public:
	DSP() {
	}

	template <typename T>
	std::pair<std::vector<int>, std::vector<double>> xcorr_complete(T x1, T x2, const int N, bool biased = true, bool abs_rx = true) {	// int max_k = -1
		/*
			calculates complete cross correlation function
		*/
		std::vector<int> k_vec;
		std::vector<double> Rx_vec;
		// if (max_k == -1) max_k = N;

		// puts(biased?"BIASED":"UNBIASED");

		for (int k = -(N - 1); k <= N - 1; k++) {
			/* TODO: handle integer overflow */
			int64_t Rx_sum = 0;
			if (k < 0) {
				for (int n = 0; n < N - (-k); n++) {
					Rx_sum += x2[n - k] * x1[n];
				}
			} else {
				for (int n = 0; n < N - k; n++) {
					Rx_sum += x1[n + k] * x2[n];
				}
			}

			double Rx_k = Rx_sum / (double)(biased ? N : (N - abs(k)));
			k_vec.push_back(k);
			Rx_vec.push_back(abs_rx ? abs(Rx_k) : Rx_k);
		}

		return std::make_pair(k_vec, Rx_vec);
	}

	template <typename T>
	double Rx(T& x1, T& x2, const int N, int k, bool biased = true) {
		/*
			returns Rx[k] - cross correlation in the point `k`
		*/
		/* TODO: correctly handle integer overflow */
		int64_t Rx_sum = 0;
		if (k < 0) {
			for (int n = 0; n < N - (-k); n++) {
				Rx_sum += x2[n - k] * x1[n];
			}
		} else {
			for (int n = 0; n < N - k; n++) {
				Rx_sum += x1[n + k] * x2[n];
			}
		}
		double Rx_k = Rx_sum / (double)(biased ? N : (N - abs(k)));
		return Rx_k;
	}

	/**
	 * @brief Finds cross correlation maximum
	 *
	 * @param x1 signal x1
	 * @param x2 signal x2
	 * @param out output - position of the maximum, maximum value and interpolated maximum if @quadratic_interpolation is enabled
	 * @param N length of signal xx
	 * @param max_k maximum `k` where to look for the maximum of Rx[k]
	 * @param biased biased cross correlation, if false, unbiased is used
	 * @param abs_compare use absolute values for maximum comparation
	 * @param quadratic_interpolation use quadratic interpolation
	 * @return true if XCORR_MA_ENABLED is not set, returns true everytime
	 * @return false if no peak was found (XCORR_MA_ENABLED) must be set
	 */
	template <typename T>
	bool xcorr_max(T& x1, T& x2, xcorr_result_t& out, const int N, int max_k = -1, bool biased = true, bool abs_compare = true, bool quadratic_interpolation = false) {
		double max_Rx = 0;
		int max_Rx_pos = -1;

		if (max_k == -1) max_k = N;

		// puts(biased?"BIASED":"UNBIASED");

		for (int k = -(max_k - 1); k <= max_k - 1; k++) {
			double Rx_k = Rx<T>(x1, x2, N, k, biased);	// calculate cross correlation in point k
			if (abs_compare) Rx_k = abs(Rx_k);
			if (Rx_k > max_Rx) {
				max_Rx = Rx_k;
				max_Rx_pos = k;
			}
		}

#ifdef XCORR_MA_ENABLED
		double avg = xcorr_mov_avg_buf.mean();
		xcorr_mov_avg_buf.push(max_Rx);
		// Serial.printf("max: %f, avg: %f\n", max_Rx, avg);
		if (abs(max_Rx) <= abs(avg * 1.8)) return false;
#endif

		if (quadratic_interpolation) {
			int k = max_Rx_pos;
			// absolute value should't be necessary but it's to be sure that interpolated parabole in correct (concave (convex should work too)) but if one point is positive and second is negative, parabole will be incorrect
			double Ra = abs(Rx(x1, x2, N, k - 1, biased));	// R_-1
			double Rb = abs(Rx(x1, x2, N, k, biased));		// R_0
			double Rc = abs(Rx(x1, x2, N, k + 1, biased));	// R_+1

			double n_new = k + (Ra - Rc) / (2 * (Ra - 2 * Rb + Rc));
			out.interpolated_max_pos = n_new;
		}

		out.max_pos = max_Rx_pos;  // position of maximum
		out.max_Rx = max_Rx;	   // Rx value in maximum
		return true;
	}


	static double rad2deg(double rad_angle) {
		/*
			radians to degrees conversion
		*/
		return rad_angle * (180. / PI);
	}

	/**
	 * @brief calculates TDoA angle theta in the sample `n`
	 *
	 * @param n sample number
	 * @return double angle in radians
	 */
	static double theta_from_sample(int n) {
		return acos((n * Ts * SOUND_SPEED) / MICS_DISTANCE);
	}

	/**
	 * @brief calculates TDoA angle theta of delay tau
	 *
	 * @param tau
	 * @return double angle in radians
	 */
	static double theta_from_tau(double tau) {
		/*
			returns: angle in radians
		*/
		return acos((tau * SOUND_SPEED) / MICS_DISTANCE);
	}

	/**
	 * @brief calculates theta error caused by sampling, value is then theta +- error
	 *
	 * @param n sample number
	 * @return double error in radians
	 */
	static double theta_error(int n) {
		// Thesis chapter: 4.7.2 Chyba odhadu úhlu způsobená vzorkováním
		if (n <= 0)
			return theta_from_sample(n) - theta_from_sample(n + 1);
		else
			return theta_from_sample(n - 1) - theta_from_sample(n);
	}
};

#endif