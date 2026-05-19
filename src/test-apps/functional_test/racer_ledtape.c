#include <stdio.h>

#include "racer_ledtape.h"
#include "target.h"

#define LEDTAPE_PATTERN_BUTTON_PIO BUTTON_PIO2
#define LEDTAPE_WRITE_TICKS 4

static const button_cfg_t ledtape_button_cfg =
{
    .pio = LEDTAPE_PATTERN_BUTTON_PIO
};

static void rainbow_colour(uint8_t wheel_pos,
                           uint8_t *red, uint8_t *green, uint8_t *blue)
{
    if (wheel_pos < 85)
    {
        *red = 255 - wheel_pos * 3;
        *green = wheel_pos * 3;
        *blue = 0;
        return;
    }

    if (wheel_pos < 170)
    {
        wheel_pos -= 85;
        *red = 0;
        *green = 255 - wheel_pos * 3;
        *blue = wheel_pos * 3;
        return;
    }

    wheel_pos -= 170;
    *red = wheel_pos * 3;
    *green = 0;
    *blue = 255 - wheel_pos * 3;
}

static void ledtape_fill_awake(ledbuffer_t *leds)
{
    int i;

    ledbuffer_clear(leds);
    for (i = 0; i < LED_STRIP_NUMBER; i++)
    {
        /* Soft green means the racer program is awake. */
        ledbuffer_set(leds, i, 0, 40, 0);
    }
}

static void ledtape_fill_rainbow(racer_ledtape_t *ledtape)
{
    int i;

    for (i = 0; i < LED_STRIP_NUMBER; i++)
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t wheel_pos;

        /* Spread the colours out so every LED is doing something different. */
        wheel_pos = ledtape->rainbow_offset + i * (256 / LED_STRIP_NUMBER);
        rainbow_colour(wheel_pos, &red, &green, &blue);
        ledbuffer_set(ledtape->leds, i, red, green, blue);
    }
}

int racer_ledtape_init(racer_ledtape_t *ledtape)
{
    ledtape->leds = ledbuffer_init(LEDTAPE_PIO, LED_STRIP_NUMBER);
    if (! ledtape->leds)
        return 1;

    ledtape->pattern_button = button_init(&ledtape_button_cfg);
    if (! ledtape->pattern_button)
        return 2;

    ledtape->enabled = true;
    ledtape->rainbow_mode = false;
    ledtape->rainbow_offset = 0;
    ledtape->write_ticks = 0;

    ledtape_fill_awake(ledtape->leds);
    ledbuffer_write(ledtape->leds);

    return 0;
}

void racer_ledtape_set(racer_ledtape_t *ledtape, bool enabled)
{
    ledtape->enabled = enabled;

    if (! enabled)
        ledbuffer_clear(ledtape->leds);
    else if (ledtape->rainbow_mode)
        ledtape_fill_rainbow(ledtape);
    else
        ledtape_fill_awake(ledtape->leds);

    ledbuffer_write(ledtape->leds);
}

void racer_ledtape_update(racer_ledtape_t *ledtape)
{
    button_poll(ledtape->pattern_button);

    if (button_pushed_p(ledtape->pattern_button))
    {
        ledtape->rainbow_mode = true;
        ledtape->rainbow_offset = 0;
        printf("LED tape rainbow pattern started using BUTTON_PIO2\r\n");
        fflush(stdout);
    }

    if (! ledtape->enabled)
        return;

    ledtape->write_ticks++;
    if (ledtape->write_ticks >= LEDTAPE_WRITE_TICKS)
    {
        ledtape->write_ticks = 0;

        if (ledtape->rainbow_mode)
        {
            ledtape->rainbow_offset += 4;
            ledtape_fill_rainbow(ledtape);
        }

        ledbuffer_write(ledtape->leds);
    }
}
