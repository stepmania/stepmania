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
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "InputMapper.h"

const float ITEM_X[NUM_PLAYERS] = { 260, 420 };

#define ICONS_X( p )					THEME->GetMetricF("ScreenOptions",ssprintf("IconsP%dX",p+1))
#define ARROWS_X						THEME->GetMetricF("ScreenOptions","ArrowsX")
#define LABELS_X						THEME->GetMetricF("ScreenOptions","LabelsX")
#define LABELS_ZOOM						THEME->GetMetricF("ScreenOptions","LabelsZoom")
#define LABELS_H_ALIGN					THEME->GetMetricI("ScreenOptions","LabelsHAlign")
#define ITEMS_ZOOM						THEME->GetMetricF("ScreenOptions","ItemsZoom")
#define ITEMS_START_X					THEME->GetMetricF("ScreenOptions","ItemsStartX")
#define ITEMS_GAP_X						THEME->GetMetricF("ScreenOptions","ItemsGapX")
#define ITEMS_START_Y					THEME->GetMetricF("ScreenOptions","ItemsStartY")
#define ITEMS_SPACING_Y					THEME->GetMetricF("ScreenOptions","ItemsSpacingY")
#define EXPLANATION_X(p)				THEME->GetMetricF("ScreenOptions",ssprintf("ExplanationP%dX",p+1))
#define EXPLANATION_Y(p)				THEME->GetMetricF("ScreenOptions",ssprintf("ExplanationP%dY",p+1))
#define EXPLANATION_ON_COMMAND(p)		THEME->GetMetric ("ScreenOptions",ssprintf("ExplanationP%dOnCommand",p+1))
#define EXPLANATION_TOGETHER_X			THEME->GetMetricF("ScreenOptions","ExplanationTogetherX")
#define EXPLANATION_TOGETHER_Y			THEME->GetMetricF("ScreenOptions","ExplanationTogetherY")
#define EXPLANATION_TOGETHER_ON_COMMAND	THEME->GetMetric ("ScreenOptions","ExplanationTogetherOnCommand")
#define ITEMS_SPACING_Y					THEME->GetMetricF("ScreenOptions","ItemsSpacingY")
#define EXPLANATION_ZOOM				THEME->GetMetricF("ScreenOptions","ExplanationZoom")
#define COLOR_SELECTED					THEME->GetMetricC("ScreenOptions","ColorSelected")
#define COLOR_NOT_SELECTED				THEME->GetMetricC("ScreenOptions","ColorNotSelected")
#define NUM_SHOWN_ITEMS					THEME->GetMetricI("ScreenOptions","NumShownItems")

ScreenOptions::ScreenOptions( CString sClassName ) : Screen(sClassName)
{
	LOG->Trace( "ScreenOptions::ScreenOptions()" );

	m_SoundChangeCol.Load( THEME->GetPathToS("ScreenOptions change") );
	m_SoundNextRow.Load( THEME->GetPathToS("ScreenOptions next") );
	m_SoundPrevRow.Load( THEME->GetPathToS("ScreenOptions prev") );
	m_SoundStart.Load( THEME->GetPathToS("Common start") );

	m_Menu.Load( sClassName );
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

void ScreenOptions::Init( InputMode im, OptionRow OptionRows[], int iNumOptionLines )
{
	LOG->Trace( "ScreenOptions::Set()" );

	m_InputMode = im;
	m_OptionRow = OptionRows;
	m_iNumOptionRows = iNumOptionLines;

	this->ImportOptions();

	for( int l=0; l<m_iNumOptionRows; l++ )
		if( m_OptionRow[l].bOneChoiceForAllPlayers )
			m_iSelectedOption[PLAYER_2][l] = m_iSelectedOption[PLAYER_1][l];



	int p;

	// init highlights
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_Highlight[p].Load( (PlayerNumber)p, false );
		m_framePage.AddChild( &m_Highlight[p] );
	}

	// init underlines
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		for( int l=0; l<m_iNumOptionRows; l++ )
		{	
			m_OptionIcons[p][l].Load( (PlayerNumber)p, "", false );
			m_framePage.AddChild( &m_OptionIcons[p][l] );

			m_Underline[p][l].Load( (PlayerNumber)p, true );
			m_framePage.AddChild( &m_Underline[p][l] );
		}
	}

	// init m_textItems from optionLines
	int r;
	for( r=0; r<m_iNumOptionRows; r++ )		// foreach line
	{
		vector<BitmapText *> & textItems = m_textItems[r];
		const OptionRow &optline = m_OptionRow[r];

		unsigned c;

		m_framePage.AddChild( &m_sprBullets[r] );

		m_framePage.AddChild( &m_textTitles[r] );		

		float fX = ITEMS_START_X;	// indent 70 pixels
		for( c=0; c<optline.choices.size(); c++ )
		{
			BitmapText *bt = new BitmapText;
			textItems.push_back( bt );

			bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
			bt->SetText( optline.choices[c] );
			bt->SetZoom( ITEMS_ZOOM );
			bt->EnableShadow( false );

			// set the X position of each item in the line
			const float fItemWidth = bt->GetZoomedWidth();
			fX += fItemWidth/2;
			bt->SetX( fX );
			fX += fItemWidth/2 + ITEMS_GAP_X;
		}

		if( fX > SCREEN_RIGHT-40 )
		{
			// It goes off the edge of the screen.  Re-init with the "long row" style.
			m_bRowIsLong[r] = true;
			for( unsigned j=0; j<optline.choices.size(); j++ )	// for each option on this line
				delete textItems[j];
			textItems.clear();

			for( unsigned p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsHumanPlayer(p) )
					continue;

				BitmapText *bt = new BitmapText;
				textItems.push_back( bt );

				const int iChoiceInRow = m_iSelectedOption[p][r];

				bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
				bt->SetText( optline.choices[iChoiceInRow] );
				bt->SetZoom( ITEMS_ZOOM );
				bt->EnableShadow( false );

				/* if choices are locked together, center the item. */
				if( optline.bOneChoiceForAllPlayers )
					bt->SetX( (ITEM_X[0]+ITEM_X[1])/2 );
				else
					bt->SetX( ITEM_X[p] );

				/* If bOneChoiceForAllPlayers, then only initialize one text item. */
				if( optline.bOneChoiceForAllPlayers )
					break;
			}
		}

		for( c=0; c<textItems.size(); c++ )
			m_framePage.AddChild( textItems[c] );
	}

	InitOptionsText();

	// TRICKY:  Add one more item.  This will be "EXIT"
	{
		BitmapText *bt = new BitmapText;
		m_textItems[r].push_back( bt );

		bt->LoadFromFont( THEME->GetPathToF("ScreenOptions item") );
		bt->SetText( "EXIT" );
		bt->SetZoom( ITEMS_ZOOM );
		bt->SetShadowLength( 0 );
		float fY = ITEMS_START_Y + ITEMS_SPACING_Y*m_iNumOptionRows;
		bt->SetXY( CENTER_X, fY );

		m_framePage.AddChild( bt );
	}

	// add explanation here so it appears on top
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_textExplanation[p].LoadFromFont( THEME->GetPathToF("ScreenOptions explanation") );
		m_textExplanation[p].SetZoom( EXPLANATION_ZOOM );
		m_textExplanation[p].SetShadowLength( 0 );
		m_framePage.AddChild( &m_textExplanation[p] );
	}
	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		for( p=0; p<NUM_PLAYERS; p++ )
			m_textExplanation[p].SetXY( EXPLANATION_X(p), EXPLANATION_Y(p) );
		break;
	case INPUTMODE_TOGETHER:
		m_textExplanation[0].SetXY( EXPLANATION_TOGETHER_X, EXPLANATION_TOGETHER_Y );
		break;
	default:
		ASSERT(0);
	}

	// poke once at all the explanation metrics so that we catch missing ones early
	for( r=0; r<m_iNumOptionRows; r++ )		// foreach line
	{
		GetExplanationText( r );
		GetExplanationTitle( r );
	}


	CHECKPOINT;

	PositionItems();
	PositionUnderlines();
	PositionIcons();
	CHECKPOINT;
	RefreshIcons();
	CHECKPOINT;
	PositionCursors();
	CHECKPOINT;
	UpdateEnabledDisabled();
	CHECKPOINT;
	for( p=0; p<NUM_PLAYERS; p++ )
		OnChange( (PlayerNumber)p );
	CHECKPOINT;

	/* It's tweening into position, but on the initial tween-in we only want to
	 * tween in the whole page at once.  Since the tweens are nontrivial, it's
	 * easiest to queue the tweens and then force them to finish. */
	for( r=0; r<m_iNumOptionRows; r++ )	// foreach options row
	{
		m_textTitles[r].FinishTweening();
		m_sprBullets[r].FinishTweening();
		for( unsigned c=0; c<m_textItems[r].size(); c++ )
		{
			m_textItems[r][c]->FinishTweening();
		}

		for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
		{
			m_Underline[p][r].FinishTweening();
			m_OptionIcons[p][r].FinishTweening();
		}
	}
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
	for( int i=0; i<m_iNumOptionRows+1; i++ ) /* +1 = "exit" */
		for( unsigned j = 0; j < m_textItems[i].size(); ++j)
			delete m_textItems[i][j];
}

CString ScreenOptions::GetExplanationText( int row ) const
{
	CString sLineName = m_OptionRow[row].name;
	sLineName.Replace("\n-","");
	sLineName.Replace("\n","");
	sLineName.Replace(" ","");
	return THEME->GetMetric( "OptionExplanations", sLineName+"Help" );
}

CString ScreenOptions::GetExplanationTitle( int row ) const
{
	CString sLineName = m_OptionRow[row].name;
	sLineName.Replace("\n-","");
	sLineName.Replace("\n","");
	sLineName.Replace(" ","");
	return THEME->GetMetric( "OptionExplanations", sLineName+"Title" );
}

BitmapText &ScreenOptions::GetTextItemForRow( PlayerNumber pn, int iRow )
{
	const bool bExitRow = iRow == m_iNumOptionRows;
	if( bExitRow )
		return *m_textItems[iRow][0];

	const bool bLotsOfOptions = m_bRowIsLong[iRow];
	if( !bLotsOfOptions )
	{
		unsigned iOptionInRow = m_iSelectedOption[pn][iRow];
		return *m_textItems[iRow][iOptionInRow];
	}

	const bool bOneChoice = m_OptionRow[iRow].bOneChoiceForAllPlayers;

	if( bOneChoice )
		return *m_textItems[iRow][0];

	/* We have lots of options and this isn't a OneChoice row, which means
	 * each player has a separate item displayed.  */
	int iOptionInRow = min( (unsigned)pn, m_textItems[iRow].size()-1 );
	return *m_textItems[iRow][iOptionInRow];
}

void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	BitmapText &text = GetTextItemForRow( pn, iRow );

	iWidthOut = int(roundf( text.GetZoomedWidth() ));
	iXOut = int(roundf( text.GetDestX() ));
	/* We update m_fRowY, change colors and tween items, and then tween rows to
	 * their final positions.  (This is so we don't tween colors, too.)  m_fRowY
	 * is the actual destination position, even though we may not have set up the
	 * tween yet. */
	iYOut = int(roundf( m_fRowY[iRow] ));
}

void ScreenOptions::InitOptionsText()
{
	for( int i=0; i<m_iNumOptionRows; i++ )	// foreach options line
	{
		const float fY = ITEMS_START_Y + ITEMS_SPACING_Y*i;

		BitmapText &title = m_textTitles[i];

		title.LoadFromFont( THEME->GetPathToF("ScreenOptions title") );
		CString sText = GetExplanationTitle( i );

		title.SetText( sText );
		title.SetXY( LABELS_X, fY );
		title.SetZoom( LABELS_ZOOM );
		title.SetHorizAlign( (Actor::HorizAlign)LABELS_H_ALIGN );
		title.SetVertAlign( Actor::align_middle );		
		title.EnableShadow( false );		

		Sprite &bullet = m_sprBullets[i];
		bullet.Load( THEME->GetPathToG("ScreenOptions bullet") );
		bullet.SetXY( ARROWS_X, fY );
		
		// set the Y position of each item in the line
		for( unsigned c=0; c<m_textItems[i].size(); c++ )
			m_textItems[i][c]->SetY( fY );
	}
}

void ScreenOptions::PositionUnderlines()
{
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

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
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

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
	// handled by ScreenOptionsMaster
}


void ScreenOptions::PositionCursors()
{
	// Set the position of the highlight showing the current option the user is changing.
	// Set the position of the underscores showing the current choice for each option line.
	for( int p=0; p<NUM_PLAYERS; p++ )	// foreach player
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;

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

void ScreenOptions::UpdateText( PlayerNumber player_no, int iRow )
{
	int iChoiceInRow = m_iSelectedOption[player_no][iRow];

	if( !m_bRowIsLong[iRow] )
		return;

	const OptionRow &row = m_OptionRow[iRow];

	unsigned item_no = row.bOneChoiceForAllPlayers?0:player_no;

	/* If player_no is 2 and there is no player 1: */
	item_no = min( item_no, m_textItems[iRow].size()-1 );

	m_textItems[iRow][item_no]->SetText( m_OptionRow[iRow].choices[iChoiceInRow] );
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

		for( unsigned j=0; j<m_textItems[i].size(); j++ )
			m_textItems[i][j]->SetGlobalDiffuseColor( color );

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

			for( unsigned j=0; j<m_textItems[i].size(); j++ )
			{
				m_textItems[i][j]->StopTweening();
				m_textItems[i][j]->BeginTweening( 0.3f );
				m_textItems[i][j]->SetDiffuseAlpha( m_bRowIsHidden[i]? 0.0f:1.0f );
				m_textItems[i][j]->SetY( m_fRowY[i] );
			}
		}
	}

	bool bExitRowIsSelectedByBoth = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( GAMESTATE->IsHumanPlayer(p)  &&  m_iCurrentRow[p] != m_iNumOptionRows )
			bExitRowIsSelectedByBoth = false;

	RageColor color = bExitRowIsSelectedByBoth ? colorSelected : colorNotSelected;
	m_textItems[m_iNumOptionRows][0]->SetGlobalDiffuseColor( color );

	if( m_textItems[m_iNumOptionRows][0]->GetDestY() != m_fRowY[m_iNumOptionRows] )
	{
		m_textItems[m_iNumOptionRows][0]->StopTweening();
		m_textItems[m_iNumOptionRows][0]->BeginTweening( 0.3f );
		m_textItems[m_iNumOptionRows][0]->SetDiffuseAlpha( m_bRowIsHidden[m_iNumOptionRows]? 0.0f:1.0f );
		m_textItems[m_iNumOptionRows][0]->SetY( m_fRowY[m_iNumOptionRows] );
	}

	if( bExitRowIsSelectedByBoth )
		m_textItems[m_iNumOptionRows][0]->SetEffectDiffuseShift( 1.0f, colorSelected, colorNotSelected );
	else
		m_textItems[m_iNumOptionRows][0]->SetEffectNone();
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

	// if we are in dedicated menubutton input and arcade navigation
	// check to see if MENU_BUTTON_LEFT and MENU_BUTTON_RIGHT are being held

	// Super Hack to use the feature in ScreenOptionsMenu (where no style is set)
	bool bHoldingLeftOrRight = false;
	bHoldingLeftOrRight |= MenuI.IsValid() && MenuI.button == MENU_BUTTON_START &&
		PREFSMAN->m_bArcadeOptionsNavigation &&
		GAMESTATE->m_CurStyle != STYLE_INVALID &&
		(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) ) || 
		INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) ) );

	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
	{
		GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
		
		bHoldingLeftOrRight |= MenuI.IsValid() && MenuI.button == MENU_BUTTON_START &&
			PREFSMAN->m_bArcadeOptionsNavigation &&
			GAMESTATE->m_CurStyle != STYLE_INVALID &&
			(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) ) ||
			INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) ) );

		GAMESTATE->m_CurStyle = STYLE_INVALID;
	}

	if( bHoldingLeftOrRight )
	{
		Screen::MenuUp( MenuI.player, type );
		return;
	}

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
	const int total = NUM_SHOWN_ITEMS;
	const int halfsize = total / 2;

	/* Total number of rows, including "EXIT". */
	const int NumRows = m_iNumOptionRows + 1;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	const int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	const int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( m_InputMode == INPUTMODE_TOGETHER || !BothPlayersActivated )
	{
		/* Simply center the cursor. */
		first_start = max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	} else {
		/* First half: */
		const int earliest = min( P1Choice, P2Choice );
		first_start = max( earliest - halfsize/2, 0 );
		first_end = first_start + halfsize;

		/* Second half: */
		const int latest = max( P1Choice, P2Choice );

		second_start = max( latest - halfsize/2, 0 );

		/* Don't overlap. */
		second_start = max( second_start, first_end );

		second_end = second_start + halfsize;
	}

	first_end = min( first_end, NumRows );
	second_end = min( second_end, NumRows );

	/* If less than total (and NumRows) are displayed, fill in the empty
	 * space intelligently. */
	while(1)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= NumRows || sum >= total)
			break; /* nothing more to display, or no room */

		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
			second_start--;
		/* Otherwise, expand either end. */
		else if( first_start > 0 )
			first_start--;
		else if( second_end < NumRows )
			second_end++;
		else
			ASSERT(0); /* do we have room to grow or don't we? */
	}

	int pos = 0;
	for( int i=0; i<NumRows; i++ )		// foreach line
	{
		float ItemPosition;
		if( i < first_start )
			ItemPosition = -0.5f;
		else if( i < first_end )
			ItemPosition = (float) pos++;
		else if( i < second_start )
			ItemPosition = halfsize - 0.5f;
		else if( i < second_end )
			ItemPosition = (float) pos++;
		else
			ItemPosition = (float) total - 0.5f;
			
		float fY = ITEMS_START_Y + ITEMS_SPACING_Y*ItemPosition;
		m_fRowY[i] = fY;
		m_bRowIsHidden[i] = i < first_start ||
							(i >= first_end && i < second_start) ||
							i >= second_end;
	}
}


void ScreenOptions::OnChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	/* Update m_fRowY[] and m_bRowIsHidden[]. */
	PositionItems();

	/* Do positioning. */
	PositionUnderlines();
	RefreshIcons();
	PositionIcons();
	UpdateEnabledDisabled();

	/* Update all players, since changing one player can move both cursors. */
	for( int p=0; p<NUM_PLAYERS; p++ )
		TweenCursor( (PlayerNumber) p );

	int iCurRow = m_iCurrentRow[pn];

	bool bIsExitRow = iCurRow == m_iNumOptionRows;

	BitmapText *pText = NULL;
	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		pText = &m_textExplanation[pn];
		pText->FinishTweening();
		pText->Command( EXPLANATION_ON_COMMAND(pn) );
		break;
	case INPUTMODE_TOGETHER:
		pText = &m_textExplanation[0];
		pText->FinishTweening();
		pText->Command( EXPLANATION_TOGETHER_ON_COMMAND );
		break;
	}

	if( bIsExitRow )
		pText->SetText( "" );
	else
		pText->SetText( GetExplanationText(iCurRow) );
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


void ScreenOptions::ChangeValue( PlayerNumber pn, int iDelta ) 
{
	int iCurRow = m_iCurrentRow[pn];
	OptionRow &row = m_OptionRow[iCurRow];

	const int iNumOptions = row.choices.size();
	if( PREFSMAN->m_bArcadeOptionsNavigation )
	{
		if( iCurRow == m_iNumOptionRows || iNumOptions <= 1 )	// 1 or 0
		{
			if( iDelta < 0 )
			{
				MenuUp( pn );
				return;
			}
			else
			{
				MenuDown( pn );
				return;
			}
		}
	}

	if( iCurRow == m_iNumOptionRows	)	// EXIT is selected
		return;		// don't allow a move

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
//		if( m_InputMode == INPUTMODE_INDIVIDUAL  &&  p != pn )
		if( p != pn )  // don't check for INPUTMODE_INDIVIDUAL because on regular
			           // options it moves everything by 2
			continue;	// skip

		int iNewSel = m_iSelectedOption[p][iCurRow] + iDelta;
		wrap( iNewSel, iNumOptions );
		
		if( row.bOneChoiceForAllPlayers )
		{
			for( int p2=0; p2<NUM_PLAYERS; p2++ )
			{
				m_iSelectedOption[p2][iCurRow] = iNewSel;
				UpdateText( (PlayerNumber)p2, iCurRow );
			}
		}
		else
		{
			m_iSelectedOption[p][iCurRow] = iNewSel;
			UpdateText( (PlayerNumber)p, iCurRow );
		}
		OnChange( (PlayerNumber)p );
	}
	m_SoundChangeCol.Play();
}


void ScreenOptions::MenuUp( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL  &&  p != pn )
			continue;	// skip

		if( m_iCurrentRow[p] == 0 )	// on first row
			m_iCurrentRow[p] = m_iNumOptionRows;	// on exit
		else
			m_iCurrentRow[p]--;
		OnChange( (PlayerNumber)p );
	}
	m_SoundPrevRow.Play();
}


void ScreenOptions::MenuDown( PlayerNumber pn ) 
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL  &&  p != pn )
			continue;	// skip

		if( m_iCurrentRow[p] == m_iNumOptionRows )	// on exit
			m_iCurrentRow[p] = 0;	// on first row
		else
			m_iCurrentRow[p]++;
		OnChange( (PlayerNumber)p );
	}
	m_SoundNextRow.Play();
}
