#pragma once

///--- string and hash pair ---
class CStringHash //string-hash pair
{
public:
	WCHAR* text{};
	UINT32 textHash{ 0 };  //hash of the string

	FORCEINLINE const UINT32 getHash() const { return textHash; }

	CStringHash() { text = new WCHAR[1]; text[0] = 0; textHash = 0; }
	~CStringHash() { SAFE_DELETE_ARRAY( text ); textHash = 0; }

	void Dispose() {
		if ( text != nullptr ) {
			SAFE_DELETE_ARRAY( text ); 
			textHash = 0;
			text = new WCHAR[1]; text[0] = 0;
		}
	}

	CStringHash( WCHAR const * const strText )
	{
		int len = wcslen( strText );
		if ( len == 0 )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		text = new WCHAR[len + 1]; text[len] = 0;
		wcscpy_s( text, len + 1, strText );
		textHash = FastHash( text, wcslen( text ) );
	}

	CStringHash( char const * const strText )
	{
		int len = strlen( strText );
		if ( len == 0 )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		text = new WCHAR[len + 1]; text[len] = 0;
		mbstowcs_s( null, text, len + 1, strText, len );
		textHash = FastHash( text, wcslen( text ) );
	}

	//copy constructor	
	CStringHash( const CStringHash & o )
	{
		Init( o.text );
	}

	CStringHash& operator= ( const CStringHash & o )
	{
		Init( o.text );
		return *this;
	}

	const bool IsEmpty() const {
		return ( textHash == 0 );
	}

	const bool IsSet() const {
		return ( textHash != 0 );
	}

	const bool IsEqual( WCHAR* stext ) const {
		return ( textHash == FastHash( stext, wcslen( stext ) ) );
	}

	void Init( WCHAR const * const strText )
	{
		Dispose();

		if ( strText == nullptr )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		int len = wcslen( strText );
		if ( len == 0 )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		text = new WCHAR[len + 1]; text[len] = 0;
		wcscpy_s( text, len + 1, strText );
		textHash = FastHash( text, wcslen( text ) );
	}

	void Init( CHAR const * const strText )
	{
		Dispose();

		if ( strText == nullptr )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		int len = strlen( strText );
		if ( len == 0 )
		{
			text = nullptr;
			textHash = 0;
			return;
		}

		text = new WCHAR[len + 1]; text[len] = 0;
		mbstowcs_s( null, text, len + 1, strText, len );

		textHash = FastHash( text, wcslen( text ) );
	}

	void Reset()
	{
		SAFE_DELETE_ARRAY( text );
		textHash = 0;
		text = new WCHAR[1]; text[0] = 0;
	}

	bool operator== ( CStringHash const & o ) const
	{
		return ( textHash == o.textHash );
	}
	bool operator!= ( CStringHash const & o ) const
	{
		return ( textHash != o.textHash );
	}
};


