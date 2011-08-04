doNotCopy=["/home"]
forceCopy=["/usr/bin/env","/lib/ld-linux.so.2","/lib/ld-linux-x86-64.so.2","/lib/ld-2.8.90.so","/usr/lib/libsqlite3.so","/lib/tfurca-makejail-to-pizda"]
chroot="/home/hector"
testCommandsInsideJail=["/bin/launchtool -u hector --chdir='/nest' -t hector-install \". /nest/set-limits.sh; /usr/bin/env HOME=/nest NEST=/ LD_LIBRARY_PATH=/nest/lib /nest/1exec\""]
processNames=["1exec"]
cleanJailFirst=0
#sleepAfterStartCommand=2
users=["hector"]
groups=["service"]

keepStraceOutputs=1

