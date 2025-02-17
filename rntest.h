#ifndef RNTEST_H_
#define RNTEST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*rnt_Gen)();

void rnt_all(rnt_Gen);

uint64_t rnt_period(rnt_Gen);
void rnt_buckets(rnt_Gen, uint64_t period, size_t n_buckets);
void rnt_monobit(rnt_Gen, size_t n_words);
void rnt_runs(rnt_Gen, size_t n_bits);

#ifdef __cplusplus
} // extern "C"
#endif


#ifdef RNTEST_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
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

void rnt_all(rnt_Gen gen) {
	uint64_t period = rnt_period(gen);

	size_t n_buckets = period / 1024;

	rnt_buckets(gen, period, n_buckets);
	rnt_monobit(gen, period);
	rnt_runs(gen, period * 32);
}

uint64_t rnt_period(rnt_Gen gen) {
	printf("period ");

	uint32_t first = gen();
	uint64_t count = 0;
	while (1) {
		++count;
		uint32_t r = gen();
		if (r == first) {
			break;
		}
	}

	printf("%llu\n", count);
	return count;
}

void rnt_buckets(rnt_Gen gen, uint64_t period, size_t n_buckets) {
	printf("buckets\n");

	uint32_t *buckets = calloc(sizeof(*buckets), n_buckets);
	for (uint64_t i = 0; i < period; ++i) {
		buckets[gen() % n_buckets] += 1;
	}

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
}

static void rnt_fill(rnt_Gen gen, uint8_t *buffer, size_t count) {
	for (size_t i = 0; i < count / 4; ++i) {
		((uint32_t *)buffer)[i] = gen();
	}
	size_t rem = count - count / 4 * 4;
	if (rem) {
		uint32_t r = gen();
		for (size_t i = 0; i < rem; ++i) {
			buffer[count / 4 * 4 + i] = r;
			r >>= 8;
		}
	}
}

void rnt_monobit(rnt_Gen gen, size_t n_words) {
	printf("monobit\n");

	size_t ones = 0;
	for (size_t i = 0; i < n_words; ++i) {
		ones += popcount32(gen());
	}

	size_t n_bits = n_words * 32;

	printf("  %f%% are ones\n", (double)ones / n_bits * 100);
}

void rnt_runs(rnt_Gen gen, size_t n_bits) {
	printf("runs\n");

	uint32_t r = gen();
	size_t ones = r & 1;
	int previous_bit = r & 1;
	size_t runs = 1;

	for (size_t i = 1; i < n_bits; ++i) {
		int bit = (r >> (i % 32)) & 1;
		ones += bit;
		runs += bit != previous_bit;
		previous_bit = bit;
		if (i % 32 == 0) {
			r = gen();
		}
	}


	size_t zeros = n_bits - ones;

	double expected_runs = 2.0 * zeros * ones / n_bits + 1.0;
	//double variance = 2.0 * zeros * ones * (2.0 * zeros * ones - n_bits) / (n_bits * n_bits * (n_bits - 1));
	//double z_score = (runs - expected_runs) / sqrt(variance);
	//double p_value = 0.5 * (1 + erf(z_score / sqrt(2.0)));

	printf("  expected %f\n", expected_runs);
	printf("  actual %zu\n", runs);
	printf("  ratio %f\n", runs / expected_runs);
	//printf("  z %f\n", z_score);
	//printf("  p %f\n", p_value);
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RNTEST_IMPLEMENTATION

#endif // RNTEST_H_