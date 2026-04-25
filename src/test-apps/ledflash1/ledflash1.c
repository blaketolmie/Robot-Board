#include <pio.h>
#include "target.h"
#include "pacer.h"

#include <stdint.h>

#define PACER_RATE 200
#define LED_FLASH_RATE 1
#define NUM_TOGGLES 12

int
main (void)
{
    uint32_t ticks = 0;
    uint8_t toggles = 0;

    /* Start LED ON */
    pio_config_set (LED_STATUS_PIO, PIO_OUTPUT_HIGH);

    pacer_init (PACER_RATE);

    while (toggles < NUM_TOGGLES)
    {
        pacer_wait ();

        ticks++;

        if (ticks >= PACER_RATE / LED_FLASH_RATE / 2)
        {
            ticks = 0;

            pio_output_toggle (LED_STATUS_PIO);
            toggles++;
        }
    }

    /* Ensure LED ends OFF */
    pio_output_high (LED_STATUS_PIO);

    while (1)
    {
        /* Stop here */
    }
}