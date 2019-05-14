#include "cmsis.h"
#include "gap_common.h"
#include "mbed_wait_api.h"
#include "pin.h"
#include <stdlib.h>

int main( void )
{
    printf("Blink LED on GPIO_A4_A44 (digital pin 4) \n");

    PinName led = GPIO_A4_A44;

    init_pin(led);

    for(int i = 0; i < 10; i++){
        /* Write 1 to GPIO. */
        set_pin(led, 1);

        /* Wait 10 ms */
        wait(1);

        /* Write 0 to GPIO. */
        set_pin(led, 0);

        /* Wait 10 ms */
        wait(1);
    }

    exit(0);
}
