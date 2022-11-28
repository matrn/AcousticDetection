# pip3 install numpy scipy

import numpy as np
from scipy.io.wavfile import write

FILENAME = 't.csv'


def get_sample_rate():
	with open('../AcousticDetection_SW/lib/i2s_mic/config.hpp', 'r') as f:
		for line in f.read().splitlines():
			if 'I2S_SAMPLE_RATE' in line:
				return int(line.split('I2S_SAMPLE_RATE ')[1].split(' ')[0])
	raise Exception('Bitrate not parsed')
SAMPLE_RATE = get_sample_rate() #44100

print(f'Reading `{FILENAME}` with sample rate {SAMPLE_RATE} Hz')
with open(FILENAME, 'r') as f:
	lines = f.read().splitlines()
	data = np.ndarray([len(lines), 2], dtype=np.int16)
	for i,line in enumerate(lines):
		sp = line.split(',')
		try:
			sp = [int(s) for s in sp]
		except ValueError:
			print(f'ERR: >{line}< parse failed')
			continue
		assert(len(sp) == 2)
		data[i] = [sp[0], sp[1]]
print(data.shape)
#data = np.random.uniform(-1, 1, rate) # 1 second worth of random samples between -1 and 1
#scaled = np.int16(data / np.max(np.abs(data)) * 32767)
write('test.wav', SAMPLE_RATE, data)