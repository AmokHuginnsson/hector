doNotCopy=["/home"]
forceCopy=["/usr/bin/env"]
chroot="/home/hector"
testCommandsInsideJail=["/bin/sh -c 'cd /nest;env HOME=/nest NEST=/nest LD_LIBRARY_PATH=/nest/lib /nest/1exec'"]
processNames=["1exec"]
cleanJailFirst=0

users=["hector"]
groups=["service"]

keepStraceOutputs=1

