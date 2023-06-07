#include "data_block.h"
#include "core/io/marshalls.h"
#include "core/crypto/aes_context.h"

constexpr int MAX_BLOCK_SIZE = 4096;

DataBlock::DataBlock(PackedByteArray data, uint32_t crc, bool is_compressed, bool is_encrypted) :
	_data(data), _size(data.size()), _crc(crc), _is_compressed(is_compressed), _is_encrypted(is_encrypted){_verify_size();}

DataBlock::DataBlock(PackedByteArray data) :
	_data(data), _size(data.size()), _crc(0) {_verify_size();}

bool DataBlock::verify_crc(uint32_t crc_expected){
	uint32_t actual_crc = _crc32(_data.ptrw());
	if (actual_crc == crc_expected) {
		return true;
	}
	return false;
}

PackedByteArray DataBlock::get_data() {
        return _data;
}

uint32_t DataBlock::get_crc() {
	return _crc;
}

void DataBlock::set_data(PackedByteArray data) {
	_data = data;
}

void DataBlock::crc() {
	if (_size == 0) {
		ERR_FAIL_MSG("DataBlock::crc() called on empty data block");
	}
	if (_is_compressed) {
		ERR_FAIL_MSG("DataBlock::crc() called on compressed data block");
	}
	_crc = _crc32(_data.ptrw());

}
void DataBlock::encrypt(String key) {
	PackedByteArray key_array = key.md5_buffer();
	AESContext aes_context;
	if (_data.size() % 16 != 0) {
		int padding = 16 - (_data.size() % 16);
		for (int i = 0; i < padding; i++) {
			_data.push_back(0);
		}
	}
	aes_context.start(AESContext::Mode::MODE_CBC_ENCRYPT, key_array, _iv);
	_data = aes_context.update(_data);
	aes_context.finish();
	_is_encrypted = true;
}

void DataBlock::decrypt(String key) {
	PackedByteArray key_array = key.md5_buffer();
	AESContext aes_context;
	aes_context.start(AESContext::Mode::MODE_CBC_DECRYPT, key_array, _iv);
	_data = aes_context.update(_data);
	aes_context.finish();
	_is_encrypted = false;
}

void DataBlock::compress() {
	Compression::Mode cm = Compression::MODE_ZSTD;
	PackedByteArray compressed_array;
	compressed_array.resize(Compression::get_max_compressed_buffer_size(_size, cm));
	_size = Compression::compress(compressed_array.ptrw(), _data.ptrw(), _size, cm);
	if (_size == 0) {
		ERR_FAIL_MSG("DataBlock::compress() failed to compress data");
	}
	_data = compressed_array.slice(0,_size);
	_is_compressed = true;
}

void DataBlock::decompress() {
	Compression::Mode cm = Compression::MODE_ZSTD;
	PackedByteArray decompressed_array;
	decompressed_array.resize(1024);
	_size = Compression::decompress(decompressed_array.ptrw(), 1024 ,_data.ptrw(), _data.size(), cm);
	if (_size == 0) {
		ERR_FAIL_MSG("DataBlock::compress() failed to compress data");
	}
	_data = decompressed_array.slice(0, _size);
	_is_compressed = false;
}

uint32_t DataBlock::_crc32(uint8_t *data) {
	int crc = 0xFFFFFFFF;
	for (uint32_t i = 0; i < _size; i++) {
		crc = crc ^ data[i];
		for (int j = 0; j < 8; j++) {
			crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
		}
	}
	return ~crc;
}

void DataBlock::_verify_size() {
	ERR_FAIL_COND_MSG(_data.size() > MAX_BLOCK_SIZE, "DataBlock size exceeds maximum size of " + String::num(MAX_BLOCK_SIZE) + " bytes");
}
