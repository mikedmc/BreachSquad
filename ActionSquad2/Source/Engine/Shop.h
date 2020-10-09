#pragma once

class CShop {
private:
	struct sShopEntry {
		CStringHash shName;
		INT32		nPrice;
		bool		bOwned;		//is item owned?
		//CTOR
		sShopEntry() : nPrice(0), bOwned(false)
		{}
	};

private:
	static const int			MAX_SHOP_ITEMS = 50;				//max number of shop entries

	sShopEntry					m_arrShopEntries[MAX_SHOP_ITEMS];	//actual shop entries array
	int							m_nEntriesCnt;						//total number of shop entries

public:
	CShop();
	// Loads shop entries from XML 
	HRESULT						LoadItemsAndPrices();
	// Sets all bOwned on false
	void						Reset();
	// Save the complete list of owned UINT32 shop entries names hashes
	HRESULT						SaveOwnedItems(FILE* fl);
	// Loads the complete list of owned UINT32 shop entries names hashes
	HRESULT						LoadOwnedItems(FILE* fl);
	// Mark item as bought
	HRESULT						UnlockItem(UINT32 dwNameHash, bool bVerbose = false);
	// Gets the price of the item
	// \returns price, 0 if item is already owned, -1 if not found
	int							GetItemPrice(UINT32 dwNameHash);
};

///--- SINGLETON ---
CShop& UTGetShop();
