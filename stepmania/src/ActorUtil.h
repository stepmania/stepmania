#ifndef ActorUtil_H
#define ActorUtil_H

#include "Actor.h"
#include "RageTexture.h"


#define SET_XY( actor )			UtilSetXY( actor, m_sName )
#define ON_COMMAND( actor )		UtilOnCommand( actor, m_sName )
#define OFF_COMMAND( actor )	UtilOffCommand( actor, m_sName )
#define SET_XY_AND_ON_COMMAND( actor )		UtilSetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )		UtilCommand( actor, m_sName, command_name )


void UtilSetXY( Actor& actor, CString sClassName );
inline void UtilSetXY( Actor* pActor, CString sClassName ) { UtilSetXY( *pActor, sClassName ); }


void UtilCommand( Actor& actor, CString sClassName, CString sCommandName );

inline void UtilOnCommand( Actor& actor, CString sClassName ) { UtilCommand( actor, sClassName, "On" ); }
inline void UtilOffCommand( Actor& actor, CString sClassName ) { UtilCommand( actor, sClassName, "Off" ); }
inline void UtilSetXYAndOnCommand( Actor& actor, CString sClassName )
{
	UtilSetXY( actor, sClassName );
	UtilOnCommand( actor, sClassName );
}

/* convenience */
inline void UtilCommand( Actor* pActor, CString sClassName, CString sCommandName ) { if(pActor) UtilCommand( *pActor, sClassName, sCommandName ); }
inline void UtilOnCommand( Actor* pActor, CString sClassName ) { if(pActor) UtilOnCommand( *pActor, sClassName ); }
inline void UtilOffCommand( Actor* pActor, CString sClassName ) { if(pActor) UtilOffCommand( *pActor, sClassName ); }
inline void UtilSetXYAndOnCommand( Actor* pActor, CString sClassName ) { if(pActor) UtilSetXYAndOnCommand( *pActor, sClassName ); }

// Return a Sprite, BitmapText, or Model depending on the file type
Actor* LoadFromActorFile( CString sIniPath, CString sLayer = "Actor" );
Actor* MakeActor( RageTextureID ID );


// creates the appropriate Actor derivitive on load and
// automatically deletes Actor on deconstruction.
class AutoActor
{
public:
	AutoActor()						{ m_pActor = NULL; }
	~AutoActor()					{ Unload(); }
	operator const Actor* () const	{ return m_pActor; }
	operator Actor* ()				{ return m_pActor; }
	const Actor *operator->() const { return m_pActor; }
	Actor *operator->()				{ return m_pActor; }
	void Unload()					{ if(m_pActor) { delete m_pActor; m_pActor=NULL; } }
	bool IsLoaded() const			{ return m_pActor != NULL; }
	void Load( CString sPath );
	void LoadAndSetName( CString sScreenName, CString sActorName );

protected:
	Actor* m_pActor;
};

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
