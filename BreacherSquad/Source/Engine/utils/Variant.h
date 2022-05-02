#pragma once

// Class that holds multiple types of values
class CVariant
{
public:
	enum VariantType
	{
		K_ARGTYPE_NONE = 0,	// default, not set

		K_ARGTYPE_INT32,
		K_ARGTYPE_FLOAT,
		K_ARGTYPE_BOOL,
		K_ARGTYPE_UINT32,
		K_ARGTYPE_HEXCOLOR,	//DWORD
		K_ARGTYPE_VOIDP,
		// special
		K_ARGTYPE_STRING,
	};
	VariantType		eType;		// arg type
	CStringHash		shName;		// arg name 

	union
	{
		INT32	m_asINT32;
		FLOAT	m_asFloat;
		bool	m_asBool;
		UINT32  m_asUINT32;
		VOID*	m_asVoid;
	};
	CStringHash		m_strArg;	//argument string

	inline bool IsSet() {
		return eType != K_ARGTYPE_NONE;
	}

	// constructor
	CVariant();
	// copy constr
	CVariant( const CVariant &o );
	// Serializes to file and returns true for success
	bool					Serialize( FILE *fl );
	// Deserializes variant from file and returns true for success
	bool					Deserialize( FILE* fl );
	// Deserializes from file and returns allocated variant or nullptr
	static CVariant* DeserializeAlloc( FILE *f );


	bool operator== ( CVariant const & o ) const
	{
		if ( eType == K_ARGTYPE_STRING )
			return ( m_strArg.textHash == o.m_strArg.textHash );
		if ( eType == K_ARGTYPE_FLOAT )
			return ( m_asFloat == o.m_asFloat );
		//defaults on UINT32 valabil pentru toate celelalte
		return ( m_asUINT32 == o.m_asUINT32 );
	}

	// Copies the value of another variant
	void CopyValueFrom( CVariant *cv );

	void Set_INT32( const WCHAR* argName, INT32 int32Val ) { shName.Init( argName ); m_asINT32 = int32Val; eType = K_ARGTYPE_INT32; }
	void Set_UINT32( const WCHAR* argName, UINT32 uint32Val ) { shName.Init( argName ); m_asUINT32 = uint32Val; eType = K_ARGTYPE_UINT32; }
	void Set_HEXCOLOR( const WCHAR* argName, UINT32 uint32Val ) { shName.Init( argName ); m_asUINT32 = uint32Val; eType = K_ARGTYPE_HEXCOLOR; }
	void Set_BOOL( const WCHAR* argName, bool boolVal ) { shName.Init( argName ); m_asBool = boolVal; eType = K_ARGTYPE_BOOL; }
	void Set_FLOAT( const WCHAR* argName, float floatVal ) { shName.Init( argName ); m_asFloat = floatVal; eType = K_ARGTYPE_FLOAT; }
	void Set_STRING( const WCHAR* argName, WCHAR* strVal ) { shName.Init( argName ); m_strArg.Init( strVal ); m_asUINT32 = 0.0f; eType = K_ARGTYPE_STRING; }
	void Set_STRING( const WCHAR* argName, CHAR* strVal ) { shName.Init( argName ); m_strArg.Init( strVal ); m_asUINT32 = 0.0f; eType = K_ARGTYPE_STRING; }
	void Set_VOIDP( const WCHAR* argName, void* voidP ) { shName.Init( argName ); m_asVoid = voidP; eType = K_ARGTYPE_VOIDP; }

	void Set_AUTO( const WCHAR* argName, WCHAR* strVal );
	
	int asString( WCHAR *destStr, int maxLen );

	int asString( CHAR *destStr, int maxLen );

	INT32 asInt32();;

	float asFloat();;
};
