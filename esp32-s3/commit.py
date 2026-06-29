import subprocess
Import("env")
try:
    com = subprocess.check_output(["git", "rev-list", "--count", "HEAD"]).strip().decode("utf-8")
except:
    com = "0"
print(com)

env.Append(CPPDEFINES=[("COMMIT", com)])
