#include <stdlib.h>
#include <math.h>
#include "gap_common.h"
#include "rtx_lib.h"
#include "rtx_core_c.h"

// FEATURE_CLUSTER
/*#include "gap_dmamchan.h"*/

#define FC_FREQ  (50000000)
#define F_DIV    (1000)
#define RUNS     (1000000000)
#define TIMER    TIMER1

performance_t perf;

int main()
{
    unsigned int time_ms=0;
    int value=18;
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);
    printf("FC Frequency: %d kHz\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV);

    /* Resets and initialize TIMER (on FC) */
    Timer_Initialize(TIMER, 0);
    Timer_Enable(TIMER);

    for (int i = 0; i < RUNS; i++) {
        if (i % 100000 == 0){
            time_ms = (unsigned int)(Timer_ReadCycle(TIMER) 
                    / (FLL_GetFrequency(uFLL_SOC)/1000));
            printf("%d - %d\n" , time_ms, i);
        }
        wait(10);
    }
    Timer_Disable(TIMER);
    exit(0);
}
