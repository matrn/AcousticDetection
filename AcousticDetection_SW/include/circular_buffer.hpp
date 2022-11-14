#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <Arduino.h>


template<typename T, size_t N>
class CircularBuffer {
	T data[N] = {0};
	int current_pos = 0;
	int zero_pos = 0;

   public:
	CircularBuffer(){
		
	}

	void push(T sample){
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

	T operator[](const int i) const{
		int pos = i+zero_pos;
		if(pos >= N){
			pos -= N;
		}
		return data[pos];
	}

	double avg(){
		long sum = 0;
		for(int i = 0; i < N; i ++) sum += this->data[i];
		return sum/(double)N;
	}
};


#endif