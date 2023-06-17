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

SSTable SSTable::LoadFromFile(const String &database_name, const String &file_name) {
	const Ref<FileAccess> file_access = FileAccess::open("user://data/"+database_name+"/"+file_name, FileAccess::READ);
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
	const OS * os = OS::get_singleton();
	const String file_name = String::num_uint64(os->get_unix_time());
	const Ref<DirAccess> dir_access = DirAccess::create(DirAccess::ACCESS_USERDATA);
	dir_access->make_dir_recursive("user://data/"+_database_name);
	const String path = "user://data/"+_database_name +"/"+file_name+".dat";
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

void SSTable::_generate_blocks( RedBlackTree &rbt) {
	_generate_blocks_helper(rbt.get_root(), rbt.get_tnull());

}

void SSTable::_write_index_to_file(const Ref<FileAccess> &file_access) {
	file_access->seek(0);
	file_access->store_64(_index_entries.size());
	for (int i = 0; i < _index_entries.size(); i++) {
		file_access->store_64(_index_entries[i].key);
		file_access->store_64(_index_entries[i].offset);
		file_access->store_16(_index_entries[i].filterOneSize);
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
	return blocks;

}

uint64_t SSTable::read_index_from_file(Ref<FileAccess> file_access) {
	file_access->seek(0);
	const uint64_t index_entry_count = file_access->get_64();
	for (uint64_t i = 0; i < index_entry_count; i++) {
		IndexBlockEntry entry;
		entry.key = file_access->get_64();
		entry.offset = file_access->get_64();
		entry.filterOneSize = file_access->get_16();
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
		_keys_values.push_back({p_node->key, p_node->value.to_json_string()});
		_generate_blocks_helper(p_node->right, p_tnull);
	}
}
