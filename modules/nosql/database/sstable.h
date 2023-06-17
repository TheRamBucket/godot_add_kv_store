#ifndef SSTABLE_H
#define SSTABLE_H

#include "core/io/file_access.h"
#include "red_black_tree.h"
#include "data_block.h"


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
	static SSTable CreateFromTree(RedBlackTree& rbt, String database_name);
	static SSTable LoadFromFile(const String &database_name, const String &file_name);
	void WriteToFile();
	RedBlackTree to_red_black_tree() const;

private:
	SSTable merge(Vector<SSTable> tables);
	SSTable() = default;
	void _generate_blocks( RedBlackTree &rbt);
	void _generate_blocks_helper(NodePtr p_node, NodePtr p_tnull);
	void _write_index_to_file(const Ref<FileAccess> &file_access);
	Vector<Vector<Pair<uint64_t, String>>> _split_to_blocks(Vector<Pair<uint64_t, String>> keys_values);
	uint64_t read_index_from_file(Ref<FileAccess> file_access);

	String _database_name;

	Vector<IndexBlockEntry> _index_entries;
	Vector<DataBlock> _data_blocks;
	Vector<Pair<uint64_t, String>> _keys_values;

};

#endif
