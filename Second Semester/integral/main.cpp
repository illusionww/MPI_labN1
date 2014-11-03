/*
 *	Integrate f(x) = sin(1/(3-x)) from 0 to 2.99.
 *
 *  Created by Ivashkin Vladimir on 03.11.14.
 */

#include <stdio.h>
#include <math.h>

#define f(x) sin(1.0/x)
#define h(x) x*x*0.001
#define ih(x) x*0.002
#define start 0.01
#define end 3.0

int main(int argc, char *argv[]) {
    double x = start;
    double result = 0.0;
    long n =



#pragma omp parallel while
    for (int i = 0; i < n; i++) {
        x = ih(end) - ih(start);
        result += f(x) * h(x);
    }

    printf("J = %.10lf\n", result);

    return 0;
}