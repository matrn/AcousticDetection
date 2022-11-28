import asyncio
import tornado.web, tornado.autoreload
import numpy as np
import struct

PORT = 5005
file = None
file_raw = None
FILENAME = "t.csv"
FILENAME_RAW = "t.raw"

class MainHandler(tornado.web.RequestHandler):
	def post(self):
		global file, file_raw

		raw_data = self.request.body
		file_raw.write(raw_data)
		file_raw.flush()
		#print(raw_data, len(raw_data))
		print(len(raw_data))
		if len(raw_data)%4 != 0:
			self.set_status(400)
			return self.finish('wrong size')
	
		frame_len_elements = int(len(raw_data)/2/2)    # /2 because of stereo, /2 because sizeof(int16) == 2
		
		#left = raw_data[0:frame_len_raw]
		#right = raw_data[frame_len_raw:frame_len_raw*2]

		audio_data = np.ndarray([frame_len_elements, 2], dtype=np.int16)
		cnt = 0
		for i in range(0, len(raw_data), 4):
			left = raw_data[i:i+2]
			right = raw_data[i+2:i+4]
			audio_data[cnt] = [struct.unpack('<H', left)[0], struct.unpack('<H', right)[0]]
			cnt += 1

		#print(data)
		for sample in audio_data:
			file.write(f'{sample[0]},{sample[1]}\n')
		file.flush()

def open_file():
	global file, file_raw
	file = open(FILENAME, "w")
	file_raw = open(FILENAME_RAW, "wb")

def close_file():
	global file, file_raw
	if file is not None:
		file.close()
	
	if file_raw is not None:
		file_raw.close()


settings = {
	"debug": True,
}

def make_app():
	return tornado.web.Application([
			(r"/i2s_samples", MainHandler),
		], **settings)


if __name__ == "__main__":
	open_file()
	app = make_app()
	app.listen(PORT)
	tornado.autoreload.add_reload_hook(close_file)
	try:
		print(f'Starting server on port {PORT}')
		tornado.ioloop.IOLoop.current().start()
	except KeyboardInterrupt:
		close_file()
		print('Ending server')
