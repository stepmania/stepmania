#ifndef ReceptorArrowROW_H
#define ReceptorArrowROW_H
/*
-----------------------------------------------------------------------------
 Class: ReceptorArrowRow

 Desc: A row of ReceptorArrow objects

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ReceptorArrow.h"
#include "ActorFrame.h"
#include "StyleDef.h"
#include "GameConstantsAndTypes.h"


class ReceptorArrowRow : public ActorFrame
{
public:
	ReceptorArrowRow();

	void Load( PlayerNumber pn, CString NoteSkin, float fYReverseOffset );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void CopyTweening( const ReceptorArrowRow &from );

	void Step( int iCol );
	void SetPressed( int iCol );
	
protected:
	int m_iNumCols;
	PlayerNumber m_PlayerNumber;
	float m_fYReverseOffsetPixels;

	ReceptorArrow	m_ReceptorArrow[MAX_NOTE_TRACKS];
};

#endif
