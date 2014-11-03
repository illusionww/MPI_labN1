#include <stdio.h>
#include <stdlib.h>

#define ISIZE  10000
#define JSIZE  10000

int main(int argc, char **argv) {
    double a[ISIZE][JSIZE];
    int i, j;
    FILE *ff;

/*    #pragma omp parallel for
    for (i=0; i<ISIZE; i++) {
        for (j=0; j<JSIZE; j++) {
              a[i][j] = 10*i + j;
        }
    }
    ff = fopen("aaas","w");    
    for(i=0; i < ISIZE; i++) {
        for (j=0; j < JSIZE; j++) {
            fprintf(ff,"%f ",a[i][j]);
        }
        fprintf(ff,"\n");
    }*/      
}
