#include "dxstdafx.h"
#include "SmartLink.h"

std::map<CSmartLink*, IActiveInterface*> CSmartLink::m_links;

void CSmartLink::SetLink( CSmartLink* p_link, IActiveInterface* p_target )
{
	_ASSERT( p_target != nullptr && p_link != nullptr );
	p_link->pTo = p_target;
	m_links[p_link] = p_target;
}

void CSmartLink::RemoveLink( CSmartLink* p_link )
{
	_ASSERT( p_link != nullptr );
	p_link->pTo = nullptr;
	m_links.erase( p_link );
}

void CSmartLink::RemoveAllLinksTo( IActiveInterface* p_target )
{
	_ASSERT( p_target != nullptr );
	for ( auto it = m_links.begin(); it != m_links.end(); ) {
		if ( it->second == p_target )
			it = m_links.erase( it );
		else
			++it;
	}
}

void CSmartLink::RemoveAllLinks()
{
	for ( auto & link : m_links )
	{
		CSmartLink* lnk = link.first;
		lnk->pTo = nullptr;
	}
	m_links.clear();
}

bool CSmartLink::IsSet()
{
	return pTo != nullptr;
}

CSmartLink::CSmartLink() :
	pTo( nullptr )
{
}

CSmartLink::~CSmartLink()
{
	// just remove link and reset pointer
	pTo = nullptr;
}
