#ifndef SSTABLE_H
#define SSTABLE_H

#include "red_black_tree.h"
#include "core/io/file_access.h"
#include "core/variant/variant.h"
#include "bloom.h"


class RedBlackTree;
class BloomFilter;
class DataBlock;

struct IndexBlockEntry {
	uint64_t key;
	uint64_t offset;
	uint16_t filterOneSize;
	Vector<bool> filterOne;
	uint16_t filterTwoSize;
	Vector<bool> filterTwo;
};

class SSTable {

public:
	SSTable CreateFromTree(RedBlackTree& rbt);
	SSTable LoadFromFile(String path);
	void WriteToFile(String path);
	RedBlackTree to_red_black_tree();

private:
	SSTable merge(Vector<SSTable> tables);
	SSTable() = default;
	void _write_index_block();
	void _write_data_blocks();
	void _generate_blocks( RedBlackTree &rbt);
	void _generate_blocks_helper(NodePtr p_node, NodePtr p_tnull);

	Vector<IndexBlockEntry> _index_entries;
	Vector<DataBlock> _data_blocks;
	Vector<uint64_t> _keys;
	Vector<Variant> _values;
	//Ref<BloomFilter> _current_filter = nullptr;
	Ref<IndexBlockEntry> _current_index_entry = nullptr;
	DataBlock * _current_data_block = nullptr;
};

#endif
