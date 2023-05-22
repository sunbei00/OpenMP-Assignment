#include "DS_timer.h"
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <stack>

#define NUM_OF_THREADS 10
#define BIN_SIZE 10
#define DATA_SIZE (1024 * 1024 * 1024)
void initData();
void initBin();

float* DATA;
int** BIN;

int main(void) {
	const float split = (float)10 / BIN_SIZE;
	omp_set_num_threads(NUM_OF_THREADS);
	initData();
	initBin();

	DS_timer timer(4);
	timer.setTimerName(0, (char*)"Serial");
	timer.setTimerName(1, (char*)"Version 1");
	timer.setTimerName(2, (char*)"Version 2");
	timer.setTimerName(3, (char*)"Version 3");


#pragma region Serial
	{
		printf("Start Serial..\n");
		timer.onTimer(0);
		for (int i = 0; i < DATA_SIZE; i++) {
			for (int j = 0; j < BIN_SIZE; j++) {
				if (j * split <= DATA[i] && DATA[i] < (j + 1) * split) {
					++BIN[0][j];
					break;
				}
			}
			if (DATA[i] == 10)
				++BIN[0][BIN_SIZE - 1];
		}
		timer.offTimer(0);
		printf("End Serial..\n");
	}
#pragma endregion

#pragma region Version1
	{
		printf("Start Version1..\n");
		timer.onTimer(1);
		omp_lock_t lock;
		omp_init_lock(&lock);
#pragma omp parallel for
		for (int i = 0; i < DATA_SIZE; i++) {
			for (int j = 0; j < BIN_SIZE; j++) {
				if (j * split <= DATA[i] && DATA[i] < (j + 1) * split) {
					//omp_set_lock(&lock);
#pragma omp atomic
					++BIN[1][j];
					//omp_unset_lock(&lock);
					break;
				}
			}
			if (DATA[i] == 10) {
				//omp_set_lock(&lock);
#pragma omp atomic
				++BIN[1][BIN_SIZE - 1];
				//omp_unset_lock(&lock);
			}
		}
		timer.offTimer(1);
		printf("End Version1..\n");
	}
#pragma endregion

#pragma region Version2
	{
		printf("Start Version2..\n");
		timer.onTimer(2);
		omp_lock_t lock;
		omp_init_lock(&lock);
#pragma omp parallel
		{
			int TMP_BIN[BIN_SIZE];
			memset(TMP_BIN, 0, sizeof(int) * BIN_SIZE);
#pragma omp for
			for (int i = 0; i < DATA_SIZE; i++) {
				for (int j = 0; j < BIN_SIZE; j++) {
					if (j * split <= DATA[i] && DATA[i] < (j + 1) * split) {
						++TMP_BIN[j];
						break;
					}
				}
				if (DATA[i] == 10) {
					++TMP_BIN[BIN_SIZE - 1];
				}
			}

			omp_set_lock(&lock);
			for (int i = 0; i < BIN_SIZE; i++) {
				BIN[2][i] += TMP_BIN[i];
			}
			omp_unset_lock(&lock);
		}
		timer.offTimer(2);
		printf("End Version2..\n");
	}
#pragma endregion

#pragma region Version3
	{
		printf("Start Version3..\n");
		timer.onTimer(3);
		std::stack<int*> stack;
		omp_lock_t lock;
		omp_init_lock(&lock);
		int tmp = NUM_OF_THREADS;
		bool odd;
		int prev;
#pragma omp parallel
		{
			const int tid = omp_get_thread_num();
			int TMP_BIN[BIN_SIZE];
			memset(TMP_BIN, 0, sizeof(int) * BIN_SIZE);
#pragma omp for
			for (int i = 0; i < DATA_SIZE; i++) {
				for (int j = 0; j < BIN_SIZE; j++) {
					if (j * split <= DATA[i] && DATA[i] < (j + 1) * split) {
						++TMP_BIN[j];
						break;
					}
				}
				if (DATA[i] == 10) {
					++TMP_BIN[BIN_SIZE - 1];
				}
			}

		re:
#pragma omp barrier // https://www.openmp.org/spec-html/5.0/openmpsu38.html single 앞에는 implicit barrier가 존재하지 않음.
#pragma omp single
			{
				if (tmp % 2 == 1)
					odd = true;
				else
					odd = false;

				prev = tmp;
				tmp /= 2;
			}
			// implicit barrier

			if (tid >= tmp && tid < prev) { // tmp = 10 -> tid : 5 ~ 9 , tmp = 9 -> tid : 4~9, tmp = 3 -> tid : 1~2, tmp = 2 -> tid : 1
				omp_set_lock(&lock);
				stack.push(TMP_BIN);
				omp_unset_lock(&lock);
			}
#pragma omp barrier

			if (tid < tmp) { // tmp = 10 -> tid : 0~4, tmp = 9 -> tid : 0~3, tmp = 3 -> tid : 0, tmp = 2 -> tid : 0
				omp_set_lock(&lock);
				int* pop = stack.top();
				stack.pop();
				omp_unset_lock(&lock);
				for (int i = 0; i < BIN_SIZE; i++)
					TMP_BIN[i] += pop[i];
			}
			if (tid == 0 && odd) {
				omp_set_lock(&lock);
				int* pop = stack.top();
				stack.pop();
				omp_unset_lock(&lock);
				for (int i = 0; i < BIN_SIZE; i++) {
					TMP_BIN[i] = TMP_BIN[i] + pop[i];
				}
			}
#pragma omp barrier
			if (tmp != 1)
				goto re;

			if (tid == 0)
				memcpy(BIN[3], TMP_BIN, sizeof(int) * BIN_SIZE);

		}
		timer.offTimer(3);
		printf("End Version3..\n");
	}
#pragma endregion

	bool correct = true;
	for (int i = 0; i < BIN_SIZE; i++) {
		if (BIN[0][i] != BIN[1][i] || BIN[2][i] != BIN[3][i] || BIN[0][i] != BIN[2][i])
			correct = false;
	}
	for (int j = 0; j < 4; j++) {
		printf("Version %d : ", j);
		for (int i = 0; i < BIN_SIZE; i++)
			printf("%d ", BIN[j][i]);
		printf("\n");
	}

	if (correct)
		printf("Correct!\n\n");
	else
		printf("Fail!\n\n");

	timer.printTimer();

	// delete
}

// 10 -> 5개 스레드 사용
// 5 -> 2개 스레드 사용
// 3 -> 1개 스레드 사용
// 2 -> 1개 스레드 사용
// 리턴

// 10 -> 5개 스레드 사용
// 5 -> 2개 스레드 사용 (단, 마지막 스레드는 1개 더)
// 2 -> 1개 스레드 사용


void initData() {
	DATA = (float*)malloc(sizeof(float) * DATA_SIZE);
	srand(time(0));
#pragma omp parallel for
	for (int i = 0; i < DATA_SIZE; i++)
		DATA[i] = (float)rand() / RAND_MAX * 10;
}

void initBin() {
	BIN = new int* [BIN_SIZE];
	for (int i = 0; i < 4; i++) {
		BIN[i] = new int[BIN_SIZE];
		memset(BIN[i], 0, sizeof(int) * BIN_SIZE);
	}
}