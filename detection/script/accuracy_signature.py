#/bin/python

import os
import sys

interval = 2500000

data = open('recog_res', 'r')
activities = data.readlines()
data.close()

for i in range(0, 100):
	threshold = i*1.0/100

	crypto = 0
	next_cycle = 0

	for activity in activities:
		activity_list = activity.split()
		if float(activity_list[1]) < threshold and int(activity_list[0]) > next_cycle:
			crypto += 1
			next_cycle = int(activity_list[0]) + interval

	print '{0} {1}'.format(threshold, crypto)
