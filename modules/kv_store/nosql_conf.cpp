#include "nosql_conf.h"

const Vector<uint8_t> DEFAULT_KEY = { 0x46, 0x75, 0x72, 0x72, 0x69, 0x65, 0x73, 0x20,
									  0x6d, 0x61, 0x6b, 0x65, 0x20, 0x74, 0x68, 0x65,
									  0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65,
									  0x74, 0x20, 0x67, 0x6f, 0x20, 0x62, 0x72, 0x72};

NoSQLConfig::NoSQLConfig(const String &p_config_file_path) {
	_config_file = Ref<ConfigFile>(memnew(ConfigFile));
	_config_file_path = p_config_file_path;
	if (FileAccess::exists(p_config_file_path)) {
		_config_file->load_encrypted(p_config_file_path, DEFAULT_KEY);
	} else {
		_write_default_config();
	}
}

NoSQLConfig::~NoSQLConfig() {
	if (_config_file.is_valid()) {
		_config_file->save_encrypted(_config_file_path, DEFAULT_KEY);
	}
}

void NoSQLConfig::set_database_dir(const String &p_database_dir) {
	if (!_config_file.is_valid()) return;
	_config_file->set_value("database", "database_dir", p_database_dir);
	_config_file->save_encrypted(_config_file_path, DEFAULT_KEY);
}

String NoSQLConfig::get_database_dir() const {
	if (!_config_file.is_valid()) return String();
	return _config_file->get_value("database", "database_dir", "");
}

void NoSQLConfig::add_database_config(const String &p_database_name, const Dictionary &p_config) {
	if (!_config_file.is_valid()) return;
	_config_file->set_value("database", p_database_name, p_config);
	_config_file->save_encrypted(_config_file_path, DEFAULT_KEY);
}

Dictionary NoSQLConfig::get_database_config(const String &p_database_name) const {
	if (!_config_file.is_valid()) return Dictionary();
	return _config_file->get_value("database", p_database_name, Dictionary());
}

void NoSQLConfig::_write_default_config() {
	Dictionary default_config;
	default_config["database_dir"] = "user://databases/untitled";
	default_config["use_encryption"] = true;
	default_config["encryption_key"] = DEFAULT_KEY;
	default_config["use_compression"] = true;
	add_database_config("untitled", default_config);
	_config_file->save_encrypted(_config_file_path, DEFAULT_KEY);
}



