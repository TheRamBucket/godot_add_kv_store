#ifndef BLOOM_H
#define BLOOM_H

#include "core/variant/variant.h"

class BloomFilter {
public:
	BloomFilter(uint64_t p_num_elements, double p_false_positive_probability);
	Array hash(const String &data);
	uint64_t nthHash(uint32_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize);
	void add(const uint64_t data);
	bool mayContain(const uint64_t data);
	uint8_t _numHashes;
	Vector<bool> _bits;
};

#endif
