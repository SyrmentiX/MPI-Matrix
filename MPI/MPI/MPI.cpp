#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

const int MAX_NUMBER = 1500;
const int MIN_NUMBER = 1;

using namespace std;

int min(int left, int right) {
	if (left > right)
		return right;
	return left;
}

int getStep(int size, int numtask, int rank) {
	if (size % numtask == 0) {
		return size / numtask;
	}
	if (rank == 0) {
		int h = size / (numtask - 1);
		return size - (numtask - 1) * h;
	}
	return size / (numtask - 1);
}

void merge(int* array, int left, int mid, int right) {
	int it1 = 0;
	int it2 = 0;
	int *res = new int[right - left];

	while ((left + it1 < mid) && (mid + it2 < right)) {
		if (array[left + it1] < array[mid + it2]) {
			res[it1 + it2] = array[left + it1];
			++it1;
		} else {
			res[it1 + it2] = array[mid + it2];
			++it2;
		}
	}

	while (left + it1 < mid) {
		res[it1 + it2] = array[left + it1];
		++it1;
	}

	while (mid + it2 < right) {
		res[it1 + it2] = array[mid + it2];
		++it2;
	}

	for (int i = left; i < right; ++i) {
		array[i] = res[i - left];
	}

	delete[] res;
}

int main(int *argc, char **argv) {

	double starttime, endtime;
	int numtask, rank;

	MPI_Init(argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtask);
	
	int size = 0;
	int *array = NULL;
	int *rarray = NULL;
	int *sendcounts = new int[numtask];
	int* displs = new int[numtask];

	if (rank == 0) {
		scanf_s("%d", &size);
		array = new int[size];
	
		displs[0] = size - getStep(size, numtask, 0);
		sendcounts[0] = getStep(size, numtask, 0);

		starttime = MPI_Wtime();

		for (int i = 1; i < numtask; ++i) {
			int counts = getStep(size, numtask, i);
			sendcounts[i] = counts;
			displs[i] = counts * (i - 1);
		}

		for (int i = 0; i < size; ++i) {
			array[i] = rand() % MAX_NUMBER + MIN_NUMBER;
			printf("%d ", array[i]);
		}
		printf("\n\n\n");
	}

	MPI_Bcast(sendcounts, numtask, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(displs, numtask, MPI_INT, 0, MPI_COMM_WORLD);

	int step = sendcounts[rank];
	rarray = new int[step];
	
	MPI_Scatterv(array, sendcounts, displs, MPI_INT, rarray, step, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 1; i < step; i *= 2) {
		for (int j = 0; j < step - i; j += 2 * i) {
			merge(rarray, j, j + i, min(j + 2 * i, step));
		}
	}

	MPI_Gatherv(rarray, step, MPI_INT, array, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0) {		
		for (int i = sendcounts[numtask - 1]; i < size; i *= 2) {
			for (int j = 0; j < size - i; j += 2 * i) {
				merge(array, j, j + i, min(j + 2 * i, size));
			}
		}
		endtime = MPI_Wtime();
		
		for (int i = 0; i < size; ++i) {
			printf("%d ", array[i]);
		}
		printf("\n");

		
		printf("That took %f seconds\n", endtime - starttime);
	}

	delete[] rarray;
	delete[] array;
	delete[] sendcounts;
	delete[] displs;
	MPI_Finalize();
	return 0;
}
/*
* 
* if (step != 0) {
		for (int i = step * rank; i < )
	}
function mergeSortIterative(a : int[n]):
	for i = 1 to n, i *= 2
		for j = 0 to n - i, j += 2 * i
			merge(a, j, j + i, min(j + 2 * i, n))
*/

