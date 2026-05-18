#include <stdio.h>

#include "racer_sleep.h"
#include "mcu_sleep.h"
#include "delay.h"
#include "target.h"

#define SLEEP_BUTTON_PIO SLEEP_PIO
#define BUTTON_RELEASE_DEBOUNCE_MS 50

static const button_cfg_t sleep_button_cfg =
{
    .pio = SLEEP_BUTTON_PIO
};

static const mcu_sleep_wakeup_t sleep_wakeups[] =
{
    {
        .pio = SLEEP_BUTTON_PIO,
        .active_high = false
    }
};

static const mcu_sleep_cfg_t sleep_cfg =
{
    /*
       WAIT mode wakes up and keeps running from after mcu_sleep().

       If you want deeper BACKUP mode later, change the next line to:

           .mode = MCU_SLEEP_MODE_BACKUP,

           the 3 modes are MCU_SLEEP_MODE_SLEEP (least power saving), MCU_SLEEP_MODE_WAIT, MCU_SLEEP_MODE_BACKUP (most power saving)

       BACKUP mode wakes using WKUP2 too, but it restarts the MCU like
       pressing nRST, so main() starts again from the beginning.
    */
    .mode = MCU_SLEEP_MODE_WAIT,
    .debounce = 0,
    .num_wakeups = 1,
    .wakeups = sleep_wakeups
};

static void sleep_button_release_wait(void)
{
    while (! pio_input_get(SLEEP_BUTTON_PIO))
        delay_ms(5);

    delay_ms(BUTTON_RELEASE_DEBOUNCE_MS);
}

int racer_sleep_init(racer_sleep_t *sleep)
{
    sleep->sleeping = false;
    sleep->button = button_init(&sleep_button_cfg);
    if (! sleep->button)
        return 1;

    return 0;
}

void racer_sleep_poll(racer_sleep_t *sleep)
{
    button_poll(sleep->button);
}

bool racer_sleep_toggle_requested_p(racer_sleep_t *sleep)
{
    return button_pushed_p(sleep->button);
}

void racer_sleep_toggle(racer_sleep_t *sleep)
{
    sleep->sleeping = true;

    printf("WAIT sleep toggled ON using WKUP2. Release SLEEP_PIO, then press it again to wake.\r\n");
    fflush(stdout);

    sleep_button_release_wait();

    while (pio_input_get(SLEEP_BUTTON_PIO))
        mcu_sleep(&sleep_cfg);

    sleep->sleeping = false;

    printf("WAIT sleep toggled OFF by SLEEP_PIO/WKUP2.\r\n");
    fflush(stdout);

    sleep_button_release_wait();
}
