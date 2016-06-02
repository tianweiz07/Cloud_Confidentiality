#/bin/python

import os
import sys

interval = 2500000
boundary = 30
window = int(sys.argv[3])
threshold1 = 0.35
threshold2 = 300

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

next_cycle = 0
num_attack = 0
time_total = 0
for x in range(len(dtw)):
	if dtw[x] < threshold1 and timestamp_v[x] > next_cycle:
		next_cycle = timestamp_v[x] + interval
		for y in range(len(timestamp_a)):
			if timestamp_a[y] > timestamp_v[x]:
				for k in range(boundary):
					if min(cache[y+k:y+k+window])-max(cache[y-window+1:y+1]) >= threshold2:
						num_attack += 1
						time_total += timestamp_a[y+k+window] - timestamp_a[y+k]
						break
				break

print time_total*1.0/3/num_attack
