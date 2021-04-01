
#include <mpi.h>

#include <stdlib.h>
#include <stdio.h>

#include <math.h>

double* serial_mergesort(double array[], int n);
double* serial_merge(double array1[], int n1, double array2[], int n2);
double* mergesort(double array[], int n);
double* merge(double* array, int n);

void printArray(double x[], int n) {
	printf("[ ");
	for (int i = 0; i < n; i++) {
		printf("%lf ", x[i]);
	}
	printf("]\n");
}

//Random double between 0-100;
double rand_double() {
	double r = (double)rand() / RAND_MAX; //Random between 0-1;
	return r*100;
}

int p = 0;
int rank;

int main (int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (rank == 0) {
		printf("Cores: %d\n", p);
	}

	double* array = NULL;
	int n = 120000000;

	if (rank == 0) {
		array = malloc(n * sizeof(double));
		for (int i = 0; i < n; i++) {
			array[i] = rand_double();
		}
	}
	
	double start = MPI_Wtime();
	double* sArray = mergesort(array, n);//serial_mergesort(array, n);
	double end = MPI_Wtime();

	if (rank == 0) {
		for (int i = 1; i < n; i++) {
			if (sArray[i-1] > sArray[i]) {
				printf("%d, FALSE!", rank);
				break;
			}
		}
		printf("%d	%lf\n", p, end-start);
	}

	free(array);
	free(sArray);

	MPI_Finalize();
	return 0;
}

double* mergesort(double array[], int n) {
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int partition = n/p; //Assumed to be dividable
	double* pArray = malloc(partition * sizeof(double));

	MPI_Scatter(array, partition, MPI_DOUBLE,
				pArray, partition, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	double* sArray = serial_mergesort(pArray, partition);
	free(pArray);

	return merge(sArray, partition);
}

double* merge(double* array, int n) {
	int i = 0;
	int pow2i = 1;
	double* sArray = array;
	while ((rank/pow2i) % 2 == 0) {
		if (rank + pow2i < p) {
			double* rec = malloc(n*sizeof(double));
			MPI_Recv(rec, n, MPI_DOUBLE, rank+pow2i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			double* newsArray = serial_merge(sArray, n, rec, n);
			free(sArray);
			free(rec);
			sArray = newsArray;
			n = n*2;
		} else {
			break;
		}
		i++;
		pow2i = pow2i * 2;
	}
	if (rank != 0) {
		MPI_Send(sArray, n, MPI_DOUBLE, rank-pow2i, 0, MPI_COMM_WORLD);
	}

	return sArray;
}

double* serial_mergesort(double array[], int n) {
	if (n == 1) {
		double* res = malloc(sizeof(double));
		res[0] = array[0];
		return res;
	}
	// printf("Split \n");
	int mid = n/2;
	double* head = &(array[0]); //size [mid];
	// printf("Head: ");
	// printArray(head, mid);
	double* tail = &(array[mid]); //size [n-mid];
	// printf("Tail: ");
	// printArray(tail, n-mid);

	double* sHead = serial_mergesort(head, mid);
	double* sTail = serial_mergesort(tail, n-mid);

	double* res = serial_merge(sHead, mid, sTail, n-mid);
	free(sHead);
	free(sTail);

	// printf("Merged: ");
	// printArray(res, n);
	return res;
}

double* serial_merge(double array1[], int n1, double array2[], int n2) {
	int i1 = 0;
	int i2 = 0;

	int i = 0;
	double* res = malloc((n1+n2)*sizeof(double));

	while (i1 != n1 && i2 != n2) {
		if (array1[i1] < array2[i2]) {
			res[i] = array1[i1];
			i1++;
		} else {
			res[i] = array2[i2];
			i2++;
		}
		i++;
	}
	
	while (i1 != n1) {
		res[i] = array1[i1];
		i1++;
		i++;
	}

	while (i2 != n2) {
		res[i] = array2[i2];
		i2++;
		i++;
	}

	return res;
}