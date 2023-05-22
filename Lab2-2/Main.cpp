#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "DS_timer.h"

#define F(x) ((x)*(x))

int a;
int b;
int n;
double dx;
const int numThreads = 3;

double SerialVal = 0;
double ParallelVal = 0;

int main(int argc, char* argv[]) {


	if (argc < 4) {
		printf("[command]> command a b n\n");
		printf("Set : [a,b] , number of blocks : n\n");
		return -1;
	}

	// Initialize program argument
	{
		a = atoi(argv[1]);
		b = atoi(argv[2]);
		n = atoi(argv[3]);
		dx = ((double)b - a) / n;
	}

	// Init Timer
	DS_timer timer(2);
	timer.setTimerName(0, (char*)"Serial");
	timer.setTimerName(1, (char*)"Parallel");

	// Serial
	timer.onTimer(0);

	for (int i = 0; i < n; i++) {
		SerialVal += ((F(a + dx * i)+ F(a + dx * (i+1)))/2 * dx);
	}

	timer.offTimer(0);


	// Parallel
	timer.onTimer(1);

	double tidValue[numThreads];
	for (int i = 0; i < numThreads; i++)
		tidValue[i] = 0;

#pragma omp parallel num_threads(numThreads)
	{
		int tid = omp_get_thread_num();
		double value = 0;

#pragma omp for
		for (int i = 0; i < n; i++) {	
			value += (( F(a + dx * i) + F(a + dx * (i+1)) )/2 * dx);
		}

		tidValue[tid] = value;
	}
	for (int i = 0; i < numThreads; i++)
		ParallelVal += tidValue[i];

	timer.offTimer(1);


	// Check Result
	bool isCorrect = true;

	if (abs(SerialVal - ParallelVal) > 10e-6)
		isCorrect = false;

	printf("Serial : %.15lf\n", SerialVal);
	printf("Parallel : %.15lf\n", ParallelVal);

	if (isCorrect)
		printf("Correct\n\n");
	else
		printf("Fail\n\n");

	timer.printTimer();
}