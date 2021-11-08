#pragma once
// Author: DMC

template< class type >
int PoolGCStandardSortCompare( const void *a, const void *b )
{
	return *( type* ) a - *( type* ) b;
}


// Garbage collector memory pool template class.
// Always keeps order when deallocating objects
// Garbage Collector GC method must be called from outside the class at intervals. Some failsafe GC calls are implemented too but don't count on them.
template < class type >
class PoolGC
{
private:
	const byte  FLAG_ALIVE = 0x1;

private:
	int			m_size;							// memory size
	int			m_killed;						// counts how many killed elements we have
	byte		*m_listFlags;					// list flags (ALIVE is most important)
	type		*m_list;
	int			m_elements;						// actual number of elements inside

public:
	typedef int	cmp( const type *, const type * );

	PoolGC( int nPoolSize );
	~PoolGC();

	void		Alloc( int size );
	void		Release();
	void		Resize( int newsize );
	int			GetCapacity() const;				// returns number of elements we have allocated memory for
	int			Count() const;
	int			Insert( const type &element, int index );
	int			Add( const type &element );
	int			AddUnique( const type &element );
	int			Find( const type &element ) const; // -1 if not found
	void		Kill( int index );
	bool		IsAlive( int index );
	void		ClearAll();
	void		Sort( int( *compare )( const void *elem1, const void *elem2 ) );
	type		*GetPool();
	const type	*GetPool() const;

	// use these instead of copy operator
	void		CopyTo( PoolGC<type>& other ) const;
	void		Clone( const PoolGC<type>& other );

	type		&operator[]( int index );
	const type	&operator[]( int index ) const;

	int			GC();					// Garbage collect
private:
	DISALLOW_COPY_AND_ASSIGN( PoolGC<type> );
};

template < class type >
bool PoolGC<type>::IsAlive( int index )
{

	_ASSERT( ( index >= 0 ) && ( index < m_elements ) );
	return m_listFlags[ index ] & PoolGC::FLAG_ALIVE;
}

// Garbage Collect!
// removes all dead objects	and returns number of cleared items
template < class type >
int PoolGC<type>::GC()
{
	if ( m_killed == 0 )
		return;
	// live cursor starts after first dead element is found and only moves ahead so we don't process more than we need
	int nLiveCursor = -1;
	int nAliveCnt = 0;
	for ( int i = 0; i < m_elements; i++ )
	{
		// found dead one, look for first live one
		if ( m_listFlags[ i ] & PoolGC::FLAG_ALIVE == false )
		{
			if ( nLiveCursor < 0 )
				nLiveCursor = i + 1;
			while ( ( nLiveCursor < m_elements ) && ( m_listFlags[ nLiveCursor ] & PoolGC::FLAG_ALIVE == false ) )
				nLiveCursor++;
			// we have live one
			if ( nLiveCursor < m_elements )
			{
				m_list[ i ] = m_list[ nLiveCursor ];
				m_listFlags[ i ] |= PoolGC::FLAG_ALIVE;
				m_listFlags[ nLiveCursor ] &= ~PoolGC::FLAG_ALIVE;
				// after marking current element as dead move onto the next so we don't process it again
				nLiveCursor++;
			}
			else
			{
				// no more objects alive
				break;
			}
		}
		else
		{
			nAliveCnt++;
		}
	}

	int nClearedCnt = m_elements - nAliveCnt;
	m_elements = nAliveCnt;
	m_killed = 0;

	return nClearedCnt;
}


template < class type >
PoolGC<type>::PoolGC( int nPoolSize )
{
	m_size = 0;
	m_elements = 0;
	m_list = nullptr;
	m_listFlags = nullptr;
	m_killed = 0;

	Alloc( nPoolSize );
}

template < class type >
PoolGC<type>::~PoolGC()
{
	Release();
}

template < class type >
inline int PoolGC<type>::GetCapacity() const
{
	return m_size;
}

template < class type >
inline int PoolGC<type>::Count() const
{
	return m_elements;
}


// Allocates the list memory (or reallocates, clearing the list)
template < class type >
void PoolGC<type>::Alloc( int size )
{
	if ( size <= 0 )
	{
		Release();
		return;
	}

	if ( size == m_size )
	{
		m_elements = 0;
		m_killed = 0;
		return;
	}

	Release();

	m_size = size;
	m_list = new type[ m_size ];
	m_listFlags = new byte[ m_size ];
}


template< class type >
void PoolGC<type>::Resize( int newsize )
{
	type *temp = nullptr;
	byte *tempFlags = nullptr;

	if ( newsize <= 0 )
	{
		Release();
		return;
	}

	if ( newsize == m_size )
		return;

	temp = m_list;
	tempFlags = m_listFlags;
	m_size = newsize;
	if ( m_size < m_elements )
	{
		m_elements = m_size;
	}

	m_list = new type[ m_size ];
	m_listFlags = new byte[ m_size ];
	for ( int i = 0; i < m_elements; i++ )
	{
		m_list[ i ] = temp[ i ];
		m_listFlags[ i ] = tempFlags[ i ];
	}

	if ( temp )
		delete[] temp;
	if ( tempFlags )
		delete[] tempFlags;
}


template < class type >
void PoolGC<type>::Release()
{
	if ( m_list )
		delete[] m_list;
	if ( m_listFlags )
		delete[] m_listFlags;

	m_list = nullptr;
	m_listFlags = nullptr;
	m_size = 0;
	m_elements = 0;
	m_killed = 0;
}

// Returns index of element inserted into the list or -1 if no more space (calls GC before erroring)
// Insert doesn't take into account dead or alive flags so you can insert wherever you wish and GC will put them in line
template < class type >
int PoolGC<type>::Insert( const type &element, int index )
{
	if ( m_elements >= m_size )
	{
		// no more size, try a GC. If GC doesn't clear anything
		if ( GC() <= 0 )
		{
			ErrorBox( K_ERR_WARNING, L"PoolGC::Insert: Pool overflow!" );
			return -1;
		}
	}

	if ( index >= m_size )
		return -1;

	if ( index < 0 )
		index = 0;
	else
		if ( index > m_elements )
			index = m_elements;

	for ( int i = m_elements; i > index; --i )
	{
		m_list[ i ] = m_list[ i - 1 ];
		m_listFlags[ i ] = m_listFlags[ i - 1 ];
	}

	m_elements++;
	m_list[ index ] = element;
	m_listFlags[ index ] |= PoolGC::FLAG_ALIVE;
	return index;
}

// Returns index of added element or -1 if not enough space (tries calling the GC). 
// WARNING: Index might change after garbage collection.
template < class type >
inline int PoolGC<type>::Add( const type &element )
{
	if ( m_elements >= m_size )
	{
		if ( GC() <= 0 )
		{
			ErrorBox( K_ERR_WARNING, L"PoolGC::Add: Pool overflow!" );
			return -1;
		}
	}

	m_list[ m_elements ] = element;
	m_listFlags[ m_elements ] |= PoolGC::FLAG_ALIVE;
	m_elements++;
	return m_elements - 1;
}

template < class type >
int PoolGC<type>::AddUnique( const type &element )
{
	int index = Find( element );
	if ( index < 0 )
		return Add( element );
	return -1;
}


// Returns index of first element found or -1 if not found
template < class type >
int PoolGC<type>::Find( const type &element ) const
{
	for ( int i = 0; i < m_elements; i++ )
	{
		if ( m_listFlags[ i ] & PoolGC::FLAG_ALIVE == false )
			continue;
		if ( m_list[ i ] == element )
			return i;
	}
	return -1;
}

// marks item as killed (removes ALIVE flag)
template < class type >
inline void PoolGC<type>::Kill( int index )
{
	if ( m_elements > 0 && index >= 0 && index < m_elements )
	{
		if ( m_listFlags[ index ] | PoolGC::FLAG_ALIVE )
		{
			m_killed++;
			m_listFlags[ index ] &= ~PoolGC::FLAG_ALIVE;
		}
	}
}

template < class type >
inline void PoolGC<type>::ClearAll()
{
	m_elements = 0;
}

// EXPENSIVE! Appropriate comparing function must be provided.
template < class type >
inline void PoolGC<type>::Sort( int( *compare )( const void *elem1, const void *elem2 ) )
{
	// call garbage collector before sorting so we only keep alive items and only sort those
	GC();

	if ( m_elements <= 1 )
		return;
	qsort( ( void* ) m_list, ( unsigned int ) m_elements, sizeof( type ), compare );
}

/*
================
GetList()
================
*/
template < class type >
inline type *PoolGC<type>::GetPool()
{
	return m_list;
}

template < class type >
inline const type *PoolGC<type>::GetPool() const
{
	return m_list;
}

/*
================
operator[]
================
*/
template < class type >
inline type &PoolGC<type>::operator[]( int index )
{
	_ASSERT( index >= 0 && index < m_elements );
	return m_list[ index ];
}

/*
================
operator[]
================
*/
template < class type >
inline const type &PoolGC<type>::operator[]( int index ) const
{
	_ASSERT( index >= 0 && index < m_elements );
	return m_list[ index ];
}


/*
================
operator=
================
*/
template< class type >
void PoolGC<type>::CopyTo( PoolGC<type>& other ) const
{
	other.Alloc( m_elements ); // only allocates if their capacity is smaller than what they need
	other.m_elements = m_elements;

	for ( int i = 0; i < m_elements; i++ )
	{
		other.m_list[ i ] = m_list[ i ];
		other.m_listFlags[ i ] = m_listFlags[ i ];
	}
}

template< class type >
void PoolGC<type>::Clone( const PoolGC<type>& other )
{
	other.CopyTo( *this );
}
