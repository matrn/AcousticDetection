clear;
close all;

%x1 = [9752, 9704, 9560, 9640, 9688, 9704, 9656, 9912, 9720, 9664, 9704, 9472, 9552, 9328, 9152, 8864, 8296, 8296, 8328, 8216, 8576, 8536, 8056, 7952, 8192, 8272, 8040, 8168, 8632, 9064, 9296, 9624, 9600, 9464, 9280, 9344, 9648, 9784, 10224, 11064, 11336, 12784, 14088, 13496, 13848, 13512, 11928, 10864, 12304, 12632, 11168, 10760, 10496, 9840, 10336, 12072, 10968, 8912, 8720, 8600, 9336, 10840, 10216, 9024, 8880, 9088, 9840, 10720, 11184, 11352, 11192, 11432, 12176, 12616, 12240, 12176, 11840, 9200, 6568, 6752, 7360, 7520, 6576, 5960, 7088, 6920, 6072, 6088, 7288, 7728, 6416, 5624, 5744, 5136, 4576, 5656, 6712, 5672, 6696, 7824, 8504, 10448, 11072, 11544, 12840, 13816, 14800, 15792, 14448, 14208, 15888, 15816, 15568, 17168, 16448, 15480, 14792, 13040, 11280, 9040, 7232, 5584, 3232, 704, 680, 2072, 664, 16, 4888, 8120, 6728, 5288, 5616, 6736, 7776, 8072, 6832, 4888, 5896, 7984, 6960, 5392, 7008, 9328, 10312, 12072, 13576, 12872, 12352, 14248, 14848, 13096, 13424, 14544, 14424, 15176, 17200, 18800, 17008, 13064, 10928, 12488, 15128, 14144, 12456, 12248, 10376, 8816, 11520, 13048, 7640, -136, -16, 8600, 15216, 8656, -2432, 376, 11752, 9728, 3424, 8704, 9864, 7672, 8928, 13960, 13720, 14032, 13640, 14552, 21864, 24304, 20288, 16952, 18800, 25552, 30568, 29552, 23872, 17360, 15184, 11768, 8456, 6072, 7104, 8120, 7944, 6256, 6288, 5480, 5176, 4464, 4216, 4784, 3992, 3192, 312, 936, 2168, 2608, 2888, 6320, 12328, 11768, 10296, 12016, 10160, 9768, 11832, 12072, 8000, 4272, 3008, 816, 24, -752];
%x2 = [16952, 16744, 16152, 16072, 16048, 15728, 16040, 16216, 15608, 15288, 15584, 15504, 15560, 15520, 15800, 16584, 16800, 17000, 16896, 16936, 17080, 16896, 17264, 17504, 17784, 18736, 19096, 19968, 21408, 21392, 21608, 21904, 20912, 19368, 20128, 21696, 20224, 19632, 19624, 18008, 17944, 19224, 18680, 16640, 16504, 16224, 16008, 17808, 17616, 17056, 17320, 16968, 16896, 17656, 18776, 19064, 19656, 19880, 19488, 19792, 19816, 19960, 20200, 19096, 16344, 15376, 15800, 16336, 16008, 14544, 15032, 14192, 11288, 10504, 13344, 14040, 12152, 12288, 11632, 10248, 11352, 12296, 12296, 12192, 13504, 13840, 13904, 17032, 18272, 17800, 18912, 21000, 22128, 23864, 24128, 23136, 25176, 26064, 24992, 26296, 26272, 24352, 22712, 20392, 19048, 17832, 16008, 14496, 12216, 10192, 9248, 9704, 8872, 7280, 10880, 14392, 12248, 11072, 11936, 12672, 14640, 15776, 15296, 14008, 15192, 16640, 15656, 14648, 15256, 17696, 19232, 20744, 21832, 20896, 20592, 22912, 25216, 24624, 24504, 25792, 23496, 22232, 24648, 26216, 24232, 19720, 16296, 14752, 16688, 18488, 18576, 18512, 16120, 14360, 17208, 16696, 12600, 8040, 5736, 11264, 19592, 16064, 3384, 3200, 16008, 17240, 9600, 13856, 15352, 12720, 12856, 17672, 19152, 23880, 29176, 24416, 27288, 32184, 32744, 30392, 31976, 26712, 24456, 28856, 28656, 24136, 24600, 24304, 21544, 22816, 24416, 21144, 17440, 13616, 14344, 14328, 11240, 7320, 10456, 17248, 10880, 3528, 3384, 6344, 7688, 7680, 14968, 17016, 12512, 11472, 13472, 15416, 15672, 16536, 17688, 18600, 20008, 19000, 14112, 15824, 16512, 14616, 16112, 19808, 18576, 13536, 10432, 7376, 7200, 9536, 11992, 8992, 9120, 12320, 11256, 12552];
x1 = [-7985, -12761, -10905, -7090, -3734, -1219, -748, -864, -699, -2759, -5457, -6317, -7341, -8760, -6500, -3579, -4129, -6692, -10722, -9676, -5323, -3256, -4935, -8062, -8525, -6386, -3370, -2057, -3518, -6620, -7362, -3405, 2620, 4766, 3094, 1654, 4661, 8173, 3460, -1117, -1239, -352, 545, 3894, 6330, 4122, 3941, 5841, 2723, -1186, 1803, 6337, 5537, 5549, 9225, 7570, 2636, 2646, 6296, 5856, 7986, 9106, 5676, 6711, 9389, 7254, 3577, 7671, 12016, 14467, 13882, 9828, 8114, 5465, 5837, 7399, 5482, 3894, 6362, 5906, 2788, 1334, -256, 2521, 7900, 7244, 4919, 2406, -550, -2483, -4214, -2200, -1541, -3381, -4332, -5278, -8411, -11112, -11784, -12317, -12327, -10665, -10235, -9911, -8333, -6771, -7545, -6987, -4880, -5583, -8083, -9954, -6752, -3206, -2485, -208, 1497, 1889, 1327, -1575, -1375, 695, 75, 698, 1254, 519, 2012, 1777, 184, -1720, -3183, -2071, 1155, 2205, -2102, -6547, -6786, -384, 3825, 2781, 2743, 3289, 4952, 8799, 10547, 5134, 1708, 6555, 5746, 4637, 5109, 1435, 4299, 3653, 2658, 3660, 3585, 447, -1331, 2059, 1528, 1616, 1071, -430, -739, 2600, 3371, 2682, 1628, 1139, -386, -3472, -2878, -1959, -1421, -565, -2322, -6494, -6509, -5348, -5649, -4516, -5325, -4786, -682, 905, -323, -4017, -4468, -1861, -891, -3838, -6258, -2274, -1270, -2319, -2163, -4096, -5315, -3536, 1281, 1594, 1634, 4017, 2948, 365, 2883, 8340, 7930, 6626, 7576, 6634, 6688, 8582, 5971, 2157, 2930, 5267, 6760, 5446, 890, 1541, 3933, 1113, -1564, -2684, -3358, -1021, 848, -1860, -2610, -2068, -2417, -1460, -1572, -1484, 363, -1918, -4644, -4140, -1647, 953, 916, 359, -394, -1128, -130, 1550, 2014, 1507, 323, -22, 370, -1207, -3448, -3670, -3659, -2336, -1492, -1388, -1165, -431, 1491, 3051, 2315, 1055, -62, -1973, -1675, -3114, -5266, -2479, -722, -3478, -1188, 417, -2961, -3746, -3247, -3438, -1852, 21, -291, -1953, -3167, -3943, -5587, -3791, -1252, 514, 1279, -367, 1250, 1547, -2028, -2841, 13, 3956, 4584, 3833, 1189, -2960, -5105, -5831, -5737, -3284, -1843, -2553, -3196, -1988, 517, -117, -684, 1831, 3957, 2937, 2778, 492, -2030, -1491, 2164, 5033, 3191, 695, 131, 1970, 3224, 3071, 2519, 3162, 6138, 4715, 715, 4063, 2186, -4616, -2688, -1298, -3251, -1090, 627, -3496, -3534, 2011, -1511, -4959, -1742, 210, 1280, 289, -2744, -1922, 727, 3979, 4567, 2848, 3273, 4064, 4067, 910, 2825, 4666, 4482, 5035, 3529, 2247, 2763, 1133, -1784, -1591, 1240, 1345, -1261, 329, 2903, 2328, -1787, -4626, -4050, -2573, -816, -1707, -1962, 135, 622, 938, 4109, 4728, 2544, 1451, 2651, 3877, 17, -2175, -748, 1383, 1176, 106, 3561, 5743, ];
x2 = [-8117, -5676, -3439, -4813, -6540, -6211, -7283, -6166, -3239, -3702, -3891, -4807, -5230, -1939, -2097, -2502, -89, -1224, -5913, -5707, -2030, -579, 1343, 2456, 4747, 8683, 6151, 304, 222, 1668, 3315, 5522, 6598, 6917, 3362, -710, 2261, 297, -5152, 1521, 3401, -1736, 1528, 3904, -1411, -3595, 1270, 3167, 4063, 8138, 4169, 2268, 5488, 4220, 5270, 7643, 8860, 10383, 12987, 12610, 11226, 10921, 11426, 10512, 7755, 8892, 8407, 10316, 9208, 5833, 5539, 3383, 1318, 71, -921, -2700, -3606, -1795, 1445, -82, -2753, -5227, -6696, -6766, -7372, -6759, -5765, -7712, -10705, -7419, -4493, -7190, -9882, -9200, -8410, -7343, -4202, -4204, -5686, -6041, -6962, -6927, -4476, -3549, -4627, -4851, -3642, -2903, -3176, -2000, -230, 779, -840, -1251, 1427, 3099, 1491, -844, -95, 105, -679, 748, 1808, 3406, 3260, 139, 1538, 4522, 1619, -717, 1582, 4782, 4174, 2937, 6322, 7298, 4853, 2836, 2893, -1305, -3442, 1383, -463, -668, 3807, 891, 1142, 4424, 4001, 2508, 4327, 4705, 3729, 6494, 4277, 959, 1866, 2472, 643, 2839, 3768, 2397, 441, -225, -1295, -1264, -3121, -6417, -5584, -7820, -8764, -10248, -12348, -9958, -6516, -5595, -6575, -6678, -3820, -2488, -3755, -5416, -4612, -1204, 1410, 2, -510, 4196, 4503, 744, 156, 2107, 2840, 3089, 2937, 1394, 2715, 2765, 775, 2699, 2341, 1881, 4071, 2658, 2636, 1326, -864, 2284, 6216, 3688, -834, 762, 3510, 3916, 2384, 796, 16, 1175, 3657, 2526, 1937, 2655, 3273, 6240, 7680, 8385, 9199, 6841, 3910, 2514, 3277, 2932, 2037, 3602, 2095, 1948, 2482, 2029, 2730, 1156, 646, 1874, 1216, -990, -4913, -5368, -3981, -5897, -6915, -6352, -6528, -7687, -8688, -8508, -7081, -5101, -3451, -3817, -4893, -6372, -6284, -9756, -11955, -7639, -5896, -5874, -4220, -3126, -1038, 3159, 3143, 4759, 7343, 6018, 4571, 5068, 8378, 9288, 7321, 7212, 8495, 6828, 6345, 7217, 5620, 4223, 2305, 613, 41, 24, -552, -837, -384, -2414, -4481, -4242, -6732, -8746, -7446, -7376, -6963, -8720, -11292, -7475, -3677, -3802, -1958, 1331, 1980, 1210, 1355, 1164, 1350, 2335, 3339, 2210, -1969, -2143, -932, -8663, -8275, -329, -2495, -5586, -2406, 1198, -1727, -2006, 1420, -1507, -4811, -2306, 1729, -583, -2148, -513, -534, 804, 327, -2762, -3572, -410, 1656, 1047, 2105, 5678, 7161, 4861, 4668, 2876, -3738, -1975, 4754, -453, -1882, 2615, 649, 749, 4617, 7649, 6458, 5537, 4093, 5144, 7558, 5120, 4830, 4461, 5254, 4739, 3627, 4520, 2777, -332, 205, 3515, 1889, -728, -2828, -2685, -2407, -3250, -3025, -2689, -1691, -2466, -3629, -4074, -3373, -1476, -1828, -3242, -2737, -523, 399, 301, 2019, 3112, 3280, 4399, ];

%x1 = x1-mean(x1);
%x2 = x2-mean(x2);

%x1 = x1/max(abs(x1));
%x2 = x2/max(abs(x2));

x1_shift = 0; %4;
x1 = [zeros(1, x1_shift) x1(1:end-x1_shift)];
x2_shift = 0; %13;
x2 = [zeros(1, x2_shift) x2(1:end-x2_shift)];

figure(1);
TL = tiledlayout(3,2);
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

%figure(2);
%TL = tiledlayout(2, 1);

nexttile;
plot(lag, rk_biased);
title('biased');
grid on;

nexttile;
plot(lag, rk_unbiased);
title('unbiased');
grid on;

nexttile([1, 2]);
[tau_est,R,lags] = gccphat(x1', x2'); %, fs);
plot(lags, R);
title("GCC-PHAT");
xlabel('k [-]');
ylabel('R[k]');
%xlim([-1024,1024]);
disp(tau_est);


%nexttile;
%plot(lag, rk_normalized);