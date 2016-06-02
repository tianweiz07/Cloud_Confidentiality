#/bin/python

import os
import sys

interval = 2500000
boundary = 30
window = int(sys.argv[3])
threshold1 = 0.35

data1 = open(sys.argv[1], 'r')
activities1 = data1.readlines()
data1.close()

data2 = open(sys.argv[2], 'r')
activities2 = data2.readlines()
data2.close()

timestamp_v = [int(x.split()[0]) for x in activities1]
timestamp_a = [int(x.split(":")[0]) for x in activities2]

dtw = [float(x.split()[1]) for x in activities1]
cache = [int(x.split(":")[1]) for x in activities2]

for threshold2 in range(100, -5, -5):
	next_cycle = 0
	num_crypto = 0
	num_attack = 0
	for x in range(len(dtw)):
		if dtw[x] < threshold1 and timestamp_v[x] > next_cycle:
			next_cycle = timestamp_v[x] + interval
			num_crypto += 1
			for y in range(len(timestamp_a)):
				if timestamp_a[y] > timestamp_v[x]:
					if min(cache[y+boundary:y+boundary+window])-max(cache[y-window+1:y+1]) >= threshold2:
						num_attack += 1
#					else:
#						print '{0} {1}'.format(timestamp_v[x], timestamp_a[y])
					break
	print '{0} {1}/{2}= {3}'.format(threshold2, num_attack, num_crypto, num_attack*1.0/num_crypto)
