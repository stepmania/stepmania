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

	void Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	void Step( int iCol );
	void UpdateBars( int iCol );
	
protected:
	int m_iNumCols;
	PlayerNumber m_PlayerNumber;
	float m_fYReverseOffsetPixels;

	GrayArrow	m_GrayArrow[MAX_NOTE_TRACKS];
};

#endif
