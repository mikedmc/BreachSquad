#pragma once

template < class type >
class Queue
{
public:
	Queue()
	{
		head = 0;
		tail = 0;
		numElements = 0;
		data = NULL;
	}

	Queue(unsigned int numElements)
	{
		data = NULL;
		Alloc(numElements);
	}

	virtual ~Queue()
	{
		delete[] data;
	}

	void			Alloc(unsigned int numElements);
	type *			New();
	type *			PopFirst();
	type *			PopLast();
	type *			Get(unsigned int idx);
	const type *	Get(unsigned int idx) const;

	int				GetCount() const;

protected:
	type			*data;
	int				numElements;
	unsigned int	head;
	unsigned int	tail;
};

template < class type >
void Queue<type>::Alloc(unsigned int _numElements)
{
	delete[] data;

	head = 0;
	tail = 0;
	numElements = _numElements;
	data = new type[numElements];
}

template < class type >
type * Queue<type>::Get(unsigned int idx) {
	return &data[(tail + idx) % numElements];
}

template < class type >
const type * Queue<type>::Get(unsigned int idx) const {
	return &data[(tail + idx) % numElements];
}

template < class type >
int Queue<type>::GetCount() const {
	return (int)(head - tail);
}

template < class type >
type *Queue<type>::PopFirst()
{
	if (head > tail)
	{
		return &data[(tail++) % numElements];
	}
	return NULL;
}

template < class type >
type *Queue<type>::PopLast()
{
	if (head > tail)
	{
		return &data[(--head) % numElements];
	}
	return NULL;
}

template < class type >
type * Queue<type>::New()
{
	if ((head - tail) >= (unsigned int)numElements)
		return NULL;

	type *ev = &data[(head++) % numElements];
	return ev;
}

//////////////////////////////////////////////////////////////////////////

template < class type, int numElements >
class StaticQueue
{
public:
	StaticQueue()
	{
		Clear();
	}

	virtual ~StaticQueue() {};

	type *			New();
	type *			New_Insert(unsigned int idx);
	type *			PopFirst();
	type *			PopLast();
	type *			Get(unsigned int idx);
	const type *	Get(unsigned int idx) const;
	int				GetCount() const;
	void			Clear();

protected:
	type			data[numElements];
	unsigned int	head;
	unsigned int	tail;
};

template < class type, int numElements >
void StaticQueue<type, numElements>::Clear() {
	head = 0;
	tail = 0;
	memset(data, 0, sizeof(data));
}

template < class type, int numElements >
type* StaticQueue<type, numElements>::Get(unsigned int idx) {
	return &data[(tail + idx) % numElements];
}

template < class type, int numElements >
const type* StaticQueue<type, numElements>::Get(unsigned int idx) const {
	return &data[(tail + idx) % numElements];
}

template < class type, int numElements >
int StaticQueue<type, numElements>::GetCount() const {
	return (int)(head - tail);
}

template < class type, int numElements >
type* StaticQueue<type, numElements>::PopFirst()
{
	if (head > tail)
		return &data[(tail++) % numElements];
	return NULL;
}

template < class type, int numElements >
type* StaticQueue<type, numElements>::PopLast()
{
	if (head > tail)
		return &data[(--head) % numElements];
	return NULL;
}

template < class type, int numElements >
type* StaticQueue<type, numElements>::New()
{
	if ((head - tail) >= (unsigned int)numElements)
		return NULL;

	return &data[(head++) % numElements];
}

template < class type, int numElements >
type* StaticQueue<type, numElements>::New_Insert(unsigned int idx)
{
	// push last
	if (idx >= (unsigned int)GetCount())
	{
		return New();
	}

	if ((head - tail) >= (unsigned int)numElements)
		return NULL; // not enough room

	// push first
	if (idx == 0 && tail > 0)
	{
		return &data[(--tail) % numElements];
	}

	// insert at random location
	for (unsigned int i = (head - tail); i > idx; --i)
		data[(tail + i) % numElements] = data[(tail + (i - 1)) % numElements];

	++head;
	return &data[(tail + idx) % numElements];
}

//////////////////////////////////////////////////////////////////////////

// the only difference from Queue is that New() doesn't return NULL when out of elements, overwrites the previous ones
template < class type >
class CircularQueue : public Queue<type>
{
public:
	CircularQueue()
	{
	}

	CircularQueue(unsigned int numElements) : Queue<type>::Queue(numElements)
	{
	}

	virtual			~CircularQueue()
	{
	}

	virtual type*	New()
	{
		if ((this->head - this->tail) >= (unsigned int)this->numElements)
			++this->tail; // pop first

		type *ev = &this->data[(this->head++) % this->numElements];
		return ev;
	}
};

//////////////////////////////////////////////////////////////////////////

template < class type, int numElements >
class CircularStaticQueue
{
public:
	CircularStaticQueue()
	{
		Clear();
	}

	virtual ~CircularStaticQueue()
	{

	}

	virtual type *	New();
	type *			PopFirst();
	type *			PopLast();
	type *			Get(unsigned int idx);
	const type *	Get(unsigned int idx) const;
	// Returns items starting with the most recent (last added)
	type *			GetFromLast( unsigned int idx );
	type *			GetLast();
	const type *	GetLast() const;
	int				GetCount() const;

	void			Clear();

protected:
	type			data[numElements];
	unsigned int	head;
	unsigned int	tail;
};

template < class type, int numElements >
type * CircularStaticQueue<type, numElements>::GetFromLast( unsigned int idx )
{
	int idxer = (head - 1 - idx );
	// make sure we have data
	_ASSERT( idxer > tail );

	return &data[idxer % numElements];
}

template < class type, int numElements >
void CircularStaticQueue<type, numElements>::Clear() {
	head = 0;
	tail = 0;
	memset(data, 0, sizeof(data));
}

template < class type, int numElements >
type* CircularStaticQueue<type, numElements>::Get(unsigned int idx) {
	return &data[(tail + idx) % numElements];
}

template < class type, int numElements >
const type* CircularStaticQueue<type, numElements>::Get(unsigned int idx) const {
	return &data[(tail + idx) % numElements];
}

template < class type, int numElements >
type* CircularStaticQueue<type, numElements>::GetLast() {
	return &data[(head - 1) % numElements];
}

template < class type, int numElements >
const type* CircularStaticQueue<type, numElements>::GetLast() const {
	return &data[(head - 1) % numElements];
}

template < class type, int numElements >
int CircularStaticQueue<type, numElements>::GetCount() const {
	return (int)(head - tail);
}

template < class type, int numElements >
type* CircularStaticQueue<type, numElements>::PopFirst()
{
	if (head > tail)
		return &data[(tail++) % numElements];
	return NULL;
}

template < class type, int numElements >
type* CircularStaticQueue<type, numElements>::PopLast()
{
	if (head > tail)
		return &data[(--head) % numElements];
	return NULL;
}

template < class type, int numElements >
type* CircularStaticQueue<type, numElements>::New()
{
	if ((head - tail) >= (unsigned int)numElements)
		++tail; // pop first

	type *ev = &data[(head++) % numElements];
	return ev;
}
