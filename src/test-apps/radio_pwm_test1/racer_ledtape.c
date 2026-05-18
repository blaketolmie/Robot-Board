#include "racer_ledtape.h"
#include "target.h"

#define NUM_LEDS 15
#define LEDTAPE_WRITE_TICKS 10

static void ledtape_fill_awake(ledbuffer_t *leds)
{
    int i;

    ledbuffer_clear(leds);
    for (i = 0; i < NUM_LEDS; i++)
    {
        /* Soft green means the racer program is awake. */
        ledbuffer_set(leds, i, 0, 40, 0);
    }
}

int racer_ledtape_init(racer_ledtape_t *ledtape)
{
    ledtape->leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);
    if (! ledtape->leds)
        return 1;

    ledtape->enabled = true;
    ledtape_fill_awake(ledtape->leds);
    ledbuffer_write(ledtape->leds);

    return 0;
}

void racer_ledtape_set(racer_ledtape_t *ledtape, bool enabled)
{
    ledtape->enabled = enabled;

    if (enabled)
        ledtape_fill_awake(ledtape->leds);
    else
        ledbuffer_clear(ledtape->leds);

    ledbuffer_write(ledtape->leds);
}

void racer_ledtape_update(racer_ledtape_t *ledtape)
{
    static uint8_t ticks = 0;

    if (! ledtape->enabled)
        return;

    ticks++;
    if (ticks >= LEDTAPE_WRITE_TICKS)
    {
        ticks = 0;
        ledbuffer_write(ledtape->leds);
    }
}
