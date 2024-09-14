#include <Arduino.h>
#ifndef _LUCES_
#define _LUCES_
#define NUM_LEDS 52
#define SOLID_GREEN 0
#define RAINBOW 1

void rainbowCycle(uint8_t wait);
void luces(void *);
void setBrillo(float);
void setStripe(int r, int g, int b);
void setLucesEffect(int e);

#endif