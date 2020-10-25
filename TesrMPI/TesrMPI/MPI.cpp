#include <stdio.h>
#include "mpi.h"

int* getElement(int* mas, int size, int i, int j) {
    return(&mas[size * i + j]);
}

void printMatrix(int* mas, int m) {
    printf("\n");
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++)
            printf("%i ", *getElement(mas, m, i, j));
        printf("\n");
    }
    printf("\n");
}

void getMatrix(int* mas, int* p, int i, int j, int m, int n) {
    int ki, kj, di, dj;
    di = 0;
    for (ki = 0; ki < m - 1; ki++) {
        if (ki == i) di = 1;
        dj = 0;
        for (kj = 0; kj < m - 1; kj++) {
            if (kj == j) dj = 1;
            *getElement(p, n, ki, kj) = *getElement(mas, m, ki + di, kj + dj);
        }
    }
}

int determinant(int* mas, int m) {
    int i, j, d, k, n;

    j = 0; d = 0; 
    k = 1; 
    n = m - 1;

    if (m == 1) {
        d = *getElement(mas, m, 0, 0);
        return(d);
    }
    else {
        int* p = new int[n * n];
        for (i = 0; i < m; i++) {
            getMatrix(mas, p, i, 0, m, n);
            d += k * (*getElement(mas, m, i, 0)) * determinant(p, n);
            k = -k;
        }
        delete(p);
    }
    return(d);
}


int main(int* argc, char** argv) {
	int numtasks, rank;
	
	MPI_Init(argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int size = 0;
    int d = 0;
    int* mas;
    MPI_Status stat;

    if (rank == 0) {
        scanf_s("%i", &size);
        mas = new int[size * size];
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                *getElement(mas, size, i, j) = i + j;
            }
        }

        double starttime, endtime;
        starttime = MPI_Wtime();

        int dest = 1;
        int k = 1;
        int n = size - 1;
        int* subMatrix = new int[n * n];
        for (int i = 0; i < size; ++i) {
            getMatrix(mas, subMatrix, i, 0, size, n);
            if (dest < numtasks) {
                int data[3] = {n, *getElement(mas, size, i, 0) , k};
                MPI_Send(&data, 3, MPI_INT, dest, 0, MPI_COMM_WORLD);
                MPI_Send(subMatrix, n * n, MPI_INT, dest, 0, MPI_COMM_WORLD);
                k = -k;
                ++dest;
            }
            else {
                d += k * (*getElement(mas, size, i, 0)) * determinant(subMatrix, n);
                k = -k;
            }
        }
        for (int i = 1; i < dest; ++i) {
            int result = 0;
            MPI_Recv(&result, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &stat);
            d += result;
        }

        endtime = MPI_Wtime();
        printf("That took %f seconds\n", endtime - starttime);

        printf("%i\n", d);
        
        delete(subMatrix);
        delete(mas);
    }
    else {
        int element = 0, k = 1;
        int data[3];
        MPI_Recv(&data, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);

        size = data[0];
        element = data[1];
        k = data[2];

        mas = new int[size * size];
        MPI_Recv(mas, size * size, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);

        d = k * element * determinant(mas, size);
        MPI_Send(&d, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        delete(mas);
    }

    MPI_Finalize();
	return 0;
}