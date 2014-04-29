#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#define tau 0.001
#define h   0.125

#define nT floor(1.0/tau)
#define nX floor(1.0/h)

#define SXLEN (int)floor(nX/coreNum)
#define MXLEN (int)(nX - (coreNum - 1)*SXLEN)

#define PHI(k) 1.0 - (double)k*tau // it's k, [t][0]
#define PSI(m) 1.0 - (double)m*h   // it's m, [0][x]
#define F(k,m) 1.0

#define TYPE 2

void masterThread();
void slaveThread(int coreNum, int rank);

MPI_Status status;

int main(int argc, char* argv[]) {
    int rank, coreNum;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &coreNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            
    if (rank == 0) {
        if (tau/h > 1.0)
            printf("warning: unstable scheme\n");
        printf("number of cores: %d\n", coreNum);
        printf("nX: %f\n", nX);
        masterThread(coreNum, rank); 
    } else {
        slaveThread(coreNum, rank);
    }
    
    MPI_Finalize();
}

void masterThread(int coreNum, int rank) {
    double **u;
    
    int k; // it's T
    int m; // it's X
    
    // allocate memory
    u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc(MXLEN * sizeof (double));
    
    // initial&boundary conditions
    for (k = 0; k <= nT; k++)
        u[k][0] = PHI(k);
    for (m = 0; m <= MXLEN; m++)
        u[0][m] = PSI(m);
    
    // second layer
    for (m = 1; m <= MXLEN; m++)
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;
    
    // main cycle
    for (k = 1; k < nT; k++) {
        for (m = 1; m < MXLEN; m++)
            u[k+1][m] = u[k-1][m] - tau/h*(u[k][m+1] - u[k][m-1]) + 2*tau*F(k, m);
        u[k+1][m] = u[k][m] - tau/h*(u[k][m] - u[k][m-1]) + tau*F(k, MXLEN);
        if (rank+1 != coreNum) 
            MPI_Send(&u[k+1][m], 1, MPI_DOUBLE, rank+1, TYPE, MPI_COMM_WORLD);
    }
    
    int i;
    double ***n = (double ***) malloc((coreNum-1) * sizeof (double **));
    for (i = 1; i < coreNum; i++){
        n[i] = (double **) malloc((nT+1) * sizeof (double *));
        for (k = 0; k < (nT+1); k++) {
            n[i][k] = (double *) malloc(SXLEN * sizeof (double));
        }
    }
    
    for (i = 1; i < coreNum; i++) {
        MPI_Recv(&n[i][0][0], (nT+1)*(SXLEN+1), MPI_DOUBLE, i, TYPE, MPI_COMM_WORLD, &status);
    }
    
}

void slaveThread(int coreNum, int rank) {
    double **u;
    
    int k; // it's T
    int m; // it's X
    
    // allocate memory
    u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc((SXLEN+1) * sizeof (double));
    
    // initial conditions
    for (m = 0; m <= SXLEN; m++)
        u[0][m] = PSI(m + nX - (coreNum - rank)*floor(nX/coreNum));
    
    // second layer
    for (m = 0; m <= SXLEN; m++)
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;
    
    // main cycle
    for (k = 1; k < nT; k++) {
        MPI_Recv(&u[k+1][0], 1, MPI_DOUBLE, rank-1, TYPE, MPI_COMM_WORLD, &status);
        
        for (m = 1; m < SXLEN; m++)
            u[k+1][m] = u[k-1][m] - tau/h*(u[k][m+1] - u[k][m-1]) + 2*tau*F(k, m);
        u[k+1][m] = u[k][m] - tau/h*(u[k][m] - u[k][m-1]) + tau*F(k, SXLEN);
        if (rank+1 != coreNum) 
            MPI_Send(&u[k+1][m], 1, MPI_DOUBLE, rank+1, TYPE, MPI_COMM_WORLD);
    }
    
    MPI_Send(&u[0][0], (nT+1)*(SXLEN+1), MPI_DOUBLE, 0, TYPE, MPI_COMM_WORLD);
}