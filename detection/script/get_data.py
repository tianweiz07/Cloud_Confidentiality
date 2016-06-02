#/bin/python

import os
import sys

def getIndex(target, start_index, perf_val):
	if target == 0:
		i = start_index
		flag = 0
		while flag < 10 and i < len(perf_val):
			if perf_val[i] == 0:
				flag += 1
			else:
				flag = 0
			i += 1
		if i < len(perf_val):
			return i
		else:
			print "error1"
			return 0
	if target == 1:
		i = start_index
		flag = 0
		while flag < 10 and i < len(perf_val):
			if perf_val[i] > 100:
				flag += 1
			else:
				flag = 0
			i += 1
		if i < len(perf_val):
			return i-10
		else:
			print "error2"
			return 0
	
def data_cut(input_file, sample, length):
	data1 = open(input_file, 'r')
	activities1 = data1.readlines()
	data1.close()
	perf_val1 = [int(x.split()[1]) for x in activities1]

	feature1 = [[0 for x in range(length)] for x in range(sample)]

	feature_index = 0
	index = 0
	while feature_index < sample:
		index1 = getIndex(1, index, perf_val1)
		for i in range(length):
			feature1[feature_index][i] = perf_val1[index1+i]
	
		index = getIndex(0, index1+length, perf_val1)
		feature_index += 1

	return feature1


def data_cut1(input_file, sample, length):
	data1 = open(input_file, 'r')
	activities1 = data1.readlines()
	data1.close()
	perf_val1 = [int(x.split()[1]) for x in activities1]

	feature1 = [[0 for x in range(length)] for x in range(sample)]

	feature_index = 0
	index = 0
	while feature_index < sample:
		index = 0
		for i in range(length):
			feature1[feature_index][i] = perf_val1[index+i]
	
		index += length
		feature_index += 1

	return feature1
