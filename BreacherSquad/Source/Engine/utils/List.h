#pragma once

#include <stdlib.h>

#ifdef _DEBUG
	#include <assert.h>
#else
    #undef assert
	#define assert(param)
#endif

template< class type >
int StandardSortCompare( const void *a, const void *b )
{
	return *(type*)a - *(type*)b;
}

//
// "List" is actually implemented using a vector, bad choice of name
//
template < class type >
class List
{

private:
	int			m_size; // memory size
	type		*m_list;
	int			m_elements; // actual number of elements inside
	bool		m_bUserMemory; // initialized with a custom memory buffer, we don't have the right to realloc

public:
	typedef int	cmp(const type *, const type *);

				List();
				List(int initialAllocSize);
				List(type* data, int dataSize); // initialize with a custom memory buffer. can't resize this one
				~List();

	void		Alloc(int size);
	void		Free();
	void		Resize(int newsize);
	int			GetCapacity() const; // returns number of elements we have allocated memory for
	int			GetNumElements() const;
	void		SetNumElements(int numElements); // sets the current number of elements and resizes if necessary
	void		Condense();
	int			Insert(const type &element, int index);
	int			Add(const type &element);
	int			AddUnique(const type &element);
	int			Find(const type &element) const; // -1 if not found
	void		Remove(int index);
	void		RemoveAndKeepOrder(int index);
	void		Reset();
	void		Sort(int (*compare)(const void *elem1, const void *elem2));
	void		Shuffle(int seed);
	type		*GetList();
	const type	*GetList() const;

	// use these instead of copy operator
	void		CopyTo(List<type>& other) const;
	void		Clone(const List<type>& other);

	type		&operator[]( int index );
	const type	&operator[]( int index ) const;
	
private:
	DISALLOW_COPY_AND_ASSIGN(List<type>);
};


/*
=====================
List()
=====================
*/
template < class type >
List<type>::List()
{
	m_size	      = 0;
	m_elements    = 0;
	m_list	      = NULL;
	m_bUserMemory = false;
}

template < class type >
List<type>::List(int initialAllocSize)
{
	m_size	      = 0;
	m_elements    = 0;
	m_list	      = NULL;
	m_bUserMemory = false;

	Alloc(initialAllocSize);
}

template < class type >
List<type>::List(type* data, int dataSize)
{
	m_size	      = dataSize;
	m_elements    = 0;
	m_list	      = data;
	m_bUserMemory = true;
}

/*
=====================
~List()
=====================
*/
template < class type >
List<type>::~List()
{
	Free();
}

/*
=====================
GetCapacity()
=====================
*/
template < class type >
inline int List<type>::GetCapacity() const
{
	return m_size;
}

/*
=====================
GetNumElements()
=====================
*/
template < class type >
inline int List<type>::GetNumElements() const
{
	return m_elements;
}

/*
=====================
GetNumElements()
=====================
*/
template < class type >
inline void List<type>::SetNumElements(int numElements)
{
	assert( numElements >= 0 );
	if ( numElements > m_size ) {
		Resize( numElements );
	}
	m_elements = numElements;
}

/*
=====================
Alloc()
=====================
*/
template < class type >
void List<type>::Alloc(int size)
{
	if ( size <= 0 )
	{
		Free();
		return;
	}

	if ( size <= m_size )// daca e de aceeasi marime sau mai mica doar se reseteaza elementele
	{
		m_elements = 0;
		return;
	}
	
	Free();

	m_size = size;
	m_list = new type[m_size];
}


/*
================
Resize( int size )
================
*/
template< class type >
void List<type>::Resize(int newsize)
{
	assert(!m_bUserMemory);
	if (m_bUserMemory)
		return;

	type *temp = nullptr;

	if ( newsize <= 0 )
	{
		Free();
		return;
	}

	if ( newsize == m_size )
		return;

	temp   = m_list;
	if ( temp == nullptr )
		return;
	m_size = newsize;
	if ( m_size < m_elements )
	{
		m_elements = m_size;
	}

	m_list = new type[ m_size ];
	for( int i = 0 ; i < m_elements ; i++ )
	{
		m_list[ i ] = temp[ i ];
	}

	if ( temp )
		delete[] temp;
}


/*
=====================
Free()
=====================
*/
template < class type >
void List<type>::Free()
{
	if ( m_list && !m_bUserMemory )
		delete [] m_list;

	m_list	   = NULL;
	m_size	   = 0;
	m_elements = 0;
}

/*
=====================
Condense()
=====================
*/
template < class type >
void List<type>::Condense()
{
	assert(!m_bUserMemory);
	if ( !m_list || m_bUserMemory )
		return;

	if ( m_elements )
		Resize(m_elements);
	else
		Free();
}


/*
================
Insert(type *element, int index)
================
*/
template < class type >
int List<type>::Insert(const type &element, int index)
{
	if ( index >= m_size )
		return -1;

	if ( m_elements == m_size )
	{
		Resize( 2 * (m_elements+1) );
	}

	if ( index < 0 )
		index = 0;
	else
	if ( index > m_elements )
		index = m_elements;

	for ( int i = m_elements; i > index; --i )
		m_list[i] = m_list[i - 1];

	m_elements++;
	m_list[index] = element;
	return index;
}

/*
================
Add(type *element)
	- adauga un element la sfarsitul listei
	- returneaza indexul la care s-a adaugat elementul
================
*/
template < class type >
inline int List<type>::Add(const type &element)
{
	if ( m_elements >= m_size )
	{
		assert(!m_bUserMemory);
		if (m_bUserMemory)
			return (m_elements - 1);

		Resize( 2 * (m_elements+1) );// ii dubleaza capacitatea
	}

	m_list [ m_elements++ ] = element;
	return m_elements-1;
}

/*
================
AddUnique(type *element)
	- adauga un element la sfarsitul listei doar daca acesta nu exista deja
	- returneaza indexul la care s-a adaugat elementul sau -1 daca nu s-a adaugat
================
*/
template < class type >
int List<type>::AddUnique(const type &element)
{
	int index = Find(element);
	if ( index < 0 )
		return Add(element);
	return -1;
}


/*
================
Find(const type &element) const
	- intoarce indexul elementului sau -1 daca nu a fost gasit
================
*/
template < class type >
int List<type>::Find(const type &element) const
{
	for ( int i = 0 ; i < m_elements ; i++ )
	{
		if ( m_list[i] == element )
			return i;
	}
	return -1;
}

/*
================
Remove()
	- scade numarul de elemente cu 1
================
*/
template < class type >
inline void List<type>::Remove(int index)
{
	if (m_elements > 0 && index >= 0)
	{
		if (m_elements > 1 && index < (m_elements - 1))
			m_list[index] = m_list[m_elements - 1];
		m_elements--;		
	}
}

/*
================
RemoveAndKeepOrder()
	- scade numarul de elemente cu 1 si pastreaza ordinea
================
*/
template < class type >
inline void List<type>::RemoveAndKeepOrder(int index)
{
	if( m_elements > 0 )
	{
		if( m_elements > 1 && index < ( m_elements - 1 ) )
		{
			for( int i( index ); i < m_elements -1; ++i )
				m_list[i] = m_list[i +1];
		}
		m_elements--;		
	}
}

/*
================
Reset()
	- reseteaza la 0 numarul de elemente
================
*/
template < class type >
inline void List<type>::Reset()
{
	m_elements = 0;
}

/*
================
Sort(int (__cdecl *compare )(const void *elem1, const void *elem2 ))
================
*/
template < class type >
inline void List<type>::Sort(int (*compare)(const void *elem1, const void *elem2 ))
{
	if ( m_elements <= 0 )
		return;
	qsort((void*)m_list, (unsigned int)m_elements, sizeof(type), compare);
}

/*
================
GetList()
================
*/
template < class type >
inline type *List<type>::GetList()
{
	return m_list;
}

template < class type >
inline const type *List<type>::GetList() const
{
	return m_list;
}

/*
================
operator[]
================
*/
template < class type >
inline type &List<type>::operator[](int index)
{
	assert(index >= 0 && index < m_elements);
	return m_list[index];
}

/*
================
operator[]
================
*/
template < class type >
inline const type &List<type>::operator[](int index) const
{
	assert(index >= 0 && index < m_elements);
	return m_list[index];
}

/*
================
operator=
================
*/
//template< class type >
//List<type> &List<type>::operator=( const List<type> &other )
//{
//	Free();
//
//	m_elements = other.m_elements;
//	m_size	   = other.m_size;
//
//	if ( m_size )
//	{
//		m_list = new type[ m_size ];
//		for( int i = 0 ; i < m_elements ; i++ )
//		{
//			m_list[ i ] = other.m_list[ i ];
//		}
//	}
//	return *this;
//}

/*
================
operator=
================
*/
template< class type >
void List<type>::CopyTo(List<type>& other) const
{
	other.Alloc(m_elements); // only allocates if their capacity is smaller than what they need
	other.m_elements = m_elements;

	for( int i = 0 ; i < m_elements ; i++ )
	{
		other.m_list[ i ] = m_list[ i ];
	}
}

template< class type >
void List<type>::Clone(const List<type>& other)
{
	other.CopyTo(*this);
}

template< class type >
void List<type>::Shuffle(int seed)
{
	Random rnd;
	rnd.SetSeed(seed);
	for (int i = 0; i < m_elements; ++i)
	{
		int targetIndex = rnd.RandomInt(m_elements);

		type temp           = m_list[i];
		m_list[i]           = m_list[targetIndex];
		m_list[targetIndex] = temp;
	}
}
