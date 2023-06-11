#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

#include "core/variant/variant.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"

#include <iostream>


struct RBTNode {
	uint64_t key;
	Variant value;
	RBTNode *parent;
	RBTNode *left;
	RBTNode *right;
	uint8_t color; // 1 -> Red, 0 -> Black
};

typedef RBTNode *NodePtr;

enum class ACTION_TYPE : uint8_t {
	INSERT,
	DELETE
};

class RedBlackTree {
public:
	RedBlackTree(String dir = "c:/db");
	NodePtr get_root();
	NodePtr get_tnull();
	NodePtr search_tree(uint64_t p_key);
	void insert_node(uint64_t p_key, Variant p_value, bool p_wal = true);
	void delete_node(uint64_t p_key, bool p_wal = true);
	NodePtr minimum(NodePtr p_node);
	NodePtr maximum(NodePtr p_node);
	void inorder() {
		inOrderHelper(this->root);
	}


private:
	NodePtr root;
	NodePtr TNULL;
	String db_dir;
	void _store_wal_entry(NodePtr p_node, ACTION_TYPE p_action) const;
	void _run_wal_file();
	void _reset_wal_file() const;
	void _init_null_node(NodePtr p_node, NodePtr p_parent);
	NodePtr _search_tree(NodePtr p_node, uint64_t p_key) const;
	void _fix_insert(NodePtr p_node);
	void _left_rotate(NodePtr p_node);
	void _right_rotate(NodePtr p_node);
	void _fix_delete(NodePtr p_node);
	void _rb_transplant(NodePtr u, NodePtr v);
	void _delete_node(NodePtr p_node, uint64_t p_key, bool p_wal);
	void inOrderHelper(NodePtr node) {
		int64_t testi;
		float testf;
		String tests;
		if (node != TNULL) {
			inOrderHelper(node->left);
			switch(node->value.get_type()) {

				case Variant::Type::FLOAT:
					testf = node->value;
					std::cout<<"key: "<<node->key<<" | data: " << testf << std::endl;
					break;

				case Variant::INT:
					testi = node->value;
					std::cout<<"key: "<<node->key<<" | data: " << testi << std::endl;
					break;


				case Variant::STRING:
					tests = node->value;
					std::cout<<"key: "<<node->key<<" | data: " << tests.utf8().get_data() << std::endl;
					break;


			}
			inOrderHelper(node->right);
		}

	}

};

#endif // RED_BLACK_TREE_H
