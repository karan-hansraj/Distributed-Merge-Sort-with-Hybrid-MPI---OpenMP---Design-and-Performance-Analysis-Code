#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 100000000

void merge(int arr[], int low, int mid, int high)
{
    int i, j, k;
    int n1 = mid - low + 1;
    int n2 = high - mid;

    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++)
        L[i] = arr[low + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0; 
    j = 0; 
    k = low;

    while (i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];

    while (i < n1)
        arr[k++] = L[i++];

    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}

void mergeSort(int arr[], int low, int high)
{
    if (low < high)
    {
        int mid = (low + high) / 2;
        mergeSort(arr, low, mid);
        mergeSort(arr, mid + 1, high);
        merge(arr, low, mid, high);
    }
}

void finalMerge(int *A, int n1, int *B, int n2)
{
    int *temp = (int *)malloc((n1 + n2) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < n1 && j < n2)
        temp[k++] = (A[i] <= B[j]) ? A[i++] : B[j++];

    while (i < n1) temp[k++] = A[i++];
    while (j < n2) temp[k++] = B[j++];

    for (i = 0; i < n1 + n2; i++)
        A[i] = temp[i];

    free(temp);
}

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_n = N / size;
    int *local_arr = (int *)malloc(local_n * sizeof(int));
    int *arr = NULL;

    if (rank == 0)
    {
        arr = (int *)malloc(N * sizeof(int));
        srand(time(NULL));
        for (int i = 0; i < N; i++)
            arr[i] = rand();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    MPI_Scatter(arr, local_n, MPI_INT,
                local_arr, local_n, MPI_INT,
                0, MPI_COMM_WORLD);

    mergeSort(local_arr, 0, local_n - 1);

    MPI_Gather(local_arr, local_n, MPI_INT,
               arr, local_n, MPI_INT,
               0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        int curr_size = local_n;

        for (int i = 1; i < size; i++)
        {
            finalMerge(arr, curr_size, arr + i * local_n, local_n);
            curr_size += local_n;
        }

        double end = MPI_Wtime();
        printf("MPI Merge Sort completed\n");
        printf("Elements: %d\n", N);
        printf("Processes: %d\n", size);
        printf("Time taken: %.3f seconds\n", end - start);
    }

    free(local_arr);
    if (rank == 0) free(arr);

    MPI_Finalize();
    return 0;
}
