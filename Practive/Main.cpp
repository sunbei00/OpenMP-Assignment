
#include <stdio.h>
#include <stdio.h>

#define F(x) (x*x)

int main(void) {

	int a = 2;
#pragma omp parallel num_threads(10)
	{
		printf("test %d\n", a);
#pragma omp barrier
		if (a == 2) {
#pragma omp single
			a = 3;
		}

		printf("test %d\n", a);

	}
}