#include <cstdio>
#include <sciplot/sciplot.hpp>
#include <vector>
#include "../AcousticDetection_SW/lib/dsp/dsp.hpp"


const int x1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};
const int x2[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const std::vector<int> lag = {-11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const std::vector<double> rk_biased = {0, 0.83333333333333492, 2.4166666666666683, 4.6666666666666661, 7.5, 10.833333333333334, 14.583333333333334, 18.666666666666668, 23, 27.5, 32.083333333333336, 27.5, 23, 18.666666666666668, 14.583333333333334, 10.833333333333334, 7.5, 4.6666666666666661, 2.4166666666666665, 0.833333333333334, 7.894919286223335E-16, 1.578983857244667E-15, 1.578983857244667E-15};
const std::vector<double> rk_unbiased = {0, 5.00000000000001, 9.6666666666666732, 13.999999999999998, 18, 21.666666666666668, 25, 28, 30.666666666666668, 33, 35, 27.5, 25.09090909090909, 22.4, 19.444444444444443, 16.25, 12.857142857142858, 9.3333333333333321, 5.8, 2.5000000000000022, 3.157967714489334E-15, 9.473903143468002E-15, 1.8947806286936004E-14};


int main() {
	DSP dsp;
	std::pair<std::vector<int>, std::vector<double>> cpp_biased;
	std::pair<std::vector<int>, std::vector<double>> cpp_unbiased;
	
	// /*
	// Rx[k] = 1/N * sum from n=0 to N-1-k of x[n]*x[n+k]
	// */
	// int N = sizeof(x1)/sizeof(int);
	// bool biased = true;
	// printf("N=%d\n", N);
	// for (int k = -(N-1); k <= N-1; k++) {
	// 	int Rx_sum = 0;
	// 	if(k < 0){
	// 		for (int n = 0; n < N - (-k); n++) {
	// 			Rx_sum += x2[n-k] * x1[n];
	// 		}
	// 	} else {
	// 		for (int n = 0; n < N - k; n++) {
	// 			Rx_sum += x1[n+k] * x2[n];
	// 		}
	// 	}

	// 	double Rx_k = Rx_sum / (double)(biased ? N : (N - abs(k)));
	// 	k_vec.push_back(k);
	// 	Rx_vec.push_back(Rx_k);	
	// }


	// Create a Plot object
	sciplot::Plot2D plot;

	plot.size(1000, 1000);

	// Set the font name and size
	plot.fontName("Palatino");
	plot.fontSize(12);

	// Set the x and y labels
	plot.xlabel("k");
	plot.ylabel("Rx[k]");

	// Set some options for the legend
	plot.legend()
		.atOutsideBottom()
		.fontSize(10)
		.displayHorizontal()
		.displayExpandWidthBy(2);

	cpp_biased = dsp.xcorr_complete<const int[]>(x1, x2, sizeof(x1)/sizeof(int), -1, true);
	cpp_unbiased = dsp.xcorr_complete<const int[]>(x1, x2, sizeof(x1)/sizeof(int), -1, false);
	plot.drawCurve(cpp_biased.first, cpp_biased.second).label("Cpp_{biased}");
	plot.drawCurve(cpp_unbiased.first, cpp_unbiased.second).label("Cpp_{unbiased}");
	plot.drawCurve(lag, rk_biased).label("matlab_{biased}");
	plot.drawCurve(lag, rk_unbiased).label("matlab_{unbiased}");

	// Create figure to hold plot
	sciplot::Figure fig = {{plot}};
	// Create canvas to hold figure
	sciplot::Canvas canvas = {{fig}};

	// Show the plot in a pop-up window
	canvas.show();

	std::pair<int, double> max_biased;
	std::pair<int, double> max_unbiased;
	bool biased_found, unbiased_found;
	biased_found = dsp.xcorr_max<const int[]>(x1, x2, max_biased, sizeof(x1)/sizeof(int), -1, true);
	unbiased_found = dsp.xcorr_max<const int[]>(x1, x2, max_unbiased, sizeof(x1)/sizeof(int), -1, false);
	printf("%d BIASED max: k=%d, Rk=%f\n", biased_found, max_biased.first, max_biased.second);
	printf("%d UNBIASED max: k=%d, Rk=%f\n", unbiased_found, max_unbiased.first, max_unbiased.second);
	return 0;
}