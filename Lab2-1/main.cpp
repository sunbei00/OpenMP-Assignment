#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "DS_timer.h"
#include "DS_definitions.h"

// Set the size of matrix and vector
// matrix A = m by n
// vector b = n by 1
#define m (100) // row
#define n (100) // col

#define GenFloat (rand() % 100 + ((float)(rand() % 100) / 100.0))
void genRandomInput();

float A[m][n];
float X[n];
float Y_serial[m];
float Y_parallel[m];

int main(int argc, char** argv)
{
	DS_timer timer(2);
	timer.setTimerName(0, (char*)"Serial");
	timer.setTimerName(1, (char*)"Parallel");

	genRandomInput();


	// Init Buffer
	for (int i = 0; i < m; i++) {
		Y_serial[i] = 0;
		Y_parallel[i] = 0;
	}
		


	//** 1. Serial code **//
	timer.onTimer(0);


	//** HERE
	//** Write your code implementing the serial algorithm here
	for (int row = 0; row < m; row++) 
		for (int col = 0; col < n; col++) 
			Y_serial[row] += A[row][col] * X[col];

	timer.offTimer(0);

	//** 2. Parallel code **//
	timer.onTimer(1);


	//** HERE
	//** Write your code implementing the parallel algorithm here
	const int threadCount = 8;
#pragma omp parallel num_threads(threadCount)
	{
#pragma omp for
		for (int row = 0; row < m; row++) {
			for (int col = 0; col < n; col++) {
				Y_parallel[row] += A[row][col] * X[col];
			}
		}
	}

	timer.offTimer(1);

	//** 3. Result checking code **//
	bool isCorrect = false;

	//** HERE
	//** Wriet your code that compares results of serial and parallel algorithm
	// Set the flag 'isCorrect' to true when they are matched

	for (int i = 0; i < m; i++) {
		if (Y_serial[i] != Y_parallel[i]) {
			isCorrect = true;
			break;
		}
	}

	if (isCorrect)
		printf("Results are not matched :(\n");
	else
		printf("Results are matched! :)\n");

	timer.printTimer();
	EXIT_WIHT_KEYPRESS;
}

void genRandomInput(void) {
	// A matrix
	LOOP_INDEX(row, m) {
		LOOP_INDEX(col, n) {
			A[row][col] = GenFloat;
		}
	}

	LOOP_I(n)
		X[i] = GenFloat;

	memset(Y_serial, 1, sizeof(float) * m);
	memset(Y_parallel, 0, sizeof(float) * m);
}