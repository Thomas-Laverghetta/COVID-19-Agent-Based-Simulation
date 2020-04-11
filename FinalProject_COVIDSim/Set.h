#ifndef SET_H
#define SET_H

/*
Set - set of things
Defined using a template so that can be used to hold anything.  Type supplied when object is instantiated.
Template structure requires implementation to be provided in the header.
*/
template <class T>
class Set
{
public:
	Set()
	{
		_head = 0;
		_tail = 0;
		_size = 0;
	}

	void AddEntity(T* t)
	{
		Node* node = new Node(t);
		if (_head == 0) {	//empty list
			_head = _tail = node;
		}
		else {
			_tail = _tail->next = node;
		}
		_size++;
	}

	void RemoveEntity(T* t)
	{
		if (_head != 0) {
			Node* iter = _head;
			Node* prev = 0;
			while (iter != 0 ? iter->t != t : false) {
				prev = iter;
				iter = iter->next;
			}
			if (iter != 0) {
				_size--;
				if (prev == 0) {
					_head = _head->next;
				}
				else {
					prev->next = iter->next;
				}
				if (iter == _tail)
					_tail = prev;
				//				delete iter;
			}
		}
	}

	bool EntityExists(T * t)
	{
		bool exists = false;
		for (Node* iter = _head; iter != 0; iter = iter->next) {
			exists = exists || (iter->t == t);
		}
		return(exists);
	}

	bool IsEmpty() { return (_size == 0); }
	int GetSize() { return _size; }
private:
	struct Node
	{
		Node(T* t) {
			this->t = t;
			next = 0;
		}
		T* t;
		Node* next;
	};

	Node* _head;
	Node* _tail;
	int _size;
};

#endif
