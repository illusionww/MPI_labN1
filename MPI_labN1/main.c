#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define T 1
#define X 1
#define tau 0.01
#define h 0.02

#define nT floor((double)T/(double)tau)
#define nX floor((double)X/(double)h)
#define PHI sin(x/300)
#define PSI t*t
#define F cos(t*x)

double phi(int k);
double psi(int m);
double f(int k, int m);
void err(char methodName[]);
void showAll();
void write();

double **u;

int main(int argc, char* argv[]) {
    int k, m;

    if ((double)tau/(double)h > 1.0) {
        printf("warning: unstable scheme\n");
    }
    
    u = (double **) malloc((nT+1) * sizeof (double *));
    for (k = 0; k < (nT+1); k++)
        u[k] = (double *) malloc((nX+1) * sizeof (double));

    for (k = 0; k <= nT; k++) {
        u[k][0] = phi(k);
    }

    for (m = 0; m <= nX; m++) {
        u[0][m] = psi(m);
    }

    for (m = 1; m <= nX; m++) {
        u[1][m] = u[0][m] + tau * u[0][m] + tau*tau/2*(u[0][m+1]-u[0][m-1])/2/h;
    }

    for (k = 1; k < nT; k++) {
        for (m = 1; m < nX; m++) {
            u[k+1][m] = 2*tau*(f(k, m) - (u[k][m+1] - u[k][m-1])/2/h) + u[k-1][m];
        }
    }

    write();
    
    return 0;
}

double phi(int k) {
    double x = (double) k*h;

 //   if (x >= 0 && x <= X) {
        return PHI;
 //   }
    err("phi");
}

double psi(int m) {
    double t = (double) m*h;

//    if (t >= 0 && t <= T) {
        return PSI;
  //  }
    err("psi");
}

double f(int k, int m) {
    double x = (double) k*h;
    double t = (double) m*h;

//    if (x >= 0 && x <= X && t >= 0 && t <= T) {
        return F;
 //   }
    err("function");
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
            printf("%f\t", u[k][m]);
        }
        printf("\n");
    }
    printf("\n");
}

void write() {
    int k, m;
    FILE *fp;
    
    for (k = 0; k <= nT; k++) {
        char name[80] = "";
        char k_string[15];
        
        strcpy(name,"out1/output");        
        sprintf(k_string, "%d", k);
        strcat(name, k_string);
        strcat(name,".txt");
        fp = fopen(name, "wb");
        printf("name: %s\n", name);
        if(fp == NULL) {
            printf("govno");
            return 0;
	}
        for (m = 0; m <= nX; m++) {
            char str[80];
            char u_string[15];
            
            sprintf(u_string, "%f", (double)m*h);
            strcpy(str, u_string);
            strcat(str,"\t");
            sprintf(u_string, "%f", u[k][m]);
            strcat(str, u_string);
            strcat(str,"\n");
            fwrite(str, strlen(str), 1, fp);
        }
        fclose(fp);
    }
}