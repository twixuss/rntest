#include <stdint.h>

#define RNTEST_IMPLEMENTATION
#include <rntest.h>

#include "common_stuff.h"

uint32_t i;
uint32_t mulxor64hi32() {
	const uint64_t k = 13043817825332782213; // next_odd(2^63.5)
	uint64_t x = i++;
	x = x*k^k;
	//x = x*k^k;
	//x = x*k^k;
	//x = x*k^k;
	return x >> 32;
}

int main() {
	rnt_all(mulxor64hi32);
}