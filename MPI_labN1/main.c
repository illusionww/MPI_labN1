#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define tau 0.001
#define h   0.005

#define nT floor(1.0/tau)
#define nX floor(1.0/h)

#define PHI 1.0 - (double)k*tau // it's k, [t][0]
#define PSI 1.0 - (double)m*h   // it's m, [0][x]
#define F   1.0

void err(char methodName[]);
void showAll();
void write();

double **u;

int main(int argc, char* argv[]) {
    int k; // it's T
    int m; // it's X

    if (tau/h > 1.0)
        printf("warning: unstable scheme\n");
    
    // allocate memory
    u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc((nX+1) * sizeof (double));

    for (k = 0; k <= nT; k++)
        u[k][0] = PHI;

    for (m = 0; m <= nX; m++)
        u[0][m] = PSI;

    for (m = 1; m <= nX; m++)
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;

    for (k = 1; k < nT; k++) {
        for (m = 1; m < nX; m++)
            u[k+1][m] = u[k-1][m] - tau/h*(u[k][m+1] - u[k][m-1]) + 2*tau*F;
        m = nX;
        u[k+1][m] = u[k][m] - tau/h*(u[k][m] - u[k][m-1]) + tau*F;
    }

    write();
    
    return 0;
}

void err(char methodName[]) {
    printf("error in %s\n", methodName);
    exit(1);
}

void showAll() {
    int k, m;

    printf("\n");
    for (k = nT; k >= 0; k--) {
        for (m = 0; m <= nX; m++) {
            printf("%f ", u[k][m]);
        }
        printf("\n");
    }
    printf("\n");
}

void write() {
    int k, m;
    FILE *fp;
    
    char name[80] = "output.txt";
    fp = fopen(name, "wb");
    printf("name: %s\n", name);
    if(fp == NULL) {
        printf("govno");
        return;
    }
        
    for (k = 0; k <= nT; k++) {
        char p_string[15] = "";
        sprintf(p_string, "%d %d\n", k, (int)nX+1);
        fwrite(p_string, strlen(p_string), 1, fp);
        for (m = 0; m <= nX; m++) {
            char str[80] = "";
            sprintf(str, "%d %f %f\n", m+1, (double)m*h, u[k][m]);
            fwrite(str, strlen(str), 1, fp);
        }
        fwrite("\n", strlen("\n"), 1, fp);
        fwrite("\n", strlen("\n"), 1, fp);
    }
    fclose(fp);
}
