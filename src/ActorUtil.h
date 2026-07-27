#ifndef ActorUtil_H
#define ActorUtil_H

#include "Actor.h"
#include "RageTexture.h"

#include <vector>

class XNode;

typedef Actor* (*CreateActorFn)();

template<typename T>
Actor *CreateActor() { return new T; }

/**
 * @brief Register the requested Actor with the specified external name.
 *
 * This should be called manually only if needed. */
#define REGISTER_ACTOR_CLASS_WITH_NAME( className, externalClassName ) \
	struct Register##className { \
		Register##className() { ActorUtil::Register(#externalClassName, CreateActor<className>); } \
	}; \
	className *className::Copy() const { return new className(*this); } \
	static Register##className register##className

/**
 * @brief Register the requested Actor so that it's more accessible.
 *
 * Each Actor class should have a REGISTER_ACTOR_CLASS in its CPP file. */
#define REGISTER_ACTOR_CLASS( className ) REGISTER_ACTOR_CLASS_WITH_NAME( className, className )

enum FileType
{
	FT_Bitmap,
	FT_Sprite,
	FT_Sound,
	FT_Movie,
	FT_Directory,
	FT_Xml,
	FT_Model,
	FT_Lua,
	FT_Ini,
	NUM_FileType,
	FileType_Invalid
};
const RString& FileTypeToString( FileType ft );

/** @brief Utility functions for creating and manipulating Actors. */
namespace ActorUtil
{
	void InitFileTypeLists();
	std::vector<RString> const& GetTypeExtensionList(FileType ft);
	void AddTypeExtensionsToList(FileType ft, std::vector<RString>& add_to);

	// Every screen should register its class at program initialization.
	void Register( const RString& sClassName, CreateActorFn pfn );

	apActorCommands ParseActorCommands( const RString &sCommands, const RString &sName = "" );
	void SetXY( Actor& actor, const RString &sMetricsGroup );
	inline void PlayCommand( Actor& actor, const RString &sCommandName ) { actor.PlayCommand( sCommandName ); }
	inline void OnCommand( Actor& actor )
	{
		ASSERT_M( actor.HasCommand("On"), ssprintf("%s is missing an OnCommand.", actor.GetLineage().c_str()) );
		actor.PlayCommand("On");
	}
	inline void OffCommand( Actor& actor )
	{
		// HACK:  It's very often that we command things to TweenOffScreen
		// that we aren't drawing.  We know that an Actor is not being
		// used if its name is blank.  So, do nothing on Actors with a blank name.
		// (Do "playcommand" anyway; BGAs often have no name.)
		if( actor.GetName().empty() )
			return;
		ASSERT_M( actor.HasCommand("Off"), ssprintf("Actor %s is missing an OffCommand.", actor.GetLineage().c_str()) );
		actor.PlayCommand("Off");
	}

	void LoadCommand( Actor& actor, const RString &sMetricsGroup, const RString &sCommandName );
	void LoadCommandFromName( Actor& actor, const RString &sMetricsGroup, const RString &sCommandName, const RString &sName );
	void LoadAllCommands( Actor& actor, const RString &sMetricsGroup );
	void LoadAllCommandsFromName( Actor& actor, const RString &sMetricsGroup, const RString &sName );

	inline void LoadAllCommandsAndSetXY( Actor& actor, const RString &sMetricsGroup )
	{
		LoadAllCommands( actor, sMetricsGroup );
		SetXY( actor, sMetricsGroup );
	}
	inline void LoadAllCommandsAndOnCommand( Actor& actor, const RString &sMetricsGroup )
	{
		LoadAllCommands( actor, sMetricsGroup );
		OnCommand( actor );
	}
	inline void SetXYAndOnCommand( Actor& actor, const RString &sMetricsGroup )
	{
		SetXY( actor, sMetricsGroup );
		OnCommand( actor );
	}
	inline void LoadAllCommandsAndSetXYAndOnCommand( Actor& actor, const RString &sMetricsGroup )
	{
		LoadAllCommands( actor, sMetricsGroup );
		SetXY( actor, sMetricsGroup );
		OnCommand( actor );
	}

	/* convenience */
	inline void SetXY( Actor* pActor, const RString &sMetricsGroup ) { SetXY( *pActor, sMetricsGroup ); }
	inline void PlayCommand( Actor* pActor, const RString &sCommandName ) { if(pActor) pActor->PlayCommand( sCommandName ); }
	inline void OnCommand( Actor* pActor ) { if(pActor) ActorUtil::OnCommand( *pActor ); }
	inline void OffCommand( Actor* pActor ) { if(pActor) ActorUtil::OffCommand( *pActor ); }

	inline void LoadAllCommands( Actor* pActor, const RString &sMetricsGroup ) { if(pActor) LoadAllCommands( *pActor, sMetricsGroup ); }

	inline void LoadAllCommandsAndSetXY( Actor* pActor, const RString &sMetricsGroup ) { if(pActor) LoadAllCommandsAndSetXY( *pActor, sMetricsGroup ); }
	inline void LoadAllCommandsAndOnCommand( Actor* pActor, const RString &sMetricsGroup ) { if(pActor) LoadAllCommandsAndOnCommand( *pActor, sMetricsGroup ); }
	inline void SetXYAndOnCommand( Actor* pActor, const RString &sMetricsGroup ) { if(pActor) SetXYAndOnCommand( *pActor, sMetricsGroup ); }
	inline void LoadAllCommandsAndSetXYAndOnCommand( Actor* pActor, const RString &sMetricsGroup ) { if(pActor) LoadAllCommandsAndSetXYAndOnCommand( *pActor, sMetricsGroup ); }

	// Return a Sprite, BitmapText, or Model depending on the file type
	Actor* LoadFromNode( const XNode* pNode, Actor *pParentActor = nullptr );
	Actor* MakeActor( const RString &sPath, Actor *pParentActor = nullptr );
	RString GetSourcePath( const XNode *pNode );
	RString GetWhere( const XNode *pNode );
	bool GetAttrPath( const XNode *pNode, const RString &sName, RString &sOut, bool optional= false );
	bool LoadTableFromStackShowErrors( Lua *L );

	bool ResolvePath( RString &sPath, const RString &sName, bool optional= false );

	void SortByZPosition( std::vector<Actor*> &vActors );

	FileType GetFileType( const RString &sPath );
};

#define SET_XY( actor )			ActorUtil::SetXY( actor, m_sName )
#define ON_COMMAND( actor )		ActorUtil::OnCommand( actor )
#define OFF_COMMAND( actor )		ActorUtil::OffCommand( actor )
#define SET_XY_AND_ON_COMMAND( actor )	ActorUtil::SetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )	ActorUtil::PlayCommand( actor, command_name )
#define LOAD_ALL_COMMANDS( actor )	ActorUtil::LoadAllCommands( actor, m_sName )
#define LOAD_ALL_COMMANDS_AND_SET_XY( actor )			ActorUtil::LoadAllCommandsAndSetXY( actor, m_sName )
#define LOAD_ALL_COMMANDS_AND_ON_COMMAND( actor )		ActorUtil::LoadAllCommandsAndOnCommand( actor, m_sName )
#define LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( actor )	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( actor, m_sName )


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
