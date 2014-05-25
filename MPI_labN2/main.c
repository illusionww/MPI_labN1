#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define f(x) sin(1.0/x)
#define h(x) min(x*x*0.01, 0.00001)

#define start 0.01
#define end 3.0
#define thread_num 7

double *res;
double *bounds;

void *thread_func(void *arg) {
	int j = (int)arg;
	double x = bounds[j];
	
        printf("Core %d from %f to %f\n", j+1, bounds[j], bounds[j+1]);
        
	while(x < bounds[j+1]) {
            res[j] += f(x)*h(x);
            x += h(x);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {	
    double x = start;
    double result = 0.0;
    int n = 0;
    
    while (x + h(x) < end) {
        x += h(x);
        n++;
    }  
    
    printf("%d\n", n);
    x = start;
    
    pthread_t threads[thread_num];
    bounds = calloc(thread_num, sizeof(double));
    res = calloc(thread_num, sizeof(double));

    int i;
    for(i = 0; i <= thread_num - 1; i++) {
        int j = 0;
        while (j < (i + 1) * n / thread_num) {
            x += h(x);
            j++;
        }  
        bounds[i] = x;
        x = start;
        printf("%f ", bounds[i]);
    }
    printf("\n");

    for(i = 0; i < thread_num - 1; i++) {
        if(pthread_create(&threads[i], NULL, &thread_func, (void*)i)){
            printf("Error during creating thread #%d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    printf("Core %d from %f to %f\n", 0, 0.0, bounds[0]);
    while(x < bounds[0]) {
            result += f(x)*h(x);
            x += h(x);
    }
        
    for(i = 0; i < thread_num - 1; i++) {
        if(pthread_join(threads[i], NULL)){
            printf("Error during waitng thread #%d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    
    for(i = 0; i <= thread_num - 1; i++) {
        result += res[i];
    }

    printf("J = %.10lf\n", result);
    
    return 0;
}