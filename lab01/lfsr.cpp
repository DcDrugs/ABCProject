#include <iostream>
#include "lfsr.h"

<<<<<<< HEAD

unsigned get_bit(uint16_t x,
                 unsigned n) {
    return (x >> n) ^ ((x >> (n + 1)) << 1);
}

void set_bit(uint16_t * x,
             unsigned n,
             unsigned v) {
    *x= *x >> 1;
    (v)? *x |= (1 << n) : *x &= ~(1 << n);
}

void lfsr_calculate(uint16_t *reg) {
	
    	set_bit(reg, 15, get_bit(*reg, 0) ^ get_bit(*reg, 2)^ get_bit(*reg, 3) ^ get_bit(*reg, 5));
}

