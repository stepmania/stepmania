/* ActorUtil - Utility functions for creating and manipulating Actors. */

#ifndef ActorUtil_H
#define ActorUtil_H

#include "Actor.h"
#include "RageTexture.h"

class XNode;

typedef Actor* (*CreateActorFn)();

template<typename T>
Actor *CreateActor() { return new T; }

// Each Actor class should have a REGISTER_ACTOR_CLASS in its CPP file.
#define REGISTER_ACTOR_CLASS_WITH_NAME( className, externalClassName ) \
	struct Register##className { \
		Register##className() { ActorUtil::Register(#externalClassName, CreateActor<className>); } \
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
	FT_Lua, 
	FT_Model, 
	NUM_FileType, 
	FT_Invalid 
};
const RString& FileTypeToString( FileType ft );

namespace ActorUtil
{
	// Every screen should register its class at program initialization.
	void Register( const RString& sClassName, CreateActorFn pfn );
	Actor* Create( const RString& sClassName, const XNode* pNode, Actor *pParentActor );

	apActorCommands ParseActorCommands( const RString &sCommands, const RString &sName = "" );
	void SetXY( Actor& actor, const RString &sType );
	void LoadCommand( Actor& actor, const RString &sType, const RString &sCommandName );
	void LoadCommandFromName( Actor& actor, const RString &sType, const RString &sCommandName, const RString &sName );
	void LoadAndPlayCommand( Actor& actor, const RString &sType, const RString &sCommandName );
	void LoadAllCommands( Actor& actor, const RString &sType );
	void LoadAllCommandsFromName( Actor& actor, const RString &sType, const RString &sName );
	inline void OnCommand( Actor& actor, const RString &sType ) { LoadAndPlayCommand( actor, sType, "On" ); }
	inline void OffCommand( Actor& actor, const RString &sType ) { LoadAndPlayCommand( actor, sType, "Off" ); }
	inline void SetXYAndOnCommand( Actor& actor, const RString &sType )
	{
		SetXY( actor, sType );
		OnCommand( actor, sType );
	}

	/* convenience */
	inline void SetXY( Actor* pActor, const RString &sType ) { SetXY( *pActor, sType ); }
	inline void LoadAndPlayCommand( Actor* pActor, const RString &sType, const RString &sCommandName ) { if(pActor) LoadAndPlayCommand( *pActor, sType, sCommandName ); }
	inline void OnCommand( Actor* pActor, const RString &sType ) { if(pActor) OnCommand( *pActor, sType ); }
	inline void OffCommand( Actor* pActor, const RString &sType ) { if(pActor) OffCommand( *pActor, sType ); }
	inline void SetXYAndOnCommand( Actor* pActor, const RString &sType ) { if(pActor) SetXYAndOnCommand( *pActor, sType ); }

	// Return a Sprite, BitmapText, or Model depending on the file type
	Actor* LoadFromNode( const XNode* pNode, Actor *pParentActor = NULL );
	Actor* MakeActor( const RString &sPath, const XNode *pParent = NULL, Actor *pParentActor = NULL );
	RString GetSourcePath( const XNode *pNode );
	RString GetWhere( const XNode *pNode );
	bool GetAttrPath( const XNode *pNode, const RString &sName, RString &sOut );

	void ResolvePath( RString &sPath, const RString &sName );

	void SortByZPosition( vector<Actor*> &vActors );

	FileType GetFileType( const RString &sPath );

	void SetParamFromStack( Lua *L, RString sName, LuaReference *pOld=NULL );
	void GetParam( Lua *L, const RString &sName );

	struct ActorParam
	{
		ActorParam( RString sName, RString sValue );
		~ActorParam();
		void Release();

	private:
		RString m_sName;
		LuaReference *m_pOld;
	};
};

#define SET_XY( actor )			ActorUtil::SetXY( actor, m_sName )
#define ON_COMMAND( actor )		ActorUtil::OnCommand( actor, m_sName )
#define OFF_COMMAND( actor )		ActorUtil::OffCommand( actor, m_sName )
#define SET_XY_AND_ON_COMMAND( actor )	ActorUtil::SetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )	ActorUtil::LoadAndPlayCommand( actor, m_sName, command_name )


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
