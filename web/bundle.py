from jsmin import jsmin   # pip3 install jsmin
import os
import shutil
#import rjsmin

import gzip




SRC_DIR = 'src/'
OUT_DIR = 'dist/'

def minify(path):
	with open(path, 'r') as js_file:
		data = js_file.read()
		minified = jsmin(data)
		#minified = rjsmin.jsmin(data)
	return minified


def gzip_compress(content):
	return gzip.compress(bytes(content, 'utf-8'))

print("--- Cleaning out directory ---")
for file in os.listdir(OUT_DIR):
	print(f'Removing {file}')
	os.remove(OUT_DIR+file)


print("--- Starting bundle process ---")
for file in os.listdir(SRC_DIR):
	path = SRC_DIR + file
	path_out = OUT_DIR + file

	if len(file) > 32:
		print(f'ERROR: file >{file}< - filename length must be <= 32 characters but len(filename) = {len(file)}')
		exit(1)

	if os.path.isfile(path):
		# minify JS file
		if file.endswith('.js'):
			print(f'Minifying {file}')
			with open(path_out, 'w') as out_file:
				out_file.write(minify(path))
		
		# just copy
		else:
			print(f'Copying {file}')
			shutil.copy(path, path_out)

print("--- Starting compression ---")
for file in os.listdir(OUT_DIR):
	path = OUT_DIR + file

	if file.endswith(('.js', '.css')):
		print(f'Compressing {file}')
		with open(path, 'r') as infile:
			with open(path + '.gz', 'wb') as outfile:
				#print(gzip_compress(infile.read()))
				outfile.write(gzip_compress(infile.read()))

		os.remove(path)

print("DONE!")