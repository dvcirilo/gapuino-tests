#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gap_common.h"
#include "gap_cluster.h"
#define FC_FREQ       (300000000)
#define CLUSTER_FREQ  (200000000)
#define F_DIV         (1000000)
#define NPOINTS       (10000000)
//#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)
#define DATA_MAX      (10)
#define DEBUG

int split;

void pi (void* arg){
    float factor;
    int n = (__core_ID()+1)*split;
    float *sum = (float *) arg;
    for (int i = split*__core_ID(); i < n; i++) {
      factor = (i % 2 == 0) ? 1 : -1; 
      sum[__core_ID()] += factor/(2*i+1);
#     ifdef DEBUG
      printf("Thread %d > i = %d, my_sum = %d\n",
             __core_ID(), i, sum[__core_ID()]);
#     endif
    }

}
void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(pi, (void *) L1_mem);
}

int main() {
    int n, i;
    int thread_count;
    float sum = 0;

    thread_count = CORE_NUMBER;
    n = 80;
    CLUSTER_Start(0,CORE_NUMBER);
    float *L1_mem= L1_Malloc(CORE_NUMBER*sizeof(float));
    for (int i = 0; i < CORE_NUMBER; i++) {
       L1_mem[i] = 0;
    }
    split = n/CORE_NUMBER;
    CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
    printf("Waiting...\n");

    CLUSTER_Wait(0);

    for (int i = 0; i < CORE_NUMBER; i++) {
       sum += L1_mem[i];
    }
    L1_Free(L1_mem, CORE_NUMBER*sizeof(float));
    CLUSTER_Stop(0);
    sum = 4.0*sum;
    printf("With n = %d terms and %d threads,\n", n, thread_count);
    printf("   Our estimate of pi = %d\n", (int)(sum*100000));
    printf("                   pi = %d\n", (int)( 4.0*atan(1.0)*100000 ));
    exit (0);
}  /* main */


