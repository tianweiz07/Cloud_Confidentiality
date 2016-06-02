#/bin/python

import os
import sys
import get_data

sample = 100
length = 10



def distance(v1, v2):
	_sum = 0
	for i in range(length):
		_sum += (v1[i]-v2[i])*(v1[i]-v2[i])

	return _sum

num_class = len(sys.argv)-1

feature = []
avg = [[0 for i in range(length)] for i in range(num_class+1)]

for i in range(num_class):
	ret = get_data.data_cut(sys.argv[i+1], sample, length)
	feature.append(ret)

for i in range(length):
	for j in range(sample):
		for k in range(num_class):
			avg[k][i] += feature[k][j][i]
	_sum = 0
	for k in range(num_class):
		avg[k][i] /= sample
		_sum += avg[k][i]
	avg[num_class][i] = _sum/num_class

t1 = 0
for i in range(num_class):
	t1 += sample*distance(avg[i], avg[num_class])
t2 = 0
for i in range(num_class):
	for j in range(sample):
		t2 += distance(feature[i][j], avg[i])

print t1
print t2

print t1*1.0/t2
