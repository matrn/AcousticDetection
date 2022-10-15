#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <Arduino.h>


template<typename T, size_t N>
class CircularBuffer {
	T data[N];
	int current_pos = 0;
	int zero_pos = 0;

   public:
	CircularBuffer(){
		
	}

	void push(T sample){
		data[current_pos] = sample;
		current_pos ++;
		if(current_pos >= N){
			current_pos = 0;
			zero_pos ++;
			if(zero_pos >= N){
				zero_pos = 0;
			}
		}
	}

	T operator[](int i){
		int pos = i+zero_pos;
		if(pos >= N){
			pos -= N;
		}
		return data[pos];
	}
};


#endif