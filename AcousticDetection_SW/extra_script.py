# # pip install zeroconf
# from zeroconf import ServiceBrowser, ServiceListener, Zeroconf


# class MyListener(ServiceListener):
#     def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
#         print(f"Service {name} updated")

#     def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
#         print(f"Service {name} removed")

#     def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
#         info = zc.get_service_info(type_, name)
#         print(f"Service {name} added, service info: {info}")


# zeroconf = Zeroconf()
# listener = MyListener()
# browser = ServiceBrowser(zeroconf, "_arduino._tcp.local.", listener)

# print(zeroconf.get_service_info("_arduino._tcp.local.", "AcousticDetection.local."))
# try:
#     input("Press enter to exit...\n\n")
# finally:
#     zeroconf.close()


print("EXTRA Python script for Platformio")
import subprocess
import sys

Import("env")

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)


def before_buildfs(source, target, env):
	print(source, target, env)
	print("Before build fs")
	subprocess.run(['bash', '../web/bundle_all.sh'], stderr=sys.stderr, stdout=sys.stdout)


env.Execute("$PYTHONEXE -m pip install jsmin")   # docs: https://docs.platformio.org/en/stable/scripting/examples/extra_python_packages.html

# docs: https://docs.platformio.org/en/latest/scripting/actions.html
# issue with buildfs: https://github.com/platformio/platformio-core/issues/3842
env.AddPreAction("$BUILD_DIR/spiffs.bin", before_buildfs)



######### CONFIG parser #########

try:
	import re
except:
	env.Execute("$PYTHONEXE -m pip install regex")
	import re


CPP_CONFIG = 'include/config.hpp'

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

print(config)


OTA_PORT = int(config.get('ota_port', 3232))
OTA_IP = config.get('ota_ip', f'{config.get("hostname")}.local')
OTA_PASSWORD = config.get('ota_password')
print(OTA_PORT, OTA_IP, OTA_PASSWORD)


env.Replace(
	UPLOAD_PROTOCOL="espota",
	UPLOAD_PORT=OTA_IP,
	UPLOAD_FLAGS=[f'--port={OTA_PORT}', f'--auth={OTA_PASSWORD}'],
)

#env.Append(CPPDEFINES=[("OTA_PASSWORD", "\\\"" + OTA_PASSWORD + "\\\"")])