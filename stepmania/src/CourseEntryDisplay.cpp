#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CourseEntryDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseEntryDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include <math.h>
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "StyleDef.h"
#include "ActorUtil.h"


void CourseEntryDisplay::Load()
{
	m_sprFrame.SetName( "Bar" );
	m_sprFrame.Load( THEME->GetPathToG("CourseEntryDisplay bar") );
	SET_XY_AND_ON_COMMAND( &m_sprFrame );
	this->AddChild( &m_sprFrame );

	this->m_size.x = (float) m_sprFrame.GetTexture()->GetSourceFrameWidth();
	this->m_size.y = (float) m_sprFrame.GetTexture()->GetSourceFrameHeight();

	m_textNumber.SetName( "Number" );
	m_textNumber.LoadFromFont( THEME->GetPathToF("CourseEntryDisplay number") );
	SET_XY_AND_ON_COMMAND( &m_textNumber );
	this->AddChild( &m_textNumber );

	m_TextBanner.SetName( "TextBanner" );
	SET_XY_AND_ON_COMMAND( &m_TextBanner );
	this->AddChild( &m_TextBanner );

	m_textFoot.SetName( "Foot" );
	m_textFoot.LoadFromTextureAndChars( THEME->GetPathToG("CourseEntryDisplay difficulty 2x1"),"10" );
	SET_XY_AND_ON_COMMAND( &m_textFoot );
	this->AddChild( &m_textFoot );

	m_textDifficultyNumber.SetName( "Difficulty" );
	m_textDifficultyNumber.LoadFromFont( THEME->GetPathToF("Common normal") );
	SET_XY_AND_ON_COMMAND( &m_textDifficultyNumber );
	this->AddChild( &m_textDifficultyNumber );

	m_textModifiers.SetName( "Modifiers" );
	m_textModifiers.LoadFromFont( THEME->GetPathToF("Common normal") );
	SET_XY_AND_ON_COMMAND( &m_textModifiers );
	this->AddChild( &m_textModifiers );
}


void CourseEntryDisplay::LoadFromSongAndNotes( int iNum, Song* pSong, Steps* pNotes, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = SONGMAN->GetSongColor( pSong );
	RageColor colorNotes = SONGMAN->GetDifficultyColor( pNotes->GetDifficulty() );

	m_TextBanner.LoadFromSong( pSong );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf("%d", pNotes->GetMeter()) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

void CourseEntryDisplay::LoadFromDifficulty( int iNum, Difficulty dc, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = SONGMAN->GetDifficultyColor( dc );

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( "?" );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

void CourseEntryDisplay::LoadFromMeterRange( int iNum, int iLow, int iHigh, CString sModifiers )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = RageColor(1,1,1,1);

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf(iLow==iHigh?"%d":"%d-%d", iLow, iHigh) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );

	m_textModifiers.SetText( sModifiers );
}

