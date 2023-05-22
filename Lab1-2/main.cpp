#include <omp.h>
#include "common.h"
#include "DS_timer.h"

#define DECRYPT_ALGORITHM(a,b,c) (c += a*50 + b)

uchar* readData(char* _fileName, int _dataSize);
bool writeData(char* _fileName, int _dataSize, uchar* _data);

// Lab 1-2
int main(int argc, char** argv)
{
	DS_timer timer(2, 1);


	timer.setTimerName(0, (char*)"Serial Algorithm");
	timer.setTimerName(1, (char*)"Parallel Algorithm");

	// timer.setTimerName(0, "Serial Algorithm"); // can't implicit transform
	// timer.setTimerName(1, "Parallel Algorithm");

	if (argc < 6) {
		printf("It requires five arguments\n");
		printf("Usage: Extuction_file inputImgA inputImgB Width Height OutputFileName\n");
		return -1;
	}

	int width = atoi(argv[3]);
	int height = atoi(argv[4]);
	int dataSize = width * height * 3;

	// Read input data
	uchar* A = readData(argv[1], dataSize);
	uchar* B = readData(argv[2], dataSize);

	uchar* serialC = new uchar[dataSize];
	uchar* parallelC = new uchar[dataSize];
	memset(serialC, 0, sizeof(uchar) * dataSize);
	memset(parallelC, 0, sizeof(uchar) * dataSize);

	// Decrypt the image
	// The algorith is defined as DECRYPT_ALGORITHM(a,b,c)
	// See the definition at the top of this source code

	// 1. Serial algorithm
	timer.onTimer(0);
	for (int i = 0; i < dataSize; i++) {
		DECRYPT_ALGORITHM(A[i], B[i], serialC[i]);
	}
	timer.offTimer(0);

	timer.onTimer(1);
	// ***************************************************************
	// Wirte the decyprt result to parallelC array
	// Hint: DECRYPT_ALGORITHM(A[i], B[i], parallelC[i])

	// 2. Parallel algorithm


	/* Write your code from here! */

	int callThreadNum = 2; // default number of thread

	if (argc >= 7) { // 6번째 인자가 있으면 스레딩 수를 변환한다.
		callThreadNum = atoi(argv[6]);
		callThreadNum = callThreadNum <= 0 ? 1 : callThreadNum;
		callThreadNum = callThreadNum >= 10 ? 10 : callThreadNum;
	}

#pragma omp parallel num_threads(callThreadNum)
	{
		// for문에서 호출 시 system call의 비용이 증가하기 때문에 외부에 변수로 선언
		const int numOfThread = omp_get_num_threads();
		const int threadNum = omp_get_thread_num();
		const int calDataSize = (dataSize / numOfThread);
		const int start = threadNum * calDataSize;
		const int end = (threadNum + 1) * calDataSize;


		for (int i = start; i < end; i++)
			DECRYPT_ALGORITHM(A[i], B[i], parallelC[i]);

		if (threadNum == 0) // 사용자 환경에 따른 callThreadNum의 수가 무시될 경우를 상정
			callThreadNum = numOfThread;
	}


	// 공약수가 없을 경우 소수점을 잃어버리면서 마지막 부분이 처리 안됨.
	// ex> DataSize 10 numThreads 4 ,calDataSize : 2, [0,2) , [2,4) , [4,6) , [6,8)
	// ex> DataSize 10 numThreads 3 ,calDataSize : 3, [0,3) , [3,6) , [6,9)
	// ex> DataSize 09 numThreads 2 ,calDataSize : 4, [0,4) , [4,8)
	// ex> DataSize 13 numThreads 3 ,calDataSize : 4, [0,4) , [4,8) , [8,12) 
	{
		const int count = dataSize - (int)(dataSize / callThreadNum) * (callThreadNum);
		for (int i = 0; i < count; i++)
			DECRYPT_ALGORITHM(A[(dataSize - 1) - i], B[(dataSize - 1) - i], parallelC[(dataSize - 1) - i]);
	}


	// **************************************************************
	timer.offTimer(1);

	// Check the results
	bool isCorrect = true;
	for (int i = 0; i < dataSize; i++) {
		if (serialC[i] != parallelC[i]) {
			isCorrect = false;
			break;
		}
	}

	if (isCorrect)
		printf("The results is correct - Good job!\n");
	else
		printf("The result is not correct! :(\n");

	printf("Your computer has %d logical cores\n", omp_get_num_procs());
	timer.printTimer();


	if (!writeData(argv[5], dataSize, parallelC))
		printf("Fail to write the data\n");
	else
		printf("The decrption result was written to %s\n", argv[5]);
}

uchar* readData(char* _fileName, int _dataSize)
{
	uchar* data;
	data = new uchar[_dataSize];
	memset(data, 0, sizeof(uchar) * _dataSize);

	FILE* fp = NULL;
	fopen_s(&fp, _fileName, "rb");
	if (!fp) {
		printf("Fail to open %s\n", _fileName);
		return NULL;
	}

	fread(data, sizeof(uchar), _dataSize, fp);
	fclose(fp);

	return data;
}

bool writeData(char* _fileName, int _dataSize, uchar* _data)
{
	FILE* fp = NULL;
	fopen_s(&fp, _fileName, "wb");
	if (!fp) {
		printf("Fail to open %s\n", _fileName);
		return false;
	}

	fwrite(_data, sizeof(uchar), _dataSize, fp);
	fclose(fp);

	return true;
}
