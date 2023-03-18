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