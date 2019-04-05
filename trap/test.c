//* Compile: gcc -g -Wall -fopenmp -o omp_trap2 omp_trap3.c
#include "cmsis.h"
#include <math.h>
#include "gap_common.h"
#include "mbed_wait_api.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>


#define FC_FREQ       (250000000)
#define CLUSTER_FREQ  (150000000)
#define F_DIV         (1000000)
#define NPOINTS       (10000000)
//#define NUM_THREADS   (8)
#define CORE_NUMBER   (8)
#define DATA_MAX      (10)

typedef struct integral
{
	int a;
	int b;
	int n;
	int soma;

} integral;

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

int f(int x) {
   int return_val;

   return_val = x*x;
   return return_val;
}  /* f */

void Trap(void* arg){
    int approx;
    int i;
    int h;
    integral *prov = (integral *) arg;

    h = (prov[__core_ID()].b-prov[__core_ID()].a)/prov[__core_ID()].n; 
    printf("Core %d - h = %d\n", __core_ID(), h);

    approx = (f(prov[__core_ID()].a) + f(prov[__core_ID()].b))/2; 

    for (i=1;i<prov[__core_ID()].n;i++){
        approx += f(prov[__core_ID()].a + i*h);
    }
    prov[__core_ID()].soma = h*approx;
}  

void Master_Entry(void* arg) 
{
	CLUSTER_CoresFork(Trap, arg);
}


int main()
{
	int global_result = 0; /* Store result in global_result */
	int a=0, b=80000;                 /* Left and right endpoints*/
	int n=80;                    /* Total number of trapezoid*/
	int thread_count=CORE_NUMBER;
    int soma_global = 0;

	CLUSTER_Start(0, CORE_NUMBER);                                       

	integral *integ = (integral *) L1_Malloc(CORE_NUMBER*sizeof(integral));

	int h = (b-a)/n;

	printf("h = %d\n",h);

	for (int i=0;i<thread_count;i++)
	{
		integ[i].soma = 0;
		integ[i].a = a +h*i;
		integ[i].n = n;
		integ[i].b = a+h*(i+1);
		printf("os valores do cluster %d sao a = %d, b = %d, n = %d\n",i,integ[i].a,integ[i].b,integ[i].n);
	}                                                                    

    if (FLL_SetFrequency(uFLL_CLUSTER, CLUSTER_FREQ, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }

	CLUSTER_SendTask(0, Master_Entry,(void*) integ, 0);

    printf("Waiting...\n");

	CLUSTER_Wait(0);

    for (int i = 0; i < thread_count; i++) {
       soma_global += integ[i].soma;
    }

   printf("With n = %d trapezoids, our estimate\n", n);
   printf("of the integral from %d to %d = %d\n",a,b,soma_global);
   exit(0);
}

