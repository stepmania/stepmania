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


float UtilCommand( Actor& actor, CString sClassName, CString sCommandName );
inline float UtilCommand( Actor* pActor, CString sClassName, CString sCommandName ) { return UtilCommand( *pActor, sClassName, sCommandName ); }

inline float UtilOnCommand( Actor& actor, CString sClassName ) { return UtilCommand( actor, sClassName, "On" ); }
inline float UtilOnCommand( Actor* pActor, CString sClassName ) { return UtilOnCommand( *pActor, sClassName ); }

inline float UtilOffCommand( Actor& actor, CString sClassName ) { return UtilCommand( actor, sClassName, "Off" ); }
inline float UtilOffCommand( Actor* pActor, CString sClassName ) { return UtilOffCommand( *pActor, sClassName ); }

inline float UtilSetXYAndOnCommand( Actor& actor, CString sClassName )
{
	UtilSetXY( actor, sClassName );
	return UtilOnCommand( actor, sClassName );
}
inline float UtilSetXYAndOnCommand( Actor* pActor, CString sClassName ) { return UtilSetXYAndOnCommand( *pActor, sClassName ); }


// Return a Sprite, BitmapText, or Model depending on the file type
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


/* Actor command parsing helpers. */
#define HandleParams int iMaxIndexAccessed = 0;
#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)
#define CheckHandledParams if( iMaxIndexAccessed != (int)asTokens.size()-1 ) { IncorrectActorParametersWarning( asTokens, iMaxIndexAccessed, asTokens.size() ); }
void IncorrectActorParametersWarning( const CStringArray &asTokens, int iMaxIndexAccessed, int size );
inline CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
}


#endif
