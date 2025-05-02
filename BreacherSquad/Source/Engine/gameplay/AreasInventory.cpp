#include "dxstdafx.h"
#include "AreasInventory.h"

CAreasInventory::CAreasInventory()
{
	
}

CAreasInventory::~CAreasInventory()
{
	Release();
}

OPRESULT CAreasInventory::LoadAreasSpecs(WCHAR* strXMLPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(strXMLPath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Areas Inventory XML:%s\n", strXMLPath);
	}

	Release();

	WCHAR tmppath[MAX_PATH_STD];
	WCHAR Path[MAX_PATH_STD];

	pugi::xml_node areasnode = doc.root().child(L"Areas");
	for (pugi::xml_node bnode = areasnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CAreaSpecs as;
		as.strFilename = bnode.attribute( L"File" ).as_string();

		{
			FILE *fl = nullptr;

			swprintf_s( tmppath, MAX_PATH, L"media/levels/areas/%s", as.strFilename.c_str() );
			FileManager::GetMediaPath( tmppath, Path );
			int err = OS_wfopen_s( &fl, Path, L"rb" );

			if ( fl == nullptr || err != 0 )
			{
				return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"Could not open area file:%s", Path );
			}

			UINT32 arrInts[10];	//VERSION, area W in blocks, area H in blocks, 0, 0, 0, 0...
			OS_fread( arrInts, sizeof( UINT32 ), 10, fl );
			if ( arrInts[0] != K_EDITOR_LEVEL_FILE_FORMAT_VERSION )
			{
				return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"[Error] LoadAreasSpecs(%s)::Wrong file version found: %d !", Path, arrInts[0] );
			}
			as.sizeBL.x = arrInts[1];
			as.sizeBL.y = arrInts[2];

			CHAR charArr[MAX_PATH];
			// read connectors setup
			OS_freadString( fl, charArr );
			mbstowcs( tmppath, charArr, MAX_PATH );
			as.strSpecs = tmppath;
			// read tags
			OS_freadString( fl, charArr );
			mbstowcs( tmppath, charArr, MAX_PATH );
			as.strTags = tmppath;

			fclose( fl );
		}
		arrAreas.push_back(as);
	}

	return K_OP_OK;
}

void CAreasInventory::Release()
{
	arrAreas.clear();
}


std::vector<CAreaSpecs> CAreasInventory::GetAreas()
{
	std::vector<CAreaSpecs> retArr(arrAreas);
	return retArr;
}


