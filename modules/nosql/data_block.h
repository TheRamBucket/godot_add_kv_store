#pragma once

#include "core/io/file_access.h"
#include "core/variant/variant.h"



class DataBlock {

public:

	enum DataCompressionType {
		COMPRESSION_FASTLZ = FileAccess::COMPRESSION_FASTLZ,
		COMPRESSION_DEFLATE = FileAccess::COMPRESSION_DEFLATE,
		COMPRESSION_ZSTD = FileAccess::COMPRESSION_ZSTD,
		COMPRESSION_GZIP = FileAccess::COMPRESSION_GZIP,
		COMPRESSION_NONE = 4
	};

	DataBlock( PackedByteArray data, uint32_t size, DataCompressionType compression_type, uint32_t crc);
	DataBlock(PackedByteArray data, uint32_t size, DataCompressionType compression_type);
	DataBlock(PackedByteArray data, uint32_t size);
	bool verify_crc();
	PackedByteArray get_data();
	uint8_t get_compression_type();
	uint32_t get_crc();
	void set_data(PackedByteArray data);
	void set_compression_type(uint8_t compression_type);
	void crc();
	Compression::Mode get_true_mode();
	void compress();
	void decompress();

private:
	DataCompressionType _compression_type = COMPRESSION_NONE;
	PackedByteArray _data;
	uint32_t _size;
	uint32_t _crc;
	bool _is_compressed = false;
	uint32_t _crc32(uint8_t *data);

};
