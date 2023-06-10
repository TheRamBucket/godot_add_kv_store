#include "sstable.h"
#include "data_block.h"
#include "red_black_tree.h"
#include "core/io/stream_peer.h"
#include "thirdparty/graphite/src/inc/opcodes.h"

SSTable SSTable::CreateFromTree(RedBlackTree rbt) {
}

SSTable SSTable::LoadFromFile(String path) {
}

void SSTable::WriteToFile(String path) {
}

RedBlackTree SSTable::to_red_black_tree() {
}

SSTable SSTable::merge(Vector<SSTable> tables) {
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
	bool filter_two_set = false;

	for (int i = 0; i < _keys.size(); i++){
		if (!index_set) {
			current_index_entry.key = _keys[i];
			current_index_entry.offset = current_offset;
		}

		if (current_block_size + sizeof(_keys[i]) + sizeof(_values[i]) + sizeof(uint16_t) > DataBlock::MAX_BLOCK_SIZE) {
			_data_blocks.push_back(*_current_data_block);
			delete _current_data_block;
			DataBlock temp(current_data.get_data_array());
			_data_blocks.push_back(temp);
			current_block_size = 0;
		}

		if (current_block_size >= DataBlock::MAX_BLOCK_SIZE / 2) {
			if (filter_one_set) {
				BloomFilter filter_two(i, 0.01);
				for (int j = 0; j < i; j++) {
					filter_two.add(_keys[j]);
				}
				current_index_entry.filterTwo = filter_two._bits;
				current_index_entry.filterTwoSize = filter_two._bits.size();
				filter_two_set = true;
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
