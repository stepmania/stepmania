#include "global.h"
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

const int MAX_ITEMS_PER_ROW = 8;
const float ITEM_X[NUM_PLAYERS] = { 260, 420 };

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


ScreenOptions::ScreenOptions( CString sClassName, bool bEnableTimer )
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_sClassName = sClassName;

	m_SoundChangeCol.Load( THEME->GetPathTo("Sounds","ScreenOptions change") );
	m_SoundNextRow.Load( THEME->GetPathTo("Sounds","ScreenOptions next") );
	m_SoundPrevRow.Load( THEME->GetPathTo("Sounds","ScreenOptions prev") );
	m_SoundStart.Load( THEME->GetPathTo("Sounds","Common start") );

	m_Menu.Load( sClassName, bEnableTimer, false );	// no style icon
	this->AddChild( &m_Menu );

	// add everything to m_framePage so we can animate everything at once
	this->AddChild( &m_framePage );

	m_sprPage.Load( THEME->GetPathTo("Graphics",sClassName+" page") );
	m_sprPage.SetXY( CENTER_X, CENTER_Y );
	m_framePage.AddChild( &m_sprPage );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_iCurrentRow[p] = 0;

		for( unsigned l=0; l<MAX_OPTION_LINES; l++ )
			m_iSelectedOption[p][l] = 0;
	}

	m_framePage.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_framePage.BeginTweening( 0.3f, Actor::TWEEN_DECELERATE );
	m_framePage.SetTweenX( 0 );
	memset(&m_OptionDim, 0, sizeof(m_OptionDim));
}

void ScreenOptions::Init( InputMode im, OptionRow OptionRow[], int iNumOptionLines, bool bUseIcons, bool bLoadExplanations )
{
	LOG->Trace( "ScreenOptions::Set()" );


	m_InputMode = im;
	m_OptionRow = OptionRow;
	m_iNumOptionRows = iNumOptionLines;
	m_bUseIcons = bUseIcons;
	m_bLoadExplanations = bLoadExplanations;


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
		m_framePage.AddChild( &m_sprBullets[i] );

		m_framePage.AddChild( &m_textTitles[i] );		

		for( unsigned j=0; j<m_OptionRow[i].choices.size(); j++ )
			m_framePage.AddChild( &m_textItems[i][j] );	// this array has to be big enough to hold all of the options
	}

	// TRICKY:  Add one more item.  This will be "EXIT"
	m_framePage.AddChild( &m_textItems[i][0] );

	// add explanation here so it appears on top
	m_textExplanation.LoadFromFont( THEME->GetPathTo("Fonts","ScreenOptions explanation") );
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
	bool bExitRow = iRow == m_iNumOptionRows;
	bool bLotsOfOptions = m_OptionRow[iRow].choices.size()>=MAX_ITEMS_PER_ROW;
	int iOptionInRow = bExitRow ? 0 : bLotsOfOptions ? pn : m_iSelectedOption[pn][iRow];

	BitmapText &text = m_textItems[iRow][iOptionInRow];

	iWidthOut = int(roundf( text.GetWidestLineWidthInSourcePixels() * text.GetZoomX() ));
	iXOut = int(roundf( text.GetX() ));
	iYOut = int(roundf( text.GetY() ));
}

void ScreenOptions::InitOptionsText()
{
	const float fLineGap = ITEMS_SPACING_Y - max(0, (m_iNumOptionRows-10)*2);

	// init m_textItems from optionLines
	int i;
	for( i=0; i<m_iNumOptionRows; i++ )	// foreach options line
	{
		OptionRow &optline = m_OptionRow[i];

		float fY = ITEMS_START_Y + fLineGap*i;

		BitmapText &title = m_textTitles[i];

		title.LoadFromFont( THEME->GetPathTo("Fonts","ScreenOptions title") );
		CString sText = optline.name;

		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( LABELS_ZOOM );
		title.SetHorizAlign( (Actor::HorizAlign)LABELS_H_ALIGN );
		title.SetVertAlign( Actor::align_middle );		
		title.EnableShadow( false );		


		Sprite &bullet = m_sprBullets[i];
		bullet.Load( THEME->GetPathTo("Graphics","ScreenOptions bullet") );
		bullet.SetXY( ARROWS_X, fY );


		// init all text in this line and count the width of the line
		if( optline.choices.size() >= MAX_ITEMS_PER_ROW )
		{
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( GAMESTATE->IsPlayerEnabled(p) )
				{
					BitmapText &option = m_textItems[i][p];

					int iChoiceInRow = m_iSelectedOption[p][i];

					option.LoadFromFont( THEME->GetPathTo("Fonts","ScreenOptions item") );
					option.SetText( m_OptionRow[i].choices[iChoiceInRow] );
					option.SetZoom( ITEMS_ZOOM );
					option.EnableShadow( false );

					option.SetXY( ITEM_X[p], fY );

					UpdateText( (PlayerNumber)p );
				}
			}
		}
		else
		{
			float fX = ITEMS_START_X;	// indent 70 pixels
			for( unsigned j=0; j<optline.choices.size(); j++ )	// for each option on this line
			{
				BitmapText &option = m_textItems[i][j];

				option.LoadFromFont( THEME->GetPathTo("Fonts","ScreenOptions item") );
				option.SetText( optline.choices[j] );
				option.SetZoom( ITEMS_ZOOM );
				option.EnableShadow( false );

				// set the XY position of each item in the line
				float fItemWidth = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX();
				fX += fItemWidth/2;
				option.SetXY( fX, fY );
				fX += fItemWidth/2 + ITEMS_GAP_X;
			}
		}
	}

	BitmapText &option = m_textItems[i][0];
	option.LoadFromFont( THEME->GetPathTo("Fonts","ScreenOptions item") );
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
	m_textItems[line][option].StopTweening();
	m_textItems[line][option].BeginTweening(.250);
	if(m_OptionDim[line][option])
		m_textItems[line][option].SetTweenDiffuse( RageColor(.5,.5,.5,1) );
	else
		m_textItems[line][option].SetTweenDiffuse( RageColor(1,1,1,1) );
}

bool ScreenOptions::RowCompletelyDimmed(int line) const
{
	for(unsigned i = 0; i < m_OptionRow[line].choices.size(); ++i)
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
			OptionsCursor &underline = m_Underline[p][i];

			// If there's only one choice (ScreenOptionsMenu), don't show underlines.  
			// It looks silly.
			bool bOnlyOneChoice = m_OptionRow[i].choices.size() == 1;
			underline.SetDiffuse( bOnlyOneChoice ? RageColor(1,1,1,0) : RageColor(1,1,1,1) );

			int iWidth, iX, iY;
			GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
			underline.SetBarWidth( iWidth );
			underline.SetXY( (float)iX, (float)iY );
		}
	}
}


void ScreenOptions::PositionIcons()
{
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
		{
			OptionIcon &icon = m_OptionIcons[p][i];

			int iWidth, iX, iY;			// We only use iY
			GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
			icon.SetXY( ICONS_X(p), (float)iY );
		}
	}
}

void ScreenOptions::RefreshIcons()
{
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
		{
			OptionIcon &icon = m_OptionIcons[p][i];

			int iSelection = m_iSelectedOption[p][i];
			CString sSelection = m_OptionRow[i].choices[iSelection];
			if( sSelection == "ON" )
				sSelection = m_OptionRow[i].name;
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

void ScreenOptions::UpdateText( PlayerNumber player_no )
{
	int iCurRow = m_iCurrentRow[player_no];
	int iChoiceInRow = m_iSelectedOption[player_no][iCurRow];

	bool bLotsOfOptions = m_OptionRow[iCurRow].choices.size()>=MAX_ITEMS_PER_ROW;

	if( bLotsOfOptions )
		m_textItems[iCurRow][player_no].SetText( m_OptionRow[iCurRow].choices[iChoiceInRow] );
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

		m_sprBullets[i].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );

		m_textTitles[i].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );

		if( m_OptionRow[i].choices.size() >= MAX_ITEMS_PER_ROW )
			for( unsigned j=0; j<NUM_PLAYERS; j++ )
				m_textItems[i][j].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );
		else
			for( unsigned j=0; j<m_OptionRow[i].choices.size(); j++ )
				m_textItems[i][j].SetDiffuse( bThisRowIsSelected ? colorSelected : colorNotSelected );
	}

	bool bThisRowIsSelectedByBoth = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsPlayerEnabled(p)  &&  m_iCurrentRow[p] != i )
			bThisRowIsSelectedByBoth = false;
		m_textItems[i][0].SetDiffuse( bThisRowIsSelectedByBoth ? colorNotSelected : colorSelected );
	if( bThisRowIsSelectedByBoth )
		m_textItems[i][0].SetEffectDiffuseShift( 1.0f, colorSelected, colorNotSelected );
	else
		m_textItems[i][0].SetEffectNone();
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
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Menu.m_Back.IsTransitioning() || m_Menu.m_Out.IsTransitioning() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		this->PostScreenMessage( SM_BeginFadingOut, 0 );
		break;
	case SM_GoToPrevScreen:
//		this->ExportOptions();	// Don't save options if we're going back!
		this->GoToPrevState();
		break;
	case SM_GoToNextScreen:
		this->ExportOptions();
		this->GoToNextState();
		break;
	case SM_BeginFadingOut:
		if(m_Menu.IsTransitioning())
			return; /* already transitioning */
		m_Menu.StartTransitioning( SM_GoToNextScreen );

		m_SoundStart.Play();

		m_framePage.BeginTweening( 0.3f, Actor::TWEEN_ACCELERATE );
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

	bool bIsExitRow = iCurRow == m_iNumOptionRows;

	if( !bIsExitRow  &&  m_bLoadExplanations )
	{
		CString sLineName = m_OptionRow[iCurRow].name;
		if( sLineName=="" )
			sLineName = m_OptionRow[iCurRow].choices[0];
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		m_textExplanation.SetText( THEME->GetMetric(m_sClassName,sLineName) );	
	}
	else
		m_textExplanation.SetText( "" );
}


void ScreenOptions::MenuBack( PlayerNumber pn )
{
	Screen::MenuBack( pn );

	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenOptions::StartGoToNextState()
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

void ScreenOptions::MenuStart( PlayerNumber pn )
{
	if( m_Menu.IsTransitioning() )
		return;

	if( PREFSMAN->m_bArcadeOptionsNavigation )
	{
		bool bAllOnExit = true;
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsPlayerEnabled(p)  &&  m_iCurrentRow[p] != m_iNumOptionRows )
				bAllOnExit = false;

		if( m_iCurrentRow[pn] != m_iNumOptionRows )	// not on exit
			MenuDown( pn );	// can't go down any more
		else if( bAllOnExit )
			this->PostScreenMessage( SM_BeginFadingOut, 0 );
	}
	else	// !m_bArcadeOptionsNavigation
	{
		this->PostScreenMessage( SM_BeginFadingOut, 0 );
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

		const int iNumOptions = m_OptionRow[iCurRow].choices.size();
		if( iNumOptions == 1 )
			continue;

		m_iSelectedOption[p][iCurRow] = (m_iSelectedOption[p][iCurRow]-1+iNumOptions) % iNumOptions;
		
		UpdateText( (PlayerNumber)p );
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

		const int iNumOptions = m_OptionRow[iCurRow].choices.size();
		if( iNumOptions == 1 )
			continue;

		m_iSelectedOption[p][iCurRow] = (m_iSelectedOption[p][iCurRow]+1) % iNumOptions;
		
		UpdateText( (PlayerNumber)p );
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

		TweenCursor( (PlayerNumber)p );
	}
	m_SoundNextRow.Play();
	OnChange();
}


