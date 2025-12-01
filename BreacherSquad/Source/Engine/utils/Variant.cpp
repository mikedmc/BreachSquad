#include "dxstdafx.h"
#include "Variant.h"

CVariant::CVariant( const CVariant &o ) :
	eType( o.eType ),
	m_asUINT32( o.m_asUINT32 )
{
	shName.Init( o.shName.text );
	m_strArg.Init( o.m_strArg.text );
}

CVariant::CVariant() :
	eType( K_ARGTYPE_NONE ),
	m_asUINT32( 0 )
{
	m_strArg.Reset();
}

void CVariant::CopyValueFrom( CVariant *cv )
{
	eType = cv->eType;
	if ( cv->eType == K_ARGTYPE_STRING )
	{
		m_asUINT32 = 0;
		m_strArg = cv->m_strArg;
	}
	else if ( eType == K_ARGTYPE_FLOAT )
	{
		m_asFloat = cv->m_asFloat;
		m_strArg.Reset();
	}
	//defaults on UINT32 which contains all the other ones
	else
	{
		m_asUINT32 = cv->m_asUINT32;
		m_strArg.Reset();
	}
}

void CVariant::Set_AUTO( const WCHAR* argName, WCHAR* strVal )
{
	int rettype = GetTypeFromString( strVal );
	switch ( rettype )
	{
	case K_RETTYPE_INT:
	{
		WCHAR *stopstr;
		INT32 val = ( INT32 ) wcstol( strVal, &stopstr, 10 );
		Set_INT32( argName, val );
	}
	break;
	case K_RETTYPE_HEXCOLOR:
	{
		UINT32 val = 0x0;
		WCHAR* p = strVal;
		while ( *p == '#' || *p == ' ' || *p == '\t' )
			p++;
		swscanf_s( p, L"%08X", &val );

		Set_HEXCOLOR( argName, val );
	}
	break;
	case K_RETTYPE_FLOAT:
	{
		WCHAR *stopstr;
		float val = ( float ) wcstod( strVal, &stopstr );
		Set_FLOAT( argName, val );
	}
	break;
	default:
	case K_RETTYPE_EMPTY:
	case K_RETTYPE_STRING:
	{
		Set_STRING( argName, strVal );
	}
	break;
	}
}

int CVariant::asString( WCHAR *destStr, int maxLen )
{
	switch ( eType )
	{
	case K_ARGTYPE_STRING:
		if ( m_strArg.IsSet() )
			StringCchCopy( destStr, maxLen, m_strArg.text );
		else
			destStr[0] = 0;
		break;
	case K_ARGTYPE_INT32:
		StringCchPrintf( destStr, maxLen, L"%d", m_asINT32 );
		break;
	case K_ARGTYPE_FLOAT:
		StringCchPrintf( destStr, maxLen, L"%.2f", m_asFloat );
		break;
	default:
	case K_ARGTYPE_UINT32:
		StringCchPrintf( destStr, maxLen, L"%d", m_asUINT32 );
		break;
	case K_ARGTYPE_HEXCOLOR:
		StringCchPrintf( destStr, maxLen, L"#%08X", m_asUINT32 );
		break;
	case K_ARGTYPE_BOOL:
		if ( m_asBool )
			StringCchPrintf( destStr, maxLen, L"true" );
		else
			StringCchPrintf( destStr, maxLen, L"false" );
		break;
	}
	return 0;
}

int CVariant::asString( CHAR *destStr, int maxLen )
{
	switch ( eType )
	{
	case K_ARGTYPE_STRING:
		wcstombs( destStr, m_strArg.text, maxLen );
		break;
	case K_ARGTYPE_INT32:
		sprintf( destStr, "%d", m_asINT32 );
		break;
	case K_ARGTYPE_FLOAT:
		sprintf( destStr, "%.2f", m_asFloat );
		break;
	default:
	case K_ARGTYPE_UINT32:
		sprintf( destStr, "%d", m_asUINT32 );
		break;
	case K_ARGTYPE_HEXCOLOR:
		sprintf( destStr, "#%08X", m_asUINT32 );
		break;
	case K_ARGTYPE_BOOL:
		if ( m_asBool )
			sprintf( destStr, "true" );
		else
			sprintf( destStr, "false" );
		break;
	}
	return 0;
}

INT32 CVariant::asInt32()
{
	switch ( eType )
	{
	case K_ARGTYPE_STRING:
		return _wtoi( m_strArg.text );
	case K_ARGTYPE_FLOAT:
		return ( int ) m_asFloat;
	default:
		return m_asINT32;
	}
}

float CVariant::asFloat()
{
	switch ( eType )
	{
	case K_ARGTYPE_STRING:
		return _wtof( m_strArg.text );
	case K_ARGTYPE_FLOAT:
		return ( float ) m_asFloat;
	case K_ARGTYPE_HEXCOLOR:
	case K_ARGTYPE_UINT32:
		return ( float ) m_asUINT32;
	default:
		return ( float ) m_asINT32;
	}
}
