#!/usr/bin/python


import os
import string
import sys
import time
import random

cmd = 'gpg --batch --passphrase adminj310a -r FD2910E9 -d "plaintext.gpg"'

print cmd
while(True):
	os.system(cmd)
	
#	interval = random.uniform(0, 1)
	interval = 1
	time.sleep(interval)
