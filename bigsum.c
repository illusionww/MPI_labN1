/*
 *  Additional task. Parallel Algorithms
 *	Sum of two big numbers using MPI.
 *
 *  Created by Ivashkin Vladimir on 25.05.14.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

#define max(a,b) a > b ? a : b;
#define min(a,b) a < b ? a : b;

MPI_Status status;
int rank;

void sum(char* s1, char* s2, int len) {
    int i;
    
    printf("rank: %d, sum: ", rank);
    for (i = 0; i < len + 1; i++)
        printf("%d", s1[i]);
    printf(" + ");
    for (i = 0; i < len; i++)
        printf("%d", s2[i]);
    printf(" = ");
    
    for (i = len; i > 0; i--) {
        s1[i] += s2[i - 1];
        if (s1[i] > 9) {
            s1[i] -= 10;
            s1[i-1]++;
        }
    }
    for (i = 0; i < len + 1; i++)
        printf("%d ", s1[i]);
    printf("\n");
}

void spp(char* s, int len) {
    printf("rank: %d, spp!\n", rank);
    int i;
    s[len]++;
    for (i = len; i > 0; i--) {
        if (s[i] < 10)
            return;
        else {
            s[i] -= 10;
            s[i-1]++;
        }
    }
}

int main(int argc, char **argv) {
    int coreNum;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &coreNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char *str[2];
    int i, j;

    if (!rank) {
        if (argc < 3) {
            printf("No numbers!\n");
            return 0;
        }

        printf("corenum = %d\n", coreNum);

        int lsrc[] = {
            strlen(argv[1]),
            strlen(argv[2])
        };
        int resLen = max(lsrc[0], lsrc[1]);
        printf("l = %d\n", resLen);
        str[0] = (char *) malloc(resLen);
        str[1] = (char *) malloc(resLen);
        for (i = 0; i < 2; i++)
            for (j = 0; j < resLen; j++)
                str[i][j] = 0;

        int padding[2] = {0, 0};
        if (lsrc[0] < lsrc[1]) {
            padding[0] = lsrc[1] - lsrc[0];
            printf("padding0 = %d\n", padding[0]);
        } else {
            padding[1] = lsrc[0] - lsrc[1];
            printf("padding1 = %d\n", padding[1]);
        }

        for (i = 0; i < 2; i++)
            for (j = 0; j < resLen - padding[i]; j++)
                str[i][padding[i] + j] = argv[i + 1][j] - 48;

        for (i = 0; i < 2; i++) {
            printf("sum %d: ", i);
            for (j = 0; j < resLen; j++)
                printf("%d", str[i][j]);
            printf("\n");
        }

        int slaveLen = (int) floor((float) resLen / (float) coreNum);
        printf("slaveLen: %d\n", slaveLen);
        int masterLen = (int) (resLen - (coreNum - 1) * slaveLen);
        printf("masterLen: %d\n", masterLen);

        for (i = 1; i < coreNum; ++i) {
            MPI_Send(&slaveLen, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&str[0][masterLen + slaveLen * (i - 1)], slaveLen, MPI_CHAR, i, 1, MPI_COMM_WORLD);
            MPI_Send(&str[1][masterLen + slaveLen * (i - 1)], slaveLen, MPI_CHAR, i, 2, MPI_COMM_WORLD);
        }

        char* strTemp = (char *) malloc(masterLen + 1); // +1 для переполнения

        strTemp[0] = 0;
        for (i = 0; i < masterLen; i++)
            strTemp[i + 1] = str[0][i];

        printf("strTemp: ");
        for (i = 0; i < masterLen; i++)
            printf("%d", strTemp[i + 1]);
        printf("\n");
        
        sum(strTemp, str[1], masterLen);

        char* result = (char *) malloc(resLen + 1);
        for (i = 0; i < resLen + 1; i++)
            result[i] = 0;
        char* trash = (char *) malloc(slaveLen + 1);
        for (i = coreNum - 1; i > 0; i--) {
            MPI_Recv(&result[masterLen + slaveLen * (i-1)], slaveLen + 1, MPI_CHAR, i, 1, MPI_COMM_WORLD, &status);
            if (result[masterLen + slaveLen * (i + 1) - 1] == 1)
                MPI_Recv(&result[masterLen + slaveLen * (i-1)], slaveLen + 1, MPI_CHAR, i, 2, MPI_COMM_WORLD, &status);
            else
                MPI_Recv(trash, slaveLen + 1, MPI_CHAR, i, 2, MPI_COMM_WORLD, &status);
            for (j = 0; j < resLen + 1; j++)
                printf("%d", result[j]);
            printf("\n");
        }
        if (result[masterLen] == 1)
            spp(strTemp, masterLen);
        for (i = 0; i < masterLen + 1; i++)
            result[i] = strTemp[i];
        printf("result:");
        for (i = 0; i < resLen + 1; i++)
            printf("%d", result[i]);
        printf("\n");
    } else {
        int len;
        MPI_Recv(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        printf("rank %d, len %d\n", rank, len);
        str[0] = (char *) malloc(len + 1); // +1 для переполнения
        str[1] = (char *) malloc(len);

        str[0][0] = 0;
        MPI_Recv(&str[0][1], len, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
        printf("rank %d, str0: ", rank);
        for (i = 0; i < len + 1; i++)
            printf("%d", str[0][i]);
        printf("\n");
        MPI_Recv(str[1], len, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
        printf("rank %d, str1: ", rank);
        for (i = 0; i < len; i++)
            printf("%d", str[1][i]);
        printf("\n");
        sum(str[0], str[1], len);
        MPI_Send(str[0], len + 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        spp(str[0], len);
        MPI_Send(str[0], len + 1, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
