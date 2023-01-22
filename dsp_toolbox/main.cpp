#include <cassert>
#include <iostream>
#include <sciplot/sciplot.hpp>
#include <vector>

#include "../AcousticDetection_SW/lib/dsp/dsp.hpp"
#include "src/include/sampler.hpp"



// #define START 177923
// #define END 178159
// #define START 178300
// #define END 178700
#define START 51147
#define END 52843


//#define CORR_SIZE END-START
#define CORR_SIZE correlation_window_samples_num


#define FILENAME "data/r_pos.raw"


int main(){//int argc, char** argv) {
	Sampler sampler;

	CircularBuffer<audio_sample_t, CORR_SIZE> x1;
	CircularBuffer<audio_sample_t, CORR_SIZE> x2;


	CircularBuffer<uint32_t, 15> x1_MA;
	CircularBuffer<uint32_t, 15> x2_MA;


	DSP dsp;

	if(sampler.open(FILENAME) != 0){
		std::cerr << "ERROR starting sampler\n";
		return 1;
	}

	std::vector<int> sig_n;
	std::vector<audio_sample_t> sig1;
	std::vector<audio_sample_t> sig2;

	int qq = 0;
	int count = 0;
	sampler.sample([&](audio_sample_t left, audio_sample_t right) {   //x1, &x2, &dsp, &qq, &count, &sig_n, &sig1, &sig2
		sig_n.push_back((count++) + START);
		sig1.push_back(left);
		sig2.push_back(right);

		x1.push(left);
		x2.push(right);

		x1_MA.push(left*left);
		x2_MA.push(right*right);
		printf("MA %f %f\n", x1_MA.avg(), x1_MA.avg());


		// printf("%d, %d\n", left, right);
		// if(count%100 != 0) continue;
		if(qq++ >= 512){
			qq = 0;
		
			std::pair<int, double> xcorr_peak;
			bool xcorr_peak_found = dsp.xcorr_max<CircularBuffer<audio_sample_t, correlation_window_samples_num>>(x1, x2, xcorr_peak, correlation_window_samples_num, max_shift_samples_num+1, true);
			
			if(xcorr_peak_found){
				
				double tau = xcorr_peak.first*(1./I2S_SAMPLE_RATE);
				int angle = dsp.rad2deg( acos((tau*sound_speed)/mics_distance) )+0.5;
				//char str[10];
				
				//sprintf(str, "%d", angle);
				printf("N: %d, max: %f, tau: %f s, angle: %d\n", xcorr_peak.first, xcorr_peak.second, tau, angle);
			}
		}
	}, START, END);

	sampler.close();

	//return 0;

	std::vector<int> ssn;
	std::vector<double> ssv;
	int N = CORR_SIZE;
	int max_k = N; //max_shift_samples_num+1;
	

	//double max_Rx = 0;
	//int max_Rx_pos = -1;
	for (int k = 0; k < max_k; k++) {
		int Rx_sum = 0;
		for (int n = 0; n < N - k; n++) {
			Rx_sum += x2[n] * x1[n + k];
		}
		double Rx_k = Rx_sum/N;// / (double)(N - k);
		ssn.push_back(k);
		ssv.push_back(Rx_k/2000);
		// if (Rx_k > max_Rx) {
		// 	max_Rx = Rx_k;
		// 	max_Rx_pos = k;
		// }
		// Serial.println(Rx_k);
	}
	
	/*
	std::cout << "x1 = [";
	for(unsigned int i = 0; i < x1.size(); i ++) printf("%d, ", x1[i]);
	std::cout << "];\n";

	std::cout << "x2 = [";
	for(unsigned int i = 0; i < x2.size(); i ++) printf("%d, ", x2[i]);
	std::cout << "];\n";
	*/

	// std::ofstream file;
	// file.open("/home/matej/Documents/FEL/5.semestr/TPS/Miniprojekt/assets/plots/DC_filtered_data.dat");
	// std::cout << "X1" << std::endl;
	// for(int i = 0; i < x1.size(); i ++) file << i << ',' << x1[i] << std::endl;
	// file.close();

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
		.atOutsideBottom()
		.fontSize(10)
		.displayHorizontal()
		.displayExpandWidthBy(2);

	int crop_start = START; //4580;
	int crop_end = END; sig_n.size(); ///4700;
	std::vector<int> crop_n(&sig_n[crop_start], &sig_n[crop_end]);
	std::vector<audio_sample_t> crop_sig1(&sig1[crop_start], &sig1[crop_end]);
	std::vector<audio_sample_t> crop_sig2(&sig2[crop_start], &sig2[crop_end]);

	std::cout << "A" << std::endl;
	// plot.drawCurve(crop_n, crop_sig1).label("#1");
	// plot.drawCurve(crop_n, crop_sig2).label("#2");
	plot.drawCurve(ssn, x1).label("#1");
	plot.drawCurve(ssn, x2).label("#2");
	plot.drawCurve(ssn, ssv).label("Rx");

	// Create figure to hold plot
	sciplot::Figure fig = {{plot}};
	// Create canvas to hold figure
	sciplot::Canvas canvas = {{fig}};

	// Show the plot in a pop-up window
	canvas.show();

	// Save the plot to a PDF file
	canvas.save("out.pdf");
}
