clear;

fs=44100;

fileID = fopen('data/r_pos.raw','r');
raw = fread(fileID, 'int16');
samples = reshape(raw, 2, [])';
fclose(fileID);

left = samples(:,1);
right = samples(:,2);


zero = 319766-1024;
left = left(zero:zero+1024);
right = right(zero:zero+1024);

figure(1);
clf();
TL = tiledlayout(7,1);
nexttile;
plot(left);
hold on;
plot(right);
title("Signály z mikrofonů");
legend("Levý", "Pravý");
xlabel("N [-]");
ylabel('Value [-]');
xlim([0,1024]);


N = length(left);
lag = -N+1:1:N-1
Rk_biased = xcorr(left, right, 'biased');
Rk_unbiased = xcorr(left, right, 'unbiased');
Rk_normalized = xcorr(left, right, 'normalized');
Rk_none = xcorr(left, right, 'none');

%{
nexttile;
LEFT = fft(left);
ff = [0:N-1].*(fs/N);
stem(ff, abs(LEFT));
title("FFT - left");
xlabel('k [-]');
ylabel('R[k]');
%xlim([-1024,1024]);
%}

nexttile;
plot(lag, abs(xcorr(left, 'none')));
title("ACF - left");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);

nexttile;
plot(lag, abs(xcorr(right, 'none')));
title("ACF - right");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);

nexttile;
plot(lag, abs(Rk_biased));
title("Vychýlený odhad vzájemné korelační funkce");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);


nexttile;
plot(lag, abs(Rk_unbiased));
title("Nevychýlený odhad vzájemné korelační funkce");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);

nexttile;
plot(lag, abs(Rk_normalized));
title("normalized xcorr");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);

%{
nexttile;
plot(lag, abs(Rk_none));
title("none xcorr");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);
%}

nexttile;
[tau_est,R,lags] = gccphat(left, right); %, fs);
plot(lags, R);
title("GCC-PHAT");
xlabel('k [-]');
ylabel('R[k]');
xlim([-1024,1024]);
disp(tau_est);


%%
left = left/max(abs(left));
right = right/max(abs(right));
figure(1);
clf();
plot(left);
hold on;
plot(right);


%%
left = left/max(abs(left));
right = right/max(abs(right));
figure(1);
clf();
plot(left);
hold on;
plot(right);
%%
M =10;
b = (1/(M+1))*ones(1, M+1);
lf = filter(b, 1, left);
rf = filter(b, 1, right);


figure(1);
hold on;
plot(lf);
hold on;
plot(rf);
%%
figure(1);
plot(left);

N = 5000;
window = hamming(N);
figure(2);
spectrogram(left, window, [], fs, 'yaxis');
colorbar off;
%%
M =10;
b = (1/(M+1))*ones(1, M+1);
xma = filter(b, 1, left);
figure(1);
plot(xma);
%%
figure(1);
clf();
plot(left);
%hold on;
%plot(right);
xh1 = hilbert(left);
obalka = abs(xh1);
hold on;
plot(obalka, ':');
hold on;
plot(-obalka, ':');

figure(2);
sig = left;
M =10;
a = 1;
b = (1/(M+1))*ones(1, M+1);
xma = filter(b, a, abs(sig).^2);
plot(sig);
hold on;
plot(sqrt(xma));
%spectrogram(left, 44100, 'yaxis');
