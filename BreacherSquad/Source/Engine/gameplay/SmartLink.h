//--------------------------------------------------------------------------------------
// (c)2022 Dragomir Mihai - Pixel Shard
//
//	- Acts like a smart pointer of some sorts. Saves a map of all references between objects 
// keeping a pointer to the SmartLink object too so it can reset it when a target objects goes out of scope.
//	- SmartLinks should be allocated on stack (never as pointers) inside the objects themselves
//	- Use case: different ingame objects will keep pointers to each other, pointers that reset to null when the target dies
//--------------------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <string>
#include <map>

class IActiveInterface;

// self erasing links between game objects
class CSmartLink
{
private:
	// Map of links that we can use later to delete dead references
	static				std::map<CSmartLink*, IActiveInterface*>		m_links;

	// "Points to" could be void* and casted afterwards. Don't change manually, always use SetLink!
	IActiveInterface*	pTo;

public:
	CSmartLink();
	~CSmartLink();

	// Get the pointer the link points to (can be null when it's not set)
	inline IActiveInterface*	GetTo() const {
		return pTo;
	};

	// Tells if link is set
	bool				IsSet();

	// Adds a link to the links map
	static void			SetLink( CSmartLink* p_link, IActiveInterface* p_target );

	// Call this to clear link from the links map when the link owner will be deallocated.
	// Not calling this will crash the app when the target gets deallocated as it will try to reset a non existing link
	static void			RemoveLink( CSmartLink* p_link );

	// Call this to reset and remove all SmartLinks to an object that WILL be deallocated
	static void			RemoveAllLinksTo( IActiveInterface* p_target );

	// Resets all links to other objects
	static void			RemoveAllLinks();

};
