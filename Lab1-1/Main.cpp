#include <stdio.h>
#include <omp.h>
#include <algorithm>
#include <stdlib.h>

const int MAX_THREAD = 10;

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Not found argument");
		return -1;
	}
	int threadCount = atoi(argv[1]);
	std::clamp(threadCount, 1, MAX_THREAD);

	#pragma omp parallel num_threads(threadCount)
	printf("[Thread %d/%d] Hello OpenMP!\n", omp_get_thread_num(), omp_get_num_threads());

	return 0;
}


