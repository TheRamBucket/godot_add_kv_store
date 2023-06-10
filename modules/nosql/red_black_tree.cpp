#include "red_black_tree.h"

const char* wal_file = "wal.dat";

RedBlackTree::RedBlackTree(String dir) {
		TNULL = new RBTNode;
		TNULL->color = 0;
		TNULL->left = nullptr;
		TNULL->right = nullptr;
		root = TNULL;
		db_dir = dir;
		if (FileAccess::exists(db_dir + "/" + wal_file)) {
			_run_wal_file();
		}
}

NodePtr RedBlackTree::get_root() {
	return this->root;
}

NodePtr RedBlackTree::get_tnull() {
	return this->TNULL;
}

NodePtr RedBlackTree::search_tree(uint64_t p_key) {
	return _search_tree(this->root, p_key);
}

void RedBlackTree::insert_node(uint64_t p_key, Variant p_value, bool p_wal) {
	// Ordinary Binary Search Insertion
	NodePtr node = new RBTNode;
	node->parent = nullptr;
	node->key = p_key;
	node->value = p_value;
	node->left = TNULL;
	node->right = TNULL;
	node->color = 1; // new node must be red

	if (p_wal) {
		_store_wal_entry(node, ACTION_TYPE::INSERT);
	}

	NodePtr y = nullptr;
	NodePtr x = this->root;

	while (x != TNULL) {
		y = x;
		if (node->key < x->key) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	// y is parent of x
	node->parent = y;
	if (y == nullptr) {
		root = node;
	} else if (node->key < y->key) {
		y->left = node;
	} else {
		y->right = node;
	}


	// if new node is a root node, simply return
	if (node->parent == nullptr){
		node->color = 0;
		return;
	}

	// if the grandparent is null, simply return
	if (node->parent->parent == nullptr) {
		return;
	}

	// Fix the tree
	_fix_insert(node);

}

void RedBlackTree::delete_node(uint64_t p_key, bool p_wal) {
	_delete_node(this->root, p_key, p_wal);
}

NodePtr RedBlackTree::minimum(NodePtr p_node) {
	while (p_node->left != TNULL) {
		p_node = p_node->left;
	}
	return p_node;
}

NodePtr RedBlackTree::maximum(NodePtr p_node) {
	while (p_node->right != TNULL) {
		p_node = p_node->right;
	}
	return p_node;
}

void RedBlackTree::_store_wal_entry(NodePtr p_node, ACTION_TYPE p_action) const {
	if (!FileAccess::exists(db_dir + "/" + wal_file)) {
		const Ref<FileAccess> file = FileAccess::open(db_dir + "/" + wal_file, FileAccess::WRITE);
		if (!file->is_open()) {
			ERR_FAIL_MSG("Failed to open WAL file.");
		}
		file->close();
	}
	const Ref<FileAccess> file = FileAccess::open(db_dir + "/" + wal_file, FileAccess::READ_WRITE);
	if (!file->is_open()) {
		ERR_FAIL_MSG("Failed to open WAL file.");
	}

	file->seek_end();
	file->store_8(static_cast<uint8_t>(p_action));
	file->store_64(p_node->key);
	file->store_var(p_node->value);
	file->flush();
	file->close();
}

void RedBlackTree::_run_wal_file() {
	const Ref<FileAccess> file = FileAccess::open(db_dir + "/" + wal_file, FileAccess::READ);
	if (!file->is_open()) {
		return;
	}
	while (!file->eof_reached()) {
		if (file->get_length() - file->get_position() < 9) {
			break;
		}
		const ACTION_TYPE action = static_cast<ACTION_TYPE>(file->get_8());
		const uint64_t key = file->get_64();
		const Variant value = file->get_var();
		switch (action) {
			case ACTION_TYPE::INSERT:
				this->insert_node(key, value, false);
				break;
			case ACTION_TYPE::DELETE:
				this->delete_node(key, false);
				break;
		}
	}
}

void RedBlackTree::_reset_wal_file() const {
	const Ref<DirAccess> dir = DirAccess::open(db_dir);
	if (!dir->file_exists(wal_file)) {
		return;
	}
	dir->remove(wal_file);
}

void RedBlackTree::_init_null_node(NodePtr p_node, NodePtr p_parent) {
	p_node->key = 0;
	p_node->value = Variant();
	p_node->parent = p_parent;
	p_node->left = nullptr;
	p_node->right = nullptr;
	p_node->color = 0;
}

NodePtr RedBlackTree::_search_tree(NodePtr p_node, uint64_t p_key) const {
	if (p_node == TNULL || p_key == p_node->key) {
		return p_node;
	}

	if (p_key < p_node->key) {
		return _search_tree(p_node->left, p_key);
	}
	return _search_tree(p_node->right, p_key);
}

void RedBlackTree::_fix_insert(NodePtr p_node) {
	NodePtr u;
	while (p_node->parent->color == 1) {
		if (p_node->parent == p_node->parent->parent->right) {
			u = p_node->parent->parent->left;
			if (u->color == 1) {
				u->color = 0;
				p_node->parent->color = 0;
				p_node->parent->parent->color = 1;
				p_node = p_node->parent->parent;
			} else {
				if (p_node == p_node->parent->left) {
					p_node = p_node->parent;
					_right_rotate(p_node);
				}
				p_node->parent->color = 0;
				p_node->parent->parent->color = 1;
				_left_rotate(p_node->parent->parent);
			}
		} else {
			u = p_node->parent->parent->right; // uncle

			if (u->color == 1) {
				u->color = 0;
				p_node->parent->color = 0;
				p_node->parent->parent->color = 1;
				p_node = p_node->parent->parent;
			} else {
				if (p_node == p_node->parent->right) {
					p_node = p_node->parent;
					_left_rotate(p_node);
				}
				p_node->parent->color = 0;
				p_node->parent->parent->color = 1;
				_right_rotate(p_node->parent->parent);
			}
		}
		if (p_node == root) {
			break;
		}
	}
	root->color = 0;
}

void RedBlackTree::_left_rotate(NodePtr p_node) {
	NodePtr y = p_node->right;
	p_node->right = y->left;
	if (y->left != TNULL) {
		y->left->parent = p_node;
	}
	y->parent = p_node->parent;
	if (p_node->parent == nullptr) {
		this->root = y;
	} else if (p_node == p_node->parent->left) {
		p_node->parent->left = y;
	} else {
		p_node->parent->right = y;
	}
	y->left = p_node;
	p_node->parent = y;
}

void RedBlackTree::_right_rotate(NodePtr p_node) {
	NodePtr y = p_node->left;
	p_node->left = y->right;
	if (y->right != TNULL) {
		y->right->parent = p_node;
	}
	y->parent = p_node->parent;
	if (p_node->parent == nullptr) {
		this->root = y;
	} else if (p_node == p_node->parent->right) {
		p_node->parent->right = y;
	} else {
		p_node->parent->left = y;
	}
	y->right = p_node;
	p_node->parent = y;
}

void RedBlackTree::_fix_delete(NodePtr p_node) {
	NodePtr s;
	while (p_node != root && p_node->color == 0) {
		if (p_node == p_node->parent->left) {
			s = p_node->parent->right;
			if (s->color == 1) {
				s->color = 0;
				p_node->parent->color = 1;
				_left_rotate(p_node->parent);
				s = p_node->parent->right;
			}

			if (s->left->color == 0 && s->right->color == 0) {
				s->color = 1;
				p_node = p_node->parent;
			} else {
				if (s->right->color == 0) {
					s->left->color = 0;
					s->color = 1;
					_right_rotate(s);
					s = p_node->parent->right;
				}

				s->color = p_node->parent->color;
				p_node->parent->color = 0;
				s->right->color = 0;
				_left_rotate(p_node->parent);
				p_node = root;
			}
		} else {
			s = p_node->parent->left;
			if (s->color == 1) {
				s->color = 0;
				p_node->parent->color = 1;
				_right_rotate(p_node->parent);
				s = p_node->parent->left;
			}

			if (s->right->color == 0 && s->right->color == 0) {
				s->color = 1;
				p_node = p_node->parent;
			} else {
				if (s->left->color == 0) {
					s->right->color = 0;
					s->color = 1;
					_left_rotate(s);
					s = p_node->parent->left;
				}

				s->color = p_node->parent->color;
				p_node->parent->color = 0;
				s->left->color = 0;
				_right_rotate(p_node->parent);
				p_node = root;
			}
		}
	}
	p_node->color = 0;
}

void RedBlackTree::_rb_transplant(NodePtr u, NodePtr v) {
	if (u->parent == nullptr) {
		root = v;
	} else if (u == u->parent->left){
		u->parent->left = v;
	} else {
		u->parent->right = v;
	}
	v->parent = u->parent;
}

void RedBlackTree::_delete_node(NodePtr p_node, uint64_t p_key,bool p_wal) {
	NodePtr z = TNULL;
	NodePtr x, y;
	while (p_node != TNULL){
		if (p_node->key == p_key) {
			z = p_node;
		}

		if (p_node->key <= p_key) {
			p_node = p_node->right;
		} else {
			p_node = p_node->left;
		}
	}

	if (z == TNULL) {
		return;
	}
	if (p_wal) {
		_store_wal_entry(z,ACTION_TYPE::DELETE);
	}
	y = z;
	int y_original_color = y->color;
	if (z->left == TNULL) {
		x = z->right;
		_rb_transplant(z, z->right);
	} else if (z->right == TNULL) {
		x = z->left;
		_rb_transplant(z, z->left);
	} else {
		y = minimum(z->right);
		y_original_color = y->color;
		x = y->right;
		if (y->parent == z) {
			x->parent = y;
		} else {
			_rb_transplant(y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		_rb_transplant(z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}
	delete z;
	if (y_original_color == 0){
		_fix_delete(x);
	}
}
