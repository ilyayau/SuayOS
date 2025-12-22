#pragma once
#include <stdint.h>
void isr_dispatch(uint64_t vector, uint64_t err, void *frame);
