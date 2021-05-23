#pragma once

//--------------------------------------------------------------------------------------
// fast fixed size array
//--------------------------------------------------------------------------------------
template<typename TYPE, int nMaxSize> class CFixedArray
{
public:
	int nCount; //don't set by hand unless you know what you're doing
public:
	TYPE * m_pData;

	//CTOR/DTOR
	CFixedArray()
	{
		m_pData = new TYPE[nMaxSize];
		nCount = 0;
	}
	~CFixedArray()
	{
		if (m_pData != NULL)
		{
			delete [] m_pData;
			m_pData = NULL;
			nCount = 0;
		}
	}

	//Adds element and returns index
	int Add(TYPE element)
	{
		if (nCount >= nMaxSize)
		{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			ErrorBox(K_ERR_WARNING, L"CFixedArray::Add failed! Array too small! Size:%d", nMaxSize);
#endif
			return -1;
		}
		m_pData[nCount++] = element;
		return nCount - 1;
	}
	//find element
	int FindElementIndex(TYPE element)
	{
		for (int kk = 0; kk < nCount; kk++)
		{
			if (m_pData[kk] == element)
				return kk;
		}
		return -1;
	}
	//contains element
	bool Contains(TYPE element)
	{
		for (int kk = 0; kk < nCount; kk++)
		{
			if (m_pData[kk] == element)
				return true;
		}
		return false;
	}

	//Clears array
	FORCEINLINE void Clear() {
		nCount = 0;
	}

	FORCEINLINE int Count() const {
		return nCount;
	}

	const TYPE& operator[](int nIndex) const
	{
		assert(nIndex < nMaxSize);
		return m_pData[nIndex];
	}
	TYPE& operator[](int nIndex)
	{
		assert(nIndex < nMaxSize);
		return m_pData[nIndex];
	}

	inline void Sort(int(*compare)(const void *elem1, const void *elem2))
	{
		if (nCount <= 0)
			return;
		qsort((void*)m_pData, (unsigned int)nCount, sizeof(TYPE), compare);
	}

};


//--------------------------------------------------------------------------------------
// Circular array of fixed size
//--------------------------------------------------------------------------------------
template<typename TYPE, int nMaxSize> class CCircularStack
{
public:
	int nHead;	//stays on newest valid element or -1 if stack empty
	int nTail;	//stays on oldest item or -1 if stack empty (always nTail <= nHead)
	int nSize;	//stack size
public:
	TYPE * m_pData;	 //don't access directly unless you know what you're doing

	//CTOR/DTOR
	CCircularStack()
	{
		m_pData = new TYPE[nMaxSize];
		nSize = nMaxSize;
		nHead = -1;
		nTail = -1;
	}
	~CCircularStack()
	{
		if (m_pData != NULL)
		{
			delete[] m_pData;
			m_pData = NULL;
			nSize = 0;
			nHead = -1;
			nTail = -1;
		}
	}

	//Adds element and returns index
	int Add(TYPE element)
	{
		nHead++;
		//chack tail (first added item)
		if (nTail < 0)
			nTail = 0;
		//enforce stack size
		if (nHead - nTail >= nSize)
			nTail++;
		//write element
		m_pData[nHead % nSize] = element;

		return nHead;
	}
	//contains element
	bool Contains(TYPE element)
	{
		if (nHead < 0)
			return false;

		for (int kk = nHead; kk >= nTail; kk--)
		{
			if (m_pData[kk % nSize] == element)
				return true;
		}
		return false;
	}

	// Gets the index of said element (nTail <= nIndex <= nHead)
	// \returns: -1 if item not found
	int IndexOf(TYPE element)
	{
		if (nHead < 0)
			return -1;

		for (int kk = nHead; kk >= nTail; kk--)
		{
			if (m_pData[kk % nSize] == element)
				return kk;
		}
		return -1;
	}

	//Clears array
	FORCEINLINE void Clear() {
		nHead = -1;
		nTail = -1;
	}

	FORCEINLINE int Count() const {
		if (nHead < 0)
			return 0;
		return (nHead - nTail + 1);
	}

	const TYPE& operator[](int nIndex) const
	{
		assert((nIndex <= nHead) && (nIndex >= nTail) && (nIndex >= 0));
		return m_pData[nIndex % nSize];
	}
	TYPE& operator[](int nIndex)
	{
		assert((nIndex <= nHead) && (nIndex >= nTail) && (nIndex >= 0));
		return m_pData[nIndex % nSize];
	}
};


//--------------------------------------------------------------------------------------
// Template object linked list pool
//--------------------------------------------------------------------------------------
template<typename TYPE> class CLinkedPool
{
public:
	class CLinkedPoolNode
	{
	public:
		TYPE m_data;
	public:
		//links catre prev si next
		CLinkedPoolNode* m_pPrev;  //don't mess with me
		CLinkedPoolNode* m_pNext;  //don't mess with me
	};

	int m_nSize;		//cate particule sunt folosite in momentul de fata
	int m_nUsedCnt;		//cate sunt folosite
	CLinkedPoolNode*	pArrNodes; //aici se aloca nodurile
	
	CLinkedPoolNode		pListFree; //free nodes list (folosim doar next si prev)
	CLinkedPoolNode		pListUsed; //used nodes list (folosim doar next si prev)

	CLinkedPool() : pArrNodes(NULL), m_nSize(0), m_nUsedCnt(0)
	{
		pListFree.m_pNext = pListFree.m_pPrev = &pListFree;	//indica spre ele insele
		pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;	//indica spre ele insele
	}

	~CLinkedPool()
	{
		Release();
	}

	HRESULT Init(int nPoolSize);
	void Release();

	//returns null if all nodes are used
	CLinkedPoolNode* HireNode()
	{
		CLinkedPoolNode* nod = pListFree.m_pNext;
		if (nod == &pListFree)
		{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			ErrorBox(K_ERR_WARNING, L"CLinkedPool::GetFreeNode. No more free nodes!");
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
		
		m_nUsedCnt++;
		// return pointer to node
		return nod;
	}

	void DismissNode(CLinkedPoolNode* node)
	{
		assert(node != nullptr);

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

template<typename TYPE> HRESULT CLinkedPool <TYPE>::Init(int nPoolSize)
{
	m_nSize = nPoolSize;
	if (m_nSize < 10)
		m_nSize = 10;

	assert(pArrNodes == NULL);
	//aloc array-ul continuu in memorie
	pArrNodes = NULL;
	pArrNodes = new CLinkedPoolNode[m_nSize];
	if (pArrNodes == NULL)
		return E_OUTOFMEMORY;
	// place all nodes in FREE list
	pListFree.m_pNext = &pArrNodes[0];
	pListFree.m_pPrev = &pArrNodes[m_nSize - 1];
	// set first and last nodes
	pArrNodes[0].m_pNext = &pArrNodes[1];
	pArrNodes[0].m_pPrev = &pListFree;
	pArrNodes[m_nSize - 1].m_pPrev = &pArrNodes[m_nSize - 2];
	pArrNodes[m_nSize - 1].m_pNext = &pListFree;
	// set all other nodes
	for (int kk = 1; kk < m_nSize - 1; kk++)
	{
		pArrNodes[kk].m_pNext = &pArrNodes[kk + 1];
		pArrNodes[kk].m_pPrev = &pArrNodes[kk - 1];
	}
	// empty BUSY list
	pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;

	m_nUsedCnt = 0;

	return S_OK;
}

template<typename TYPE> void CLinkedPool <TYPE>::Release()
{
	m_nSize = 0;
	m_nUsedCnt = 0;
	SAFE_DELETE_ARRAY(pArrNodes);

	pListFree.m_pNext = pListFree.m_pPrev = &pListFree;
	pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;
}


//--------------------------------------------------------------------------------------
// A template simple linked list
//--------------------------------------------------------------------------------------
template<typename TYPE> class CLinkedList
{
public:
	//nested class for list node
	class CLinkedListNode
	{
	public:
		TYPE m_data;
		CLinkedListNode* m_pNext;

		CLinkedListNode() :m_pNext(NULL)
		{
		}
	};

protected:
	CLinkedListNode* m_pStart;      //pointer la start
	CLinkedListNode* m_pEnd;		 //pointer la ultimul nod
	int m_nSize;        // # of elements
public:
	CLinkedList()
	{
		m_pStart = NULL; m_pEnd = NULL; m_nSize = 0;
	}
	~CLinkedList()
	{
		RemoveAll();
	}
	/*
	const TYPE& operator[](int nIndex) const
	{
	return GetAt(nIndex);
	}
	TYPE& operator[](int nIndex)
	{
	return GetAt(nIndex);
	}

	CLinkedList& operator=(const CLinkedList <TYPE>& a)
	{
	if (this == &a)
	return *this;
	RemoveAll();
	for (int i = 0; i < a.m_nSize; i++)
	Add(a.m_pData[i]);
	return *this;
	}
	*/
	HRESULT AddLast(const TYPE& value);
	HRESULT AddBefore(const CLinkedListNode *node, const TYPE& value);
	HRESULT Remove(const TYPE& value);
	/*
	TYPE& GetAt(int nIndex) const
	{
	assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex];
	}
	*/
	int GetSize() const
	{
		return m_nSize;
	}
	int Count() const
	{
		return m_nSize;
	}
	CLinkedListNode* GetFirst()
	{
		return m_pStart;
	}
	CLinkedListNode* GetLast()
	{
		return m_pEnd;
	}
	/*
	bool    Contains(const TYPE& value)
	{
	return (-1 != IndexOf(value));
	}

	int     IndexOf(const TYPE& value)
	{
	return (m_nSize > 0) ? IndexOf(value, 0, m_nSize) : -1;
	}
	int     IndexOf(const TYPE& value, int iStart)
	{
	return IndexOf(value, iStart, m_nSize - iStart);
	}
	int     IndexOf(const TYPE& value, int nIndex, int nNumElements);

	int     LastIndexOf(const TYPE& value)
	{
	return (m_nSize > 0) ? LastIndexOf(value, m_nSize - 1, m_nSize) : -1;
	}
	int     LastIndexOf(const TYPE& value, int nIndex)
	{
	return LastIndexOf(value, nIndex, nIndex + 1);
	}
	int     LastIndexOf(const TYPE& value, int nIndex, int nNumElements);

	HRESULT Remove(int nIndex);
	*/

	void RemoveAll()
	{
		CLinkedListNode* temp = NULL;
		CLinkedListNode* iter = m_pStart;
		while (iter != null)
		{
			temp = iter;
			iter = iter->m_pNext;
			delete temp; temp = NULL;
		}

		m_nSize = 0;
		m_pEnd = NULL;
		m_pStart = NULL;
	}
};

//--------------------------------------------------------------------------------------
template<typename TYPE> HRESULT CLinkedList <TYPE>::AddLast(const TYPE& value)
{
	CLinkedListNode* nn = new CLinkedListNode();
	nn->m_data = value;

	if (m_nSize == 0)
	{
		m_pStart = nn;
	}
	else
	{
		m_pEnd->m_pNext = nn;
	}

	m_pEnd = nn;
	++m_nSize;

	return S_OK;
}


template<typename TYPE> HRESULT CLinkedList <TYPE>::Remove(const TYPE& value)
{
	if (m_nSize == 0)
		return S_OK;

	CLinkedListNode* iter = m_pStart;
	CLinkedListNode* lastnode = null;
	//se seteaza pointerul pe nodul precedent celui cautat
	while ((iter != NULL) && (iter->m_data != value))
	{
		lastnode = iter;
		iter = iter->m_pNext;
	}
	//nu a gasit nimic
	if (iter == NULL)
		return E_FAIL;

	if (lastnode != null)
	{
		lastnode->m_pNext = iter->m_pNext;
		if (iter->m_pNext == null) //ultimul nod
		{
			m_pEnd = lastnode;
		}
	}
	else //este primul nod
	{
		m_pStart = iter->m_pNext;
	}

	delete iter; iter = NULL;

	m_nSize--;

	return S_OK;
}

template<typename TYPE> HRESULT CLinkedList <TYPE>::AddBefore(const CLinkedListNode *node, const TYPE& value)
{
	if (m_nSize == 0)
		return E_FAIL;

	CLinkedListNode* iter = m_pStart;
	CLinkedListNode* lastnode = null;
	//merge pana cand pointerul next este egal cu nodul cautat (se opreste imediat inainte de nodul cautat)
	while ((iter != NULL) && (iter != node))
	{
		lastnode = iter;
		iter = iter->m_pNext;
	}

	if (iter == NULL)
		return E_FAIL;

	CLinkedListNode* nn = new CLinkedListNode();
	nn->m_data = value;
	nn->m_pNext = iter->m_pNext; //preiau indicatia nodului cautat
	
	if (lastnode == null) //este primul acum
	{
		nn->m_pNext = iter;
		m_pStart = nn;
	}
	else
	{
		lastnode->m_pNext = nn;
		nn->m_pNext = iter;
	}

	++m_nSize;

	return S_OK;
}


///----------------------------------------------------------------------------------
/// CArray - replaces CGrowableArray from dxut
/// Growable template vector
///----------------------------------------------------------------------------------
template<typename TYPE> class CArray
{
public:
	CArray()
	{
		m_pData = NULL; m_nSize = 0; m_nMaxSize = 0;
	}
	CArray(const CArray <TYPE>& a)
	{
		for (int i = 0; i < a.m_nSize; i++) Add(a.m_pData[i]);
	}
	~CArray()
	{
		RemoveAll();
	}

	const TYPE& operator[](int nIndex) const
	{
		return GetAt(nIndex);
	}
	TYPE& operator[](int nIndex)
	{
		return GetAt(nIndex);
	}

	CArray& operator=(const CArray <TYPE>& a)
	{
		if (this == &a)
			return *this;
		RemoveAll();
		for (int i = 0; i < a.m_nSize; i++)
			Add(a.m_pData[i]);
		return *this;
	}

	HRESULT SetSize(int nNewMaxSize);
	HRESULT Add(const TYPE& value);
	HRESULT Insert(int nIndex, const TYPE& value);
	HRESULT SetAt(int nIndex, const TYPE& value);
	TYPE& GetAt(int nIndex) const
	{
		assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex];
	}
	int     GetSize() const
	{
		return m_nSize;
	}
	int     Count() const
	{
		return m_nSize;
	}
	TYPE* GetData()
	{
		return m_pData;
	}
	bool    Contains(const TYPE& value)
	{
		return (-1 != IndexOf(value));
	}

	int     IndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? IndexOf(value, 0, m_nSize) : -1;
	}
	int     IndexOf(const TYPE& value, int iStart)
	{
		return IndexOf(value, iStart, m_nSize - iStart);
	}
	int     IndexOf(const TYPE& value, int nIndex, int nNumElements);

	int     LastIndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? LastIndexOf(value, m_nSize - 1, m_nSize) : -1;
	}
	int     LastIndexOf(const TYPE& value, int nIndex)
	{
		return LastIndexOf(value, nIndex, nIndex + 1);
	}
	int     LastIndexOf(const TYPE& value, int nIndex, int nNumElements);

	HRESULT Remove(int nIndex);
	void    RemoveAll()
	{
		SetSize(0);
	}
	void    Reset()
	{
		m_nSize = 0;
	}

protected:
	TYPE* m_pData;      // the actual array of data
	int m_nSize;        // # of elements (upperBound - 1)
	int m_nMaxSize;     // max allocated

	HRESULT SetSizeInternal(int nNewMaxSize);  // This version doesn't call ctor or dtor.
};

// This version doesn't call ctor or dtor.
template< typename TYPE >
HRESULT CArray<TYPE>::SetSizeInternal(int nNewMaxSize)
{
	if (nNewMaxSize < 0)
	{
		assert(false);
		return E_INVALIDARG;
	}

	if (nNewMaxSize == 0)
	{
		// Shrink to 0 size & cleanup
		if (m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}

		m_nMaxSize = 0;
		m_nSize = 0;
	}
	else if (m_pData == NULL || nNewMaxSize > m_nMaxSize)
	{
		// Grow array
		int nGrowBy = (m_nMaxSize == 0) ? 16 : m_nMaxSize;
		nNewMaxSize = max(nNewMaxSize, m_nMaxSize + nGrowBy);

		TYPE* pDataNew = (TYPE*)realloc(m_pData, nNewMaxSize * sizeof(TYPE));
		if (pDataNew == NULL)
			return E_OUTOFMEMORY;

		m_pData = pDataNew;
		m_nMaxSize = nNewMaxSize;
	}

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CArray<TYPE>::SetSize(int nNewMaxSize)
{
	int nOldSize = m_nSize;

	if (nOldSize > nNewMaxSize)
	{
		// Removing elements. Call dtor.

		for (int i = nNewMaxSize; i < nOldSize; ++i)
			m_pData[i].~TYPE();
	}

	// Adjust buffer.  Note that there's no need to check for error
	// since if it happens, nOldSize == nNewMaxSize will be true.)
	HRESULT hr = SetSizeInternal(nNewMaxSize);

	if (nOldSize < nNewMaxSize)
	{
		// Adding elements. Call ctor.

		for (int i = nOldSize; i < nNewMaxSize; ++i)
			::new (&m_pData[i]) TYPE;
	}

	return hr;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CArray<TYPE>::Add(const TYPE& value)
{
	HRESULT hr;
	if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
		return hr;

	// Construct the new element
	::new (&m_pData[m_nSize]) TYPE;

	// Assign
	m_pData[m_nSize] = value;
	++m_nSize;

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CArray<TYPE>::Insert(int nIndex, const TYPE& value)
{
	HRESULT hr;

	// Validate index
	if (nIndex < 0 ||
		nIndex > m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	// Prepare the buffer
	if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
		return hr;

	// Shift the array
	MoveMemory(&m_pData[nIndex + 1], &m_pData[nIndex], sizeof(TYPE) * (m_nSize - nIndex));

	// Construct the new element
	::new (&m_pData[nIndex]) TYPE;

	// Set the value and increase the size
	m_pData[nIndex] = value;
	++m_nSize;

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CArray<TYPE>::SetAt(int nIndex, const TYPE& value)
{
	// Validate arguments
	if (nIndex < 0 ||
		nIndex >= m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	m_pData[nIndex] = value;
	return S_OK;
}


//**************************************************************************************
// Searches for the specified value and returns the index of the first occurrence
// within the section of the data array that extends from iStart and contains the 
// specified number of elements. Returns -1 if value is not found within the given 
// section.
//**************************************************************************************
template< typename TYPE >
int CArray<TYPE>::IndexOf(const TYPE& value, int iStart, int nNumElements)
{
	// Validate arguments
	if (iStart < 0 ||
		iStart >= m_nSize ||
		nNumElements < 0 ||
		iStart + nNumElements > m_nSize)
	{
		assert(false);
		return -1;
	}

	// Search
	for (int i = iStart; i < (iStart + nNumElements); i++)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}


//**************************************************************************************
// Searches for the specified value and returns the index of the last occurrence
// within the section of the data array that contains the specified number of elements
// and ends at iEnd. Returns -1 if value is not found within the given section.
//**************************************************************************************
template< typename TYPE >
int CArray<TYPE>::LastIndexOf(const TYPE& value, int iEnd, int nNumElements)
{
	// Validate arguments
	if (iEnd < 0 ||
		iEnd >= m_nSize ||
		nNumElements < 0 ||
		iEnd - nNumElements < 0)
	{
		assert(false);
		return -1;
	}

	// Search
	for (int i = iEnd; i > (iEnd - nNumElements); i--)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}



//**************************************************************************************
template< typename TYPE >
HRESULT CArray<TYPE>::Remove(int nIndex)
{
	if (nIndex < 0 ||
		nIndex >= m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	// Destruct the element to be removed
	m_pData[nIndex].~TYPE();

	// Compact the array and decrease the size
	MoveMemory(&m_pData[nIndex], &m_pData[nIndex + 1], sizeof(TYPE) * (m_nSize - (nIndex + 1)));
	--m_nSize;

	return S_OK;
}
