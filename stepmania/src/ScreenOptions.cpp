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
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"


#define ICONS_X( p )		THEME->GetMetricF("ScreenOptions",ssprintf("IconsP%dX",p+1))
#define ARROWS_X			THEME->GetMetricF("ScreenOptions","ArrowsX")
#define LABELS_X			THEME->GetMetricF("ScreenOptions","LabelsX")
#define LABELS_ZOOM			THEME->GetMetricF("ScreenOptions","LabelsZoom")
#define LABELS_H_ALIGN		THEME->GetMetricI("ScreenOptions","LabelsHAlign")
#define ITEMS_ZOOM			THEME->GetMetricF("ScreenOptions","ItemsZoom")
#define ITEMS_START_X		THEME->GetMetricF("ScreenOptions","ItemsStartX")
#define ITEMS_GAP_X			THEME->GetMetricF("ScreenOptions","ItemsGapX")
#define ITEMS_START_Y		THEME->GetMetricF("ScreenOptions","ItemsStartY")
#define ITEMS_SPACING_Y		THEME->GetMetricF("ScreenOptions","ItemsSpacingY")
#define EXPLANATION_X		THEME->GetMetricF("ScreenOptions","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("ScreenOptions","ExplanationY")
#define EXPLANATION_ZOOM	THEME->GetMetricF("ScreenOptions","ExplanationZoom")
#define HELP_TEXT			THEME->GetMetric("ScreenOptions","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenOptions","TimerSeconds")
#define COLOR_SELECTED		THEME->GetMetricC("ScreenOptions","ColorSelected")
#define COLOR_NOT_SELECTED	THEME->GetMetricC("ScreenOptions","ColorNotSelected")


const ScreenMessage SM_PlaySample			= ScreenMessage(SM_User-4);
const ScreenMessage SM_TweenOffScreen		= ScreenMessage(SM_User-7);


ScreenOptions::ScreenOptions( CString sBackgroundPath, CString sPagePath, CString sTopEdgePath )
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_SoundChangeCol.Load( THEME->GetPathTo("Sounds","option change col") );
	m_SoundNextRow.Load( THEME->GetPathTo("Sounds","option next row") );
	m_SoundPrevRow.Load( THEME->GetPathTo("Sounds","option prev row") );
	m_SoundStart.Load( THEME->GetPathTo("Sounds","menu start") );

	m_Menu.Load(
		sBackgroundPath, 
		sTopEdgePath, 
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	// add everything to m_framePage so we can animate everything at once
	this->AddChild( &m_framePage );

	m_sprPage.Load( sPagePath );
	m_sprPage.SetXY( CENTER_X, CENTER_Y );
	m_framePage.AddChild( &m_sprPage );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iCurrentRow[p] = 0;

		for( unsigned l=0; l<MAX_OPTION_LINES; l++ )
			m_iSelectedOption[p][l] = 0;
	}

	m_Menu.TweenOnScreenFromBlack( SM_None );
	m_Wipe.OpenWipingRight(SM_None);
	this->AddChild( &m_Wipe );

	m_framePage.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_framePage.BeginTweening( 0.3f, Actor::TWEEN_BIAS_BEGIN );
	m_framePage.SetTweenX( 0 );
	ZeroMemory(&m_OptionDim, sizeof(m_OptionDim));
}

void ScreenOptions::Init( InputMode im, OptionRowData OptionRowData[], int iNumOptionLines, bool bUseIcons )
{
	LOG->Trace( "ScreenOptions::Set()" );


	m_InputMode = im;
	m_OptionRowData = OptionRowData;
	m_iNumOptionRows = iNumOptionLines;
	m_bUseIcons = bUseIcons;


	this->ImportOptions();
	if( m_InputMode == INPUTMODE_BOTH )
	{
		for( unsigned l=0; l<MAX_OPTION_LINES; l++ )
			m_iSelectedOption[PLAYER_2][l] = m_iSelectedOption[PLAYER_1][l];
	}

	// init highlights and underlines
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		for( int l=0; l<m_iNumOptionRows; l++ )
		{	
			m_Underline[p][l].Load( (PlayerNumber)p, true );
			m_framePage.AddChild( &m_Underline[p][l] );

			m_OptionIcons[p][l].Load( (PlayerNumber)p, "", false );
			m_framePage.AddChild( &m_OptionIcons[p][l] );
		}

		m_Highlight[p].Load( (PlayerNumber)p, false );
		m_framePage.AddChild( &m_Highlight[p] );

	}

	// init text
	int i;
	for( i=0; i<m_iNumOptionRows; i++ )		// foreach line
	{
		m_framePage.AddChild( &m_sprLineArrows[i] );

		m_framePage.AddChild( &m_textOptionLineTitles[i] );		

		for( unsigned j=0; j<m_OptionRowData[i].iNumOptions; j++ )
			m_framePage.AddChild( &m_textOptions[i][j] );	// this array has to be big enough to hold all of the options
	}

	// TRICKY:  Add one more item.  This will be "EXIT"
	m_framePage.AddChild( &m_textOptions[i][0] );

	// add explanation here so it appears on top
	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","option explanation") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetZoom( EXPLANATION_ZOOM );
	m_textExplanation.SetShadowLength( 2 );
	m_framePage.AddChild( &m_textExplanation );


	InitOptionsText();
	PositionUnderlines();
	PositionIcons();
	RefreshIcons();
	PositionCursors();
	UpdateEnabledDisabled();
	OnChange();
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
}


void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	int iOptionInRow = m_iSelectedOption[pn][iRow];
	BitmapText &option = m_textOptions[iRow][iOptionInRow];

	iWidthOut = int(roundf( option.GetWidestLineWidthInSourcePixels() * option.GetZoomX() ));
	iXOut = int(roundf( option.GetX() ));
	iYOut = int(roundf( option.GetY() ));
}

void ScreenOptions::InitOptionsText()
{
	const float fLineGap = ITEMS_SPACING_Y - max(0, (m_iNumOptionRows-10)*2);

	// init m_textOptions from optionLines
	int i;
	for( i=0; i<m_iNumOptionRows; i++ )	// foreach options line
	{
		OptionRowData &optline = m_OptionRowData[i];

		float fY = ITEMS_START_Y + fLineGap*i;

		BitmapText &title = m_textOptionLineTitles[i];

		title.LoadFromFont( THEME->GetPathTo("Fonts","option title") );
		CString sText = optline.szTitle;

		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( LABELS_ZOOM );
		title.SetHorizAlign( (Actor::HorizAlign)LABELS_H_ALIGN );
		title.SetVertAlign( Actor::align_middle );		
		title.TurnShadowOff();		


		Sprite &arrow = m_sprLineArrows[i];
		arrow.Load( THEME->GetPathTo("Graphics","options arrow") );
		arrow.SetXY( ARROWS_X, fY );


		// init all text in this line and count the width of the line
		float fX = ITEMS_START_X;	// indent 70 pixels
		for( unsigned j=0; j<optline.iNumOptions; j++ )	// for each option on this line
		{
			BitmapText &option = m_textOptions[i][j];

			option.LoadFromFont( THEME->GetPathTo("Fonts","option item") );
			option.SetText( optline.szOptionsText[j] );
			option.SetZoom( ITEMS_ZOOM );
			option.SetShadowLength( 2 );

			// set the XY position of each item in the line
			float fItemWidth = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX();
			fX += fItemWidth/2;
			option.SetXY( fX, fY );
			fX += fItemWidth/2 + ITEMS_GAP_X;
		}
	}

	BitmapText &option = m_textOptions[i][0];
	option.LoadFromFont( THEME->GetPathTo("Fonts","option item") );
	option.SetText( "EXIT" );
	option.SetZoom( ITEMS_ZOOM );
	option.SetShadowLength( 2 );
	float fY = ITEMS_START_Y + fLineGap*i;
	option.SetXY( CENTER_X, fY );
}

void ScreenOptions::DimOption(int line, int option, bool dim)
{
	if(m_OptionDim[line][option] == dim)
		return;

	m_OptionDim[line][option] = dim;
	m_textOptions[line][option].StopTweening();
	m_textOptions[line][option].BeginTweening(.250);
	if(m_OptionDim[line][option])
		m_textOptions[line][option].SetTweenDiffuse( RageColor(.5,.5,.5,1) );
	else
		m_textOptions[line][option].SetTweenDiffuse( RageColor(1,1,1,1) );

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
	for(unsigned i = 0; i < m_OptionRowData[line].iNumOptions; ++i)
		if(!m_OptionDim[line][i]) return false;
	return true;
}

void ScreenOptions::PositionUnderlines()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
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


void ScreenOptions::PositionIcons()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
		{
			OptionIcon &icon = m_OptionIcons[p][i];

			int iWidth, iX, iY;
			GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
			icon.SetXY( ICONS_X(p), (float)iY );
		}
	}
}

void ScreenOptions::RefreshIcons()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
		{
			OptionIcon &icon = m_OptionIcons[p][i];

			int iSelection = m_iSelectedOption[p][i];
			CString sSelection = m_OptionRowData[i].szOptionsText[iSelection];
			if( sSelection == "ON" )
				sSelection = m_OptionRowData[i].szTitle;
			icon.Load( (PlayerNumber)p, m_bUseIcons ? sSelection : "", false );
		}
	}
}


void ScreenOptions::PositionCursors()
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

void ScreenOptions::TweenCursor( PlayerNumber player_no )
{
	// Set the position of the highlight showing the current option the user is changing.
	int iCurRow = m_iCurrentRow[player_no];

	OptionsCursor &highlight = m_Highlight[player_no];

	int iWidth, iX, iY;
	GetWidthXY( player_no, iCurRow, iWidth, iX, iY );
	highlight.StopTweening();
	highlight.BeginTweening( 0.2f );
	highlight.TweenBarWidth( iWidth );
	highlight.SetTweenXY( (float)iX, (float)iY );
}

void ScreenOptions::UpdateEnabledDisabled()
{
	RageColor colorSelected = COLOR_SELECTED;
	RageColor colorNotSelected = COLOR_NOT_SELECTED;

	// init text
	int i;
	for( i=0; i<m_iNumOptionRows; i++ )		// foreach line
	{
		bool bThisRowIsSelected = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p)  &&  m_iCurrentRow[p] == i )
				bThisRowIsSelected = true;

		m_sprLineArrows[i].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );

		m_textOptionLineTitles[i].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );

		for( unsigned j=0; j<m_OptionRowData[i].iNumOptions; j++ )
			m_textOptions[i][j].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );
	}

	bool bThisRowIsSelectedByBoth = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p)  &&  m_iCurrentRow[p] != i )
			bThisRowIsSelectedByBoth = false;
		m_textOptions[i][0].SetDiffuse( bThisRowIsSelectedByBoth ? colorNotSelected : colorSelected );
	if( bThisRowIsSelectedByBoth )
		m_textOptions[i][0].SetEffectCamelion( 2.5, colorSelected, colorNotSelected );
	else
		m_textOptions[i][0].SetEffectNone();
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
		this->SendScreenMessage( SM_TweenOffScreen, 0 );
		break;
	case SM_GoToPrevScreen:
		this->ExportOptions();
		this->GoToPrevState();
		break;
	case SM_GoToNextScreen:
		this->ExportOptions();
		this->GoToNextState();
		break;
	case SM_TweenOffScreen:
		m_Menu.TweenOffScreenToBlack( SM_None, false );

		m_SoundStart.Play();
		m_Wipe.CloseWipingRight( SM_GoToNextScreen );

		m_framePage.BeginTweening( 0.3f, Actor::TWEEN_BIAS_END );
		m_framePage.SetTweenX( SCREEN_RIGHT );
		break;
	}
}


void ScreenOptions::OnChange()
{
	PositionUnderlines();
	RefreshIcons();
	UpdateEnabledDisabled();

	int iCurRow = m_iCurrentRow[PLAYER_1];
	if( iCurRow < m_iNumOptionRows )
		m_textExplanation.SetText( m_OptionRowData[iCurRow].szExplanation );
	else
		m_textExplanation.SetText( "" );
}


void ScreenOptions::MenuBack( PlayerNumber pn )
{
	Screen::MenuBack( pn );

	m_Menu.TweenOffScreenToBlack( SM_None, true );

	m_Wipe.CloseWipingLeft( SM_GoToPrevScreen );
}


void ScreenOptions::MenuStart( PlayerNumber pn )
{
	if( PREFSMAN->m_bArcadeOptionsNavigation )
	{
		bool bAllOnExit = true;
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p)  &&  m_iCurrentRow[p] != m_iNumOptionRows )
				bAllOnExit = false;

		if( m_iCurrentRow[pn] != m_iNumOptionRows )	// not on exit
			MenuDown( pn );	// can't go down any more
		else if( bAllOnExit )
			this->SendScreenMessage( SM_TweenOffScreen, 0 );
	}
	else	// !m_bArcadeOptionsNavigation
	{
		this->SendScreenMessage( SM_TweenOffScreen, 0 );
	}
}

void ScreenOptions::MenuLeft( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		int iCurRow = m_iCurrentRow[p];

		if( iCurRow == m_iNumOptionRows	)	// EXIT is selected
			return;		// don't allow a move

		const int iNumOptions = m_OptionRowData[iCurRow].iNumOptions;
		m_iSelectedOption[p][iCurRow] = (m_iSelectedOption[p][iCurRow]-1+iNumOptions) % iNumOptions;
// Chris:  I commented this out because it made wrapping a pain.  Is it used anyway?  If so, please
// let me know and I'll fix it.
//		do {
//			new_opt--;
//		} while(new_opt >= 0 && m_OptionDim[iCurRow][new_opt]);
//		
//		if( new_opt < 0 )	// can't go left any more
//			return;
//
//		m_iSelectedOption[p][iCurRow] = new_opt;
		
		TweenCursor( (PlayerNumber)p );
	}
	m_SoundChangeCol.Play();
	OnChange();
}

void ScreenOptions::MenuRight( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		int iCurRow = m_iCurrentRow[p];

		if( iCurRow == m_iNumOptionRows	)	// EXIT is selected
			return;		// don't allow a move

		const int iNumOptions = m_OptionRowData[iCurRow].iNumOptions;
		m_iSelectedOption[p][iCurRow] = (m_iSelectedOption[p][iCurRow]+1) % iNumOptions;
// Chris:  I commented this out because it made wrapping a pain.  Is it used anyway?  If so, please
// let me know and I'll fix it.
//		int new_opt = m_iSelectedOption[p][iCurRow];
//		do {
//			new_opt++;
//		} while(new_opt < m_OptionRowData[iCurRow].iNumOptions && 
//			    m_OptionDim[iCurRow][new_opt]);
//		
//		if( new_opt == m_OptionRowData[iCurRow].iNumOptions )	// can't go right any more
//			return;
//		
//		m_iSelectedOption[p][iCurRow] = new_opt;
		
		TweenCursor( (PlayerNumber)p );
	}
	m_SoundChangeCol.Play();
	OnChange();
}

void ScreenOptions::MenuUp( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		if( m_iCurrentRow[p] == 0 )	// on first row
			return;	// can't go up any more

		m_iCurrentRow[p]--;


//	Chris:  Will add back in later
//		/* Find the prev row with any un-dimmed entries. */
//		int new_row = m_iCurrentRow[p];
//		do {
///			if(--new_row < 0)
//				new_row = m_iNumOptionRows-1; // wrap around
//			if(!RowCompletelyDimmed(new_row)) break;
//		} while(new_row != m_iCurrentRow[p]);
//		m_iCurrentRow[p] = new_row;

		TweenCursor( (PlayerNumber)p );
	}
	m_SoundPrevRow.Play();
	OnChange();
}


void ScreenOptions::MenuDown( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_PLAYERS  &&  p != pn )
			continue;	// skip

		if( m_iCurrentRow[p] == m_iNumOptionRows )	// on exit
			return;	// can't go down any more

		m_iCurrentRow[p]++;

// Chris:  Commented this out, but will add back in later.
//		/* Find the next row with any un-dimmed entries. */
//		int new_row = m_iCurrentRow[p];
//		do {
//			if( ++new_row == m_iNumOptionRows )
//				new_row = 0; // wrap around
//			if(!RowCompletelyDimmed(new_row)) break;
//		} while(new_row != m_iCurrentRow[p]);
//		m_iCurrentRow[p] = new_row;

		TweenCursor( (PlayerNumber)p );
	}
	m_SoundNextRow.Play();
	OnChange();
}


