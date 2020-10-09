#pragma once

HRESULT GetDXVersion( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, WCHAR* pcDirectXVersionLetter );


HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor,
                                    WCHAR* pcDirectXVersionLetter );
HRESULT GetDirectXVersionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor,
                                          WCHAR* pcDirectXVersionLetter );
HRESULT GetFileVersion( WCHAR* szPath, ULARGE_INTEGER* pllFileVersion );
ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d );
int CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 );
