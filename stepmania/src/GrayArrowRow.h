#ifndef GRAYARROWROW_H
#define GRAYARROWROW_H
/*
-----------------------------------------------------------------------------
 Class: GrayArrowRow

 Desc: A row of GrayArrow objects

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrayArrow.h"
#include "ActorFrame.h"
#include "StyleDef.h"
#include "GameConstantsAndTypes.h"


class GrayArrowRow : public ActorFrame
{
public:
	GrayArrowRow();

	void Load( PlayerNumber pn );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Step( int iCol );
	
protected:
	int m_iNumCols;
	PlayerNumber m_PlayerNumber;

	GrayArrow	m_GrayArrow[MAX_NOTE_TRACKS];
};

#endif
