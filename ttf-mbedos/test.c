// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>
#include "test.h"

void random_gen(void *arg)
{
    int *L1_mem = (int *)arg;
    unsigned int seed=SEED;
    rand_r(&seed);
    L1_mem[__core_ID()] = rand_r(&seed);
    /*printf("%d\t%d\n", __core_ID(), L1_mem[__core_ID()]);*/
}

void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

void test_rand(int *L1_mem, int frequency, int voltage)
{
    int set_voltage_return, error=0;

    set_voltage_return = PMU_SetVoltage(voltage,0);

    if (set_voltage_return) {
        printf("Failed to change voltage! Code: %d\n",set_voltage_return);
    }
    printf("Voltage: %lu - FCMaxFreq: %d MHz - ClusterMaxFreq: %d MHz\n",
            current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
            FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    } else {
        printf("Set: Cluster Freq: %d MHz - Voltage: %lu mV\n",
                FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
    }

    /* FC send a task to Cluster */
    clock_t begin = clock();

    for(int j=0;j<NUM_TESTS;j++) {
        __disable_irq();
        Timer_Initialize(TIMER1_CLUSTER, 0);
        Timer_Enable(TIMER1_CLUSTER);
        begin = clock();
        printf("run %d ", j);
        for(int i=0;i<RUNS;i++) {
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            if (L1_mem[0]^L1_mem[1]^L1_mem[2]^L1_mem[3]^L1_mem[4]^L1_mem[5]^L1_mem[6]^L1_mem[7]){
                printf("diff after: %d clocks - %d calls\n", (int)Timer_ReadCycle(TIMER1_CLUSTER)/Timer_GetTicksUS(TIMER1_CLUSTER), i);
                error = 1;
                printf("ticks: %d\n", Timer_GetTicksUS(TIMER1_CLUSTER));
                break;
            } else {
                error = 0; 
            }
        }
        __enable_irq();
        if(!error){
            printf("Run %dk times without diff - time:%d \n", RUNS/1000, (int)Timer_ReadCycle(TIMER1_CLUSTER)/Timer_GetTicksUS(TIMER1_CLUSTER));
            error = 0;
        }
        Timer_Disable(TIMER1_CLUSTER);
    }
}

int main()
{
    /*Initialize FC Clock*/
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);

    printf("FC Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());


    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    int *L1_mem = L1_Malloc(8);

    test_rand(L1_mem, 218000000, 1000);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
