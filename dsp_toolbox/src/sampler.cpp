#include "include/sampler.hpp"

int16_t MSB_2bytes_to_int16(char *bytes) {
	union int16_cast num;
	memcpy(&num.bytes, bytes, sizeof(num.value));

	if (IS_BIG_ENDIAN)
		return num.value;
	else
		return __bswap_16(num.value);
}

int16_t Sampler::read_int16(std::ifstream &file) {
	char data[2];
	file.read(data, 2);
	return MSB_2bytes_to_int16(data);
}

int Sampler::open(std::string filename) {
	assert(sizeof(audio_sample_t) == sizeof(int16_t));

	file.open(filename, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "ERROR: Unable to open file " << filename << std::endl;
		return -1;
	}
	// get length of file:
	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);
	printf("FILE LEN: %d\n", length);

	if (length % 4 != 0) {
		std::cerr << "ERROR: Wrong file size, it should be %4=0\n";
		return -1;
	}

	this->number_of_samples = length / 2;

	return 0;
}

void Sampler::sample(std::function<void(audio_sample_t, audio_sample_t)> callback, int start, int end) {
	/*
	start & end is 2channel size, so number of samples is *2
	*/
	end = end != -1 ? end : number_of_samples / 2;

	if ((unsigned int)end > number_of_samples / 2) std::cerr << "ERROR: file size (samples): " << number_of_samples << " is to small for the set end (samples): " << end * 2;

	if (start != 0) file.seekg(SAMPLE_SIZE * start * 2);  // two channels
	for (int i = start; i < end; i++) {
		audio_sample_t left = this->read_int16(file);
		audio_sample_t right = this->read_int16(file);
		if (file.eof()) puts("EOF");

		callback(left, right);
	}
}

void Sampler::close() {
	file.close();
}
