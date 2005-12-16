/* ActorUtil - Utility functions for creating and manipulating Actors. */

#ifndef ActorUtil_H
#define ActorUtil_H

#include "Actor.h"
#include "RageTexture.h"

class XNode;

typedef Actor* (*CreateActorFn)(const CString& sDir, const XNode* pNode);

// Each Actor class should have a REGISTER_ACTOR_CLASS in its CPP file.
#define REGISTER_ACTOR_CLASS_WITH_NAME( className, externalClassName ) \
	Actor* Create##className(const CString& sDir, const XNode* pNode) { \
		className *pRet = new className; \
		pRet->LoadFromNode(sDir, pNode); \
		return pRet; } \
	class Register##className { \
	public: \
		Register##className() { ActorUtil::Register(#externalClassName,Create##className); } \
	}; \
	static Register##className register##className; \
	Actor *className::Copy() const { return new className(*this); }

#define REGISTER_ACTOR_CLASS( className ) REGISTER_ACTOR_CLASS_WITH_NAME( className, className )

enum FileType
{
	FT_Bitmap, 
	FT_Movie, 
	FT_Directory, 
	FT_Xml, 
	FT_Model, 
	NUM_FileType, 
	FT_Invalid 
};
const CString& FileTypeToString( FileType ft );

namespace ActorUtil
{
	// Every screen should register its class at program initialization.
	void Register( const CString& sClassName, CreateActorFn pfn );
	Actor* Create( const CString& sClassName, const CString& sDir, const XNode* pNode );


	void SetXY( Actor& actor, const CString &sType );
	void LoadCommand( Actor& actor, const CString &sType, const CString &sCommandName );
	void LoadCommandFromName( Actor& actor, const CString &sType, const CString &sCommandName, const CString &sName );
	void LoadAndPlayCommand( Actor& actor, const CString &sType, const CString &sCommandName, Actor* pParent = NULL );
	void LoadAllCommands( Actor& actor, const CString &sType );
	void LoadAllCommandsFromName( Actor& actor, const CString &sType, const CString &sName );
	inline void OnCommand( Actor& actor, const CString &sType, Actor* pParent = NULL ) { LoadAndPlayCommand( actor, sType, "On", pParent ); }
	inline void OffCommand( Actor& actor, const CString &sType ) { LoadAndPlayCommand( actor, sType, "Off" ); }
	inline void SetXYAndOnCommand( Actor& actor, const CString &sType, Actor* pParent = NULL )
	{
		SetXY( actor, sType );
		OnCommand( actor, sType, pParent );
	}

	/* convenience */
	inline void SetXY( Actor* pActor, const CString &sType ) { SetXY( *pActor, sType ); }
	inline void LoadAndPlayCommand( Actor* pActor, const CString &sType, const CString &sCommandName, Actor* pParent = NULL ) { if(pActor) LoadAndPlayCommand( *pActor, sType, sCommandName, pParent ); }
	inline void OnCommand( Actor* pActor, const CString &sType, Actor* pParent = NULL ) { if(pActor) OnCommand( *pActor, sType, pParent ); }
	inline void OffCommand( Actor* pActor, const CString &sType ) { if(pActor) OffCommand( *pActor, sType ); }
	inline void SetXYAndOnCommand( Actor* pActor, const CString &sType, Actor* pParent = NULL ) { if(pActor) SetXYAndOnCommand( *pActor, sType, pParent ); }

	// Return a Sprite, BitmapText, or Model depending on the file type
	Actor* LoadFromNode( const CString& sAniDir, const XNode* pNode );
	Actor* MakeActor( const CString &sPath, const XNode *pParent = NULL );

	void ResolvePath( CString &sPath, const CString &sName );

	void SortByZPosition( vector<Actor*> &vActors );

	FileType GetFileType( const CString &sPath );

	void SetParamFromStack( Lua *L, CString sName, LuaReference *pOld=NULL );
	void GetParam( Lua *L, const CString &sName );
};

#define SET_XY( actor )			ActorUtil::SetXY( actor, m_sName )
#define ON_COMMAND( actor )		ActorUtil::OnCommand( actor, m_sName )
#define OFF_COMMAND( actor )	ActorUtil::OffCommand( actor, m_sName )
#define SET_XY_AND_ON_COMMAND( actor )		ActorUtil::SetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )		ActorUtil::LoadAndPlayCommand( actor, m_sName, command_name )


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
