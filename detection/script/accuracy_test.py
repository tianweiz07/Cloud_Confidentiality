#/bin/python

import os
import sys

interval = 2500000

data = open('rsa_50', 'r')
activities = data.readlines()
data.close()

for i in range(1, 40):
	threshold = i*100

	crypto = 0
	next_cycle = 0

	for i in range(1, len(activities)-5):
#	for activity in activities:
		activity_list = activities[i].split(":")
		activity_list1 = activities[i+1].split(":")
		activity_list2 = activities[i-1].split(":")
		if int(activity_list[1]) - int(activity_list2[1]) > threshold and int(activity_list1[1]) - int(activity_list2[1]) > threshold and int(activity_list[0]) > next_cycle:
			crypto += 1
			next_cycle = int(activity_list[0]) + interval
	print '{0} {1}'.format(threshold, crypto)
