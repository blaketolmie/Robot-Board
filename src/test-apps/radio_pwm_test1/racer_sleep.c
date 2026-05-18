#include <stdio.h>

#include "racer_sleep.h"
#include "mcu_sleep.h"
#include "pio.h"
#include "irq.h"
#include "delay.h"
#include "target.h"

#define SLEEP_BUTTON_PIO SLEEP_PIO
#define SLEEP_IRQ_PRIORITY 7
#define BUTTON_RELEASE_DEBOUNCE_MS 50

static volatile bool sleep_wakeup_seen = false;

static const button_cfg_t sleep_button_cfg =
{
    .pio = SLEEP_BUTTON_PIO
};

static const mcu_sleep_cfg_t sleep_cfg =
{
    .mode = MCU_SLEEP_MODE_SLEEP,
    .debounce = 0,
    .num_wakeups = 0,
    .wakeups = 0
};

static void sleep_button_isr(void)
{
    uint32_t status;

    status = pio_irq_clear(SLEEP_BUTTON_PIO);
    if ((status & (1u << (SLEEP_BUTTON_PIO & 0x1f)))
        && ! pio_input_get(SLEEP_BUTTON_PIO))
    {
        sleep_wakeup_seen = true;
    }
}

static void sleep_button_irq_init(void)
{
    pio_irq_config_set(SLEEP_BUTTON_PIO, PIO_IRQ_FALLING_EDGE);
    pio_irq_clear(SLEEP_BUTTON_PIO);

    irq_config(PIO_ID(SLEEP_BUTTON_PIO), SLEEP_IRQ_PRIORITY,
               sleep_button_isr);
    pio_irq_disable(SLEEP_BUTTON_PIO);
    irq_enable(PIO_ID(SLEEP_BUTTON_PIO));
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

    sleep_button_irq_init();

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

    printf("Sleep toggled ON. Release SLEEP_PIO, then press it again to wake.\r\n");
    fflush(stdout);

    pio_irq_disable(SLEEP_BUTTON_PIO);
    sleep_button_release_wait();

    sleep_wakeup_seen = false;
    pio_irq_clear(SLEEP_BUTTON_PIO);
    irq_clear(PIO_ID(SLEEP_BUTTON_PIO));
    pio_irq_enable(SLEEP_BUTTON_PIO);

    while (! sleep_wakeup_seen)
        mcu_sleep(&sleep_cfg);

    pio_irq_disable(SLEEP_BUTTON_PIO);

    sleep->sleeping = false;

    printf("Sleep toggled OFF by SLEEP_PIO.\r\n");
    fflush(stdout);

    sleep_button_release_wait();
}
