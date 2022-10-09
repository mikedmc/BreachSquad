#include "dxstdafx.h"
#include "OpResult.h"


OPRESULT OP_ERR( eOpResult eCode, eOpSeverity eSeverity, WCHAR* szFormat, ... )
{
	OPRESULT opret;
	opret.code = eCode;
	opret.severity = eSeverity;

	va_list marker;
	va_start( marker, szFormat );
	wvsprintf( opret.message, szFormat, marker );
	va_end( marker );

	opret.ShowAsMessageBox();
	return opret;
}
