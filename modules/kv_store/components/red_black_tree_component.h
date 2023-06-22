// /**************************************************************************/
// /*  red_black_tree_component.h                                               */
// /**************************************************************************/
// /*                         This file is part of:                          */
// /*                             GODOT ENGINE                               */
// /*                        https://godotengine.org                         */
// /**************************************************************************/
// /* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
// /* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
// /*                                                                        */
// /* Permission is hereby granted, free of charge, to any person obtaining  */
// /* a copy of this software and associated documentation files (the        */
// /* "Software"), to deal in the Software without restriction, including    */
// /* without limitation the rights to use, copy, modify, merge, publish,    */
// /* distribute, sublicense, and/or sell copies of the Software, and to     */
// /* permit persons to whom the Software is furnished to do so, subject to  */
// /* the following conditions:                                              */
// /*                                                                        */
// /* The above copyright notice and this permission notice shall be         */
// /* included in all copies or substantial portions of the Software.        */
// /*                                                                        */
// /* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
// /* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
// /* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
// /* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
// /* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
// /* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
// /* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
// /**************************************************************************/


#ifndef RED_BLACK_TREE_COMPONENT_H
#define RED_BLACK_TREE_COMPONENT_H
#include "kvs_component.h"

struct RBTNode {
	uint64_t key;
	Variant value;
	Variant::Type value_type;
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

class RedBlackTreeComponent : KVSComponent {
public:

// ----------------------------------------------------------------
// Called from the controller to notify the component of an event
// ----------------------------------------------------------------

	void notify(String p_event, Variant p_data, Variant::Type p_data_type) override;
	void notify(String p_event) override;

// ----------------------------------------------------------------

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
	void inOrderHelper(NodePtr node);


};

#endif
