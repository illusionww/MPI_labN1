#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#define tau 0.001
#define h   0.005

#define nT floor(1.0/tau)
#define nX floor(1.0/h)

#define SXLEN floor(nX/coreNum)
#define MXLEN (nX - (coreNum - 1)*SXLEN)

#define PHI(k) 1.0 - (double)k*tau // it's k, [t][0]
#define PSI(m) 1.0 - (double)m*h   // it's m, [0][x]
#define F   1.0

#define TYPE 2

void masterThread();
void slaveThread(int coreNum, int rank);
void wr(double **u, double ***n, int coreNum);

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
        printf("MXLEN: %f\n", MXLEN);
        printf("SXLEN: %f\n", SXLEN);
        masterThread(coreNum, rank); 
    } else {
        slaveThread(coreNum, rank);
    }
    
    MPI_Finalize();
}

void masterThread(int coreNum, int rank) {  
    int k; // it's T
    int m; // it's X
    
    // allocate memory
    double **u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc((MXLEN+1) * sizeof (double));
    
    // initial&boundary conditions
    for (k = 0; k <= nT; k++)
        u[k][0] = PHI(k);
    for (m = 0; m <= MXLEN; m++)
        u[0][m] = PSI(m);
    
    // second layer
    for (m = 1; m <= MXLEN; m++)
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;
    if (coreNum > 1) {
        MPI_Send(&(u[1][(int)MXLEN]), 1, MPI_DOUBLE, 1, TYPE, MPI_COMM_WORLD);
    }
    
    // main cycle
    for (k = 1; k < nT; k++) {
        for (m = 1; m < MXLEN; m++)
            u[k+1][m] = u[k-1][m] - tau/h*(u[k][m+1] - u[k][m-1]) + 2*tau*F;
        m = (int)MXLEN;
        u[k+1][m] = u[k][m] - tau/h*(u[k][m] - u[k][m-1]) + tau*F;
        if (rank+1 != coreNum) {
            MPI_Send(&(u[k+1][m]), 1, MPI_DOUBLE, rank+1, TYPE, MPI_COMM_WORLD);
        }
    }
    
    int i;
    double ***n = (double ***) malloc((coreNum-1) * sizeof (double **));
    for (i = 0; i < coreNum-1; i++){
        n[i] = (double **) malloc((nT+1) * sizeof (double *));
        for (k = 0; k < (nT+1); k++) {
            n[i][k] = (double *) malloc(SXLEN * sizeof (double));
        }
    }
    
    for (i = 0; i < coreNum-1; i++) {
        for (k = 0; k < nT; k++) {
            MPI_Recv(&(n[i][k][0]), (SXLEN+1), MPI_DOUBLE, i+1, TYPE, MPI_COMM_WORLD, &status);
        }
    }
    
    wr(u, n, coreNum);
}

void slaveThread(int coreNum, int rank) {
    int k; // it's T
    int m; // it's X
    
    // allocate memory
    double **u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc((SXLEN+1) * sizeof (double));
    
    // initial conditions
    for (m = 0; m <= SXLEN; m++)
        u[0][m] = PSI((int)(MXLEN + SXLEN*(rank-1) + m));
    
    // second layer
    for (m = 1; m <= SXLEN; m++)
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;
    if (rank+1 != coreNum)
        MPI_Send(&(u[1][(int)SXLEN]), 1, MPI_DOUBLE, rank+1, TYPE, MPI_COMM_WORLD);
    
    // main cycle
    for (k = 1; k < nT; k++) {
        MPI_Recv(&(u[k][0]), 1, MPI_DOUBLE, rank-1, TYPE, MPI_COMM_WORLD, &status);
        
        for (m = 1; m < SXLEN; m++)
            u[k+1][m] = u[k-1][m] - tau/h*(u[k][m+1] - u[k][m-1]) + 2*tau*F;
        m = (int)SXLEN;
        u[k+1][m] = u[k][m] - tau/h*(u[k][m] - u[k][m-1]) + tau*F;
        if (rank+1 != coreNum) 
            MPI_Send(&(u[k+1][m]), 1, MPI_DOUBLE, rank+1, TYPE, MPI_COMM_WORLD);
    }
    
    for (k = 0; k < nT; k++) {
        MPI_Send(&(u[k][0]), (SXLEN+1), MPI_DOUBLE, 0, TYPE, MPI_COMM_WORLD);
    }
    
    char str[15];
    sprintf(str, "%d", rank);
    FILE *fp = fopen(str, "wb");
    for (k = 0; k <= nT; k++) {
        for (m = 0; m <= SXLEN; m++) {
            char str[80] = "";
            sprintf(str, "%d %d %f\n", k, m, u[k][m]);
            fwrite(str, strlen(str), 1, fp);
        }
    }
    fclose(fp);
}

void wr(double **u, double ***n, int coreNum) {
    FILE *fp = fopen("output.txt", "wb");
    if(fp == NULL) {
        printf("govno");
        return;
    }
    
    int k;
    for (k = 0; k <= nT; k++) {
        char p_string[15] = "";
        sprintf(p_string, "%d %d\n", k, (int)nX+1);
        fwrite(p_string, strlen(p_string), 1, fp);
        
        int m;
        for (m = 0; m <= MXLEN; m++) {
            char str[80] = "";
            sprintf(str, "%d %f %f\n", m+1, m*h, u[k][m]);
            fwrite(str, strlen(str), 1, fp);
        }
        
        int i;
        for (i = 0; i < coreNum-1; i++) {
            for (m = 1; m <= SXLEN; m++) {
                char str[80] = "";
                int t = (int)MXLEN + SXLEN*i + m;
                sprintf(str, "%d %f %f\n", t+1, t*h, n[i][k][m]);
                fwrite(str, strlen(str), 1, fp);
            }
        }
        
        if (k < nT) {
            fwrite("\n", strlen("\n"), 1, fp);
            fwrite("\n", strlen("\n"), 1, fp);
        }
    }
    fclose(fp);
}
