#ifndef RACER_LEDTAPE_H
#define RACER_LEDTAPE_H

#include <stdbool.h>

#include "ledbuffer.h"

typedef struct
{
    ledbuffer_t *leds;
    bool enabled;
} racer_ledtape_t;

int racer_ledtape_init(racer_ledtape_t *ledtape);
void racer_ledtape_set(racer_ledtape_t *ledtape, bool enabled);
void racer_ledtape_update(racer_ledtape_t *ledtape);

#endif
