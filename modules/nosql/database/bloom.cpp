#include "bloom.h"

BloomFilter::BloomFilter(uint64_t p_num_elements, double p_false_positive_probability){

	double denom = 0.480453013918201; // (ln(2))^2

	double size = -1 * (double) p_num_elements* (log(p_false_positive_probability) / denom);

	_bits.resize_zeroed((int)size);

	double ln2 = 0.693147180559945;
	_numHashes = (int) ceil( (size / p_num_elements) * ln2);  // ln(2)
}

Array BloomFilter::hash(const String &data) {

	Array hashValue;
	hashValue.resize(2);
	String srt1 = data.substr(0, (data.length() - 1)/2);
	String srt2 = data.substr(1 + (data.length() - 1)/2, data.length() - 1);
	hashValue[0] = srt1.hash64();
	hashValue[1] = srt2.hash64();

	return hashValue;
}
uint64_t BloomFilter::nthHash(uint32_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
	return (hashA + n * hashB) % filterSize;
}

void BloomFilter::add(const uint64_t data) {
	String str = String::num_uint64(data);
	auto hashValues = hash(str);

	for (int n = 0; n < _numHashes; n++) {
		uint64_t index = nthHash(n, hashValues[0], hashValues[1], _bits.size());
		_bits.set(index, true);
	}
}

bool BloomFilter::mayContain(const uint64_t data) {
	String str = String::num_uint64(data);
	auto hashValues = hash(str);
	for (int n = 0; n < _numHashes; n++) {
		uint64_t index = nthHash(n, hashValues[0], hashValues[1], _bits.size());
		if (!_bits.get(index)) {
			return false;
		}
	}

	return true;
}
