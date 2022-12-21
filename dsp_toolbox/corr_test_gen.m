x1 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0];
x2 = [0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];


figure(1);
TL = tiledlayout(2,2);
nexttile([1, 2]);
plot(x1);
hold on;
plot(x2);
legend('x1', 'x2');
grid on;

rk_biased = abs(xcorr(x1, x2, 'biased'));
rk_unbiased = abs(xcorr(x1, x2, 'unbiased'));
%rk_normalized = xcorr(x1, x2, 'normalized');

N = length(x1);
lag = -N+1:1:N-1

nexttile;
plot(lag, rk_biased);
title('biased');
grid on;

nexttile;
plot(lag, rk_unbiased);
title('unbiased');
grid on;