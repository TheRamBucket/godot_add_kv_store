#ifndef TEST_HASH_H
#define TEST_HASH_H

#include "tests/test_macros.h"
#include "modules/nosql/sstable.h"

namespace TestHash {
TEST_CASE("[Tree] testing") {
	RedBlackTree rbt;
	rbt.insert_node(1, String("Hello"));
	rbt.insert_node(2, String("World"));
	rbt.insert_node(3, String("Foo"));
	rbt.insert_node(4, String("Bar"));
	rbt.insert_node(5, 1);
	rbt.insert_node(6, 2);
	rbt.insert_node(7, 3.0);
	rbt.insert_node(8, 4.0);
	Dictionary dict;
	dict["Hello"] = 1;
	dict["World"] = 2;
	dict["Foo"] = 3.0;
	dict["Bar"] = 4.0;
	rbt.insert_node(9, dict);
	rbt.insert_node(10, Vector<int64_t>({1,2,3,4,5,6,7,8,9}));
	SSTable sst = SSTable::CreateFromTree(rbt, "test");
	sst.WriteToFile();
	Ref<DirAccess> dir = DirAccess::create(DirAccess::ACCESS_USERDATA);
	dir->change_dir("user://data/test");
	Vector<String> files = dir->get_files();
	for (int i = 0; i < files.size(); i++) {
		SSTable sst2 = SSTable::LoadFromFile("test", files[i]);
		RedBlackTree rbt2 = sst2.to_red_black_tree();
		CHECK(rbt2.get_root()->key == rbt.get_root()->key);
		rbt2.inorder();
	}
}
}

#endif // TEST_STRING_H
