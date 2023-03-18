print("EXTRA")
import subprocess
import sys

Import("env")

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)


def before_buildfs(source, target, env):
    print(source, target, env)
    print("Before build fs")
    subprocess.run(['bash', '../web/bundle_all.sh'], stderr=sys.stderr, stdout=sys.stdout)

env.Execute("$PYTHONEXE -m pip install jsmin")
env.AddPreAction("$BUILD_DIR/spiffs.bin", before_buildfs)