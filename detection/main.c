#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>

#define WINDOW_SIZE 111
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define constraint WINDOW_SIZE
#define __NR_Check 189
#define __NR_PID 1

int pid_v;
int pid_a[__NR_PID];
uint64_t INTERVAL_V;
uint64_t INTERVAL_A;
int ROUND_V; 
int ROUND_A; 

uint64_t begin_stamp;
int cal_num;
int cal_lock;

int signature_len;

struct node {
	long long event_value;
	uint64_t time_value;
	struct node *next;
};

struct node *head;
struct node *end;

int train_set[WINDOW_SIZE];
int nomi_train_set[WINDOW_SIZE];
int nomi_test_set[WINDOW_SIZE];


uint64_t rdtsc(void) {
        uint64_t a, d;
        __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
        return (d<<32) | a;
}

void cal_average(int *data, int *res) {
        int sum = 0;
        int i;
        for (i=0; i<WINDOW_SIZE; i++)
                sum += data[i];
        int average = sum/WINDOW_SIZE;
        for (i=0; i<WINDOW_SIZE; i++)
                res[i] = data[i] - average;

        return;
}

void cal_average1(struct node* head, int *res, int average) {
        int sum = 0;
        int i;
        struct node* cur = head;
        for (i=0; i<WINDOW_SIZE; i++) {
                res[i] = cur->event_value - average;
                cur = cur->next;
        }

        return;
}

double Euclidean_Distance(int *data1, int *data2, int w) {
        int i;
        double distance = 0;
        for (i=0; i<WINDOW_SIZE; i++)
                distance += (double)(data1[i]-data2[i])*(data1[i]-data2[i]);
        return sqrt(distance)/WINDOW_SIZE;
}

int DTW_Distance(int *data1, int *data2, int w) {

        int **dtw = (int **)malloc(sizeof(int *)*(WINDOW_SIZE+1));
        int i, j;
        for (i=0; i<WINDOW_SIZE+1; i++)
                dtw[i] = (int *)malloc(sizeof(int)*(WINDOW_SIZE+1));
        for (i=0; i<WINDOW_SIZE+1; i++) {
		for (j=0; j<WINDOW_SIZE+1; j++) {
	                dtw[i][j] = INT_MAX;
		}
        }

        dtw[0][0] = 0;

        for (i=1; i<WINDOW_SIZE+1; i++) {
                for (j=MAX(1, i-w); j<MIN(WINDOW_SIZE, i+w)+1; j++) {
                        dtw[i][j] = abs(data1[i-1]-data2[j-1]) + MIN(dtw[i-1][j-1], MIN(dtw[i][j-1], dtw[i-1][j]));
                }
        }

        return dtw[WINDOW_SIZE][WINDOW_SIZE];
}

void training() {
        FILE *train_file = fopen("training_set", "r");
        int i;
	int time;
        for (i=0; i<WINDOW_SIZE; i++)
                fscanf(train_file, "%d	%d", &time, &train_set[i]);

        cal_average(train_set, nomi_train_set);
	signature_len = 0;
	for (i=0; i<WINDOW_SIZE; i++) 
		signature_len += abs(nomi_train_set[i]);
        close(train_file);
        return;
}

void *cal_data(void *argv) {
	FILE *recog_res = fopen("recog_res", "a");
	int temp = -1;
	while (cal_num < ROUND_V) {
		while (cal_lock || temp == cal_num);
		double distance = (double)DTW_Distance(nomi_train_set, nomi_test_set, constraint)/(double)signature_len;
	      	fprintf(recog_res, "%lu	%f\n", head->time_value, distance);
		temp = cal_num;
	}
	close(recog_res);
}

void *perf_victim(void *argv) {

	/* Training the dataset*/
	training();

	/* Initialize performance counter structures*/
        struct perf_event_attr pe;
        int fd;
        int j;
        memset(&pe, 0, sizeof(struct perf_event_attr));
	// ARITH_MUL operations
//        pe.type = PERF_TYPE_RAW;
//        pe.config = 0x530114;
	//  Instructions or branch instructions.
        pe.type = PERF_TYPE_HARDWARE;
        pe.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
        pe.size = sizeof(struct perf_event_attr);
        pe.disabled = 1;
        pe.inherit = 0;
        pe.pinned = 1;
        pe.exclude_kernel = 0;
        pe.exclude_user = 0;
        pe.exclude_hv = 0;
        pe.exclude_host = 0;
        pe.exclude_guest = 0;

        fd = syscall(__NR_perf_event_open, &pe, pid_v, -1, -1, 0);
//        fd = syscall(__NR_perf_event_open, &pe, -1, 0, -1, 0);


	/* First round */
	struct node *start = (struct node*)malloc(sizeof(struct node));
	struct node *cur = start;

        uint64_t start_cycle;
	while(begin_stamp!= 0);
	begin_stamp = rdtsc();
        for (j=0; j<WINDOW_SIZE; j++) {
                ioctl(fd, PERF_EVENT_IOC_RESET, 0);
                ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

                start_cycle = rdtsc();
                while(rdtsc()-start_cycle<INTERVAL_V);

                ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		struct node *item = (struct node*)malloc(sizeof(struct node));
		cur->next = item;
                read(fd, &(item->event_value), sizeof(long long));
		item->time_value = (start_cycle-begin_stamp)/1000;
		cur = item;
        }
	cur->next = start;
	head = start;
	end = cur;

        long long sum = 0;
        cur = head;
	int i;
        for (i = 0; i<WINDOW_SIZE; i++) {
                sum += cur->event_value;
                cur = cur->next;
        }

        int average = sum/WINDOW_SIZE;
        cal_average1(head, nomi_test_set, average);
	
	/* running profiling and calculation*/
        for (j=0; j<ROUND_V; j++) {
                ioctl(fd, PERF_EVENT_IOC_RESET, 0);
                ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

                start_cycle = rdtsc();
                while(rdtsc()-start_cycle<INTERVAL_V);

                ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		
		cal_lock = 1;
                sum -= head->event_value;
                head = head->next;
                end = end->next;

		read(fd, &(end->event_value), sizeof(long long));
		end->time_value = (start_cycle-begin_stamp)/1000;
                sum += end->event_value;
                average = sum/WINDOW_SIZE;
                cal_average1(head, nomi_test_set, average);
		cal_num ++;
		cal_lock = 0;

        }

        close(fd);

	return;
}

void *perf_attacker(void *argv) {

        int i, j;

        uint64_t value;

        FILE *output[__NR_PID];
	for (j=0; j<__NR_PID; j++) {
		char name[10];
		sprintf(name, "%d", pid_a[j]);
		output[j] = fopen(name, "a");
	}

        struct perf_event_attr pe;
        int fd;

        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = PERF_TYPE_HW_CACHE;
        pe.config = (PERF_COUNT_HW_CACHE_LL)|(PERF_COUNT_HW_CACHE_OP_READ << 8)|(PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
        pe.size = sizeof(struct perf_event_attr);
        pe.disabled = 1;
        pe.inherit = 0;
        pe.pinned = 1;
        pe.exclude_kernel = 0;
        pe.exclude_user = 0;
        pe.exclude_hv = 0;
        pe.exclude_host = 0;
        pe.exclude_guest = 0;

        uint64_t start_cycle;

	begin_stamp = 0;
	while (begin_stamp == 0);
	int x[__NR_PID];
        for (j=0; j<ROUND_A; j++) {
		for (i=0; i<__NR_PID; i++)
			x[i] = pid_a[i];
		syscall(__NR_Check, x);

		for (i=0; i<__NR_PID; i++) {
			if (x[i] > 0) {
        			fd = syscall(__NR_perf_event_open, &pe, pid_a[i], -1, -1, 0);
		                ioctl(fd, PERF_EVENT_IOC_RESET, 0);
        		        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

      			        start_cycle = rdtsc();
		                while(rdtsc()-start_cycle<INTERVAL_A);

		                ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		                read(fd, &value, sizeof(long long));
		                fprintf(output[i], "%lu:           %lu\n", (start_cycle-begin_stamp)/1000, value);
        			close(fd);

			}
		}

	}
	for (j=0; j<__NR_PID; j++) {
	        fclose(output[j]);
	}
        return 0;
}

int main(int argc, char **argv) {
        pid_v = atoi(argv[1]);
	int i;
	for (i=0; i<__NR_PID; i++) {
		pid_a[i] = atoi(argv[2+i]);
	}
	INTERVAL_V = 300000;
	INTERVAL_A = 3000000;
	ROUND_V = 50000;
	ROUND_A = 5000;

	begin_stamp = -1;
	cal_num = 0;
	cal_lock = 1;
	pthread_t profile_victim, profile_attacker, process_data;

	pthread_create(&profile_victim, NULL, perf_victim, NULL);
	pthread_create(&process_data, NULL, cal_data, NULL);
	pthread_create(&profile_attacker, NULL, perf_attacker, NULL);

	pthread_join(profile_victim, NULL);
	pthread_join(process_data, NULL);
	pthread_join(profile_attacker, NULL);
        return 0;
}
