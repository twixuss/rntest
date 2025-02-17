#pragma once
#include <stdint.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <immintrin.h>
#endif

uint32_t bitswap32(uint32_t x) {
	x = ((x >> 16) & 0x0000ffff) | ((x & 0x0000ffff) << 16);
	x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
	x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
	x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
	x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
	return x;
}


void mul128(uint64_t a, uint64_t b, uint64_t out[2]) {
#ifdef _MSC_VER
	out[0] = _mul128(a, b, &out[1]);
#else
	#error not implemented
#endif
}

char tohexchar(uint32_t x) {
	if (x < 10) return x + '0';
	return x - 10 + 'a';
}
