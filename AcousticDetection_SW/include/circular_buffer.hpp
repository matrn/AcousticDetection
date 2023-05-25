#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <cstddef>

template <typename T, size_t N>
class CircularBuffer {
	T data[N] = {0};
	unsigned int current_pos = 0;
	unsigned int zero_pos = 0;

   public:
	void push(const T sample) {
		/*
			pushes sample to the end of the circular buffer
		*/
		if (current_pos >= N) {
			// current_pos = 0;
			zero_pos++;
			if (zero_pos >= N) {
				zero_pos = 0;
			}
		}

		data[current_pos % N] = sample;
		current_pos++;
	}

	T operator[](const unsigned int i) const {
		/*
			operator that returns value of the at `i` position
		*/
		unsigned int pos = i + zero_pos;
		if (pos >= N) {
			pos -= N;
		}
		return data[pos];
	}

	double mean() {
		/*
			returns mean (average) of values in the circulat bufffer
		*/
		double mean = 0;
		for (unsigned int i = 0; i < N; i++) mean += this->data[i] / (double)N;	 // mean is normalized during the run to prevent variable overflow
		return mean;
	}

	double var(const double mean) {
		/*
			returns variance of values in the circular buffer
		*/

		// 1/N * sum of (sample-mean)^2
		double var = 0;
		for (unsigned int i = 0; i < N; i++) var += pow(this->data[i] - mean, 2) / N;  //(sample-mean)^2
		return var;
	}

	unsigned int size() const {
		/*
			returns declared size (number of elemnts) of the circulat buffer
		*/
		return N;
	}
};

#endif