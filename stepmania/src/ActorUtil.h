/* ActorScroller - ActorFrame that moves its children. */

#ifndef ActorUtil_H
#define ActorUtil_H

#include "Actor.h"
#include "RageTexture.h"

struct XNode;


namespace ActorUtil
{
	void SetXY( Actor& actor, const CString &sScreenName );
	inline void SetXY( Actor* pActor, const CString &sScreenName ) { SetXY( *pActor, sScreenName ); }

	void RunCommand( Actor& actor, const CString &sScreenName, const CString &sCommandName );

	inline void OnCommand( Actor& actor, const CString &sScreenName ) { RunCommand( actor, sScreenName, "On" ); }
	inline void OffCommand( Actor& actor, const CString &sScreenName ) { RunCommand( actor, sScreenName, "Off" ); }
	inline void SetXYAndOnCommand( Actor& actor, const CString &sScreenName )
	{
		SetXY( actor, sScreenName );
		OnCommand( actor, sScreenName );
	}

	/* convenience */
	inline void RunCommand( Actor* pActor, const CString &sScreenName, const CString &sCommandName ) { if(pActor) RunCommand( *pActor, sScreenName, sCommandName ); }
	inline void OnCommand( Actor* pActor, const CString &sScreenName ) { if(pActor) OnCommand( *pActor, sScreenName ); }
	inline void OffCommand( Actor* pActor, const CString &sScreenName ) { if(pActor) OffCommand( *pActor, sScreenName ); }
	inline void SetXYAndOnCommand( Actor* pActor, const CString &sScreenName ) { if(pActor) SetXYAndOnCommand( *pActor, sScreenName ); }

	// Return a Sprite, BitmapText, or Model depending on the file type
	Actor* LoadFromActorFile( const CString& sAniDir, const XNode* pNode );
	Actor* MakeActor( const RageTextureID &ID );
};

#define SET_XY( actor )			ActorUtil::SetXY( actor, m_sName )
#define ON_COMMAND( actor )		ActorUtil::OnCommand( actor, m_sName )
#define OFF_COMMAND( actor )	ActorUtil::OffCommand( actor, m_sName )
#define SET_XY_AND_ON_COMMAND( actor )		ActorUtil::SetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )		ActorUtil::RunCommand( actor, m_sName, command_name )


#endif

/*
 * (c) 2003-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
