#include <cassert>
#include <iostream>
#include <sciplot/sciplot.hpp>
#include <vector>

#include "../AcousticDetection_SW/lib/dsp/dsp.hpp"
#include "src/include/sampler.hpp"



int main(int argc, char** argv) {
	Sampler sampler;

CircularBuffer<audio_sample_t, correlation_window_samples_num> x1;
CircularBuffer<audio_sample_t, correlation_window_samples_num> x2;

DSP dsp;

	sampler.load("data.csv");
	std::cout << sampler.sig_len << std::endl;

	sampler.sample([&x1, &x2, &dsp](audio_sample_t left, audio_sample_t right) {
		x1.push(left);
		x2.push(right);

		// printf("%d, %d\n", left, right);
		// if(count%100 != 0) continue;
		int N = -1;

		auto corr1 = dsp.correlation<correlation_window_samples_num>(x1, x2, correlation_window_samples_num, max_shift_samples_num + 1);
		auto corr2 = dsp.correlation<correlation_window_samples_num>(x2, x1, correlation_window_samples_num, max_shift_samples_num + 1);

		if (corr1.first != -1 && corr2.first != -1) {
			if (corr1.second > corr2.second) N = corr1.first;
			else N = -corr2.first;
		} else if (corr1.first != -1) N = corr1.first;
		else if (corr2.first != -1) N = corr2.first;
		if (N != -1 && N != 0) {			
			double tau = N * (1. / I2S_SAMPLE_RATE);
			int angle = dsp.rad2deg(acos((tau * sound_speed) / mics_distance)) + 0.5;
			
			//std::cout << angle << std::endl;
			char str[10];

			//sprintf(str, "%d", angle);
			//printf("%d, %f s, %d\n", N, tau, angle);
		}
	}, 4580, 4900);

	// Create a Plot object
	sciplot::Plot2D plot;

	plot.size(1000, 1000);

	// Set the font name and size
	plot.fontName("Palatino");
	plot.fontSize(12);

	// Set the x and y labels
	plot.xlabel("N");
	plot.ylabel("val");

	// Set some options for the legend
	plot.legend()
		.atTop()
		.fontSize(10)
		.displayHorizontal()
		.displayExpandWidthBy(2);

	
	int crop_start = 4580;
	int crop_end = 4700;
	std::vector<int> crop_n(&sampler.sig_n[crop_start], &sampler.sig_n[crop_end]);
	std::vector<audio_sample_t> crop_sig1(&sampler.sig1[crop_start], &sampler.sig1[crop_end]);
	std::vector<audio_sample_t> crop_sig2(&sampler.sig2[crop_start], &sampler.sig2[crop_end]);

	std::cout << "A" << std::endl;
	plot.drawCurve(crop_n, crop_sig1).label("#1");;
	plot.drawCurve(crop_n, crop_sig2).label("#2");

	// Create figure to hold plot
	sciplot::Figure fig = {{plot}};
	// Create canvas to hold figure
	sciplot::Canvas canvas = {{fig}};

	// Show the plot in a pop-up window
	canvas.show();

	// Save the plot to a PDF file
	canvas.save("out.pdf");
}
