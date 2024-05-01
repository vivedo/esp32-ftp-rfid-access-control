#ifndef _READER_H
#define _READER_H

#define READER_DEBUG

#include "Arduino.h"
#include <Wiegand.h>

void initReader(uint8_t, uint8_t, void (*onCardRead)(uint32_t));

#endif