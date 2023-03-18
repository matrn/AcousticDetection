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


print("--- Cleaning out directory ---", flush=True)
for file in os.listdir(OUT_DIR):
	print(f'Removing {file}', flush=True)
	os.remove(OUT_DIR+file)


print("--- Starting bundle process ---", flush=True)
for file in os.listdir(SRC_DIR):
	path = SRC_DIR + file
	path_out = OUT_DIR + file

	if len(file) > 32:
		print(f'ERROR: file >{file}< - filename length must be <= 32 characters but len(filename) = {len(file)}', flush=True)
		exit(1)

	if os.path.isfile(path):
		# minify JS file
		if file.endswith('.js'):
			print(f'Minifying {file}', flush=True)
			with open(path_out, 'w') as out_file:
				out_file.write(minify(path))
		
		# ignore hidden files - for example .gitignore
		elif file.startswith('.'):
			print(f'Skipping {file}', flush=True)
		
		# just copy
		else:
			print(f'Copying {file}', flush=True)
			shutil.copy(path, path_out)


print("--- Starting compression ---", flush=True)
for file in os.listdir(OUT_DIR):
	path = OUT_DIR + file

	if file.endswith(('.js', '.css')):
		print(f'Compressing {file}', flush=True)
		with open(path, 'r') as infile:
			with open(path + '.gz', 'wb') as outfile:
				#print(gzip_compress(infile.read()), flush=True)
				outfile.write(gzip_compress(infile.read()))

		os.remove(path)


print("DONE!", flush=True)