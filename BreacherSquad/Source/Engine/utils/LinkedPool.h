//--------------------------------------------------------------------------------------
// (c)2022 Dragomir Mihai - Pixel Shard
//
// Template object linked list pool, iterable with pointer iterator
// Keeps elements in a single array and organizes used and free elements in 2 double linked lists for fast access
// Keeps unique index(m_nID) and used/not used flags for each element
// Contained class DTOR/CTOR only get called on Init and Release when deallocationg the container array.
// Make sure you reset/construct the data after calling Hire() as nodes are always reused.
//--------------------------------------------------------------------------------------

#pragma once

template<typename TYPE> class CLinkedPool
{
public:
	class CLNode
	{
	public:
		TYPE		m_data;
	private:  
		friend struct IteratorPtr;
		friend class CLinkedPool;

		CLNode*		m_pPrev;		//don't mess with me
		CLNode*		m_pNext;		//don't mess with me

		int			m_nID;			// unique index (used for references as ID)
		bool		m_bUsed;		// flag that tells us if it's in use or if it's available for hiring

	public:
		
		int			GetID() {
			return m_nID;
		}

		int			IsAlive() {
			return m_bUsed;
		}
	};

public:
	// iterates through the USED list and returns POINTERS to list nodes.
	struct IteratorPtr
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = CLNode*;
		using pointer = CLNode**;
		using reference = CLNode*;

		IteratorPtr( CLNode* ptr ) : m_ptr( ptr ), m_ptr_next(ptr->m_pNext) {}
		// returns pointer to the contents of the node, used when : CType* temp = iterator;
		reference operator*() const { return m_ptr; }
		// returns value of the contents when CType temp = *iterator;
		//pointer operator->() { return m_ptr->m_data; }
		IteratorPtr& operator++() { 
			m_ptr = m_ptr_next; 
			if ( m_ptr ) 
				m_ptr_next = m_ptr->m_pNext; 
			return *this; 
		}
		IteratorPtr operator++( int ) {
			m_ptr = m_ptr_next;
			if ( m_ptr )
				m_ptr_next = m_ptr->m_pNext; 
			return *this;
		}

		friend bool operator== ( const IteratorPtr& a, const IteratorPtr& b ) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= ( const IteratorPtr& a, const IteratorPtr& b ) { return a.m_ptr != b.m_ptr; };

	private:
		CLNode* m_ptr;
		CLNode* m_ptr_next;				// save next pointer so we can remove/delete current element if needed
	};

	// iterator methods
public:
	IteratorPtr begin() { return IteratorPtr( pListUsed.m_pNext ); }
	IteratorPtr end() { return IteratorPtr( &pListUsed ); }

private:
	int m_nSize;						// actual pool size
	int m_nUsedCnt;						// how many are used

public:
	CLNode*				pArrNodes;		// all nodes get allocated as an array and kept as a list through pListFree and pListUsed

	CLNode				pListFree;		// free nodes circular list start node (not using his data, kept just to hold the ring)
	CLNode				pListUsed;		// used nodes circular list start node (not using his data, kept just to hold the ring)

	CLinkedPool() : pArrNodes( NULL ), m_nSize( 0 ), m_nUsedCnt( 0 )
	{
		pListFree.m_pNext = pListFree.m_pPrev = &pListFree;	
		pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;	
	}
	~CLinkedPool()
	{
		Release();
	}

	// Initializes list with maximum number of elements
	bool			Init( int nPoolSize );
	// Initializes list with maximum number of elements and provides initializer function (lambda usually)
	bool			Init( int nPoolSize, void ( *fn_initialize )(TYPE* element) );
	// Releases all list elements
	void			Release();
	// returns number of used elements
	inline int		Count() { return m_nUsedCnt; }
	
	// Returns item by global ID (which is the global index)
	CLNode*			GetByID( int nID )
	{
		if ( nID < 0 || nID >= m_nSize )
			return nullptr;
		return &pArrNodes[nID];
	}

	// Returns pointer to available list node or null if all nodes are used.
	// Doesn't call CTOR. Make sure you clear the data before using, nodes are always reused.
	CLNode* Hire()
	{
		CLNode* nod = pListFree.m_pNext;
		if ( nod == &pListFree )
		{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			ErrorBox( K_ERR_WARNING, L"CLinkedPool::GetFreeNode. No more free nodes!" );
#endif
			return nullptr;
		}
		// remove node from FREE list
		pListFree.m_pNext = pListFree.m_pNext->m_pNext;
		// put it in BUSY list
		pListUsed.m_pPrev->m_pNext = nod;
		nod->m_pPrev = pListUsed.m_pPrev;
		pListUsed.m_pPrev = nod;
		nod->m_pNext = &pListUsed;
		
		nod->m_bUsed = true;

		m_nUsedCnt++;
		// return pointer to node
		return nod;
	}

	// Returns node to the "free" list so it can be reused by another call to Hire().
	// Doesn't call DTOR.
	void Dismiss( CLNode* node )
	{
		_ASSERT( node != nullptr );
		node->m_bUsed = false;
		// link neighbours between them
		node->m_pNext->m_pPrev = node->m_pPrev;
		node->m_pPrev->m_pNext = node->m_pNext;
		// add it back to free nodes list
		pListFree.m_pNext->m_pPrev = node;
		node->m_pNext = pListFree.m_pNext;
		node->m_pPrev = &pListFree;
		pListFree.m_pNext = node;

		m_nUsedCnt--;
	}
};

template<typename TYPE>
bool CLinkedPool<TYPE>::Init( int nPoolSize, void( *fn_initialize )( TYPE* element ) )
{
	if ( !Init( nPoolSize ) )
		return false;

	for ( int kk = 0; kk < m_nSize; kk++ )
	{
		fn_initialize( &pArrNodes[kk].m_data );
	}

	return true;
}

template<typename TYPE> 
bool CLinkedPool <TYPE>::Init( int nPoolSize )
{
	m_nSize = nPoolSize;
	if ( m_nSize < 10 )
		m_nSize = 10;

	_ASSERT( pArrNodes == nullptr );
	// Allocate containing array
	pArrNodes = nullptr;
	pArrNodes = new CLNode[ m_nSize ];
	// initialize nodes
	for ( int kk = 0; kk < m_nSize; kk++ )
	{
		pArrNodes[kk].m_nID = kk;
		pArrNodes[kk].m_bUsed = false;
	}
	// place all nodes in FREE list
	pListFree.m_pNext = &pArrNodes[ 0 ];
	pListFree.m_pPrev = &pArrNodes[ m_nSize - 1 ];
	// set first and last nodes
	pArrNodes[ 0 ].m_pNext = &pArrNodes[ 1 ];
	pArrNodes[ 0 ].m_pPrev = &pListFree;
	pArrNodes[ m_nSize - 1 ].m_pPrev = &pArrNodes[ m_nSize - 2 ];
	pArrNodes[ m_nSize - 1 ].m_pNext = &pListFree;
	// set all other nodes
	for ( int kk = 1; kk < m_nSize - 1; kk++ )
	{
		pArrNodes[ kk ].m_pNext = &pArrNodes[ kk + 1 ];
		pArrNodes[ kk ].m_pPrev = &pArrNodes[ kk - 1 ];
	}
	// empty BUSY list
	pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;

	m_nUsedCnt = 0;

	return true;
}

template<typename TYPE> 
void CLinkedPool <TYPE>::Release()
{
	m_nSize = 0;
	m_nUsedCnt = 0;
	SAFE_DELETE_ARRAY( pArrNodes );

	pListFree.m_pNext = pListFree.m_pPrev = &pListFree;
	pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;
}
