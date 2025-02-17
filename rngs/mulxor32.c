#include <stdint.h>

#define RNTEST_IMPLEMENTATION
#include <rntest.h>

#include "common_stuff.h"

uint32_t i;
uint32_t mulxor32() {
	const uint32_t k = 3037000507u; // next_prime(2^31.5)

	uint32_t x = i++;
	x = x*k^k;
	x = x*k^k;
	x = x*k^k;
	x = x*k^k;
	return x; // low bits are very bad
	// return bitswap32(x);
}

int main() {
	rnt_all(mulxor32);
}