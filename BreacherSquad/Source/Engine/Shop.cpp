#include "dxstdafx.h"

CShop::CShop()
{
	m_nEntriesCnt = 0;
}

HRESULT CShop::LoadItemsAndPrices()
{
	//reset stuff
	m_nEntriesCnt = 0;
	//load document
	WCHAR xmlPath[MAX_PATH];
	FileManager::GetMediaPath(L"media/levels/data/gear_screen.xml", xmlPath);

	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		ErrorBox(K_ERR_CRITICAL, L"Unable to load Shop XML:%s\n", xmlPath);
		return E_FAIL;
	}

	pugi::xml_node rootnode = doc.root().child(L"GEAR_DATA").child(L"SHOP");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		//name
		const WCHAR* bType = bnode.name();
		m_arrShopEntries[m_nEntriesCnt].shName.Init(bType);
		//price
		m_arrShopEntries[m_nEntriesCnt].nPrice = bnode.attribute(L"nPrice").as_int();
		//owned reset
		m_arrShopEntries[m_nEntriesCnt].bOwned = false;
		
		///Increase entries count
		m_nEntriesCnt++;
	}

	return S_OK;
}

void CShop::Reset()
{
	for (int kk = 0; kk < m_nEntriesCnt; kk++)
	{
		m_arrShopEntries[kk].bOwned = false;
	}
	LOG(L"[Shop] Reset!");
}

HRESULT CShop::SaveOwnedItems(FILE* fl)
{
	if (fl == null)
		return E_FAIL;

	UINT32 arrLocalOwned[MAX_SHOP_ITEMS] = { 0 };
	int localIdx = 0;
	//actual IDs
	for (int kk = 0; kk < m_nEntriesCnt; kk++)
	{
		if (m_arrShopEntries[kk].bOwned == true)
		{
			arrLocalOwned[localIdx++] = m_arrShopEntries[kk].shName.getHash();
		}
	}

	//nr of items
	OS_fwrite(&localIdx, sizeof(int), 1, fl);
	OS_fwrite(arrLocalOwned, sizeof(DWORD), localIdx, fl);

	return S_OK;
}

HRESULT CShop::LoadOwnedItems(FILE* fl)
{
	if (fl == null)
		return E_FAIL;

	//nr of items
	int nOwnedItems = OS_freadInt32(fl);
	//actual IDs
	for (int kk = 0; kk < nOwnedItems; kk++)
	{
		UINT32 nhash = OS_freadUInt32(fl);
		//unlock it
		if (FAILED(UnlockItem(nhash)))
		{
			ErrorBox(K_ERR_WARNING, L"CShop::LoadOwnedItems - Unlocked item that isn't in the shop!");
		}
	}

	return S_OK;
}

HRESULT CShop::UnlockItem(UINT32 dwNameHash, bool bVerbose)
{
	if (dwNameHash == 0)
	{
		ErrorBox(K_ERR_WARNING, L"[Warning] CShop::Trying to unlock item with empty name hash!");
		return S_OK;
	}

	for (int kk = 0; kk < m_nEntriesCnt; kk++)
	{
		if (m_arrShopEntries[kk].shName.getHash() == dwNameHash)
		{
			m_arrShopEntries[kk].bOwned = true;
			if (bVerbose)
			{
				LOG(L"CShop::unlocked item [%s]", m_arrShopEntries[kk].shName.text);
			}
			return S_OK;
		}
	}
	return E_FAIL;
}

int CShop::GetItemPrice(UINT32 dwNameHash)
{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//weapons always unlocked
	//return 0;
#endif
	if (dwNameHash == 0)
		return 0;

	for (int kk = 0; kk < m_nEntriesCnt; kk++)
	{
		if (m_arrShopEntries[kk].shName.getHash() == dwNameHash)
		{
			if (m_arrShopEntries[kk].bOwned)
				return 0;
			else
				return m_arrShopEntries[kk].nPrice;
		}
	}
	//item not found in shop so it must be free
	return 0;
}


///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************
CShop& UTGetShop()
{
	static CShop g_Shop;
	return g_Shop;
}
