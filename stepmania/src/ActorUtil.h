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


#define SET_XY( actor )			UtilSetXY( actor, m_sName )
#define ON_COMMAND( actor )		UtilOnCommand( actor, m_sName )
#define OFF_COMMAND( actor )	UtilOffCommand( actor, m_sName )
#define SET_XY_AND_ON_COMMAND( actor )		UtilSetXYAndOnCommand( actor, m_sName )
#define COMMAND( actor, command_name )		UtilCommand( actor, m_sName, command_name )


inline void UtilSetXY( Actor& actor, CString sClassName )
{
	ASSERT( !actor.GetName().empty() );
	actor.SetXY( THEME->GetMetricF(sClassName,actor.GetName()+"X"), THEME->GetMetricF(sClassName,actor.GetName()+"Y") );
}

inline float UtilOnCommand( Actor& actor, CString sClassName )
{
	ASSERT( !actor.GetName().empty() );
	return actor.Command( THEME->GetMetric(sClassName,actor.GetName()+"OnCommand") );
}

inline float UtilCommand( Actor& actor, CString sClassName, CString sCommandName )
{
	ASSERT( !actor.GetName().empty() );
	return actor.Command( THEME->GetMetric(sClassName,actor.GetName()+sCommandName+"Command") );
}

inline float UtilOffCommand( Actor& actor, CString sClassName )
{
	// HACK:  It's very often that we command things to TweenOffScreen 
	// that we aren't drawing.  We know that an Actor is not being
	// used if it's name is blank.  So, do nothing on Actors with a blank name.
	if( actor.GetName().empty() )
		return 0;
	return actor.Command( THEME->GetMetric(sClassName,actor.GetName()+"OffCommand") );
}

inline float UtilSetXYAndOnCommand( Actor& actor, CString sClassName )
{
	UtilSetXY( actor, sClassName );
	return UtilOnCommand( actor, sClassName );
}

#endif