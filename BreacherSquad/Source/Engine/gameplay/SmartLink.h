//--------------------------------------------------------------------------------------
// (c)2022 Dragomir Mihai - Pixel Shard
//
//	Acts like a smart pointer of some sorts. Saves a map of all references between objects 
// keeping a pointer to the Link object too so it can reset it when a target objects goes out of scope.
//	Use case: different ingame objects will keep pointers to each other, pointers that reset to null when the target dies
//--------------------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <string>
#include <map>

class IActiveInterface;

// self erasing links between game objects
class CSmartLink
{
public:
	IActiveInterface*	pTo;			// "Points to" could be void* and casted afterwards

	CSmartLink();
	~CSmartLink();

	bool IsSet();

private:
	//static 	CArray<CSmartLink*> arrLinks;		// links used throughout the game get registered here as pointers
	// links map that we can use later to delete dead references
	static std::map<CSmartLink*, IActiveInterface*>		m_links;


public:
	// adds a link to the links map
	static void SetLink( CSmartLink* p_link, IActiveInterface* p_target );

	// call this to clear link from the links map
	static void RemoveLink( CSmartLink* p_link );

	// When one object will be deallocated call this to remove all references to said object
	static void RemoveAllLinksTo( IActiveInterface* p_target );

	// reset all links to other objects
	static void RemoveAllLinks();

};
