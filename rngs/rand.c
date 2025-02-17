#include <stdint.h>
#include <stdlib.h>

#define RNTEST_IMPLEMENTATION
#include <rntest.h>

#include "common_stuff.h"


int main() {
	rnt_Gen gen = {
		.fn = rand,
		.max = RAND_MAX, 
	};

	gen.period = rnt_find_period(&gen, 2);

	rnt_all(&gen);
}