#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"


#define HELP_TEXT			THEME->GetMetric("ScreenOptions","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenOptions","TimerSeconds")

const float HEADER_X		= CENTER_X;
const float HEADER_Y		= 50;
const float HELP_X			= CENTER_X;
const float HELP_Y			= SCREEN_HEIGHT-35;
const float ITEM_GAP_X		= 14;
const float LABELS_X		= 80;
const float LINE_START_Y	= 80;
const float LINE_GAP_Y		= 34;
const float ITEMS_START_X	= 160;

const ScreenMessage SM_PlaySample			= ScreenMessage(SM_User-4);
const ScreenMessage SM_GoToPrevScreen		= ScreenMessage(SM_User-5);
const ScreenMessage SM_GoToNextScreen		= ScreenMessage(SM_User-6);


ScreenOptions::ScreenOptions( CString sBackgroundPath, CString sPagePath, CString sTopEdgePath )
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_SoundChangeCol.Load( THEME->GetPathTo("Sounds","option change col") );
	m_SoundChangeRow.Load( THEME->GetPathTo("Sounds","option change row") );
	m_SoundNext.Load( THEME->GetPathTo("Sounds","menu start") );

	m_Menu.Load(
		sBackgroundPath, 
		sTopEdgePath, 
		HELP_TEXT, true, TIMER_SECONDS
		);
	this->AddSubActor( &m_Menu );

	// add everything to m_framePage so we can animate everything at once
	this->AddSubActor( &m_framePage );

	m_sprPage.Load( sPagePath );
	m_sprPage.SetXY( CENTER_X, CENTER_Y );
	m_framePage.AddSubActor( &m_sprPage );


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iCurrentRow[p] = 0;

		for( int l=0; l<MAX_OPTION_LINES; l++ )
			m_iSelectedOption[p][l] = 0;
	}

	m_Menu.TweenOnScreenFromBlack( SM_None );
	m_Wipe.OpenWipingRight(SM_None);
	this->AddSubActor( &m_Wipe );

	m_framePage.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_framePage.BeginTweening( 0.3f, Actor::TWEEN_BIAS_BEGIN );
	m_framePage.SetTweenX( 0 );
	ZeroMemory(&m_OptionDim, sizeof(m_OptionDim));
}

void ScreenOptions::Init( InputMode im, OptionLineData optionLineData[], int iNumOptionLines )
{
	LOG->Trace( "ScreenOptions::Set()" );


	m_InputMode = im;
	m_OptionLineData = optionLineData;
	m_iNumOptionLines = iNumOptionLines;


	this->ImportOptions();
	if( m_InputMode == INPUTMODE_BOTH )
	{
		for( int l=0; l<MAX_OPTION_LINES; l++ )
			m_iSelectedOption[PLAYER_2][l] = m_iSelectedOption[PLAYER_1][l];
	}

	// init highlights and underlines
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		for( int l=0; l<m_iNumOptionLines; l++ )
		{	
			m_Underline[p][l].Load( (PlayerNumber)p, true );
			m_framePage.AddSubActor( &m_Underline[p][l] );
		}

		m_Highlight[p].Load( (PlayerNumber)p, false );
		m_framePage.AddSubActor( &m_Highlight[p] );

	}

	// init text
	for( int i=0; i<m_iNumOptionLines; i++ )		// foreach line
	{
		m_framePage.AddSubActor( &m_textOptionLineTitles[i] );

		for( int j=0; j<m_OptionLineData[i].iNumOptions; j++ )
		{
			m_textOptions[i][j].SetZ( -1 );
			m_framePage.AddSubActor( &m_textOptions[i][j] );	// this array has to be big enough to hold all of the options
		}
	}



	InitOptionsText();
	PositionUnderlines();
	PositionHighlights();
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );

}


void ScreenOptions::GetWidthXY( PlayerNumber p, int iRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	int iOptionInRow = m_iSelectedOption[p][iRow];
	BitmapText &option = m_textOptions[iRow][iOptionInRow];

	iWidthOut = roundf( option.GetWidestLineWidthInSourcePixels() * option.GetZoomX() );
	iXOut = roundf( option.GetX() );
	iYOut = roundf( option.GetY() );
}

void ScreenOptions::InitOptionsText()
{
	const float fLineGap = LINE_GAP_Y - max(0, (m_iNumOptionLines-10)*2);

	// init m_textOptions from optionLines
	for( int i=0; i<m_iNumOptionLines; i++ )	// foreach options line
	{
		OptionLineData &optline = m_OptionLineData[i];

		float fY = LINE_START_Y + fLineGap*i;

		BitmapText &title = m_textOptionLineTitles[i];

		title.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
		title.SetText( optline.szTitle );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( 0.7f );
		title.SetVertAlign( Actor::align_middle );		
		title.TurnShadowOff();		
		m_framePage.AddSubActor( &title );

		// init all text in this line and count the width of the line
		float fX = ITEMS_START_X;	// indent 70 pixels
		for( int j=0; j<optline.iNumOptions; j++ )	// for each option on this line
		{
			BitmapText &option = m_textOptions[i][j];

			option.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
			option.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
			option.SetText( optline.szOptionsText[j] );
			option.SetZoom( 0.5f );
			option.SetShadowLength( 2 );
			m_framePage.AddSubActor( &option );

			// set the XY position of each item in the line
			float fItemWidth = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX();
			fX += fItemWidth/2;
			option.SetXY( fX, fY );
			fX += fItemWidth/2 + ITEM_GAP_X;
		}
	}
}

void ScreenOptions::DimOption(int line, int option, bool dim)
{
	m_OptionDim[line][option] = dim;
	m_textOptions[line][option].BeginTweening(.250);
	if(m_OptionDim[line][option])
		m_textOptions[line][option].SetTweenDiffuseColor( D3DXCOLOR(.5,.5,.5,1) );
	else
		m_textOptions[line][option].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );

	/* Don't know if I like this ...-glenn
	m_textOptionLineTitles[line].BeginTweening(.250);
	if(RowCompletelyDimmed(line))
		m_textOptionLineTitles[line].SetTweenZoom( 0.6f );
	else
		m_textOptionLineTitles[line].SetTweenZoom( 0.7f );
	*/

}

bool ScreenOptions::RowCompletelyDimmed(int line) const
{
	for(int i = 0; i < m_OptionLineData[line].iNumOptions; ++i)
		if(!m_OptionDim[line][i]) return false;
	return true;
}

void ScreenOptions::PositionUnderlines()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionLines; i++ )	// foreach options line
		{
//			Quad &underline = m_OptionUnderline[p][i];
			OptionsCursor &underline = m_Underline[p][i];

			int iWidth, iX, iY;
			GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
			underline.SetBarWidth( iWidth );
			underline.SetXY( (float)iX, (float)iY );
		}
	}
}


void ScreenOptions::PositionHighlights()
{
	// Set the position of the highlight showing the current option the user is changing.
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		int i=m_iCurrentRow[p];

		OptionsCursor &highlight = m_Highlight[p];

		int iWidth, iX, iY;
		GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
		highlight.SetBarWidth( iWidth );
		highlight.SetXY( (float)iX, (float)iY );
	}
}

void ScreenOptions::TweenHighlight( PlayerNumber player_no )
{
	// Set the position of the highlight showing the current option the user is changing.
	int iCurRow = m_iCurrentRow[player_no];

	OptionsCursor &highlight = m_Highlight[player_no];

	int iWidth, iX, iY;
	GetWidthXY( player_no, iCurRow, iWidth, iX, iY );
	highlight.BeginTweening( 0.2f );
	highlight.TweenBarWidth( iWidth );
	highlight.SetTweenXY( (float)iX, (float)iY );
}

void ScreenOptions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenOptions::Update(%f)", fDeltaTime );

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
	case SM_MenuTimer:
		this->MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		this->ExportOptions();
		this->GoToPrevState();
		break;
	case SM_GoToNextScreen:
		this->ExportOptions();
		this->GoToNextState();
		break;
	}
}


void ScreenOptions::OnChange()
{
	PositionUnderlines();
}


void ScreenOptions::MenuBack( PlayerNumber p )
{
	Screen::MenuBack( p );

	m_Menu.TweenOffScreenToBlack( SM_None, true );

	m_Wipe.CloseWipingLeft( SM_GoToPrevScreen );
}


void ScreenOptions::MenuStart( PlayerNumber p )
{
	Screen::MenuStart( p );

	m_Menu.TweenOffScreenToBlack( SM_None, false );

	m_SoundNext.PlayRandom();
	m_Wipe.CloseWipingRight( SM_GoToNextScreen );

	m_framePage.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );
	m_framePage.SetTweenX( SCREEN_RIGHT );
}

void ScreenOptions::MenuLeft( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		int iCurRow = m_iCurrentRow[p];

		int new_opt = m_iSelectedOption[p][iCurRow];
		do {
			new_opt--;
		} while(new_opt >= 0 && m_OptionDim[iCurRow][new_opt]);
		
		if( new_opt < 0 )	// can't go left any more
			return;

		m_iSelectedOption[p][iCurRow] = new_opt;
		
		TweenHighlight( (PlayerNumber)p );
	}
	m_SoundChangeCol.PlayRandom();
	OnChange();
}

void ScreenOptions::MenuRight( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		int iCurRow = m_iCurrentRow[p];
		int new_opt = m_iSelectedOption[p][iCurRow];
		do {
			new_opt++;
		} while(new_opt < m_OptionLineData[iCurRow].iNumOptions && 
			    m_OptionDim[iCurRow][new_opt]);
		
		if( new_opt == m_OptionLineData[iCurRow].iNumOptions )	// can't go right any more
			return;
		
		m_iSelectedOption[p][iCurRow] = new_opt;
		
		TweenHighlight( (PlayerNumber)p );
	}
	m_SoundChangeCol.PlayRandom();
	OnChange();
}

void ScreenOptions::MenuUp( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		/* Find the prev row with any un-dimmed entries. */
		int new_row = m_iCurrentRow[p];
		do {
			if(--new_row < 0)
				new_row = m_iNumOptionLines-1; // wrap around
			if(!RowCompletelyDimmed(new_row)) break;
		} while(new_row != m_iCurrentRow[p]);
		m_iCurrentRow[p] = new_row;

		TweenHighlight( (PlayerNumber)p );
	}
	m_SoundChangeRow.PlayRandom();
	OnChange();
}


void ScreenOptions::MenuDown( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		/* Find the next row with any un-dimmed entries. */
		int new_row = m_iCurrentRow[p];
		do {
			if( ++new_row == m_iNumOptionLines )
				new_row = 0; // wrap around
			if(!RowCompletelyDimmed(new_row)) break;
		} while(new_row != m_iCurrentRow[p]);
		m_iCurrentRow[p] = new_row;

		TweenHighlight( (PlayerNumber)p );
	}
	m_SoundChangeRow.PlayRandom();
	OnChange();
}


