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


#define SET_XY( actor )			(actor).SetXY( THEME->GetMetricF(m_sName,(actor).GetName()+"X"), THEME->GetMetricF(m_sName,(actor).GetName()+"Y") );
#define ON_COMMAND( actor )		(actor).Command( THEME->GetMetric(m_sName,(actor).GetName()+"OnCommand") )
#define OFF_COMMAND( actor )	(actor).Command( THEME->GetMetric(m_sName,(actor).GetName()+"OffCommand") )
#define SET_XY_AND_ON_COMMAND( actor )		{ SET_XY(actor);	ON_COMMAND(actor); }


#endif