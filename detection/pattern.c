#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#define WINDOW_SIZE 21
#define ALL_SIZE 1000
#define constraint WINDOW_SIZE

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

struct node {
	int event_value;
	int time_value;
	struct node *next;
}; 

int train_set[WINDOW_SIZE];
int nomi_train_set[WINDOW_SIZE];
int nomi_test_set[WINDOW_SIZE];

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

double Euclidean_Distance(int *data1, int *data2) {
	int i;
	double distance = 0;
	for (i=0; i<WINDOW_SIZE; i++)
		distance += (double)(data1[i]-data2[i])*(data1[i]-data2[i]);
	return sqrt(distance)/WINDOW_SIZE;
}

double DTW_Distance(int *data1, int *data2, int w) {
	double **dtw = (double **)malloc(sizeof(double *)*(WINDOW_SIZE+1));
	int i, j;
	for (i=0; i<WINDOW_SIZE+1; i++)
		dtw[i] = (double *)malloc(sizeof(double)*(WINDOW_SIZE+1));
	for (i=0; i<WINDOW_SIZE+1; i++) {
		for (j=0; j<WINDOW_SIZE+1; j++) {
			dtw[i][j] = DBL_MAX;
		}
	}

	dtw[0][0] = 0.0;

	for (i=1; i<WINDOW_SIZE+1; i++) {
		for (j=MAX(1, i-w); j<MIN(WINDOW_SIZE, i+w)+1; j++) {
			dtw[i][j] = (double)(data1[i-1]-data2[j-1])*(data1[i-1]-data2[j-1]) + MIN(dtw[i-1][j-1], MIN(dtw[i][j-1], dtw[i-1][j]));
		}
	}

	return sqrt(dtw[WINDOW_SIZE][WINDOW_SIZE]);
}

void training() {
	FILE *train_file = fopen("training_set", "r");
	int i;
	int time;
	for (i=0; i<WINDOW_SIZE; i++) 
		fscanf(train_file, "%d	%d", &time, &train_set[i]);

	cal_average(train_set, nomi_train_set);
	close(train_file);
	return;
}

void testing() {
	FILE *test_file = fopen("testing_set", "r");
	int i;

	// initialize the list
	struct node *start = (struct node*)malloc(sizeof(struct node));
	fscanf(test_file, "%d	%d", &(start->time_value), &(start->event_value));
	struct node *cur = start;
	struct node *head;
	struct node *end;
	for (i=1; i<WINDOW_SIZE; i++) {
		struct node *item = (struct node*)malloc(sizeof(struct node));
		cur->next = item;
		fscanf(test_file, "%d	%d", &(item->time_value), &(item->event_value));
		cur = item;
	}
	cur->next = start;
	head = start;
	end = cur;

	int sum = 0;
	cur = head;
	for (i = 0; i<WINDOW_SIZE; i++) {
		sum += cur->event_value;
		cur = cur->next;
	}
	int average = sum/WINDOW_SIZE;
	cal_average1(head, nomi_test_set, average);
	printf("%f\n", DTW_Distance(nomi_train_set, nomi_test_set, constraint));

	for (i=0; i<ALL_SIZE-WINDOW_SIZE-1; i++) {
		sum -= head->event_value;
		head = head->next;
		end = end->next;
		fscanf(test_file, "%d	%d", &(end->time_value), &(end->event_value));
		sum += end->event_value;
		average = sum/WINDOW_SIZE;
		cal_average1(head, nomi_test_set, average);
		printf("%f\n", DTW_Distance(nomi_train_set, nomi_test_set, constraint));
	}

	close(test_file);
}

int main (int argc, char* argv[]) {
	training();
	testing();
	return 0;
}
