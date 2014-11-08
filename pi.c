/******************************************************************************

 * * Filename: 		pi.c
 * * Description: 	numerical integration to calculate pi
 * * 
 * * Version: 		1.0
 * * Created:		2014/11/08
 * * Revision:		none
 * * Compiler: 		gcc
 * *
 * * Author: 		Wayne
 * * Organization:	CoDesign 

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


typedef struct thread_data { 
	double *y_total;
	pthread_mutex_t *y_total_lock;
	int threadID;
	double step;
	long thread_steps;
	long start, end;
} thread_data;

void *rectangle(void *threadD);

int main(int argc, char const *argv[])
{	
	int i;
	double pi, y_total = 0.0;
	static long num_steps;
	double step;
	if(argc == 2) {
		num_steps = atoi(argv[1]);
	}
	else {
		num_steps = 1000000;
	}
	step = 1.0 / num_steps;

	int NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN);
	int thread_steps = num_steps / NUM_THREADS;
	int remain_steps = num_steps % NUM_THREADS;

	pthread_t *threads;
	pthread_mutex_t y_total_lock;
	pthread_mutex_init(&y_total_lock, NULL);
	thread_data *threadD;
	threads = (pthread_t*)malloc(NUM_THREADS * sizeof(pthread_t));
	threadD = (thread_data*)malloc(NUM_THREADS * sizeof(thread_data));

	for(i = 0; i < NUM_THREADS; i++) {
		threadD[i].y_total = &y_total;
		threadD[i].y_total_lock = &y_total_lock;
		threadD[i].threadID = i;
		threadD[i].step = step;
		if(i == NUM_THREADS-1) threadD[i].thread_steps = thread_steps + remain_steps;
		else threadD[i].thread_steps = thread_steps;
		threadD[i].start = i * thread_steps;
		threadD[i].end = threadD[i].start + threadD[i].thread_steps;
		pthread_create(&threads[i], NULL, rectangle, &threadD[i]);
	}
	for(i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	pi = y_total * step;
	pthread_mutex_destroy(&y_total_lock);

	printf("pi = %.16lf\n", pi);
	
	return 0;
}

void *rectangle(void *threadD)
{
	long i;
	double x, y_partial;
	thread_data * tData = (thread_data*)threadD;

	for(i = tData->start; i < tData->end; i++) {
		x = (i + 0.5) * tData->step;
		y_partial += 4.0 / (1 + x*x);
	}

	pthread_mutex_lock(tData->y_total_lock);
	*tData->y_total += y_partial;
	pthread_mutex_unlock(tData->y_total_lock);

	pthread_exit(NULL);
}
