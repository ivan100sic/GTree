#pragma once

#ifndef _GTREELAZY_H
#define _GTREELAZY_H

#include "common.h"

#include <functional>
using namespace std;

namespace gtree {

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	class GTreeLazy {
	private:
		static UpdateT null_update;
		static Comp comp;
		static Adder adder;
		static Updater updater;
		static CumulativeUpdater cumulativeUpdater;
		static UpdateAdder updateAdder;

		struct Node {
			Node* left;
			Node* right;
			Node* parent;

			IndexT key;
			ValueT value;
			CumulativeValueT cumulativeValue;
			UpdateT update;

			Node (const IndexT& keyInit, const ValueT& valueInit) :
				left(nullptr),
				right(nullptr),
				parent(nullptr),
				key(keyInit),
				value(valueInit),
				cumulativeValue(valueInit),
				update(null_update) {}
		};

		Node* root;

	public:

		GTreeLazy(Node* root) : root(root) {}

		GTreeLazy() : root(nullptr) {}

		GTreeLazy(const GTreeLazy& other) : root(other.clone()) {}

		GTreeLazy& operator= (const GTreeLazy& other) {
			if (this != &other) {
				clear();
				root = other.clone();
			}
		}

		GTreeLazy(GTreeLazy&& other) : root(other.root) {}

		GTreeLazy& operator= (GTreeLazy&& other) {
			clear();
			root = other.root;
		}

		~GTreeLazy() {
			destroy();
		}

	private:
		// may one day get replaced with a call to an allocator
		void dealloc(Node* node) {
			delete node;
		}

		// may one day get replaced with a call to an allocator
		Node* alloc(const IndexT& keyInit, const ValueT& valueInit) {
			return new Node(keyInit, valueInit);
		}

		Node* alloc(Node* other) {
			return alloc(other->key, other->value);
		}

		// repairs cumulative values
		template<bool propagate>
		void repair(Node* node) {
			if (!node) return;
			doUpdates(node);
			doUpdates(node->left);
			doUpdates(node->right);
			if (!node->left && !node->right) {
				node->totalValue = node->value;
			} else if (!node->left) {
				node->totalValue = adder(node->value, node->right->totalValue);
			} else if (!node->right) {
				node->totalValue = adder(node->left->totalValue, node->value);
			} else {
				node->totalValue = adder(adder(node->left->totalValue, node->value), node->right->totalValue);
			}
			if (propagate) repair<true>(node->parent);
		}

		// Pushes pending updates. MUST be done before a node is accessed
		void doUpdates(Node* node) {
			if (!node) return;
			node->value = updater(node->update, node->value);
			node->cumulativeValue = cumulativeUpdater(node->update, node->cumulativeValue);
			if (node->left) {
				node->left->update = updateAdder(node->left->update, node->update);
			}
			if (node->right) {
				node->right->update = updateAdder(node->right->update, node->update);
			}
			node->update = null_update;
		}

		void leftRotate(Node* x) {
			doUpdates(x);
			Node* y = x->right;
			if (y) {
				doUpdates(y);
				x->right = y->left;
				if (y->left) y->left->parent = x;
				y->parent = x->parent;
			}
			if (!x->parent) root = y;
			else if (x == x->parent->left) x->parent->left = y;
			else x->parent->right = y;
			if (y) y->left = x;
			x->parent = y;
			repair<false>(x);
			repair<false>(y);
		}

		void rightRotate(Node* x) {
			doUpdates(x);
			Node* y = x->left;
			if (y) {
				doUpdates(y);
				x->left = y->right;
				if (y->right) y->right->parent = x;
				y->parent = x->parent;
			}
			if (!x->parent) root = y;
			else if (x == x->parent->left) x->parent->left = y;
			else x->parent->right = y;
			if (y) y->right = x;
			x->parent = y;
			repair<false>(x);
			repair<false>(y);
		}

		void splay(Node* x) {
			if (!x) return;
			while (x->parent) {
				if (!x->parent->parent) {
					if (x->parent->left == x) rightRotate(x->parent);
					else leftRotate(x->parent);
				} else if (x->parent->left == x && x->parent->parent->left == x->parent) {
					rightRotate(x->parent->parent);
					rightRotate(x->parent);
				} else if (x->parent->right == x && x->parent->parent->right == x->parent) {
					leftRotate(x->parent->parent);
					leftRotate(x->parent);
				} else if (x->parent->left == x && x->parent->parent->right == x->parent) {
					rightRotate(x->parent);
					leftRotate(x->parent);
				} else {
					leftRotate(x->parent);
					rightRotate(x->parent);
				}
			}
		}

		void splayHighest() {
			Node* p = root;
			if (!p) return;
			while (p->right) {
				doUpdates(p);
				p = p->right;
			}
			doUpdates(p);
			splay(p);
		}

		void splayLowest() {
			Node* p = root;
			if (!p) return;
			while (p->left) {
				doUpdates(p);
				p = p->left;
			}
			doUpdates(p);
			splay(p);
		}

		// Only to be called when there is no node in the tree with index equal to node->index.
		// Only to be called with newly created nodes.
		void insert(Node* node) {
			Node* p = root;
			if (!p) {
				root = node;
				return;
			}
			while (1) {
				doUpdates(p);
				if (comp(p->index, node->index)) {
					// go right
					if (p->right) {
						p = p->right;
					} else {
						p->right = node;
						break;
					}
				} else {
					// go left
					if (p->left) {
						p = p->left;
					} else {
						p->left = node;
						break;
					}
				}
			}
			node->parent = p;
			repair<true>(node);
			splay(node);
		}

		// The path from the root to the node must be properly updated.
		void remove(Node* node) {
			splay(node);
			doUpdates(node);
			if (node->left && node->right) {
				doUpdates(node->left);
				doUpdates(node->right);

				node->left->parent = nullptr;
				GTreeLazy tmp(node->left);
				tmp.splayHighest();

				tmp.root->right = node->right;
				node->right->parent = tmp.root;
				repair<false>(tmp.root);

				root = tmp.root;
				tmp.root = nullptr;
			} else if (node->left) {
				node->left->parent = nullptr;
				root = node->left;
			} else if (node->right) {
				node->right->parent = nullptr;
				root = node->right;
			} else {
				root = nullptr;
			}
			dealloc(node);
		}

		// Reorganizes the tree so that all the nodes >= index are in the root or to the right
		// and all others are to the left. If there are no nodes >= index, returns false.
		// Otherwise returns true
		bool lower_bound(IndexT index) {
			Node* found = nullptr;
			Node* p = root;
			while (p) {
				doUpdates(p);
				if (!comp(p->index, index)) {
					found = p;
					p = p->left;
				} else {
					p = p->right;
				}
			}
			if (!found) return false;
			splay(found);
			return true;
		}

		// Reorganizes the tree so that all the nodes <= index are in the root or to the left
		// and all others are to the right. If there are no nodes <= index, returns false.
		// Otherwise returns true
		bool upper_bound(IndexT index) {
			Node* found = nullptr;
			Node* p = root;
			while (p) {
				doUpdates(p);
				if (!comp(index, p->index)) {
					found = p;
					p = p->right;
				} else {
					p = p->left;
				}
			}
			if (!found) return false;
			splay(found);
			return true;
		}

		// root mustn't be null
		Node* detach_left() {
			doUpdates(root);
			Node* p = root->left;

			root->left = nullptr;
			p->parent = nullptr;
			repair<false>(root);

			return p;
		}

		// root mustn't be null
		Node* detach_right() {
			doUpdates(root);
			Node* p = root->right;

			root->right = nullptr;
			p->parent = nullptr;
			repair<false>(root);

			return p;
		}

		void attach_left(Node* node) {
			doUpdates(root);
			doUpdates(node);
			if (root) {
				splayLowest();
				root->left = node;
				node->parent = root;
				repair<false>(root);
			} else {
				root = node;
			}
		}

		void attach_right(Node* node) {
			doUpdates(root);
			doUpdates(node);
			if (root) {
				splayHighest();
				root->right = node;
				node->parent = root;
				repair<false>(root);
			} else {
				root = node;
			}
		}

		void split_tree(Range range, Node*& leftSplit, Node*& rightSplit) {
			
			leftSplit = nullptr;
			rightSplit = nullptr;

			if (!root) return;

			// "remove" the lower part
			if (range.l_type == 2) {
				if (upper_bound(range.l_val)) {
					leftSplit = root;
					root = detach_right();
				} else {
					// nothing to detach
				}
			} else if (range.l_type == 1) {
				if (lower_bound(range.l_val)) {
					leftSplit = root->left;
					detach_left();
				} else {
					// detach everything
					leftSplit = root;
					root = nullptr;
				}
			}

			// "remove" the upper part
			if (range.r_type == 2) {
				if (lower_bound(range.r_val)) {
					rightSplit = root;
					root = detach_left();
				} else {
					// nothing to detach
				}
			} else if (range.r_type == 1) {
				if (upper_bound(range.r_val)) {
					rightSplit = root->right;
					detach_right();
				} else {
					// detach everything
					rightSplit = root;
					root = nullptr;
				}
			}
		}

		void rejoin_tree(Node* leftSplit, Node* rightSplit) {
			attach_right(rightSplit);
			attach_left(leftSplit);
		}

		// nonrecursive and O(1) additional memory!
		Node* clone() {
			if (!root) return nullptr;

			Node* activeOld = root;
			doUpdates(root);
			int state = 0; // 0 - came from above, 1 - came from left, 2 - came from right
			Node* activeNew = alloc(root);
			while (activeOld) {
				doUpdates(activeOld);
				if (state == 0) {
					// try to go left
					if (activeOld->left) {
						activeNew->left = alloc(activeOld);
						activeOld = activeOld->left;
						activeNew->left->parent = activeNew;
						activeNew = activeNew->left;
						state = 0;
					} else {
						// activeNew->left = nullptr;
						state = 1;
					}
				} else if (state == 1) {
					// try to go right
					if (activeOld->right) {
						activeNew->right = alloc(activeOld);
						activeOld = activeOld->right;
						activeNew->right->parent = activeNew;
						activeNew = activeNew->right;
						state = 0;
					} else {
						// activeNew->right = nullptr;
						state = 2;
					}
				} else {
					// go up but only if this is not the root
					activeOld = activeOld->parent;
					if (activeNew->parent) {
						if (activeNew->parent->left == activeNew) {
							state = 1;
						} else {
							state = 2;
						}
						activeNew = activeNew->parent;
					}
				}
			}
			return activeNew;
		}

		bool equals(IndexT a, IndexT b) {
			return !comp(a, b) && !comp(b, a);
		}

	public:

		// Some predefined ranges

		Range all() {
			return Range<IndexT>(0, IndexT(), 0, IndexT());
		}

		Range single(IndexT value) {
			return Range<IndexT>(1, value, 1, value);
		}

		Range strictly_less(IndexT value) {
			return Range<IndexT>(0, IndexT(), 2, value);
		}

		Range less_or_equal(IndexT value) {
			return Range<IndexT>(0, IndexT(), 1, value);
		}

		Range strictly_greater(IndexT value) {
			return Range<IndexT>(2, value, 0, IndexT());
		}

		Range greater_or_equal(IndexT value) {
			return Range<IndexT>(1, value, 0, IndexT());
		}

		Range range_inclusive(IndexT lower, IndexT upper) {
			return Range<IndexT>(1, lower, 1, upper);
		}

		Range range_exclusive(IndexT lower, IndexT upper) {
			return Range<IndexT>(2, lower, 2, upper);
		}

		Range range_mixed(IndexT lower, IndexT upper) {
			return Range<IndexT>(1, lower, 2, upper);
		}

		bool has(IndexT index) {
			lower_bound(index);
			return equals(index, root->index);
		}

		ValueT get(IndexT index) {
			if (has(index)) {
				return root.value;
			} else {
				return ValueT();
			}
		}

		void set(IndexT index, ValueT value) {
			if (has(index)) {
				root.value = value;
				repair<false>(root);
			} else {
				Node* p = alloc(index, value);
				insert(p);
			}
		}

		void update_range(Range range, UpdateT update) {
			Node* left;
			Node* right;
			split_tree(range, left, right);
			if (root) {
				root->update = updateAdder(root->update, update);
			}
			rejoin_tree(left, right);
		}

		CumulativeValueT cumulative_value_range(Range range) {
			Node* left;
			Node* right;
			split_tree(range, left, right);
			CumulativeValueT ret;
			if (root) {
				doUpdates(root);
				ret = root->cumulativeValue;
			}
			rejoin_tree(left, right);
		}
		
		void clear() {
			if (!root) return;

			int state = 0; // 0 - came from above, 1 - came from left, 2 - came from right
			Node* active = root;
			Node* temp;

			while (active) {
				if (state == 0) {
					if (active->left) {
						active = active->left;
						state = 0;
					} else {
						state = 1;
					}
				} else if (state == 1) {
					if (active->right) {
						active = active->right;
						state = 0;
					} else {
						state = 2;
					}
				} else {
					temp = active->parent;
					dealloc(active);
					active = temp;
				}
			}

			root = nullptr;
		}
	};

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	UpdateT GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: null_update;

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	Comp GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: comp;

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	Adder GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: adder;

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	Updater GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: updater;

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	CumulativeUpdater GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: cumulativeUpdater;

	template<
		class IndexT,
		class ValueT = Void,
		class CumulativeValueT = ValueT,
		class UpdateT = NoUpdate<ValueT, CumulativeValueT>,
		class Comp = less<IndexT>,
		class Adder = _gtree_plus<ValueT, ValueT, CumulativeValueT>,
		class Updater = _gtree_plus<UpdateT, ValueT, ValueT>,
		class CumulativeUpdater = _gtree_plus<UpdateT, CumulativeValueT, CumulativeValueT>,
		class UpdateAdder = _gtree_plus<UpdateT, UpdateT, UpdateT>
	>
	UpdateAdder GTreeLazy <
		IndexT, ValueT, CumulativeValueT, UpdateT,
		Comp, Adder, Updater, CumulativeUpdater, UpdateAdder
	> :: updateAdder;
}

#endif