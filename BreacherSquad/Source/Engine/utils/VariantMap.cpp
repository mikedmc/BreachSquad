#include "dxstdafx.h"
#include "VariantMap.h"

CVariantMap::CVariantMap(CVariantMap& collection)
{
	m_variants = collection.m_variants;
}

CVariantMap::CVariantMap()
{
}

CVariantMap::~CVariantMap()
{
	m_variants.clear();
}

void CVariantMap::AppendMap(const CVariantMap& sourceCollection)
{
	if ( this != &sourceCollection ) // self-assignment check expected
	{
		// WARN: duplicates from source will be dropped
		m_variants.insert(sourceCollection.m_variants.begin(), sourceCollection.m_variants.end());
	}
}

void CVariantMap::Serialize(FILE *f)
{
	_ASSERT(f != nullptr);

	int nvars = m_variants.size();
	OS_fwrite(&nvars, sizeof(nvars), 1, f);
	for ( auto& element : m_variants )
	{
		element.second.Serialize(f);
	}
}

void CVariantMap::Deserialize(CVariantMap& vc, FILE *f)
{
	_ASSERT(f != nullptr);

	vc.Clear();

	int nvars = 0;
	OS_fread(&nvars, sizeof(nvars), 1, f);

	for ( int ii = 0; ii < nvars; ii++ )
	{
		CVariant v;
		v.Deserialize(f);

		vc.AddVariant(v);
	}
}

void CVariantMap::Deserialize(FILE *f)
{
	_ASSERT(f != nullptr);

	m_variants.clear();

	int nvars = 0;
	OS_fread(&nvars, sizeof(nvars), 1, f);

	CVariant temp;
	for ( int ii = 0; ii < nvars; ii++ )
	{
		temp.Deserialize(f);
		// add to map
		m_variants[temp.shName.text] = temp;
	}
}

void CVariantMap::DeleteVar(const std::wstring varName)
{
	m_variants.erase(varName);
}

void CVariantMap::DeleteVar(const UINT32 varHash)
{
	for ( auto it = m_variants.begin(); it != m_variants.end(); ) {
		if ( it->second.shName.textHash == varHash)
			it = m_variants.erase(it);
		else
			++it;
	}
}

void CVariantMap::Clear()
{
	m_variants.clear();
}

void CVariantMap::AddVariant(CVariant & variant)
{
	m_variants[variant.shName.text] = variant;
}

void CVariantMap::AddVariant(CVariant * variant)
{
	if ( variant == nullptr )
		return;
	m_variants[variant->shName.text] = *variant;
}

void CVariantMap::SetVarUINT32(const std::wstring varName, UINT32 val)
{
	m_variants[varName].Set_UINT32(varName.c_str(), val);
}

void CVariantMap::SetVarHEXCOLOR(const std::wstring varName, UINT32 val)
{
	m_variants[varName].Set_HEXCOLOR(varName.c_str(), val);
}

void CVariantMap::SetVarINT32(const std::wstring varName, INT32 val)
{
	m_variants[varName].Set_INT32(varName.c_str(), val);
}

void CVariantMap::SetVarFloat(const std::wstring varName, float val)
{
	m_variants[varName].Set_FLOAT(varName.c_str(), val);
}

void CVariantMap::SetVarBool(const std::wstring varName, bool val)
{
	m_variants[varName].Set_BOOL(varName.c_str(), val);
}

void CVariantMap::SetVarVoidP(const std::wstring varName, void* val)
{
	m_variants[varName].Set_VOIDP(varName.c_str(), val);
}

void CVariantMap::SetVarString(const std::wstring varName, WCHAR* strVal)
{
	m_variants[varName].Set_STRING(varName.c_str(), strVal);
}

void CVariantMap::SetVarAUTO(const std::wstring varName, WCHAR* strVal)
{
	m_variants[varName].Set_AUTO(varName.c_str(), strVal);
}

