#include "rt/rt_api.h"

int main()
{
    unsigned int start = rt_time_get_us();
    printf("Hello, world - %d Hz\n",rt_freq_get(RT_FREQ_DOMAIN_FC));
    printf("Took %u us to complete\n", rt_time_get_us());
    return 0;
}
