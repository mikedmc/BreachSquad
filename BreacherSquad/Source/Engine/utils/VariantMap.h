#pragma once

class CVariantMap
{
private:
	CVariantComplex				defaultVariant;
public:
	std::map<std::wstring, CVariantComplex>		m_variants;
	
	CVariantMap(CVariantMap&);
	CVariantMap();
	~CVariantMap();

	inline int					GetSize() { return m_variants.size(); }
	void						AppendMap(const CVariantMap& sourceMap);

	inline CVariantComplex& operator[](const std::wstring key) {
		return m_variants[key];
	}

	void AddVariant(CVariantComplex variant);
	void AddVariant(CVariantComplex * variant);

	void SetVarUINT32(const std::wstring varName, UINT32 val);
	void SetVarHEXCOLOR(const std::wstring varName, UINT32 val);
	void SetVarINT32(const std::wstring varName, INT32 val);
	void SetVarFloat(const std::wstring varName, float val);
	void SetVarBool(const std::wstring varName, bool val);
	void SetVarVoidP(const std::wstring varName, void* val);
	void SetVarString(const std::wstring varName, WCHAR* strVal);
	void SetVarAUTO(const std::wstring varName, WCHAR* strVal);

	void			Serialize(FILE *f);
	void			Deserialize(FILE *f);

	void DeleteVar(const std::wstring varName);
	void DeleteVar(const UINT32 varHash);
	void Clear();

public:
	// deserializes into a variant map structure
	static void		Deserialize(CVariantMap& cv, FILE *f);
};
