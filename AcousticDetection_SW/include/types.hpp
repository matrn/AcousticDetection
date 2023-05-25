#ifndef MCU_TYPES_HPP
#define MCU_TYPES_HPP

#include <cstdint>

typedef int32_t i2s_sample_t; 
typedef int16_t audio_sample_t;


// returned from cross correlation function (CCF)
typedef struct xcorr_result {
	int max_pos;   // position of maximum of the cross correlation function Rx
	double interpolated_max_pos;   // set if interpolation is used
	double max_Rx;   // value of Rx in max_pos
} xcorr_result_t;


#endif