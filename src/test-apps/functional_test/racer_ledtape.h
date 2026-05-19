#ifndef RACER_LEDTAPE_H
#define RACER_LEDTAPE_H

#include <stdbool.h>

#include "button.h"
#include "ledbuffer.h"

typedef struct
{
    ledbuffer_t *leds;
    button_t pattern_button;
    bool enabled;
    bool rainbow_mode;
    uint8_t rainbow_offset;
    uint8_t write_ticks;
} racer_ledtape_t;

int racer_ledtape_init(racer_ledtape_t *ledtape);
void racer_ledtape_set(racer_ledtape_t *ledtape, bool enabled);
void racer_ledtape_update(racer_ledtape_t *ledtape);

#endif
