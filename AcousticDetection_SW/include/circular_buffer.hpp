#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP


#include <cstddef>

template<typename T, size_t N>
class CircularBuffer {
	T data[N] = {0};
	unsigned int current_pos = 0;
	unsigned int zero_pos = 0;

   public:
	CircularBuffer(){
		
	}

	void push(const T sample){
		if(current_pos >= N){
			//current_pos = 0;
			zero_pos ++;
			if(zero_pos >= N){
				zero_pos = 0;
			}
		}

		data[current_pos%N] = sample;
		current_pos ++;
	}

	T operator[](const unsigned int i) const{
		unsigned int pos = i+zero_pos;
		if(pos >= N){
			pos -= N;
		}
		return data[pos];
	}

	double avg(){
		double avg = 0;
		for(unsigned int i = 0; i < N; i ++) avg += this->data[i]/(double)N;
		return avg;
	}

	double var(const double mean){
		//1/N * sum of (sample-mean)^2
		double var = 0;
		for(unsigned int i = 0; i < N; i ++) var += pow(this->data[i]-mean, 2)/N;  //(sample-mean)^2
		return var;
	}

	unsigned int size() const {
		return N;
	}
};


#endif