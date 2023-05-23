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
	env.Execute("$PYTHONEXE -m pip install jsmin")   # docs: https://docs.platformio.org/en/stable/scripting/examples/extra_python_packages.html
	print(source, target, env)
	print("Before build fs")
	subprocess.run(['bash', '../web/bundle_all.sh'], stderr=sys.stderr, stdout=sys.stdout)




######### CONFIG parser #########
from config_parser import parse_config


def before_upload(source, target, env):
	print(">> BEFORE UPLOAD")
	config = parse_config()
	print(config)
	if env.get('UPLOAD_PROTOCOL', None) == 'espota':
		OTA_PORT = int(config.get('ota_port', 3232))
		OTA_IP = config.get('ota_ip', f'{config.get("hostname")}.local')
		OTA_PASSWORD = config.get('ota_password')
		print(OTA_PORT, OTA_IP, OTA_PASSWORD)
				
		env['UPLOAD_PROTOCOL'] = "espota"
		env['UPLOAD_PORT'] = OTA_IP
		env['UPLOAD_FLAGS'] = [f'--port={OTA_PORT}', f'--auth={OTA_PASSWORD}']


#env.Append(CPPDEFINES=[("OTA_PASSWORD", "\\\"" + OTA_PASSWORD + "\\\"")])


# docs: https://docs.platformio.org/en/latest/scripting/actions.html
# issue with buildfs: https://github.com/platformio/platformio-core/issues/3842
env.AddPreAction("$BUILD_DIR/spiffs.bin", before_buildfs)
# env.AddPreAction("upload", before_upload)
before_upload(None, None, env)