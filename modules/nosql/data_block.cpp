#include "data_block.h"
#include "core/io/marshalls.h"


DataBlock::DataBlock(PackedByteArray data, uint32_t size, DataCompressionType compression_type, uint32_t crc) {
	set_data(data);
	_size = size;
	_compression_type = compression_type;
	_crc = crc;
}

DataBlock::DataBlock(PackedByteArray data, uint32_t size, DataCompressionType compression_type) {
	set_data(data);
	_size = size;
	_compression_type = compression_type;
}

DataBlock::DataBlock(PackedByteArray data, uint32_t size) {
	set_data(data);
	_size = size;
}

bool DataBlock::verify_crc() {
  return true;
}

PackedByteArray DataBlock::get_data() {
        return _data;
}

uint8_t DataBlock::get_compression_type() {
  return _compression_type;
}

uint32_t DataBlock::get_crc() {
	return _crc;
}

void DataBlock::set_data(PackedByteArray data) {
	_data = data;
}

void DataBlock::set_compression_type(uint8_t compression_type) {
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

Compression::Mode DataBlock::get_true_mode() {
	switch (_compression_type) {
		case COMPRESSION_FASTLZ: {
			return Compression::Mode::MODE_FASTLZ;
		}
		case COMPRESSION_DEFLATE: {
			return Compression::Mode::MODE_DEFLATE;
		}
		case COMPRESSION_ZSTD: {
			return Compression::Mode::MODE_ZSTD;
		}
		case COMPRESSION_GZIP: {
			return Compression::Mode::MODE_GZIP;
		}
		case COMPRESSION_NONE: {
			return Compression::Mode::MODE_FASTLZ;
		}
	}
}

void DataBlock::compress() {
	Compression::Mode cm = get_true_mode();
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
	Compression::Mode cm = get_true_mode();
	PackedByteArray decompressed_array;
	decompressed_array.resize(1024);
	_size = Compression::decompress(decompressed_array.ptrw(), 1024 ,_data.ptrw(), _data.size(), cm);
	if (_size == 0) {
		ERR_FAIL_MSG("DataBlock::compress() failed to compress data");
	}
	_data = decompressed_array.slice(0, _size);
	_is_compressed = true;
}

uint32_t DataBlock::_crc32(uint8_t *data) {
	int crc = 0xFFFFFFFF;
	for (uint8_t i = 0; i < _size; i++) {
		crc = crc ^ data[i];
		for (int j = 0; j < 8; j++) {
			crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
		}
	}
	return ~crc;
}
