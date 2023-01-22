#ifndef DSP_SAMPLER_HPP
#define DSP_SAMPLER_HPP

#include <byteswap.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../../../AcousticDetection_SW/include/types.hpp"
#include "../../../AcousticDetection_SW/lib/i2s_mic/config.hpp"

#define SAMPLE_SIZE sizeof(audio_sample_t)

#define IS_BIG_ENDIAN 1	 //(!*(unsigned char *)&(uint16_t){1})

union int16_cast {
	int16_t value;
	int8_t bytes[2];
};

/**
 * @brief converts 2 bytes array in MSB order into int16_t integer
 *
 * @param[in] bytes pointer to the array of int8_t bytes in MSB order
 * @return int16_t returns int16_t integer
 * source: https://github.com/matrn/C-digit-recognition/blob/master/lib/ceural/convert.c
 */
int16_t MSB_2bytes_to_int16(char *bytes);

class Sampler {
   public:
	const int fs = I2S_SAMPLE_RATE;

	unsigned int number_of_samples = 0;	 // number of 2channel_samples is half
	std::ifstream file;

	int16_t read_int16(std::ifstream &file);
	int open(std::string filename);

	void sample(std::function<void(audio_sample_t, audio_sample_t)> callback, int start = 0, int end = -1);

	void close();
};

#endif