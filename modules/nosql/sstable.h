#ifndef SSTABLE_H
#define SSTABLE_H

#include "core/io/file_access.h"
#include "core/variant/variant.h"

class RedBlackTree;
class BloomFilter;

struct IndexBlockEntry {
	uint64_t key;
	uint64_t offset;
	uint64_t size;
};

struct DataBlockEntry {
	PackedByteArray data;
	uint8_t compression_type;
	uint32_t crc;
};


class SSTable {

public:
	SSTable(RedBlackTree rbt);
	void write_to_disk();
	void read_from_disk();
	void merge(SSTable);
	RedBlackTree to_red_black_tree();
	uint32_t crc_hash(const PackedByteArray &data);
private:
	uint8_t _compression_type = FileAccess::COMPRESSION_ZSTD;
};

#endif
