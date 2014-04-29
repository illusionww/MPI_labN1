#include<stdio.h>
#include<mpi.h>
#include<stdlib.h>

int main(int argc, char* argv[]) {
    int n;
    int rank, coreNum, type = 2;
    MPI_Status status;

    n = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &coreNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        double sum = 0;
        int frame = n / coreNum;
        int i;

        printf("core count = %d\n", coreNum);

        for (i = 1; i <= frame; i++) {
            sum += 1.0 / i;
        }

        for (i = 1; i < coreNum; i++) {
            int left = i* frame;
            int right = (i + 1) * frame > n ? n : (i + 1) * frame;

            MPI_Send(&left, 1, MPI_INT, i, type, MPI_COMM_WORLD);
            MPI_Send(&right, 1, MPI_INT, i, type, MPI_COMM_WORLD);
        }

        for (i = 1; i < coreNum; i++) {
            double middleSum;
            MPI_Recv(&middleSum, 1, MPI_DOUBLE, i, type, MPI_COMM_WORLD, &status);
            sum += middleSum;


        }
        printf("summa = %g\n", sum);
    } else {
        double sum = 0;
        int left, right;
        int i;

        MPI_Recv(&left, 1, MPI_INT, 0, type, MPI_COMM_WORLD, &status);
        MPI_Recv(&right, 1, MPI_INT, 0, type, MPI_COMM_WORLD, &status);

        for (i = left + 1; i <= right; i++) {
            if (i != 0) {
                sum += 1.0 / i;
            }
        }

        printf("%d: %d..%d\tsum = %g\n", rank, left, right, sum);

        MPI_Send(&sum, 1, MPI_DOUBLE, 0, type, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}
