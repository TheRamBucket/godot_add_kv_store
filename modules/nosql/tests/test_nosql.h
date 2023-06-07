#ifndef TEST_HASH_H
#define TEST_HASH_H

#include "tests/test_macros.h"
#include "modules/nosql/data_block.h"

namespace TestHash {
TEST_CASE("[Tree] testing") {
	String str = "hello How are you? I am doing ok thanks for asking";
	String str2 = "hello? I are you there? I am here thanks for asking";
	PackedByteArray data;
	data.append_array(str.to_utf8_buffer());
	data.append_array(str2.to_utf8_buffer());
	DataBlock db(data);
	db.crc();
	print_line(  db.get_crc());
	print_line(String::hex_encode_buffer(db.get_data().ptrw(), db.get_data().size()));
	db.encrypt("Hello");
	db.compress();
	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	dir->make_dir("c:/temp");
	db.write("c:/temp/test.dat");
	DataBlock db2;
	db2.read("c:/temp/test.dat", 0);
	db2.decompress();
	db2.decrypt("Hello");
	String str4 = String::utf8((const char *)db2.get_data().ptrw(), db2.get_data().size());
	print_line(str4);


	CHECK(true);
}
}

#endif // TEST_STRING_H
