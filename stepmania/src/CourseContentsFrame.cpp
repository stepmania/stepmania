#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: CourseContentsFrame

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseContentsFrame.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"

const float NAME_X		= -100;
const float METER_X		= +50;
const float GAP_Y		= 34;
const float START_Y		= 0 - MAX_COURSE_CONTENTS/2*GAP_Y;


CourseContentsFrame::CourseContentsFrame()
{
	for( int i=0; i<MAX_COURSE_CONTENTS; i++ )
	{
		m_textContents[i].Load( THEME->GetPathTo(FONT_TEXT_BANNER) );
		m_textContents[i].SetHorizAlign( Actor::align_left );
		m_textContents[i].SetXY( NAME_X, START_Y + GAP_Y*i );
		m_textContents[i].SetText( "" );
		m_textContents[i].SetZoom( 0.7f );
		this->AddActor( &m_textContents[i] );

		m_Meters[i].SetXY( METER_X, START_Y + GAP_Y*i + 10 );
		this->AddActor( &m_Meters[i] );
	}
}

void CourseContentsFrame::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );
}

void CourseContentsFrame::SetFromCourse( Course* pCourse )
{
	ASSERT( pCourse != NULL );

	int i;

	// turn all lines "off"
	for( i=0; i<MAX_COURSE_CONTENTS; i++ )
	{
		m_textContents[i].SetText( "" );
		m_Meters[i].SetFromNotes( NULL );
	}

	for( i=0; i<pCourse->m_iStages-1; i++ )
	{
		m_textContents[i].SetText( pCourse->m_apSongs[i]->GetMainTitle() );
		m_textContents[i].SetDiffuseColor( DifficultyClassToColor(pCourse->m_apNotes[i]->m_DifficultyClass) );

		m_Meters[i].SetFromNotes( pCourse->m_apNotes[i] );
	}

	if( pCourse->m_iStages >= MAX_COURSE_CONTENTS )
		m_textContents[MAX_COURSE_CONTENTS-1].SetText( ssprintf("%d more...", pCourse->m_iStages-(MAX_COURSE_CONTENTS-1)) );
}
