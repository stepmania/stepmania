#ifndef ActorUtil_H
#define ActorUtil_H
/*
-----------------------------------------------------------------------------
 Class: ActorUtil

 Desc: Helpful macros

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
	AutoActor() { m_pActor = NULL; }
	~AutoActor() { Unload(); }
	operator const Actor* () const { ASSERT(m_pActor); return m_pActor; }
	operator Actor* () { ASSERT(m_pActor); return m_pActor; }
	const Actor *operator->() const { ASSERT(m_pActor); return m_pActor; }
	Actor *operator->() { ASSERT(m_pActor); return m_pActor; }
	void Unload() { if(m_pActor) { delete m_pActor; m_pActor=NULL; } }
	bool IsLoaded() const { return m_pActor != NULL; }
	void Load( CString sPath )
	{
		Unload();
		m_pActor = MakeActor( sPath );
	}

protected:
	Actor* m_pActor;
};

#endif
