#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ScreenGameplay.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"




const float HEADER_X	=		CENTER_X;
const float HEADER_Y	=		50;
const float HELP_X		=		CENTER_X;
const float HELP_Y		=		SCREEN_HEIGHT-35;
const float ITEM_GAP_X	=		12;


const ScreenMessage SM_PlaySample			= ScreenMessage(SM_User-4);
const ScreenMessage SM_GoToPrevState		= ScreenMessage(SM_User-5);
const ScreenMessage SM_GoToNextState		= ScreenMessage(SM_User-6);
const ScreenMessage SM_JustPressedNext		= ScreenMessage(SM_User-7);


ScreenOptions::ScreenOptions( CString sBackgroundPath, CString sTopEdgePath )
{
	LOG->WriteLine( "ScreenOptions::ScreenOptions()" );

	m_SoundChangeCol.Load( THEME->GetPathTo(SOUND_OPTION_CHANGE_COL) );
	m_SoundChangeRow.Load( THEME->GetPathTo(SOUND_OPTION_CHANGE_ROW) );
	m_SoundNext.Load( THEME->GetPathTo(SOUND_MENU_START) );

	m_Menu.Load(
		sBackgroundPath, 
		sTopEdgePath, 
		ssprintf("%s %s to change line   %s %s to select between options      then press NEXT", CString(char(3)), CString(char(4)), CString(char(1)), CString(char(2)) )
		);
	this->AddActor( &m_Menu );
	m_Menu.TweenOnScreenFromBlack( SM_None );

	// init row numbers and element colors
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iCurrentRow[p] = 0;
		m_SelectionHighlight[p].SetDiffuseColor( PlayerToColor((PlayerNumber)p) );
	
		this->AddActor( &m_SelectionHighlight[p] );

		for( int l=0; l<MAX_OPTION_LINES; l++ )
		{
			m_iSelectedOption[p][l] = 0;
	
			m_OptionUnderline[p][l].SetDiffuseColor( PlayerToColor(p) );
			this->AddActor( &m_OptionUnderline[p][l] );
		}
	}

	// add sub actors
	for( int i=0; i<MAX_OPTION_LINES; i++ )		// foreach line
	{
		this->AddActor( &m_textOptionLineTitles[i] );

		for( int j=0; j<MAX_OPTIONS_PER_LINE; j++ )
		{
			m_textOptions[i][j].SetZ( -1 );
			this->AddActor( &m_textOptions[i][j] );	// this array has to be big enough to hold all of the options
		}
	}


	m_Wipe.OpenWipingRight(SM_None);
	this->AddActor( &m_Wipe );
}

void ScreenOptions::Init( InputMode im, OptionLineData optionLineData[], int iNumOptionLines )
{
	LOG->WriteLine( "ScreenOptions::Set()" );


	m_InputMode = im;
	m_OptionLineData = optionLineData;
	m_iNumOptionLines = iNumOptionLines;


	this->ImportOptions();
	InitOptionsText();
	PositionUnderlines();
	PositionHighlights();
}

ScreenOptions::~ScreenOptions()
{
	LOG->WriteLine( "ScreenOptions::~ScreenOptions()" );

}


const int UNDERLINE_THICKNESS = 3;
const int HIGHLIGHT_HEIGHT = 26;


void ScreenOptions::GetWidthXY( PlayerNumber p, int iRow, float &fWidthOut, float &fXOut, float &fYOut )
{
	int iOptionInRow = m_iSelectedOption[p][iRow];
	BitmapText &option = m_textOptions[iRow][iOptionInRow];

	fWidthOut = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX() + UNDERLINE_THICKNESS*2;
	fXOut = option.GetX();
	fYOut = option.GetY();

	// offset so the underlines for each player don't overlap
	fXOut += p - (NUM_PLAYERS-1)/2.0f * UNDERLINE_THICKNESS * 2;
	fYOut += p - (NUM_PLAYERS-1)/2.0f * UNDERLINE_THICKNESS * 2;
}

void ScreenOptions::InitOptionsText()
{
	// init m_textOptions from optionLines
	for( int i=0; i<m_iNumOptionLines; i++ )	// foreach options line
	{
		OptionLineData &optline = m_OptionLineData[i];

		float fY = 60.0f + 40*i;

		BitmapText &title = m_textOptionLineTitles[i];

		title.Load( THEME->GetPathTo(FONT_HEADER2) );
		title.SetText( optline.szTitle );
		title.SetXY( 80, fY );
		title.SetZoom( 0.7f );
		title.SetVertAlign( Actor::align_middle );		
		this->AddActor( &title );

		// init all text in this line and count the width of the line
		float fX = 150;	// indent 70 pixels
		for( int j=0; j<optline.iNumOptions; j++ )	// for each option on this line
		{
			BitmapText &option = m_textOptions[i][j];

			option.Load( THEME->GetPathTo(FONT_NORMAL) );
			option.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			option.SetText( optline.szOptionsText[j] );
			option.SetZoom( 0.65f );
			option.SetShadowLength( 2 );
			this->AddActor( &option );

			// set the XY position of each item in the line
			float fItemWidth = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX();
			fX += fItemWidth/2;
			option.SetXY( fX, fY );
			fX += fItemWidth/2 + ITEM_GAP_X;
		}
	}

	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( p > PLAYER_1  &&  m_InputMode != INPUTMODE_2PLAYERS )
			continue;	// skip

		Quad &highlight = m_SelectionHighlight[p];
		this->AddActor( &highlight );

		for( int i=0; i<m_iNumOptionLines; i++ )	// foreach options line
		{
			Quad &underline = m_OptionUnderline[p][i];
			this->AddActor( &underline );
		}
	}
}

void ScreenOptions::PositionUnderlines()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( p > PLAYER_1  &&  m_InputMode != INPUTMODE_2PLAYERS )
			continue;	// skip

		for( int i=0; i<m_iNumOptionLines; i++ )	// foreach options line
		{
			Quad &underline = m_OptionUnderline[p][i];

			float fWidth, fX, fY;
			GetWidthXY( (PlayerNumber)p, i, fWidth, fX, fY );
			fY += HIGHLIGHT_HEIGHT/2;
			float fHeight = UNDERLINE_THICKNESS;

			underline.SetZoomX( fWidth );
			underline.SetZoomY( fHeight );
			underline.SetXY( fX, fY );
		}
	}
}


void ScreenOptions::PositionHighlights()
{
	// Set the position of the highlight showing the current option the user is changing.
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( p > PLAYER_1  &&  m_InputMode != INPUTMODE_2PLAYERS )
			continue;	// skip

		int i=m_iCurrentRow[p];

		Quad &highlight = m_SelectionHighlight[p];

		float fWidth, fX, fY;
		GetWidthXY( (PlayerNumber)p, i, fWidth, fX, fY );
		float fHeight = HIGHLIGHT_HEIGHT;

		highlight.SetZoomX( fWidth );
		highlight.SetZoomY( fHeight );
		highlight.SetXY( fX, fY );
	}
}

void ScreenOptions::TweenHighlight( PlayerNumber player_no )
{
	// Set the position of the highlight showing the current option the user is changing.
	int iCurRow = m_iCurrentRow[player_no];

	Quad &highlight = m_SelectionHighlight[player_no];

	float fWidth, fX, fY;
	GetWidthXY( player_no, iCurRow, fWidth, fX, fY );
	float fHeight = HIGHLIGHT_HEIGHT;

	highlight.BeginTweening( 0.2f );
	highlight.SetTweenZoomX( fWidth );
	highlight.SetTweenZoomY( fHeight );
	highlight.SetTweenXY( fX, fY );
}

void ScreenOptions::Update( float fDeltaTime )
{
	//LOG->WriteLine( "ScreenOptions::Update(%f)", fDeltaTime );

	Screen::Update( fDeltaTime );
}

void ScreenOptions::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Wipe.IsClosing() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevState:
		this->ExportOptions();
		this->GoToPrevState();
		break;
	case SM_GoToNextState:
		this->ExportOptions();
		this->GoToNextState();
		break;
	}
}


void ScreenOptions::OnChange()
{
	PositionUnderlines();
}


void ScreenOptions::MenuBack( const PlayerNumber p )
{
	Screen::MenuBack( p );

	m_Menu.TweenOffScreenToBlack( SM_None, true );

	m_Wipe.CloseWipingLeft( SM_GoToPrevState );
}


void ScreenOptions::MenuStart( const PlayerNumber p )
{
	Screen::MenuStart( p );

	m_Menu.TweenOffScreenToBlack( SM_None, false );

	m_SoundNext.PlayRandom();
	m_Wipe.CloseWipingRight( SM_GoToNextState );
}

void ScreenOptions::MenuLeft( const PlayerNumber p ) 
{
	PlayerNumber pn = p;

	switch( m_InputMode )
	{
	case INPUTMODE_P1_ONLY:
		if( pn != PLAYER_1 )
			return;
	case INPUTMODE_BOTH:
		pn = PLAYER_1;
		break;
	case INPUTMODE_2PLAYERS:
		break;	// fall through
	}

	int iCurRow = m_iCurrentRow[pn];
	if( m_iSelectedOption[pn][iCurRow] == 0 )	// can't go left any more
		return;

	m_SoundChangeCol.PlayRandom();
	m_iSelectedOption[pn][iCurRow]--;
	
	TweenHighlight( pn );
	OnChange();
}

void ScreenOptions::MenuRight( const PlayerNumber p ) 
{
	PlayerNumber pn = p;

	switch( m_InputMode )
	{
	case INPUTMODE_P1_ONLY:
		if( pn != PLAYER_1 )
			return;
	case INPUTMODE_BOTH:
		pn = PLAYER_1;
		break;
	case INPUTMODE_2PLAYERS:
		break;	// fall through
	}

	int iCurRow = m_iCurrentRow[pn];
	if( m_iSelectedOption[pn][iCurRow] == m_OptionLineData[iCurRow].iNumOptions-1 )	// can't go right any more
		return;
	
	m_SoundChangeCol.PlayRandom();
	m_iSelectedOption[pn][iCurRow]++;
	
	TweenHighlight( pn );
	OnChange();
}

void ScreenOptions::MenuUp( const PlayerNumber p ) 
{
	PlayerNumber pn = p;

	switch( m_InputMode )
	{
	case INPUTMODE_P1_ONLY:
		return;
	case INPUTMODE_BOTH:
		pn = PLAYER_1;
		break;
	case INPUTMODE_2PLAYERS:
		break;	// fall through
	}

	m_SoundChangeRow.PlayRandom();
	if( m_iCurrentRow[pn] == 0 )	
		m_iCurrentRow[pn] = m_iNumOptionLines-1; // wrap around
	else
		m_iCurrentRow[pn]--;
	
	TweenHighlight( pn );
	OnChange();
}

void ScreenOptions::MenuDown( const PlayerNumber p ) 
{
	PlayerNumber pn = p;

	switch( m_InputMode )
	{
	case INPUTMODE_P1_ONLY:
		return;
	case INPUTMODE_BOTH:
		pn = PLAYER_1;
		break;
	case INPUTMODE_2PLAYERS:
		break;	// fall through
	}

	m_SoundChangeRow.PlayRandom();
	if( m_iCurrentRow[pn] == m_iNumOptionLines-1 )	
		m_iCurrentRow[pn] = 0; // wrap around
	else
		m_iCurrentRow[pn]++;

	TweenHighlight( pn );
	OnChange();
}


