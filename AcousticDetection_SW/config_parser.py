import re
import os

file_dir = os.path.dirname(os.path.realpath(__file__))

CPP_CONFIG = file_dir + '/include/config.hpp'


def parse_config():
	config = {}

	with open(CPP_CONFIG, 'r') as f:
		for line in f.readlines():
			data_re = re.search(r'"(.*?)"', line)
			if data_re:
				if line.startswith('//'): continue   # ignore comments

				var_name = None
				var_data = data_re.group().replace('"', '')

				if "#define" in line:
					var_name = re.search(r'\s[A-Za-z_]*\s', line).group().replace(' ', '')
				else:
					var_name = re.search(r'\s[A-Za-z_]*.?=', line).group().replace(' ', '').replace('=', '')

				config[var_name] = var_data
	return config