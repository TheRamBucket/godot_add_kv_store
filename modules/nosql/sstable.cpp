#include "sstable.h"
#include "data_block.h"
#include "red_black_tree.h"
#include "core/io/stream_peer.h"

SSTable SSTable::CreateFromTree(RedBlackTree& rbt, String database_name) {
	SSTable sstable;
	sstable._database_name = database_name;
	sstable._generate_blocks(rbt);
	return sstable;
}

SSTable SSTable::LoadFromFile(String file_name) {
}

void SSTable::WriteToFile(String file_name) {
}

RedBlackTree SSTable::to_red_black_tree() {
	RedBlackTree rbt;
	for (int i = 0; i < _keys.size(); i++) {
		rbt.insert_node(_keys[i], _values[i], false);
	}
	return rbt;
}

SSTable SSTable::merge(Vector<SSTable> tables) {
	RedBlackTree rbt;
	for (int i = 0; i < tables.size(); i++) {
		if (tables[i]._keys.size() > 0) {
			for (int j = 0; j < tables[i]._keys.size(); j++) {
				rbt.insert_node(tables[i]._keys[j], tables[i]._values[j], false);
			}
		}
	}
	return CreateFromTree(rbt);
}

void SSTable::_write_index_block() {
}

void SSTable::_write_data_blocks() {
}

void SSTable::_generate_blocks( RedBlackTree &rbt) {
	_generate_blocks_helper(rbt.get_root(), rbt.get_tnull());
	StreamPeerBuffer current_data;

	uint32_t current_block_size = 0;
	IndexBlockEntry current_index_entry;
	uint64_t current_offset = 0;
	bool index_set = false;
	bool filter_one_set = false;

	for (int i = 0; i < _keys.size(); i++) {
		if (!index_set) {
			current_index_entry.key = _keys[i];
			current_index_entry.offset = current_offset;
			index_set = true;
		}

		if (current_block_size + sizeof(_keys[i]) + sizeof(_values[i]) + sizeof(uint16_t) > DataBlock::MAX_BLOCK_SIZE) {
			_data_blocks.push_back(*_current_data_block);
			delete _current_data_block;
			DataBlock temp(current_data.get_data_array());
			_data_blocks.push_back(temp);
			current_block_size = 0;
			_index_entries.push_back(current_index_entry);
			index_set = false;
			filter_one_set = false;
		}

		if (current_block_size >= DataBlock::MAX_BLOCK_SIZE / 2) {
			if (filter_one_set) {
				BloomFilter filter_two(i, 0.01);
				for (int j = 0; j < i; j++) {
					filter_two.add(_keys[j]);
				}
				current_index_entry.filterTwo = filter_two._bits;
				current_index_entry.filterTwoSize = filter_two._bits.size();
			} else {
				uint64_t element_count = i+1;
				BloomFilter filter_one(element_count, 0.01);
				for (int j = 0; j < element_count; j++) {
					filter_one.add(_keys[j]);
				}
				current_index_entry.filterOne = filter_one._bits;
				current_index_entry.filterOneSize = filter_one._bits.size();
				filter_one_set = true;
			}
		}
		current_data.put_64(_keys[i]);
		current_data.put_var(_values[i]);
	}
}

void SSTable::_generate_blocks_helper(NodePtr p_node, NodePtr p_tnull) {
	if (p_node != p_tnull) {
		_generate_blocks_helper(p_node->left, p_tnull);
		_keys.push_back(p_node->key);
		_values.push_back(p_node->value);
		_generate_blocks_helper(p_node->right, p_tnull);
	}
}
