/* MIT License, Copyright (c) 2025 twixuss

	RNTest is single-header random number generator quality testing utility 

	To use this library
#define RNTEST_IMPLEMENTATION
	in one of your c/c++ files before including rntest.h

	To start testing
rnt_Gen gen = {
	.fn = rand,             // Function you want to test.
	.max = RAND_MAX,        // Maximum number that can be returned from fn. Set to 0 if unknown.
	.period = RAND_MAX + 1, // Number of times fn can be called without repeating results. Set to 0 if unknown.
};
rnt_all(&gen); // Run all tests.

	Implemented tests:
uniformity
monobit
runs 

*/
#ifndef RNTEST_H_
#define RNTEST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNT_FIND_MAX_SAMPLES 256

typedef uint32_t (*rnt_GenFn)();

typedef struct {
	rnt_GenFn fn;
	uint32_t max;    // if 0, will be calculated from RNT_FIND_MAX_SAMPLES
	uint64_t period; // if 0, will be calculated with window size of 1
} rnt_Gen;

uint32_t rnt_find_max(rnt_Gen *, size_t n_samples);
uint64_t rnt_find_period(rnt_Gen *, size_t n_words_to_match);

void rnt_all(rnt_Gen *);
void rnt_uniformity(rnt_Gen *, size_t n_words);
void rnt_monobit(rnt_Gen *, size_t n_words);
void rnt_runs(rnt_Gen *, size_t n_bits);

float rnt_u32_to_f32(uint32_t x);
double rnt_u64_to_f64(uint64_t x);

#ifdef __cplusplus
} // extern "C"
#endif


#ifdef RNTEST_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

static int popcount32(uint32_t value) {
	int r = 0;
	for (int i = 0; i < 32; ++i) {
		r += value & 1;
		value >>= 1;
	}
	return r;
}

static void rnt_fill(rnt_Gen *gen, uint8_t *buffer, size_t count) {
	for (size_t i = 0; i < count / 4; ++i) {
		((uint32_t *)buffer)[i] = gen->fn();
	}
	size_t rem = count - count / 4 * 4;
	if (rem) {
		uint32_t r = gen->fn();
		for (size_t i = 0; i < rem; ++i) {
			buffer[count / 4 * 4 + i] = r;
			r >>= 8;
		}
	}
}

static void rnt_start_test(rnt_Gen *gen) {
	if (gen->max == 0)
		gen->max = rnt_find_max(gen, RNT_FIND_MAX_SAMPLES);
	if (gen->period == 0)
		gen->period = rnt_find_period(gen, 1);
}

void rnt_all(rnt_Gen *gen) {
	rnt_uniformity(gen, gen->period);
	rnt_monobit(gen, gen->period);
	rnt_runs(gen, gen->period * 32);
}

uint32_t rnt_find_max(rnt_Gen *gen, size_t n_samples) {
	uint32_t max = 0;
	for (size_t i = 0; i < n_samples; ++i) {
		uint32_t r = gen->fn();
		max = r > max ? r : max;
	}

	// max = ceil_to_power_of_2_minus_one(max);
	max |= max >> 1;
	max |= max >> 2;
	max |= max >> 4;
	max |= max >> 8;
	max |= max >> 16;

	return max;
}

uint64_t rnt_find_period(rnt_Gen *gen, size_t n_words_to_match) {
	printf("period ... ");

	size_t bufsize = sizeof(uint32_t) * n_words_to_match;

	uint32_t *first = malloc(bufsize);
	for (size_t i = 0; i < n_words_to_match; ++i)
		first[i] = gen->fn();

	uint32_t *next  = malloc(bufsize);
	memcpy(next, first, bufsize);

	uint64_t count = n_words_to_match;

	while (1) {
		memmove(next, next + 1, bufsize - sizeof(uint32_t));
		next[n_words_to_match - 1] = gen->fn();
		if (memcmp(first, next, bufsize) == 0) {
			break;
		}
		++count;
	}

	printf("%llu\n", count);
	return count;
}

void rnt_uniformity(rnt_Gen *gen, size_t n_words) {
	rnt_start_test(gen);

	printf("uniformity ...\n");

	uint64_t sum = 0;
	uint64_t sum_max = 0;
	for (size_t i = 0; i < n_words / 2; ++i) {
		uint32_t a = gen->fn();
		uint32_t b = gen->fn();
		sum += a;
		sum += b;
		sum_max += a > b ? a : b;
	}

	uint64_t average = sum / n_words;
	uint64_t average_max = sum_max * 2 / n_words;

	uint64_t expected_average = gen->max / 2;
	uint64_t expected_average_max = gen->max * sqrt(0.5);
	printf("  average %llu  expected %llu  ratio %f\n", average, expected_average, (double)average / expected_average);
	printf("  maxof2  %llu  expected %llu  ratio %f\n", average_max, expected_average_max, (double)average_max / expected_average_max);




	/*
	uint32_t *buckets = calloc(sizeof(uint32_t), n_buckets);

	uint32_t min = ~0;
	uint32_t max = 0;
	uint64_t average_i = 0;
	for (size_t i = 0; i < n_buckets; ++i) {
		min = buckets[i] < min ? buckets[i] : min;
		max = buckets[i] > max ? buckets[i] : max;
		average_i += buckets[i];
	}
	double average = (double)average_i / n_buckets;

	double variance = 0;
	for (size_t i = 0; i < n_buckets; ++i) {
		double x = buckets[i] - average;
		variance += x * x;
	}

	double stddev = sqrt(variance);

	double expected = (double)period / n_buckets;

	printf("  expected %f\n", expected);
	printf("  actual\n");
	printf("    min %d\n", min);
	printf("    max %d\n", max);
	printf("    average %f\n", average);
	printf("    stddev %f\n", stddev);

	free(buckets);
	*/
}

void rnt_monobit(rnt_Gen *gen, size_t n_words) {
	rnt_start_test(gen);

	printf("monobit ...\n");

	size_t ones = 0;
	for (size_t i = 0; i < n_words; ++i) {
		ones += popcount32(gen->fn());
	}

	size_t n_bits = n_words * popcount32(gen->max);

	printf("  %f%% are ones\n", (double)ones / n_bits * 100);
}

void rnt_runs(rnt_Gen *gen, size_t n_bits) {
	rnt_start_test(gen);

	printf("runs ...\n");

	uint32_t r = gen->fn();
	size_t ones = r & 1;
	int previous_bit = r & 1;
	size_t runs = 1;

	size_t useful_bits = popcount32(gen->max);

	for (size_t i = 1; i < n_bits; ++i) {
		int bit = (r >> (i % useful_bits)) & 1;
		ones += bit;
		runs += bit != previous_bit;
		previous_bit = bit;
		if (i % useful_bits == 0) {
			r = gen->fn();
		}
	}


	size_t zeros = n_bits - ones;

	double expected_runs = 2.0 * zeros * ones / n_bits + 1.0;
	//double variance = 2.0 * zeros * ones * (2.0 * zeros * ones - n_bits) / (n_bits * n_bits * (n_bits - 1));
	//double z_score = (runs - expected_runs) / sqrt(variance);
	//double p_value = 0.5 * (1 + erf(z_score / sqrt(2.0)));

	printf("  runs %zu  expected %f  ratio %f\n", runs, expected_runs, runs / expected_runs);
	//printf("  z %f\n", z_score);
	//printf("  p %f\n", p_value);
}


float rnt_u32_to_f32(uint32_t x) {
	x = 0x3f800000 | (x >> 9);
	return *(float *)&x - 1.0f;
}

double rnt_u64_to_f64(uint64_t x) {
	x = 0x3ff0000000000000 | (x >> 12);
	return *(double *)&x - 1.0f;
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RNTEST_IMPLEMENTATION

#endif // RNTEST_H_

/* 
LICENSE 

MIT License

Copyright (c) 2025 twixuss

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/