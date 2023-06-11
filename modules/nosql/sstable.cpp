#include "sstable.h"

#include "bloom.h"

#include "core/io/stream_peer.h"
#include "core/os/os.h"


SSTable SSTable::CreateFromTree(RedBlackTree& rbt, String database_name) {
	SSTable sstable;
	sstable._database_name = database_name;
	sstable._generate_blocks(rbt);
	return sstable;
}

SSTable SSTable::LoadFromFile(String database_name,String file_name) {
	Ref<FileAccess> file_access = FileAccess::open("user://data/"+database_name+"/"+file_name, FileAccess::READ);
	SSTable sstable;
	sstable._database_name = database_name;
	uint64_t entries = sstable.read_index_from_file(file_access);
	for (int i = 0; i < entries; i++) {
		DataBlock data_block;
		data_block.read(file_access);
		sstable._data_blocks.push_back(data_block);
	}
	return sstable;
}

void SSTable::WriteToFile() {
	OS * os = OS::get_singleton();
	String file_name = String::num_uint64(os->get_unix_time());
	Ref<DirAccess> dir_access = DirAccess::create(DirAccess::ACCESS_USERDATA);
	dir_access->make_dir_recursive("user://data/"+_database_name);
	String path = "user://data/"+_database_name +"/"+file_name+".dat";
	Ref<FileAccess> file_access = FileAccess::open(path, FileAccess::WRITE);
	_write_index_to_file(file_access);
	for (int i = 0; i < _data_blocks.size(); i++) {
		_data_blocks.get(i).write(file_access);
	}
	file_access->close();
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
	return CreateFromTree(rbt, _database_name);
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
		current_block_size += sizeof(_keys[i]) + sizeof(_values[i]) + sizeof(uint16_t);
		if (i == _keys.size() - 1) {
			DataBlock temp(current_data.get_data_array());
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
			_data_blocks.push_back(temp);
			current_block_size = 0;
			_index_entries.push_back(current_index_entry);
			index_set = false;
			filter_one_set = false;

		}
	}
}

void SSTable::_write_index_to_file(const Ref<FileAccess> &file_access) {
	file_access->seek(0);
	file_access->store_64(_index_entries.size());
	for (int i = 0; i < _index_entries.size(); i++) {
		file_access->store_64(_index_entries[i].key);
		file_access->store_64(_index_entries[i].offset);
		file_access->store_64(_index_entries[i].filterOneSize);
		file_access->store_64(_index_entries[i].filterTwoSize);
		for (int j = 0; j < _index_entries[i].filterOneSize; j++) {
			file_access->store_8(_index_entries[i].filterOne[j]);
		}
		for (int j = 0; j < _index_entries[i].filterTwoSize; j++) {
			file_access->store_8(_index_entries[i].filterTwo[j]);
		}
	}
}

uint64_t SSTable::read_index_from_file(Ref<FileAccess> file_access) {
	file_access->seek(0);
	uint64_t index_entry_count = file_access->get_64();
	for (int i = 0; i < index_entry_count; i++) {
		IndexBlockEntry entry;
		entry.key = file_access->get_64();
		entry.offset = file_access->get_64();
		entry.filterOneSize = file_access->get_64();
		entry.filterTwoSize = file_access->get_64();
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

void SSTable::_generate_blocks_helper(NodePtr p_node, NodePtr p_tnull) {
	if (p_node != p_tnull) {
		_generate_blocks_helper(p_node->left, p_tnull);
		_keys.push_back(p_node->key);
		_values.push_back(p_node->value);
		_generate_blocks_helper(p_node->right, p_tnull);
	}
}
