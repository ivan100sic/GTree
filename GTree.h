#ifndef _GTREE_H
#define _GTREE_H

#include <functional>
#include <iostream>
#include <queue>
using namespace std;

namespace gtree {

	struct Void {
		Void operator+ (const Void& d) const{ return *this; }
	};

	template<class IndexT, class ValueT = Void, class Comp = less<IndexT>, class Plus = plus<ValueT>>
	class GTree{
	private:
		struct Node{
			Node* left;
			Node* right;
			Node* parent;
			IndexT key;
			ValueT value, totalValue;

			Node(const IndexT& keyInit, const ValueT& valueInit) :
				left(0), right(0), parent(0),
				key(keyInit), value(valueInit), totalValue(valueInit){}

		};

		struct GTreeOwner{
			//Rule of thumb: private functions do not splay their results

			Comp smaller;
			Plus add;
			Node* root;

			void leftRotate(Node* x){
				Node* y = x->right;
				if (y){
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

			void rightRotate(Node* x){
				Node* y = x->left;
				if (y){
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

			void splay(Node* x){
				while (x->parent){
					if (!x->parent->parent){
						if (x->parent->left == x) rightRotate(x->parent);
						else leftRotate(x->parent);
					}
					else if (x->parent->left == x && x->parent->parent->left == x->parent) {
						rightRotate(x->parent->parent);
						rightRotate(x->parent);
					}
					else if (x->parent->right == x && x->parent->parent->right == x->parent) {
						leftRotate(x->parent->parent);
						leftRotate(x->parent);
					}
					else if (x->parent->left == x && x->parent->parent->right == x->parent) {
						rightRotate(x->parent);
						leftRotate(x->parent);
					}
					else {
						leftRotate(x->parent);
						rightRotate(x->parent);
					}
				}
			}

			//detaches the subtree from its parent
			//the newly formed tree with u as its root is returned
			GTreeOwner detach(Node* u){
				if (!u) return GTreeOwner();
				if (u->parent){
					if (u == u->parent->left) u->parent->left = 0;
					else if (u == u->parent->right) u->parent->right = 0;
					repair<true>(u->parent);
					u->parent = 0;
				}
				else {
					root = 0;
				}
				return GTreeOwner(u);
			}

			//attaches the given tree as the left/right child of the given node
			//if u==0 attaching as the root is assumed (left plays no role itc)
			//if the operation succeeds, the given tree becomes empty
			//the parameter left is a template for speed and elegance
			//will cause memory leaks/havoc if called with wrong parameters
			template<bool left>
			void attach(Node* u, GTreeOwner& tree){
				if (!u){
					root = tree.root;
					//no repairs necessary
				}
				else {
					if (left) u->left = tree.root;
					else u->right = tree.root;
					repair<true>(u);
				}
				tree.root = 0;
			}

			//replaces the subtree rooted at u with the one rooted at v
			//the old subtree at u is then returned
			//the nodes must be real and different
			GTreeOwner replace(Node* u, Node* v){
				bool left;
				if (u->parent){
					left = u->parent->left == u;
				}
				GTreeOwner oldTree = detach(u);
				GTreeOwner newTree = detach(v);
				if (left) attach<true>(u->parent, newTree);
				else attach<false>(u->parent, newTree);
				return oldTree;
			}

			Node* minimum(Node* u)const{
				if (!u) return 0;
				while (u->left) u = u->left;
				return u;
			}

			Node* maximum(Node* u)const{
				if (!u) return 0;
				while (u->right) u = u->right;
				return u;
			}

			Node* find(const IndexT& key, Node* node)const{
				while (node) {
					if (smaller(node->key, key)) node = node->right;
					else if (smaller(key, node->key)) node = node->left;
					else {
						return node;
					}
				}
				return 0;
			}

			template<bool propagate>
			void repair(Node* node){
				if (!node) return;
				if (!node->left && !node->right){
					node->totalValue = node->value;
				}
				else if (!node->left){
					node->totalValue = add(node->value, node->right->totalValue);
				}
				else if (!node->right){
					node->totalValue = add(node->left->totalValue, node->value);
				}
				else {
					node->totalValue = add(add(node->left->totalValue, node->value), node->right->totalValue);
				}
				if (propagate) repair<true>(node->parent);
			}

			void join(GTreeOwner& tree){
				if (!root){
					root = tree.root;
					tree.root = 0;
				}
				else {
					splay(maximum(root));
					attach<false>(root, tree);
				}
			}

			//Returns true if a new node was created
			//indices must be unique!
			bool insert(const IndexT& key, const ValueT& value = ValueT()){
				Node *z = root;
				Node *p = 0;

				while (z) {
					p = z;
					if (smaller(key, z->key)) z = z->left;
					else
						if (smaller(z->key, key)) z = z->right;
						else {
							z->value = value;
							return false;
						}
				}

				z = new Node(key, value);
				z->parent = p;

				if (!p) root = z;
				else if (smaller(p->key, z->key)) p->right = z;
				else p->left = z;
				repair<true>(z);
				splay(z);
				return true;
			}

			//Returns true if the node was found and erased
			bool erase(const IndexT &key){
				Node* z = find(key, root);
				if (!z) return false;

				splay(z);
				GTreeOwner treeLeft = detach(z->left);
				GTreeOwner treeRight = detach(z->right);

				treeLeft.join(treeRight);

				delete root;
				root = 0; //our tree does not own any nodes

				attach<false>(0, treeLeft);
				//now our tree owns all the nodes it should and no other tree does
				//root was deleted
				return true;
			}

			GTreeOwner(Node* _root = 0) : root(_root){}

			GTreeOwner(GTreeOwner& other) : root(other.root){
				other.root = 0;
			}

			GTreeOwner operator=(GTreeOwner& other){
				if (this != &other){
					root = other.root;
					other.root = 0;
				}
				return *this;
			}

			void clear(){
				Node* p = root, *tmp;
				while (p){
					if (p->left){
						tmp = p;
						p = p->left;
						tmp->left = 0;
					}
					else if (p->right){
						tmp = p;
						p = p->right;
						tmp->right = 0;
					}
					else {
						tmp = p;
						p = p->parent;
						delete tmp;
					}
				}
				root = 0;
			}

			//Nonrecursive!
			GTreeOwner clone()const{
				if (!root) return GTreeOwner();
				queue<Node*> Qold, Qnew;
				Node* newRoot = new Node(*root);
				Qold.push(root);
				Qnew.push(newRoot);
				while (!Qold.empty()){
					Node* p = Qold.front(); Qold.pop();
					Node* q = Qnew.front(); Qnew.pop();
					Node* t;
					if (p->left){
						t = new Node(*(p->left));
						t->parent = q;
						q->left = t;
						Qold.push(p->left);
						Qnew.push(t);
					}
					if (p->right){
						t = new Node(*(p->right));
						t->parent = q;
						q->right = t;
						Qold.push(p->right);
						Qnew.push(t);
					}
				}
				return GTreeOwner(newRoot);
			}

			~GTreeOwner(){
				clear();
			}

		} owner;

	public:

		class Iterator{
			//TODO, yo


		};

		class Imitator{
		private:
			Node* ptr;
			Imitator(Node* p) : ptr(p) {}
			Imitator(const Imitator& other);
			Imitator operator=(const Imitator& other);
		public:
			Imitator operator=(const ValueT& other){
				owner.splay(ptr);
				ptr->value = other;
				owner.repair(value);
			}
			operator ValueT()const{
				return ptr->value;
			}
		};

		GTree():owner(){}

		GTree(const GTree& other) : owner(other.owner.clone()){}

		GTree operator=(const GTree& other){
			if (this != &other){
				owner.clear();
				owner = other.owner.clone();
			}
			return *this;
		}

		bool insert(const IndexT& key, const ValueT& value = ValueT()){
			return owner.insert(key, value);
		}

		bool erase(const IndexT &key){
			return owner.erase(key);
		}

		bool exists(IndexT key)const{
			return owner.find(key, owner.root) != 0;
		}

		bool exists(IndexT key){
			bool r = owner.find(key, owner.root) != 0;
			owner.splay(key);
		}

		bool empty()const{
			return !owner.root;
		}

		void clear(){
			owner.clear();
		}

		ValueT operator[](IndexT key){
			Node* p = owner.find(key, owner.root);
			if (!p) return ValueT();

			owner.splay(p);
			return p->value;
		}

		ValueT operator[](IndexT key){
			Node* p = owner.find(key, owner.root);
			if (!p) return ValueT();
			return p->value;
		}

		IndexT minKey(){
			Node* p = owner.minimum(owner.root);
			if (!p) return IndexT();

			owner.splay(p);
			return p->key;
		}

		IndexT maxKey(){
			Node* p = owner.maximum(owner.root);
			if (!p) return IndexT();

			owner.splay(p);
			return p->key;
		}
	};
}

#endif