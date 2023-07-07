# Importing required module
import subprocess
 
# Using system() method to
# execute shell commands
for i in range(0, 255, 1):
    subprocess.Popen('curl localhost:8080', shell=True)
