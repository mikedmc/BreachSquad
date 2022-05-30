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
		if (m_pData != nullptr)
		{
			delete [] m_pData;
			m_pData = nullptr;
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

	int GetCapacity() {
		return nMaxSize;
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
// Template object double linked list pool
//--------------------------------------------------------------------------------------
template<typename TYPE> class CDoubleLinkedPool
{
public:
	class CLinkedPoolNode
	{
	public:
		TYPE m_data;
	public:
		CLinkedPoolNode* m_pPrev;  //don't mess with me
		CLinkedPoolNode* m_pNext;  //don't mess with me
	};

	int m_nSize;		//cate particule sunt folosite in momentul de fata
	int m_nUsedCnt;		//cate sunt folosite
	CLinkedPoolNode*	pArrNodes;	// all nodes get allocated as an array and kept as a list through pListFree and pListUsed
	
	CLinkedPoolNode		pListFree; //free nodes list
	CLinkedPoolNode		pListUsed; //used nodes list

	CDoubleLinkedPool() : pArrNodes(NULL), m_nSize(0), m_nUsedCnt(0)
	{
		pListFree.m_pNext = pListFree.m_pPrev = &pListFree;	//indica spre ele insele
		pListUsed.m_pNext = pListUsed.m_pPrev = &pListUsed;	//indica spre ele insele
	}

	~CDoubleLinkedPool()
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

template<typename TYPE> HRESULT CDoubleLinkedPool <TYPE>::Init(int nPoolSize)
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

template<typename TYPE> void CDoubleLinkedPool <TYPE>::Release()
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

