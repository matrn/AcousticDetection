#ifndef DSP_SAMPLER_HPP
#define DSP_SAMPLER_HPP

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "../../../AcousticDetection_SW/include/types.hpp"
#include "../../../AcousticDetection_SW/lib/i2s_mic/config.hpp"
#include "csv.hpp"

std::vector<std::string> read_and_split_line(std::istream& file_stream, char delimiter = ',') {
	std::string line_str;
	std::vector<std::string> line_vec;

	if (!std::getline(file_stream, line_str)) return line_vec;

	std::stringstream line_stream(line_str);
	std::string item;

	while (std::getline(line_stream, item, delimiter)) {
		line_vec.push_back(item);
	}
	return line_vec;
}

class Sampler {
   public:
	const int fs = I2S_SAMPLE_RATE;

	std::vector<int> sig_n;
	std::vector<audio_sample_t> sig1;
	std::vector<audio_sample_t> sig2;
	size_t sig_len = 0;
	audio_sample_t x1_prev = 0;
	audio_sample_t y1_prev = 0;
	audio_sample_t x2_prev = 0;
	audio_sample_t y2_prev = 0;
	void DCfilter(audio_sample_t* x1, audio_sample_t* x2){
		// source: https://ccrma.stanford.edu/~jos/filters/DC_Blocker_Software_Implementations.html
		y1_prev = *x1 - x1_prev + 0.995*y1_prev;    // = y1_new
		x1_prev = *x1;
		*x1 = y1_prev;

		y2_prev = *x2 - x2_prev + 0.995*y2_prev;   // = y2_new
		x2_prev = *x2;
		*x2 = y2_prev;
	}

	void load(std::string filename) {
		assert(sizeof(audio_sample_t) == sizeof(int16_t));


		std::ifstream file(filename);

		int count = 0;
		// int from = 4580;
		// int to = 4900;
		long x1_sum = 0;
		long x2_sum = 0;
		while (true) {
			auto line = read_and_split_line(file);
			if (line.size() == 0) break;
			// for (auto i : line) {
			// 	std::cout << i << ";";
			// }
			// std::cout << std::endl;
			std::vector<int> line_int;
			std::transform(line.begin(), line.end(), std::back_inserter(line_int), [](const std::string& str) { return std::stoi(str); });
			assert(line_int.size() == 2);
			count++;
			// if (count >= from && count <= to) {
			sig_n.push_back(count);
			audio_sample_t x1 = line_int.at(0);
			audio_sample_t x2 = line_int.at(1);
			x1_sum += x1;
			x2_sum += x2;
			DCfilter(&x1, &x2);
			sig1.push_back(x1);
			sig2.push_back(x2);
			// printf("%d, %d\n", line_int.at(0), line_int.at(1));
			// }
		}
		std::cout << "X1 DC offset: " << x1_sum/(double)count << std::endl;
		std::cout << "X2 DC offset: " << x2_sum/(double)count << std::endl;
		
		sig_len = count;

		file.close();
	}

	void sample(std::function<void(audio_sample_t, audio_sample_t)> callback, int start = 0, int end = -1) {
		end = end != -1 ? end : sig_len;
		for(size_t i = start; i < end; i ++){
			// printf("%d: ;", i);
			callback(this->sig1[i], this->sig2[i]);
		}
	}
};

#endif