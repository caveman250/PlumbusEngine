import subprocess
import os

subprocess.run(['cmake', '--build', '.', '--', '-j12'], cwd=os.path.dirname(os.path.realpath(__file__)) + '/../bin/Debug/Engine')
subprocess.run(['cp', os.path.dirname(os.path.realpath(__file__)) + '/../bin/Debug/PlumbusTester/libPlumbusEngine.so', os.path.dirname(os.path.realpath(__file__)) + '/bin/x64/Debug/net5.0/'])