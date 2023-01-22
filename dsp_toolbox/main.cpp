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
// #define START 76650//1000//51147
// #define END   80000//76800//100000 //52843
#define START 0
#define END -1

//#define CORR_SIZE END-START
#define CORR_SIZE correlation_window_samples_num


#define FILENAME "data/r_pos.raw"


#define Z_SIZE 20

int main(){//int argc, char** argv) {
	Sampler sampler;

	CircularBuffer<audio_sample_t, CORR_SIZE> x1;
	CircularBuffer<audio_sample_t, CORR_SIZE> x2;


	CircularBuffer<int16_t, Z_SIZE> x1_z;
	CircularBuffer<int16_t, Z_SIZE> x2_z;

	CircularBuffer<int16_t, 2> x1_drv;
	CircularBuffer<int16_t, 2> x2_drv;

	DSP dsp;

	if(sampler.open(FILENAME) != 0){
		std::cerr << "ERROR starting sampler\n";
		return 1; 
	}

	std::vector<int> sig_n;
	std::vector<audio_sample_t> sig1;
	std::vector<audio_sample_t> sig2;

	std::vector<uint32_t> sig1_energy;
	std::vector<uint32_t> sig2_energy;

	std::vector<double> sig1_mean;
	std::vector<double> sig2_mean;

	std::vector<double> sig1_envelope;
	std::vector<double> sig2_envelope;
	
	std::vector<bool> sig1_onset;
	std::vector<bool> sig2_onset;

	int qq = 0;
	int count = 0;
	bool start_grab = false;
	double sum1 = 0;
	double sum2 = 0;
	sampler.sample([&](audio_sample_t left, audio_sample_t right) {   //x1, &x2, &dsp, &qq, &count, &sig_n, &sig1, &sig2
		int pos = (count++) + START;
		sig_n.push_back(pos);
		sig1.push_back(abs(left));
		sig2.push_back(abs(right));

		uint32_t left_energy = left*left;
		uint32_t right_energy = right*right;

		x1.push(left);
		x2.push(right);

		x1_drv.push(left);
		x2_drv.push(right);

		x1_z.push(abs(left));
		x2_z.push(abs(right));

		double x1_mean = x1_z.avg();
		double x2_mean = x2_z.avg();

		double x1_std = sqrt(x1_z.var(x1_mean));
		double x2_std = sqrt(x2_z.var(x2_mean));

		//printf("mean %f %f, std %f %f\n", x1_mean, x2_mean, x1_std, x2_std);
		

		sig1_energy.push_back(left_energy);
		sig2_energy.push_back(right_energy);

		sig1_mean.push_back(x1_mean);
		sig2_mean.push_back(x2_mean);

		double x1_e = x1_mean + x1_std; //pow(1*(sqrt(x1_mean) + pow(x1_std, 1/2)), 2);
		double x2_e = x2_mean + x2_std; //pow(1*(sqrt(x2_mean) + pow(x2_std, 1/2)), 2);
		double p = 0.2;
		sum1 = (p*abs(left)+sum1*(1-p))/2;
		sum2 = (p*abs(right)+sum2*(1-p))/2;
		sig1_envelope.push_back(sum1);
		sig2_envelope.push_back(sum2);
		//sig1_envelope.push_back(x1_drv[1]-x1_drv[0]);
		//sig2_envelope.push_back(x2_drv[1]-x2_drv[0]);
		//printf("MA %f %f\n", x1_MA.avg(), x1_MA.avg());

		bool detect = false;
		
		sig1_onset.push_back(sum1 > 500);
		sig2_onset.push_back(sum2 > 500);

		// if(left_energy > 30*left_energy_avg && right_energy > 30*right_energy_avg)
		// 	detect = true;

		if(sum1 > 500 && sum2>500) detect = true;

		// printf("%d, %d\n", left, right);
		// if(count%100 != 0) continue;
		// if(qq++ >= 512){
		// 	qq = 0;
		if(detect && start_grab == false){
			start_grab = true;
			printf("Start grab: %d\n", pos);
		}
		if(start_grab && ++qq >= CORR_SIZE){
			printf("Start processing: %d\n", pos);
			qq = 0;
			start_grab = false;
			if(pos == 319766 && false){
				sciplot::Plot2D plot_sig;
				plot_sig.xlabel("N");
				plot_sig.ylabel("val");
				plot_sig.drawCurve(sig_n, x1).label("#1_{value}");
				plot_sig.drawCurve(sig_n, x2).label("#2_{value}");

				std::pair<std::vector<int>, std::vector<double>> rk_biased = dsp.xcorr_complete<CircularBuffer<audio_sample_t, CORR_SIZE>>(x1, x2, CORR_SIZE, true);
				sciplot::Plot2D plot_corr_biased;
				plot_corr_biased.xlabel("k");
				plot_corr_biased.ylabel("Rx");
				plot_corr_biased.drawCurve(rk_biased.first, rk_biased.second).label("Rx_{biased}");
				
				std::pair<std::vector<int>, std::vector<double>> rk_unbiased = dsp.xcorr_complete<CircularBuffer<audio_sample_t, CORR_SIZE>>(x1, x2, CORR_SIZE, false);
				sciplot::Plot2D plot_corr_unbiased;
				plot_corr_unbiased.xlabel("k");
				plot_corr_unbiased.ylabel("Rx");
				plot_corr_unbiased.drawCurve(rk_unbiased.first, rk_unbiased.second).label("Rx_{unbiased}");
				
				sciplot::Figure fig = {{plot_sig}, {plot_corr_biased}, {plot_corr_unbiased}};	// Create figure to hold plot
				sciplot::Canvas canvas = {{fig}};	// Create canvas to hold figure

				// Show the plot in a pop-up window
				canvas.size(1000, 600);
				canvas.show();
			}
			//-----
			std::pair<int, double> xcorr_peak;
			bool xcorr_peak_found = dsp.xcorr_max<CircularBuffer<audio_sample_t, correlation_window_samples_num>>(x1, x2, xcorr_peak, correlation_window_samples_num, max_shift_samples_num+1, true);
			
			if(xcorr_peak_found){				
				double tau = xcorr_peak.first*(1./I2S_SAMPLE_RATE);
				int angle = dsp.rad2deg( acos((tau*sound_speed)/mics_distance) )+0.5;
				//char str[10];
				
				//sprintf(str, "%d", angle);
				printf("pos: %d, N: %d, max: %f, tau: %f s, angle: %d\n", pos, xcorr_peak.first, xcorr_peak.second, tau, angle);
			}
			
		}
	}, START, END);

	sampler.close();

	return 0;

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
	sciplot::Plot2D plot_signals;

	//plot.size(1000, 1000);

	// Set the font name and size
	//plot.fontName("Palatino");
	//plot.fontSize(12);

	// Set the x and y labels
	plot_signals.xlabel("N");
	plot_signals.ylabel("val");

	// Set some options for the legend
	// plot.legend()
	// 	.atOutsideBottom()
	// 	//.fontSize(10)
	// 	.displayHorizontal();
	// 	//.displayExpandWidthBy(2);

	std::cout << "A" << std::endl;
	// plot.drawCurve(crop_n, crop_sig1).label("#1");
	// plot.drawCurve(crop_n, crop_sig2).label("#2");
	plot_signals.drawCurve(sig_n, sig1).label("#1_{value}");
	plot_signals.drawCurve(sig_n, sig2).label("#2_{value}");
	plot_signals.drawCurve(sig_n, sig1_mean).label("#1_{mean}");
	plot_signals.drawCurve(sig_n, sig2_mean).label("#2_{mean}");
	plot_signals.drawCurve(sig_n, sig1_envelope).label("#1_{envelope}");
	plot_signals.drawCurve(sig_n, sig2_envelope).label("#2_{envelope}");

	sciplot::Plot2D plot_energy;
	plot_energy.xlabel("N");
	plot_energy.ylabel("val^2");
	plot_energy.drawCurve(sig_n, sig1_energy).label("#1_{energy}");
	plot_energy.drawCurve(sig_n, sig2_energy).label("#2_{energy}");

	sciplot::Plot2D plot_energy_MA;
	plot_energy_MA.xlabel("N");
	plot_energy_MA.ylabel("val^2");
	plot_energy_MA.drawCurve(sig_n, sig1_envelope).label("#1_{env}");
	plot_energy_MA.drawCurve(sig_n, sig2_envelope).label("#2_{env}");

	sciplot::Plot2D plot_onset;
	plot_onset.xlabel("N");
	plot_onset.ylabel("bool");
	plot_onset.drawCurve(sig_n, sig1_onset).label("#1_{onset}");
	plot_onset.drawCurve(sig_n, sig2_onset).label("#2_{onset}");

	//plot.drawCurve(sig_n, ssv).label("Rx");

	
	sciplot::Figure fig = {{plot_signals}, {plot_energy}, {plot_energy_MA}, {plot_onset}};	// Create figure to hold plot
	sciplot::Canvas canvas = {{fig}};	// Create canvas to hold figure

	// Show the plot in a pop-up window
	canvas.size(1000, 600);
	canvas.show();

	// Save the plot to a PDF file
	//canvas.save("out.pdf");
}
