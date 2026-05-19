#include <stdio.h>

#include "racer_sleep.h"
#include "mcu_sleep.h"
#include "delay.h"
#include "target.h"

#define SLEEP_BUTTON_PIO SLEEP_PIO
#define BUTTON_RELEASE_DEBOUNCE_MS 50
#define WAIT_WAKEUP_FAST_STARTUP_MASK 0x0007ffffu
#define WAIT_WAKEUP_POLARITY_MASK 0x0000ffffu
#define WAIT_WAKEUP2_ENABLE PMC_FSMR_FSTT2
#define WAIT_WAKEUP2_ACTIVE_LOW 0
#define WAIT_MODE_LOW_POWER BIT(20)

static const button_cfg_t sleep_button_cfg =
{
    .pio = SLEEP_BUTTON_PIO
};

/*
   WAIT mode wakes up and keeps running from after the WAIT command.
   PA2 is WKUP2 on the SAM4S, so WAIT mode needs fast startup input 2.

   If you want deeper BACKUP mode later, you can use code like this:

       static const mcu_sleep_wakeup_t backup_wakeups[] =
       {
           {
               .pio = SLEEP_BUTTON_PIO,
               .active_high = false
           }
       };

       static const mcu_sleep_cfg_t backup_sleep_cfg =
       {
           .mode = MCU_SLEEP_MODE_BACKUP,
           .debounce = 0,
           .num_wakeups = 1,
           .wakeups = backup_wakeups
       };

       mcu_sleep(&backup_sleep_cfg);

   BACKUP mode wakes using WKUP2 too, but it restarts the MCU like
   pressing nRST, so main() starts again from the beginning.
*/
static void wait_mode_wkup2_sleep(void)
{
    uint32_t mor;

    if (! pio_input_get(SLEEP_BUTTON_PIO))
        return;

    /* Keep SUPC set for WKUP2 as well, which is needed for BACKUP mode. */
    SUPC->SUPC_WUIR = SUPC_WUIR_WKUPEN2;

    /*
       WAIT mode wakes from PMC fast startup inputs.  The button is
       active-low, so FSTP2 is kept clear for the low WKUP2 level.
    */
    PMC->PMC_FSMR =
        (PMC->PMC_FSMR & ~WAIT_WAKEUP_FAST_STARTUP_MASK)
        | WAIT_MODE_LOW_POWER | WAIT_WAKEUP2_ENABLE;
    PMC->PMC_FSPR =
        (PMC->PMC_FSPR & ~WAIT_WAKEUP_POLARITY_MASK) | WAIT_WAKEUP2_ACTIVE_LOW;

    mor = PMC->CKGR_MOR & ~CKGR_MOR_KEY_Msk;
    PMC->CKGR_MOR = mor | CKGR_MOR_KEY(0x37) | CKGR_MOR_WAITMODE;

    while (! (PMC->PMC_SR & PMC_SR_MCKRDY))
        ;
}

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

void racer_sleep_arm(racer_sleep_t *sleep)
{
    sleep->sleeping = true;

    printf("Sleep requested. Release SLEEP_PIO, then the racer will sleep.\r\n");
    fflush(stdout);

    sleep_button_release_wait();
}

void racer_sleep_wait_for_wake(racer_sleep_t *sleep)
{
    printf("WAIT sleep ON using WKUP2. Press SLEEP_PIO again to wake.\r\n");
    fflush(stdout);
    wait_mode_wkup2_sleep();
    sleep->sleeping = false;
}

void racer_sleep_finish(racer_sleep_t *sleep)
{
    (void)sleep;

    printf("WAIT sleep OFF by SLEEP_PIO/WKUP2.\r\n");
    fflush(stdout);
}
