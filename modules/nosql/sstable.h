#ifndef SSTABLE_H
#define SSTABLE_H

#include "core/io/file_access.h"
#include "core/variant/variant.h"
#include "red_black_tree.h"

class RedBlackTree;
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
	SSTable CreateFromTree(RedBlackTree& rbt, String database_name);
	SSTable LoadFromFile(String file_name);
	void WriteToFile();
	RedBlackTree to_red_black_tree();

private:
	SSTable merge(Vector<SSTable> tables);
	SSTable() = default;
	void _write_index_block();
	void _write_data_blocks();
	void _generate_blocks( RedBlackTree &rbt);
	void _generate_blocks_helper(NodePtr p_node, NodePtr p_tnull);
	void _write_index_to_file(Ref<FileAccess> file_access);
	uint64_t read_index_from_file(Ref<FileAccess> file_access);

	String _database_name;

	Vector<IndexBlockEntry> _index_entries;
	Vector<DataBlock> _data_blocks;
	Vector<uint64_t> _keys;
	Vector<Variant> _values;
	Ref<IndexBlockEntry> _current_index_entry = nullptr;
	DataBlock * _current_data_block = nullptr;
};

#endif
