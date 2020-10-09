#pragma once

#ifdef ENABLE_STEAM

#include "steam_api.h"

#pragma warning( push )
#pragma warning( disable : 4512 ) // disable assignment operator not generated warning ! :S

// Network message types
enum EMessage
{
	// voice chat messages
	k_EMsgVoiceChatBegin = 0, 
	k_EMsgVoiceChatPing = k_EMsgVoiceChatBegin+1,	// just a keep alive message
	k_EMsgVoiceChatData = k_EMsgVoiceChatBegin+2,	// voice data from another player

	// force 32-bit size enum so the wire protocol doesn't get outgrown later
	k_EForceDWORD  = 0x7fffffff, 
};

//////////////////////////////////////////////////////////////////////////////
// voice msg
//////////////////////////////////////////////////////////////////////////////

struct MsgVoiceChatPing_t
{
	MsgVoiceChatPing_t() : m_dwMessageType( k_EMsgVoiceChatPing ) {}
	unsigned long GetMessageType() const { return m_dwMessageType; }

private:
	const unsigned long m_dwMessageType;
};

// voice chat data
struct MsgVoiceChatData_t
{
	MsgVoiceChatData_t() : m_dwMessageType( k_EMsgVoiceChatData ) {}
	unsigned long GetMessageType() const { return m_dwMessageType; }
	
	void SetDataLength( uint32 unLength ) { m_uDataLength = unLength; }
	uint32 GetDataLength() const { return m_uDataLength; }

private:
	const unsigned long m_dwMessageType;
	uint32 m_uDataLength;
};

#pragma warning( pop ) // restore warning flags; assignment operator not generated warning END

#endif ENABLE_STEAM
