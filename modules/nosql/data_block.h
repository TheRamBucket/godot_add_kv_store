#pragma once

#include "core/io/file_access.h"
#include "core/variant/variant.h"



class DataBlock {

public:

	DataBlock( PackedByteArray data, uint32_t crc, bool is_compressed, bool is_encrypted);
	DataBlock(PackedByteArray data);
	bool verify_crc(uint32_t crc_expected);
	PackedByteArray get_data();
	uint32_t get_crc();
	void set_data(PackedByteArray data);
	void crc();
	void encrypt(String key);
	void decrypt(String key);
	void compress();
	void decompress();

private:
	PackedByteArray _data;
	uint32_t _size;
	uint32_t _crc;
	bool _is_compressed = false;
	bool _is_encrypted = false;
	uint32_t _crc32(uint8_t *data);
	void _verify_size();
	PackedByteArray _iv = {199, 90, 18, 143, 107, 157, 62, 161, 226, 65, 185, 247, 52, 7, 92, 147};
};
