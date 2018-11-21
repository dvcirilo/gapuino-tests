// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>

#define CORE_NUMBER   (8)
#define SEED          (10)
#define V_MAX         (1200)
#define V_MIN         (1000)
#define V_STEP        (50)
#define F_MAX         (350000000)
#define F_MIN         (150000000)
#define F_STEP        (5000000)
#define F_DIV         (1000000)

void random_gen(void *arg)
{
    int *L1_mem = (int *)arg;
    int i;
    unsigned int seed=SEED;
    /*if(__core_ID() == 4)*/
        /*seed = 12;*/
    for(i=0;i<1000000;i++)
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

int main()
{
    int set_voltage_return;
    FLL_SetFrequency(uFLL_SOC, 100000000, 1);
    printf("Default FC Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    random_gen(NULL);

    clock_t begin = clock();
    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);
    printf("%d\n",CLUSTER_GetCoreMask());

    int *L1_mem = L1_Malloc(8);

    for (int i = V_MAX; i>=V_MIN; i-=V_STEP){
        set_voltage_return = PMU_SetVoltage(i,0);

        if (set_voltage_return) {
            printf("Failed to change voltage! Code: %d\n",set_voltage_return);
        }

        printf("Voltage: %lu - FCMaxFreq: %d MHz - FCMaxFreq: %d MHz\n",
                current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
                FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);

        for (int frequency = F_MIN; frequency <= F_MAX; frequency+=F_STEP) {
            /* Set frequency */
            if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 0) == -1) {
                printf("Error of changing frequency, check Voltage value!\n");
            }

            /* FC send a task to Cluster */
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            if (L1_mem[0]^L1_mem[1]^L1_mem[2]^L1_mem[3]^L1_mem[4]^L1_mem[5]^L1_mem[6]^L1_mem[7]){
                printf("Diff at: Cluster Freq: %d MHz - Voltage: %lu mV\n",
                        FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
                break;
            }
        }
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    /*clock_gettime(CLOCK_REALTIME, &end);*/
    printf("elapsed: %d us\n", (clock()-begin)/CLOCKS_PER_SEC);


    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
