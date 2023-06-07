#ifndef TEST_HASH_H
#define TEST_HASH_H

#include "tests/test_macros.h"
#include "modules/nosql/data_block.h"

#include <iomanip>
#include <sstream>

const char * uint8ArrayToHexString(const uint8_t* uint8Array, int size) {
	std::stringstream ss;

	for (int i = 0; i < size; ++i) {
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(uint8Array[i]);
	}

	return ss.str().c_str();
}

namespace TestHash {
TEST_CASE("[Tree] testing") {
	String str = "hello How are you? I am doing ok thanks for asking";
	String str2 = "hello? I are you there? I am here thanks for asking";
	PackedByteArray data;
	data.append_array(str.to_utf8_buffer());
	data.append_array(str2.to_utf8_buffer());
	DataBlock db(data, data.size(), DataBlock::COMPRESSION_ZSTD);
	db.crc();
	print_line(  db.get_crc());
	db.compress();
	print_line(String(uint8ArrayToHexString(db.get_data().ptrw(), db.get_data().size())));
	db.decompress();
	print_line(String(uint8ArrayToHexString(db.get_data().ptrw(), db.get_data().size())));


	CHECK(true);
}
}

#endif // TEST_STRING_H
