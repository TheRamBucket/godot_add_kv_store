#include "sstable.h"

#include "bloom.h"

#include "core/io/stream_peer.h"
#include "core/os/os.h"

#include <string>


SSTable SSTable::CreateFromTree(RedBlackTree &rbt, String database_name) {
	SSTable sstable;
	sstable._database_name = database_name;
	sstable._generate_blocks(rbt);
	return sstable;
}

SSTable SSTable::LoadFromFile(const String &database_name, const String &file_name) {
	const Ref<FileAccess> file_access = FileAccess::open("user://data/" + database_name + "/" + file_name, FileAccess::READ);
	SSTable sstable;
	sstable._database_name = database_name;
	const uint64_t entries = sstable.read_index_from_file(file_access);
	for (uint32_t i = 0; i < entries; i++) {
		DataBlock data_block;
		data_block.read(file_access);
		sstable._data_blocks.push_back(data_block);
	}
	return sstable;
}

void SSTable::WriteToFile() {
	const OS *os = OS::get_singleton();
	const String file_name = String::num_uint64(os->get_unix_time());
	const Ref<DirAccess> dir_access = DirAccess::create(DirAccess::ACCESS_USERDATA);
	dir_access->make_dir_recursive("user://data/" + _database_name);
	const String path = "user://data/" + _database_name + "/" + file_name + ".dat";
	const Ref<FileAccess> file_access = FileAccess::open(path, FileAccess::WRITE);
	_write_index_to_file(file_access);
	for (int i = 0; i < _data_blocks.size(); i++) {
		_data_blocks.get(i).write(file_access);
	}
	file_access->close();
}

RedBlackTree SSTable::to_red_black_tree() const {
	RedBlackTree rbt;
	for (const auto &_key_value : _keys_values) {
		rbt.insert_node(_key_value.first, _key_value.second, false);
	}
	return rbt;
}

SSTable SSTable::merge(Vector<SSTable> tables) {
	return SSTable();
}

BloomFilter* createBloomFilter(Vector<uint64_t> keys) {
	BloomFilter* bloom_filter = new BloomFilter(keys.size(), 0.01);
	for (int i = 0; i < keys.size(); i++) {
		bloom_filter->add(keys[i]);
	}
	return bloom_filter;
}

void SSTable::_generate_blocks(RedBlackTree &rbt) {
	_generate_blocks_helper(rbt.get_root(), rbt.get_tnull());
	Vector<Vector<Pair<uint64_t, String>>> blocks = _split_to_blocks(_keys_values);
	uint64_t total_offset = 0;
	Vector<uint64_t> keys;
	Vector<BloomFilter*> bloom_filters;
	for (int i = 0; i < blocks.size(); i++) {
		DataBlock data_block;
		IndexBlockEntry index_block_entry;

		Ref stream_peer_buffer = memnew(StreamPeerBuffer);
		for (int j = 0; j < blocks[i].size(); j++) {
			stream_peer_buffer->put_u64(blocks[i][j].first);
			stream_peer_buffer->put_string(blocks[i][j].second);
			keys.push_back(blocks[i][j].first);
			total_offset += sizeof(uint64_t) + blocks[i][j].second.length() * sizeof(char32_t);
			if (total_offset >= DataBlock::MAX_BLOCK_SIZE / 2 && bloom_filters.size() == 0) {
				bloom_filters.push_back(createBloomFilter(keys));
				index_block_entry.num_entries_one = keys.size();
				keys.clear();
			}
		}
		bloom_filters.push_back(createBloomFilter(keys));
		data_block.set_data(stream_peer_buffer->get_data_array());
		_data_blocks.push_back(data_block);
		index_block_entry.num_entries_two = keys.size();
		index_block_entry.false_positive_probability = 0.01;
		for (int j = 0; j < bloom_filters.size(); j++) {
			if (j == 0) {
				index_block_entry.key = blocks[i][0].first;
				index_block_entry.offset = total_offset;
				if (!(index_block_entry.num_entries_one > 0)) {
					index_block_entry.num_entries_one = index_block_entry.num_entries_two;
					index_block_entry.num_entries_two = 0;
					index_block_entry.filterTwoSize = 0;
					index_block_entry.filterTwo = Vector<bool>();
				}
				index_block_entry.filterOneSize = bloom_filters[0]->_bits.size();
				index_block_entry.filterOne = bloom_filters[0]->_bits;
			} else {
				index_block_entry.filterTwoSize = bloom_filters[1]->_bits.size();
				index_block_entry.filterTwo = bloom_filters[1]->_bits;
			}

			_index_entries.push_back(index_block_entry);
		}

		total_offset = DataBlock::MAX_BLOCK_SIZE + 1;
	}
}

void SSTable::_write_index_to_file(const Ref<FileAccess> &file_access) {
	file_access->seek(0);
	file_access->store_64(_index_entries.size());
	for (int i = 0; i < _index_entries.size(); i++) {
		file_access->store_64(_index_entries[i].key);
		file_access->store_64(_index_entries[i].offset);
		file_access->store_double(_index_entries[i].false_positive_probability);
		file_access->store_64(_index_entries[i].num_entries_one);
		file_access->store_16(_index_entries[i].filterOneSize);
		file_access->store_64(_index_entries[i].num_entries_two);
		file_access->store_16(_index_entries[i].filterTwoSize);
		for (int j = 0; j < _index_entries[i].filterOneSize; j++) {
			file_access->store_8(_index_entries[i].filterOne[j]);
		}
		for (int j = 0; j < _index_entries[i].filterTwoSize; j++) {
			file_access->store_8(_index_entries[i].filterTwo[j]);
		}
	}
}

Vector<Vector<Pair<uint64_t, String>>> SSTable::_split_to_blocks(Vector<Pair<uint64_t, String>> keys_values) {
	constexpr uint64_t MAX_BLOCK_SIZE = 2040;
	uint64_t block_size = 0;
	Vector<Vector<Pair<uint64_t, String>>> blocks;
	Vector<Pair<uint64_t, String>> block;
	for (const auto &key_value : keys_values) {
		if (block_size + sizeof(key_value.first) + (key_value.second.length() * static_cast<uint64_t>(sizeof(char32_t))) > MAX_BLOCK_SIZE) {
			blocks.push_back(block);
			block = Vector<Pair<uint64_t, String>>();
			block_size = 0;
		}
		block.push_back(key_value);
		block_size += sizeof(key_value.first) + (key_value.second.length() * static_cast<uint64_t>(sizeof(char32_t)));
	}
	if (block.size() > 0) {
		blocks.push_back(block);
	}
	return blocks;
}

uint64_t SSTable::read_index_from_file(Ref<FileAccess> file_access) {
	file_access->seek(0);
	const uint64_t index_entry_count = file_access->get_64();
	for (uint64_t i = 0; i < index_entry_count; i++) {
		IndexBlockEntry entry;
		entry.key = file_access->get_64();
		entry.offset = file_access->get_64();
		entry.false_positive_probability = file_access->get_double();
		entry.num_entries_one = file_access->get_64();
		entry.filterOneSize = file_access->get_16();
		entry.num_entries_two = file_access->get_64();
		entry.filterTwoSize = file_access->get_16();
		for (int j = 0; j < entry.filterOneSize; j++) {
			entry.filterOne.push_back(file_access->get_8());
		}
		for (int j = 0; j < entry.filterTwoSize; j++) {
			entry.filterTwo.push_back(file_access->get_8());
		}
		_index_entries.push_back(entry);
	}
	return index_entry_count;
}

void SSTable::_generate_blocks_helper(const NodePtr p_node, NodePtr p_tnull) {
	if (p_node != p_tnull) {
		_generate_blocks_helper(p_node->left, p_tnull);
		_keys_values.push_back({ p_node->key, p_node->value.to_json_string() });
		_generate_blocks_helper(p_node->right, p_tnull);
	}
}
