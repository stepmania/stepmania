//	from VirtualDub
//	Copyright (C) 1998-2001 Avery Lee, All Rights Reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

// Don't use this.  It's only here for the crash code.
#ifndef f_DISASM_LIST_H
#define f_DISASM_LIST_H

class ListNode {
public:
	ListNode *next, *prev;

	void Remove() {
		next->prev = prev;
		prev->next = next;
#ifdef _DEBUG
		prev = next = 0;
#endif
	}

	void InsertBefore(ListNode *node) {
		next = node->next;
		prev = node;
		if (node->next) node->next->prev = this;
		node->next = this;
	}

	ListNode *NextFromHead() const {
		return prev;
	}
};

class List {
private:
public:
	ListNode head, tail;

	// <--- next             prev --->
	//
	// head <-> node <-> node <-> tail

	List() {
		head.next = tail.prev = 0;
		head.prev = &tail;
		tail.next = &head;
	}

	void AddTail(ListNode *node) {
		node->InsertBefore(&tail);
	}

	bool IsEmpty() const throw() {
		return !head.prev->prev;
	}

	ListNode *AtHead() const throw() {
		return head.prev;
	}

	ListNode *AtTail() const throw() {
		return tail.next;
	}
};

// Templated classes... templated classes good.

template<class T> class List2;

template<class T>
class ListNode2 : public ListNode {
friend List2<T>;
public:
	void InsertBefore(ListNode2<T> *node) { ListNode::InsertBefore(node); }

	void Remove() { ListNode::Remove(); }
	T *NextFromHead() const { return (T *)ListNode::NextFromHead(); }
	T *NextFromTail() const { return (T *)ListNode::NextFromTail(); }
};

template<class T>
class List2 : public List {
public:
	List2<T>() {}

	void AddHead(ListNode2<T> *node) { List::AddHead(node); }
	void AddTail(ListNode2<T> *node) { List::AddTail(node); }
	T *AtHead() const { return (T *)List::AtHead(); }
	T *AtTail() const { return (T *)List::AtTail(); }

	// I must admit to being pampered by STL (end is different though!!)

	T *begin() const { return AtHead(); }
	T *end() const { return AtTail(); }

	class iterator {
	protected:
		ListNode2<T> *node;
		ListNode2<T> *next;

	public:
		iterator() {}
		iterator(const iterator& src) throw() : node(src.node), next(src.next) {}

		bool operator!() const throw() { return 0 == next; }
		T *operator->() const throw() { return (T *)node; }
		operator bool() const throw() { return 0 != next; }
		T& operator *() const throw() { return *(T *)node; }
	};

	// fwit: forward iterator (SAFE if node disappears)
	// rvit: reverse iterator (SAFE if node disappears)

	class fwit : public iterator {
	public:
		fwit() throw() {}
		fwit(const fwit& src) throw() : iterator(src) {}
		fwit(ListNode2<T> *start) throw() {
			node = start;
			next = start->NextFromHead();
		}

		const fwit& operator=(ListNode2<T> *start) throw() {
			node = start;
			next = start->NextFromHead();

			return *this;
		}

		fwit& operator++() throw() {
			node = next;
			next = node->NextFromHead();

			return *this;
		}

		const fwit& operator+=(int v) throw() {
			while(next && v--) {
				node = next;
				next = node->NextFromHead();
			}

			return *this;
		}

		fwit operator+(int v) const throw() {
			fwit t(*this);

			t += v;

			return t;
		}

		// This one's for my sanity.

		void operator++(int) throw() {
			++*this;
		}
	};
};

#endif
