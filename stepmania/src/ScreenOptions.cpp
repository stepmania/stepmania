#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"

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
#define COLOR_SELECTED		THEME->GetMetricC("ScreenOptions","ColorSelected")
#define COLOR_NOT_SELECTED	THEME->GetMetricC("ScreenOptions","ColorNotSelected")

const int total = 10;
const int halfsize = total / 2;

ScreenOptions::ScreenOptions( CString sClassName, bool bEnableTimer ) : Screen("ScreenOptions")
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_sName = sClassName;

	m_SoundChangeCol.Load( THEME->GetPathToS("ScreenOptions change") );
	m_SoundNextRow.Load( THEME->GetPathToS("ScreenOptions next") );
	m_SoundPrevRow.Load( THEME->GetPathToS("ScreenOptions prev") );
	m_SoundStart.Load( THEME->GetPathToS("Common start") );

	m_Menu.Load( sClassName, bEnableTimer, false );	// no style icon
	this->AddChild( &m_Menu );

	// add everything to m_framePage so we can animate everything at once
	this->AddChild( &m_framePage );

	m_sprPage.Load( THEME->GetPathToG(sClassName+" page") );
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
	m_framePage.SetX( 0 );
	memset(&m_bRowIsLong, 0, sizeof(m_bRowIsLong));
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
		if( !GAMESTATE->IsHumanPlayer(p) )
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
			m_framePage.AddChild( &m_textItems[i][j] );
	}

	// TRICKY:  Add one more item.  This will be "EXIT"
	m_framePage.AddChild( &m_textItems[i][0] );

	// add explanation here so it appears on top
	m_textExplanation.LoadFromFont( THEME->GetPathToF("ScreenOptions explanation") );
	m_textExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	m_textExplanation.SetZoom( EXPLANATION_ZOOM );
	m_textExplanation.SetShadowLength( 0 );
	m_framePage.AddChild( &m_textExplanation );


	InitOptionsText();
	PositionItems();
	PositionUnderlines();
	PositionIcons();
	RefreshIcons();
	PositionCursors();
	UpdateEnabledDisabled();
	OnChange();

	/* It's tweening into position, but on the initial tween-in we only want to
	 * tween in the whole page at once.  Since the tweens are nontrivial, it's
	 * easiest to queue the tweens and then force them to finish. */
	for( i=0; i<m_iNumOptionRows; i++ )	// foreach options line
	{
		m_textTitles[i].FinishTweening();
		m_sprBullets[i].FinishTweening();
		for( unsigned j=0; j<m_OptionRow[i].choices.size(); j++ )
			m_textItems[i][j].FinishTweening();

		for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
		{
			m_Underline[p][i].FinishTweening();
			m_OptionIcons[p][i].FinishTweening();
		}
	}
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
}


void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	bool bExitRow = iRow == m_iNumOptionRows;
	bool bLotsOfOptions = m_bRowIsLong[iRow];
	int iOptionInRow = bExitRow ? 0 : bLotsOfOptions ? pn : m_iSelectedOption[pn][iRow];

	BitmapText &text = m_textItems[iRow][iOptionInRow];

	iWidthOut = int(roundf( text.GetWidestLineWidthInSourcePixels() * text.GetZoomX() ));
	iXOut = int(roundf( text.GetDestX() ));
	/* We update m_fRowY, change colors and tween items, and then tween rows to
	 * their final positions.  (This is so we don't tween colors, too.)  m_fRowY
	 * is the actual destination position, even though we may not have set up the
	 * tween yet. */
	iYOut = int(roundf( m_fRowY[iRow] ));
}

void ScreenOptions::InitOptionsText()
{
	// init m_textItems from optionLines
	int i;
	for( i=0; i<m_iNumOptionRows; i++ )	// foreach options line
	{
		OptionRow &optline = m_OptionRow[i];

		const float fY = ITEMS_START_Y + ITEMS_SPACING_Y*i;

		BitmapText &title = m_textTitles[i];

		title.LoadFromFont( THEME->GetPathToF("ScreenOptions title") );
		CString sText = optline.name;

		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( LABELS_ZOOM );
		title.SetHorizAlign( (Actor::HorizAlign)LABELS_H_ALIGN );
		title.SetVertAlign( Actor::align_middle );		
		title.EnableShadow( false );		


		Sprite &bullet = m_sprBullets[i];
		bullet.Load( THEME->GetPathToG("ScreenOptions bullet") );
		bullet.SetXY( ARROWS_X, fY );


		// init all text in this line and count the width of the line
		float fX = ITEMS_START_X;	// indent 70 pixels
		for( unsigned j=0; j<optline.choices.size(); j++ )	// for each option on this line
		{
			BitmapText &option = m_textItems[i][j];

			option.LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
			option.SetText( optline.choices[j] );
			option.SetZoom( ITEMS_ZOOM );
			option.EnableShadow( false );

			// set the XY position of each item in the line
			float fItemWidth = option.GetWidestLineWidthInSourcePixels() * option.GetZoomX();
			fX += fItemWidth/2;
			option.SetXY( fX, fY );
			fX += fItemWidth/2 + ITEMS_GAP_X;
		}

		m_bRowIsLong[i] = fX > SCREEN_RIGHT-40;	// goes off edge of screen

		if( m_bRowIsLong[i] )
		{
			// re-init with "long row" style
			for( unsigned j=0; j<optline.choices.size(); j++ )	// for each option on this line
				m_textItems[i][j].SetText( "" );

			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsHumanPlayer(p) )
					continue;

				BitmapText &option = m_textItems[i][p];

				const int iChoiceInRow = m_iSelectedOption[p][i];

				option.LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
				option.SetText( m_OptionRow[i].choices[iChoiceInRow] );
				option.SetZoom( ITEMS_ZOOM );
				option.EnableShadow( false );

				option.SetY( fY );
				/* If we're in INPUTMODE_BOTH, center the items. */
				if( m_InputMode == INPUTMODE_BOTH )
					option.SetX( (ITEM_X[0]+ITEM_X[1])/2 );
				else
					option.SetX( ITEM_X[p] );

				UpdateText( (PlayerNumber)p );
			}
		}
	}

	BitmapText &option = m_textItems[i][0];
	option.LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
	option.SetText( "EXIT" );
	option.SetZoom( ITEMS_ZOOM );
	option.SetShadowLength( 0 );
	float fY = ITEMS_START_Y + ITEMS_SPACING_Y*i;
	option.SetXY( CENTER_X, fY );
}

void ScreenOptions::PositionUnderlines()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
		{
			OptionsCursor &underline = m_Underline[p][i];

			/* Don't tween X movement and color changes. */
			int iWidth, iX, iY;
			GetWidthXY( (PlayerNumber)p, i, iWidth, iX, iY );
			underline.SetGlobalX( (float)iX );
			underline.SetGlobalDiffuseColor( RageColor(1,1,1, 1.0f) );

			// If there's only one choice (ScreenOptionsMenu), don't show underlines.  
			// It looks silly.
			bool bOnlyOneChoice = m_OptionRow[i].choices.size() == 1;
			bool hidden = bOnlyOneChoice || m_bRowIsHidden[i];

			if( underline.GetDestY() != m_fRowY[i] )
			{
				underline.StopTweening();
				underline.BeginTweening( 0.3f );
			}

			/* XXX: diffuse doesn't work since underline is an ActorFrame */
			underline.SetDiffuse( RageColor(1,1,1,hidden? 0.0f:1.0f) );
			underline.SetBarWidth( iWidth );
			underline.SetY( (float)iY );
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
			icon.SetX( ICONS_X(p) );

			if( icon.GetDestY() != m_fRowY[i] )
			{
				icon.StopTweening();
				icon.BeginTweening( 0.3f );
			}

			icon.SetY( (float)iY );
			/* XXX: this doesn't work since icon is an ActorFrame */
			icon.SetDiffuse( RageColor(1,1,1, m_bRowIsHidden[i]? 0.0f:1.0f) );
		}
	}
}

void ScreenOptions::RefreshIcons()
{
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

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
	highlight.SetXY( (float)iX, (float)iY );
}

void ScreenOptions::UpdateText( PlayerNumber player_no )
{
	int iCurRow = m_iCurrentRow[player_no];
	int iChoiceInRow = m_iSelectedOption[player_no][iCurRow];

	bool bLotsOfOptions = m_bRowIsLong[iCurRow];

	if( bLotsOfOptions )
		m_textItems[iCurRow][player_no].SetText( m_OptionRow[iCurRow].choices[iChoiceInRow] );
}

void ScreenOptions::UpdateEnabledDisabled()
{
	RageColor colorSelected = COLOR_SELECTED;
	RageColor colorNotSelected = COLOR_NOT_SELECTED;

	// init text
	for( int i=0; i<m_iNumOptionRows; i++ )		// foreach line
	{
		bool bThisRowIsSelected = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
			if( GAMESTATE->IsHumanPlayer(p)  &&  m_iCurrentRow[p] == i )
				bThisRowIsSelected = true;

		/* Don't tween selection colors at all. */
		RageColor color = bThisRowIsSelected ? colorSelected : colorNotSelected;
		m_sprBullets[i].SetGlobalDiffuseColor( color );
		m_textTitles[i].SetGlobalDiffuseColor( color );

		if( m_bRowIsLong[i] )
			for( unsigned j=0; j<NUM_PLAYERS; j++ )
				m_textItems[i][j].SetGlobalDiffuseColor( color );
		else
			for( unsigned j=0; j<m_OptionRow[i].choices.size(); j++ )
				m_textItems[i][j].SetGlobalDiffuseColor( color );

		if( m_sprBullets[i].GetDestY() != m_fRowY[i] )
		{
			m_sprBullets[i].StopTweening();
			m_textTitles[i].StopTweening();
			m_sprBullets[i].BeginTweening( 0.3f );
			m_textTitles[i].BeginTweening( 0.3f );

			m_sprBullets[i].SetDiffuseAlpha( m_bRowIsHidden[i]? 0.0f:1.0f );
			m_textTitles[i].SetDiffuseAlpha( m_bRowIsHidden[i]? 0.0f:1.0f );

			m_sprBullets[i].SetY( m_fRowY[i] );
			m_textTitles[i].SetY( m_fRowY[i] );

			if( m_bRowIsLong[i] )
				for( unsigned j=0; j<NUM_PLAYERS; j++ )
				{
					m_textItems[i][j].StopTweening();
					m_textItems[i][j].BeginTweening( 0.3f );
					m_textItems[i][j].SetDiffuseAlpha( m_bRowIsHidden[i]? 0.0f:1.0f );
					m_textItems[i][j].SetY( m_fRowY[i] );
				}
			else
				for( unsigned j=0; j<m_OptionRow[i].choices.size(); j++ )
				{
					m_textItems[i][j].StopTweening();
					m_textItems[i][j].BeginTweening( 0.3f );
					m_textItems[i][j].SetDiffuseAlpha( m_bRowIsHidden[i]? 0.0f:1.0f );
					m_textItems[i][j].SetY( m_fRowY[i] );
				}
		}

		/* Hide the text of all but the first player, so we don't overdraw. */
		if( m_InputMode == INPUTMODE_BOTH && m_bRowIsLong[i] )
		{
			for( unsigned j=1; j<NUM_PLAYERS; j++ )
			{
				m_textItems[i][j].StopTweening();
				m_textItems[i][j].SetDiffuse( RageColor(1,1,1,0) );
			}
		}
	}

	bool bExitRowIsSelectedByBoth = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p)  &&  m_iCurrentRow[p] != m_iNumOptionRows )
			bExitRowIsSelectedByBoth = false;

	RageColor color = bExitRowIsSelectedByBoth ? colorSelected : colorNotSelected;
	m_textItems[m_iNumOptionRows][0].SetGlobalDiffuseColor( color );

	if( m_textItems[m_iNumOptionRows][0].GetDestY() != m_fRowY[m_iNumOptionRows] )
	{
		m_textItems[m_iNumOptionRows][0].StopTweening();
		m_textItems[m_iNumOptionRows][0].BeginTweening( 0.3f );
		m_textItems[m_iNumOptionRows][0].SetDiffuseAlpha( m_bRowIsHidden[m_iNumOptionRows]? 0.0f:1.0f );
		m_textItems[m_iNumOptionRows][0].SetY( m_fRowY[m_iNumOptionRows] );
	}

	if( bExitRowIsSelectedByBoth )
		m_textItems[m_iNumOptionRows][0].SetEffectDiffuseShift( 1.0f, colorSelected, colorNotSelected );
	else
		m_textItems[m_iNumOptionRows][0].SetEffectNone();
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
		m_framePage.SetX( SCREEN_RIGHT );
		break;
	}
}


void ScreenOptions::PositionItems()
{
	/* Total number of rows, including "EXIT". */
	const int NumRows = m_iNumOptionRows + 1;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	const int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	const int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	/* First half: */
	const int earliest = min( P1Choice, P2Choice );
	int first_start = max( earliest - halfsize+1, 0 );
	int first_end = first_start + halfsize - 1;

	/* Second half: */
	int latest = max( P1Choice, P2Choice );

	int second_start = max( latest - halfsize + 1, 0 );
	/* Never overlap: */
	second_start = max( second_start, first_end + 1 );
	int second_end = second_start + halfsize - 1;

	if( second_end >= NumRows )
	{
		first_start = max(0, NumRows - total);
		first_end = NumRows;
		second_start = 9999;
		second_end = 9999;
	}

	bool is_split = false;
	if(first_end+1 < second_start)
		is_split = true;

	for( int i=0; i<NumRows; i++ )		// foreach line
	{
		float ItemPosition;
		if( i < first_start )
			ItemPosition = -0.5f;
		else if( i <= first_end )
			ItemPosition = float(i - first_start);
		else if( i < second_start )
			ItemPosition = halfsize - 0.5f;
		else if( i <= second_end )
			ItemPosition = float(halfsize + i - second_start);
		else
			ItemPosition = (float) total - 0.5f;
			
		float fY = ITEMS_START_Y + ITEMS_SPACING_Y*ItemPosition;
		m_fRowY[i] = fY;
		m_bRowIsHidden[i] = i < first_start ||
							(i > first_end && i < second_start) ||
							i > second_end;
	}
}


void ScreenOptions::OnChange()
{
	/* Update m_fRowY[] and m_bRowIsHidden[]. */
	PositionItems();

	/* Do positioning. */
	PositionUnderlines();
	RefreshIcons();
	PositionIcons();
	UpdateEnabledDisabled();

	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		TweenCursor( (PlayerNumber)pn );

	int iCurRow = m_iCurrentRow[PLAYER_1];

	bool bIsExitRow = iCurRow == m_iNumOptionRows;

	if( !bIsExitRow  &&  m_bLoadExplanations )
	{
		CString sLineName = m_OptionRow[iCurRow].name;
		if( sLineName=="" )
			sLineName = m_OptionRow[iCurRow].choices[0];
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		m_textExplanation.SetText( THEME->GetMetric(m_sName,sLineName) );	
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
			if( GAMESTATE->IsHumanPlayer(p)  &&  m_iCurrentRow[p] != m_iNumOptionRows )
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
	}
	m_SoundNextRow.Play();
	OnChange();
}


