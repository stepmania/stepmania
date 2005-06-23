#include "global.h"
#include "ScreenOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "InputMapper.h"
#include "ActorUtil.h"
#include "ProfileManager.h"
#include "ScreenDimensions.h"
#include "Command.h"


/*
 * These navigation types are provided:
 *
 * All modes:
 *  left, right -> change option
 *  up, down -> change row
 *   (in toggle modes, focus on nearest item to old focus)
 *
 * NAV_THREE_KEY:
 *  start -> move to next row
 *  left+right+start -> move to prev row
 *  (next screen via "exit" entry)
 * This is the minimal navigation, for using menus with only three buttons.
 *
 * NAV_FIVE_KEY:
 *  start -> next screen
 * This is a much more convenient navigation, requiring five keys.
 *
 * NAV_TOGGLE_THREE_KEY:
 *  start -> on first choice, move to next row; otherewise toggle option and move to first choice
 *  left+right+start -> move to prev row
 *
 * NAV_TOGGLE_FIVE_KEY:
 *  start -> toggle option
 *
 * Regular modes and toggle modes must be enabled by the theme.  We could simply
 * automatically switch to toggle mode for multiselect rows, but that would be strange
 * for non-multiselect rows (eg. scroll speed).
 *
 * THREE_KEY modes are navigatable with only MenuLeft, MenuRight and MenuStart, and
 * are used when PREFSMAN->m_bArcadeOptionsNavigation is enabled.  However, they can
 * still use MenuUp and MenuDown for nonessential behavior.
 *
 * NAV_THREE_KEY_MENU:
 *  left, right -> change row
 *  up, down -> change row
 *  start -> next screen
 * This is a specialized navigation for ScreenOptionsMenu.  It must be enabled to
 * allow screens that use rows to select other screens to work with only three
 * buttons.  (It's also used when in five-key mode.)
 *
 * We don't want to simply allow left/right to move up and down on single-entry
 * rows when in NAV_THREE_KEY, becasue left and right shouldn't exit the "exit" row
 * in player options menus, but it should in the options menu.
 */

static CString ROW_Y_NAME( size_t r )					{ return ssprintf("Row%dY",int(r+1)); }
static CString EXPLANATION_X_NAME( size_t p )			{ return ssprintf("ExplanationP%dX",int(p+1)); }
static CString EXPLANATION_Y_NAME( size_t p )			{ return ssprintf("ExplanationP%dY",int(p+1)); }
static CString EXPLANATION_ON_COMMAND_NAME( size_t p )	{ return ssprintf("ExplanationP%dOnCommand",int(p+1)); }

static CString OPTION_EXPLANATION( CString s )
{
	return THEME->GetMetric("OptionExplanations",s);
}

//REGISTER_SCREEN_CLASS( ScreenOptions );	// can't be instantiated
ScreenOptions::ScreenOptions( CString sClassName ) : ScreenWithMenuElements(sClassName),
	NUM_ROWS_SHOWN					(m_sName,"NumRowsShown"),
	ROW_Y							(m_sName,ROW_Y_NAME,NUM_ROWS_SHOWN),
	ROW_Y_OFF_SCREEN_TOP			(m_sName,"RowYOffScreenTop"),
	ROW_Y_OFF_SCREEN_CENTER			(m_sName,"RowYOffScreenCenter"),
	ROW_Y_OFF_SCREEN_BOTTOM			(m_sName,"RowYOffScreenBottom"),
	EXPLANATION_X					(m_sName,EXPLANATION_X_NAME,NUM_PLAYERS),
	EXPLANATION_Y					(m_sName,EXPLANATION_Y_NAME,NUM_PLAYERS),
	EXPLANATION_ON_COMMAND			(m_sName,EXPLANATION_ON_COMMAND_NAME,NUM_PLAYERS),
	EXPLANATION_TOGETHER_X			(m_sName,"ExplanationTogetherX"),
	EXPLANATION_TOGETHER_Y			(m_sName,"ExplanationTogetherY"),
	EXPLANATION_TOGETHER_ON_COMMAND	(m_sName,"ExplanationTogetherOnCommand"),
	SHOW_SCROLL_BAR					(m_sName,"ShowScrollBar"),
	SCROLL_BAR_HEIGHT				(m_sName,"ScrollBarHeight"),
	SCROLL_BAR_TIME					(m_sName,"ScrollBarTime"),
	EXPLANATION_ZOOM				(m_sName,"ExplanationZoom"),
	SHOW_EXIT_ROW					(m_sName,"ShowExitRow"),
	SEPARATE_EXIT_ROW				(m_sName,"SeparateExitRow"),
	SEPARATE_EXIT_ROW_Y				(m_sName,"SeparateExitRowY"),
	SHOW_EXPLANATIONS				(m_sName,"ShowExplanations"),
	ALLOW_REPEATING_CHANGE_VALUE_INPUT(m_sName,"AllowRepeatingChangeValueInput"),
	CURSOR_TWEEN_SECONDS			(m_sName,"CursorTweenSeconds")
{
	m_fLockInputSecs = 0.0001f;	// always lock for a tiny amount of time so that we throw away any queued inputs during the load.
	
	LOG->Trace( "ScreenOptions::ScreenOptions()" );
}


void ScreenOptions::Init()
{
	ScreenWithMenuElements::Init();

	m_OptionsNavigation = PREFSMAN->m_bArcadeOptionsNavigation? NAV_THREE_KEY:NAV_FIVE_KEY;

	m_SoundChangeCol.Load( THEME->GetPathS(m_sName,"change"), true );
	m_SoundNextRow.Load( THEME->GetPathS(m_sName,"next"), true );
	m_SoundPrevRow.Load( THEME->GetPathS(m_sName,"prev"), true );
	m_SoundToggleOn.Load( THEME->GetPathS(m_sName,"toggle on"), true );
	m_SoundToggleOff.Load( THEME->GetPathS(m_sName,"toggle off"), true );

	// add everything to m_framePage so we can animate everything at once
	m_framePage.SetName( "Frame" );
	this->AddChild( &m_framePage );

	m_bMoreShown = false;
	FOREACH_PlayerNumber( p )
	{
		m_iCurrentRow[p] = -1;
		m_iFocusX[p] = -1;
		m_bWasOnExit[p] = false;
		m_bGotAtLeastOneStartPressed[p] = false;
	}

	ON_COMMAND( m_framePage );
}

void ScreenOptions::LoadOptionIcon( PlayerNumber pn, int iRow, CString sText )
{
	m_pRows[iRow]->LoadOptionIcon( pn, sText );
}

void ScreenOptions::InitMenu( InputMode im, const vector<OptionRowDefinition> &vDefs, const vector<OptionRowHandler*> &vHands )
{
	LOG->Trace( "ScreenOptions::Set()" );

	ASSERT( vDefs.size() == vHands.size() );

	m_InputMode = im;

	for( unsigned r=0; r<vDefs.size(); r++ )		// foreach row
	{
		m_pRows.push_back( new OptionRow() );
		OptionRow &row = *m_pRows.back();
		const OptionRowDefinition &def = vDefs[r];
		OptionRowHandler* hand = vHands[r];
		
		bool bFirstRowGoesDown = m_OptionsNavigation==NAV_TOGGLE_THREE_KEY;

		row.LoadMetrics( m_sName );
		row.LoadNormal( def, hand, bFirstRowGoesDown );

		vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		this->ImportOptions( r, vpns );
	
		CHECKPOINT_M( ssprintf("row %i: %s", r, row.GetRowDef().name.c_str()) );

		row.AfterImportOptions();
	}

	m_sprPage.Load( THEME->GetPathG(m_sName,"page") );
	m_sprPage->SetName( "Page" );
	SET_XY_AND_ON_COMMAND( m_sprPage );
	m_framePage.AddChild( m_sprPage );

	// init line line highlights
	FOREACH_HumanPlayer( p )
	{
		m_sprLineHighlight[p].Load( THEME->GetPathG(m_sName,"line highlight") );
		m_sprLineHighlight[p].SetName( "LineHighlight" );
		m_sprLineHighlight[p].SetX( SCREEN_CENTER_X );
		m_framePage.AddChild( &m_sprLineHighlight[p] );
		ON_COMMAND( m_sprLineHighlight[p] );
	}
	
	// init cursors
	FOREACH_HumanPlayer( p )
	{
		m_Cursor[p].Load( m_sName, OptionsCursor::cursor );
		m_Cursor[p].Set( p );
		m_framePage.AddChild( &m_Cursor[p] );
	}
	
	for( unsigned r=0; r<m_pRows.size(); r++ )		// foreach row
	{
		OptionRow &row = *m_pRows[r];
		m_framePage.AddChild( &row );
	}

	if( SHOW_EXIT_ROW )
	{
		// TRICKY:  Add "EXIT" item
		m_pRows.push_back( new OptionRow() );
		OptionRow &row = *m_pRows.back();
		row.LoadMetrics( m_sName );
		row.LoadExit();
		m_framePage.AddChild( &row );
	}

	// add explanation here so it appears on top
	FOREACH_PlayerNumber( p )
	{
		m_textExplanation[p].LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
		m_textExplanation[p].SetZoom( EXPLANATION_ZOOM );
		m_textExplanation[p].SetShadowLength( 0 );
		m_framePage.AddChild( &m_textExplanation[p] );
	}

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		FOREACH_PlayerNumber( p )
			m_ScrollBar.EnablePlayer( p, GAMESTATE->IsHumanPlayer(p) );
		m_ScrollBar.Load( "DualScrollBar" );
		SET_XY( m_ScrollBar );
		m_framePage.AddChild( &m_ScrollBar );
	}

	m_sprMore.Load( THEME->GetPathG( m_sName,"more") );
	m_sprMore->SetName( "More" );
	SET_XY_AND_ON_COMMAND( m_sprMore );
	COMMAND( m_sprMore, m_bMoreShown? "ShowMore":"HideMore" );
	m_framePage.AddChild( m_sprMore );

	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		{
			FOREACH_PlayerNumber( p )
				m_textExplanation[p].SetXY( EXPLANATION_X.GetValue(p), EXPLANATION_Y.GetValue(p) );
		}
		break;
	case INPUTMODE_SHARE_CURSOR:
		m_textExplanation[0].SetXY( EXPLANATION_TOGETHER_X, EXPLANATION_TOGETHER_Y );
		break;
	default:
		ASSERT(0);
	}

	FOREACH_PlayerNumber( p )
	{
		m_sprDisqualify[p].Load( THEME->GetPathG(m_sName,"disqualify") );
		m_sprDisqualify[p]->SetName( ssprintf("DisqualifyP%i",p+1) );
		SET_XY_AND_ON_COMMAND( m_sprDisqualify[p] );
		m_sprDisqualify[p]->SetHidden( true );	// unhide later if handicapping options are discovered
		m_framePage.AddChild( m_sprDisqualify[p] );
	}

	// poke once at all the explanation metrics so that we catch missing ones early
	for( int r=0; r<(int)m_pRows.size(); r++ )		// foreach row
	{
		GetExplanationText( r );
	}

	// put focus on the first enabled row
	FOREACH_PlayerNumber( p )
	{
		for( unsigned r=0; r<m_pRows.size(); r++ )
		{
			const OptionRow &row = *m_pRows[r];
			if( row.GetRowDef().IsEnabledForPlayer(p) )
			{
				m_iCurrentRow[p] = r;
				break;
			}
		}
	}

	// Hide highlight if no rows are enabled.
	FOREACH_HumanPlayer( p )
		if( m_iCurrentRow[p] == -1 )
			m_sprLineHighlight[p].SetHidden( true );


	CHECKPOINT;

	PositionItems();
	PositionAllUnderlines();
	PositionIcons();
	RefreshAllIcons();
	PositionCursors();
	UpdateEnabledDisabled();

	FOREACH_PlayerNumber( p )
		OnChange( p );

	CHECKPOINT;

	/* It's tweening into position, but on the initial tween-in we only want to
	 * tween in the whole page at once.  Since the tweens are nontrivial, it's
	 * easiest to queue the tweens and then force them to finish. */
	for( int r=0; r<(int) m_pRows.size(); r++ )	// foreach options line
	{
		OptionRow &row = *m_pRows[r];
		row.FinishTweening();
	}

	m_sprMore->FinishTweening();

	this->SortByDrawOrder();
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
	for( unsigned i=0; i<m_pRows.size(); i++ )
		SAFE_DELETE( m_pRows[i] );
}

CString ScreenOptions::GetExplanationText( int iRow ) const
{
	OptionRow &row = *m_pRows[iRow];

	CString sLineName = row.GetRowDef().name;
	ASSERT( !sLineName.empty() );
	return SHOW_EXPLANATIONS ? OPTION_EXPLANATION(sLineName) : "";
}

BitmapText &ScreenOptions::GetTextItemForRow( PlayerNumber pn, int iRow, int iChoiceOnRow )
{
	ASSERT_M( iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );
	OptionRow &row = *m_pRows[iRow];
	return row.GetTextItemForRow( pn, iChoiceOnRow );
}

void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut )
{
	ASSERT_M( iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );
	OptionRow &row = *m_pRows[iRow];
	row.GetWidthXY( pn, iChoiceOnRow, iWidthOut, iXOut, iYOut );
}

void ScreenOptions::PositionUnderlines( int r, PlayerNumber pn )
{
	OptionRow &row = *m_pRows[r];
	row.PositionUnderlines( pn );
}

void ScreenOptions::PositionAllUnderlines()
{
	for( unsigned r=0; r<m_pRows.size(); r++ )
		FOREACH_HumanPlayer( p )
			PositionUnderlines( r, p );
}

void ScreenOptions::PositionIcons()
{
	for( unsigned i=0; i<m_pRows.size(); i++ )	// foreach options line
	{
		OptionRow &row = *m_pRows[i];
		row.PositionIcons();
			continue;
	}
}

void ScreenOptions::RefreshIcons( int row, PlayerNumber pn )
{
	// overridden by ScreenOptionsMaster
}

void ScreenOptions::RefreshAllIcons()
{
	FOREACH_HumanPlayer( p )
        for( unsigned r=0; r<m_pRows.size(); ++r )
			this->RefreshIcons( r, p );
}

void ScreenOptions::PositionCursors()
{
	// Set the position of the cursor showing the current option the user is changing.
	// Set the position of the underlines showing the current choice for each option line.
	FOREACH_HumanPlayer( pn )
	{
		const int iRow = m_iCurrentRow[pn];
		if( iRow == -1 )
			continue;

		ASSERT_M( iRow >= 0 && iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );
		OptionRow &OptionRow = *m_pRows[iRow];
		OptionsCursor &cursor = m_Cursor[pn];

		const int iChoiceWithFocus = OptionRow.GetChoiceInRowWithFocus(pn);
		if( iChoiceWithFocus == -1 )
			continue;	// skip

		int iWidth, iX, iY;
		GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );
		cursor.SetBarWidth( iWidth );
		cursor.SetXY( (float)iX, (float)iY );
	}
}

void ScreenOptions::TweenCursor( PlayerNumber pn )
{
	// Set the position of the cursor showing the current option the user is changing.
	const int iRow = m_iCurrentRow[pn];
	ASSERT_M( iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );

	const OptionRow &row = *m_pRows[iRow];
	const int iChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);

	int iWidth, iX, iY;
	GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );

	OptionsCursor &cursor = m_Cursor[pn];
	if( cursor.GetDestX() != (float) iX || cursor.GetDestY() != (float) iY )
	{
		cursor.StopTweening();
		cursor.BeginTweening( CURSOR_TWEEN_SECONDS );
		cursor.SetXY( (float)iX, (float)iY );
	}

	cursor.SetBarWidth( iWidth );

	if( GAMESTATE->IsHumanPlayer(pn) )  
	{
		COMMAND( m_sprLineHighlight[pn], "Change" );
		if( row.GetRowType() == OptionRow::ROW_EXIT )
			COMMAND( m_sprLineHighlight[pn], "ChangeToExit" );
		m_sprLineHighlight[pn].SetY( (float)iY );
	}
}

/* For "long row-style" rows, update the text on screen to contain the currently-
 * focused options. 
 *
 * This used to update a single player, but it's not always clear what that means
 * when dealing with bOneChoiceForAllPlayers and disabled players, which was brittle.
 * Update the whole row. */
void ScreenOptions::UpdateText( int iRow )
{
	OptionRow &row = *m_pRows[iRow];
	row.UpdateText();
}

void ScreenOptions::UpdateEnabledDisabled( int r )
{
	OptionRow &row = *m_pRows[r];

	bool bRowHasFocus[NUM_PLAYERS];
	FOREACH_PlayerNumber( pn )
		bRowHasFocus[pn] = GAMESTATE->IsHumanPlayer(pn) && m_iCurrentRow[pn] == (int)r;

	row.SetRowFocus( bRowHasFocus );
	row.UpdateEnabledDisabled();
}

void ScreenOptions::UpdateEnabledDisabled()
{
	for( unsigned r=0; r<m_pRows.size(); r++ )
		UpdateEnabledDisabled( r );
}

void ScreenOptions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenOptions::Update(%f)", fDeltaTime );

	m_fLockInputSecs = max( 0, m_fLockInputSecs-fDeltaTime );

	ScreenWithMenuElements::Update( fDeltaTime );
}

void ScreenOptions::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Cancel.IsTransitioning() || m_Out.IsTransitioning() || m_fLockInputSecs > 0 )
		return;

	if( !GAMESTATE->IsHumanPlayer(MenuI.player) )
		return;

	if( type == IET_RELEASE )
	{
		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
		case MENU_BUTTON_SELECT:
		case MENU_BUTTON_RIGHT:
		case MENU_BUTTON_LEFT:
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_START) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_LEFT) );
		}
	}

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		this->BeginFadingOut();
		break;
	case SM_GoToPrevScreen:
//		this->ExportOptions();	// Don't save options if we're going back!
		this->GoToPrevScreen();
		break;
	case SM_GoToNextScreen:
		for( unsigned r=0; r<m_pRows.size(); r++ )		// foreach row
		{
			vector<PlayerNumber> vpns;
			FOREACH_HumanPlayer( p )
				vpns.push_back( p );
			this->ExportOptions( r, vpns );
		}
		this->GoToNextScreen();
		break;
	case SM_BeginFadingOut:
		if(IsTransitioning())
			return; /* already transitioning */
		StartTransitioning( SM_GoToNextScreen );

		SCREENMAN->PlayStartSound();

		OFF_COMMAND( m_framePage );
		break;
	case SM_GainFocus:
		INPUTFILTER->SetRepeatRate( 0.25f, 12, 0.25f, 12 );
		break;
	case SM_LoseFocus:
		INPUTFILTER->ResetRepeatRate();
		break;
	}
}


void ScreenOptions::PositionItems()
{
	const int total = NUM_ROWS_SHOWN;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	vector<OptionRow*> Rows( m_pRows );
	OptionRow *ExitRow = NULL;

	if( (bool)SEPARATE_EXIT_ROW && Rows.back()->GetRowType() == OptionRow::ROW_EXIT )
	{
		ExitRow = &*Rows.back();

		/* Remove the exit row for purposes of positioning everything else. */
		if( P1Choice == (int) Rows.size()-1 )
			--P1Choice;
		if( P2Choice == (int) Rows.size()-1 )
			--P2Choice;

		Rows.erase( Rows.begin()+Rows.size()-1, Rows.end() );
	}

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( m_InputMode == INPUTMODE_SHARE_CURSOR || !BothPlayersActivated )
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

	first_end = min( first_end, (int) Rows.size() );
	second_end = min( second_end, (int) Rows.size() );

	/* If less than total (and Rows.size()) are displayed, fill in the empty
	 * space intelligently. */
	while(1)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= (int) Rows.size() || sum >= total)
			break; /* nothing more to display, or no room */

		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
			second_start--;
		/* Otherwise, expand either end. */
		else if( first_start > 0 )
			first_start--;
		else if( second_end < (int) Rows.size() )
			second_end++;
		else
			ASSERT(0); /* do we have room to grow or don't we? */
	}

	int pos = 0;
	for( int i=0; i<(int) Rows.size(); i++ )		// foreach row
	{
		float fY;
		if( i < first_start )
			fY = ROW_Y_OFF_SCREEN_TOP;
		else if( i < first_end )
			fY = ROW_Y.GetValue( pos++ );
		else if( i < second_start )
			fY = ROW_Y_OFF_SCREEN_CENTER;
		else if( i < second_end )
			fY = ROW_Y.GetValue( pos++ );
		else
			fY = ROW_Y_OFF_SCREEN_BOTTOM;
			
		OptionRow &row = *Rows[i];

		row.SetRowY( fY );
		bool bHidden = 
			i < first_start ||
			(i >= first_end && i < second_start) ||
			i >= second_end;
		row.SetRowHidden( bHidden );
	}

	if( ExitRow )
	{
		ExitRow->SetRowY( SEPARATE_EXIT_ROW_Y );
		ExitRow->SetRowHidden( second_end != (int) Rows.size() );
	}
}


void ScreenOptions::OnChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	const int iCurRow = m_iCurrentRow[pn];

	if( iCurRow == -1 )
		return;
	
	/* Update m_fY and m_bHidden[]. */
	PositionItems();

	/* Do positioning. */
	PositionAllUnderlines();
	RefreshIcons( iCurRow, pn );
	PositionIcons();
	UpdateEnabledDisabled();

	if( SHOW_SCROLL_BAR )
	{
		float fPercent = 0;
		if( m_pRows.size() > 1 )
			fPercent = m_iCurrentRow[pn] / float(m_pRows.size()-1);
		m_ScrollBar.SetPercentage( pn, fPercent );
	}


	/* If the last row is EXIT, and is hidden, then show MORE. */
	const bool ShowMore = m_pRows.back()->GetRowType() == OptionRow::ROW_EXIT && m_pRows.back()->GetRowHidden();
	if( m_bMoreShown != ShowMore )
	{
		m_bMoreShown = ShowMore;
		COMMAND( m_sprMore, m_bMoreShown? "ShowMore":"HideMore" );
	}

	/* Update all players, since changing one player can move both cursors. */
	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsHumanPlayer(p) )
			TweenCursor( p );

		int iCurrentRow = m_iCurrentRow[pn];
		OptionRow &row = *m_pRows[iCurrentRow];
		const bool ExitSelected = row.GetRowType() == OptionRow::ROW_EXIT;
		if( p == pn || GAMESTATE->GetNumHumanPlayers() == 1 )
		{
			if( m_bWasOnExit[p] != ExitSelected )
			{
				m_bWasOnExit[p] = ExitSelected;
				COMMAND( m_sprMore, ssprintf("Exit%sP%i", ExitSelected? "Selected":"Unselected", p+1) );
			}
		}
	}

	const CString text = GetExplanationText( iCurRow );

	BitmapText *pText = NULL;
	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		pText = &m_textExplanation[pn];
		if( pText->GetText() != text )
		{
			pText->FinishTweening();
			pText->RunCommands( EXPLANATION_ON_COMMAND.GetValue(pn) );
			pText->SetText( text );
		}
		break;
	case INPUTMODE_SHARE_CURSOR:
		pText = &m_textExplanation[0];
		if( pText->GetText() != text )
		{
			pText->FinishTweening();
			pText->RunCommands( EXPLANATION_TOGETHER_ON_COMMAND );
			pText->SetText( text );
		}
		break;
	}
}


void ScreenOptions::MenuBack( PlayerNumber pn )
{
	Screen::MenuBack( pn );

	Cancel( SM_GoToPrevScreen );
}

bool ScreenOptions::IsOnLastRow( PlayerNumber pn ) const
{
	int iCurRow = m_iCurrentRow[pn];
	return iCurRow == (int)(m_pRows.size()-1);
}

bool ScreenOptions::AllAreOnLastRow() const
{
	FOREACH_HumanPlayer( p )
	{
		if( !IsOnLastRow(p) )
			return false;
	}
	return true;
}

void ScreenOptions::MenuStart( PlayerNumber pn, const InputEventType type )
{
	switch( type )
	{
	case IET_FIRST_PRESS:
		m_bGotAtLeastOneStartPressed[pn] = true;
		break;
	case IET_RELEASE:
		return;	// ignore
	default:	// repeat type
		if( !m_bGotAtLeastOneStartPressed[pn] )
			return;	// don't allow repeat
		break;
	}
	
	int iCurRow = m_iCurrentRow[pn];
	OptionRow &row = *m_pRows[iCurRow];
	

	/* If we are in a three-button mode, check to see if MENU_BUTTON_LEFT and
	 * MENU_BUTTON_RIGHT are being held. */
	switch( m_OptionsNavigation )
	{
	case NAV_THREE_KEY:
	case NAV_TOGGLE_THREE_KEY:
		{
			bool bHoldingLeftAndRight = 
				INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) &&
				INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) );
			if( bHoldingLeftAndRight )
			{
				MoveRow( pn, -1, type != IET_FIRST_PRESS );		
				return;
			}
		}
	}

	//
	// Check whether Start ends this screen.
	//
	{
		bool bEndThisScreen = false;

		// If there's no exit row, then pressing Start on any row ends the screen.
		if( !SHOW_EXIT_ROW )
			bEndThisScreen = true;

		// If all players are on "Exit"
		if( AllAreOnLastRow() )
			bEndThisScreen = true;

		/* Don't accept START to go to the next screen if we're still transitioning in. */
		if( type != IET_FIRST_PRESS || IsTransitioning() )
			bEndThisScreen = false;
		
		if( bEndThisScreen )
		{
			this->BeginFadingOut();
			return;
		}
	}

	if( row.GetFirstItemGoesDown() )
	{
		int iChoiceInRow = row.GetChoiceInRowWithFocus(pn);
		if( iChoiceInRow == 0 )
		{
			MenuDown( pn, type );
			return;
		}
	}
	
	if( row.GetRowDef().selectType == SELECT_MULTIPLE )
	{
		int iChoiceInRow = row.GetChoiceInRowWithFocus(pn);
		bool bSelected = !row.GetSelected( pn, iChoiceInRow );
		row.SetSelected( pn, iChoiceInRow, bSelected );

		if( bSelected )
			m_SoundToggleOn.Play();
		else
			m_SoundToggleOff.Play();

		PositionUnderlines( iCurRow, pn );
		RefreshIcons( iCurRow, pn );

		if( row.GetFirstItemGoesDown() )
		{
			// move to the first choice in the row
			ChangeValueInRow( pn, -row.GetChoiceInRowWithFocus(pn), type != IET_FIRST_PRESS );
		}
	}
	else	// data.selectType != SELECT_MULTIPLE
	{
		switch( m_OptionsNavigation )
		{
		case NAV_THREE_KEY:
			// don't wrap
			if( iCurRow == (int)m_pRows.size()-1 )
				return;
			MenuDown( pn, type );
			break;
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( row.GetRowDef().selectType != SELECT_MULTIPLE )
			{
				int iChoiceInRow = row.GetChoiceInRowWithFocus(pn);
				if( row.GetRowDef().bOneChoiceForAllPlayers )
					row.SetOneSharedSelection( iChoiceInRow );
				else
					row.SetOneSelection( pn, iChoiceInRow );
			}
			if( row.GetFirstItemGoesDown() )
				ChangeValueInRow( pn, -row.GetChoiceInRowWithFocus(pn), type != IET_FIRST_PRESS );	// move to the first choice
			else
				ChangeValueInRow( pn, 0, type != IET_FIRST_PRESS );
			break;
		case NAV_THREE_KEY_MENU:
			/* Don't accept START to go to the next screen if we're still transitioning in. */
			if( type == IET_FIRST_PRESS && !IsTransitioning() )
				this->BeginFadingOut();
			break;
		case NAV_FIVE_KEY:
			/* Jump to the exit row.  (If everyone's already on the exit row, then
			 * we'll have already gone to the next screen above.) */
			MoveRow( pn, m_pRows.size()-m_iCurrentRow[pn]-1, type != IET_FIRST_PRESS );
			break;
		}
	}
}

void ScreenOptions::StoreFocus( PlayerNumber pn )
{
	/* Long rows always put us in the center, so don't update the focus. */
	int iCurrentRow = m_iCurrentRow[pn];
	const OptionRow &row = *m_pRows[iCurrentRow];
	if( row.GetRowDef().layoutType == LAYOUT_SHOW_ONE_IN_ROW )
		return;

	int iWidth, iY;
	GetWidthXY( pn, m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), iWidth, m_iFocusX[pn], iY );
	LOG->Trace("cur selection %ix%i @ %i", 
		m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), m_iFocusX[pn]);
}

/* Left/right */
void ScreenOptions::ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat )
{
	const int iCurRow = m_iCurrentRow[pn];
	if( iCurRow == -1	)	// no row selected
		return;		// don't allow a move

	OptionRow &row = *m_pRows[iCurRow];
	
	const int iNumChoices = row.GetRowDef().choices.size();

	if( m_OptionsNavigation == NAV_THREE_KEY_MENU && iNumChoices <= 1 )	// 1 or 0
	{
		/* There are no other options on the row; move up or down instead of left and right.
		 * This allows navigating the options menu with left/right/start. 
		 *
		 * XXX: Only allow repeats if the opposite key isn't pressed; otherwise, holding both
		 * directions will repeat in place continuously, which is weird. */
		MoveRow( pn, iDelta, Repeat );
		return;
	}

	if( iNumChoices <= 1 )	// nowhere to move
		return;

	if( Repeat && !ALLOW_REPEATING_CHANGE_VALUE_INPUT )
		return;

	bool bOneChanged = false;


	int iCurrentChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
	int iNewChoiceWithFocus = iCurrentChoiceWithFocus + iDelta;
	ASSERT( iNumChoices > 0 );
	wrap( iNewChoiceWithFocus, iNumChoices );
	
	if( iCurrentChoiceWithFocus != iNewChoiceWithFocus )
		bOneChanged = true;

	row.SetChoiceInRowWithFocus( pn, iNewChoiceWithFocus );
	StoreFocus( pn );

	if( row.GetRowDef().bOneChoiceForAllPlayers )
	{
		/* If this row is bOneChoiceForAllPlayers, then lock the cursors together
		 * for this row.  Don't do this in toggle modes, since the current selection
		 * and the current focus are detached. */
		bool bForceFocusedChoiceTogether = false;
		if( m_OptionsNavigation!=NAV_TOGGLE_THREE_KEY &&
			m_OptionsNavigation!=NAV_TOGGLE_FIVE_KEY &&
			row.GetRowDef().bOneChoiceForAllPlayers )
		{
			bForceFocusedChoiceTogether = true;
		}

		/* Also lock focus if the screen is explicitly set to share cursors. */
		if( m_InputMode == INPUTMODE_SHARE_CURSOR )
			bForceFocusedChoiceTogether = true;

		if( bForceFocusedChoiceTogether )
		{
			// lock focus together
			FOREACH_HumanPlayer( p )
			{
				row.SetChoiceInRowWithFocus( p, iNewChoiceWithFocus );
				StoreFocus( p );
			}
		}

		FOREACH_PlayerNumber( p )
		{
			if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
			{
				;	// do nothing
			}
			else
			{
				if( row.GetRowDef().selectType == SELECT_MULTIPLE )
					;	// do nothing.  User must press Start to toggle the selection.
				else
					row.SetOneSelection( p, iNewChoiceWithFocus );			
			}
		}
	}
	else
	{
		if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
		{
			;	// do nothing
		}
		else
		{
			if( row.GetRowDef().selectType == SELECT_MULTIPLE )
				;	// do nothing.  User must press Start to toggle the selection.
			else
				row.SetOneSelection( pn, iNewChoiceWithFocus );
		}
	}

	UpdateText( iCurRow );

	OnChange( pn );

	if( m_OptionsNavigation != NAV_THREE_KEY_MENU )
		m_SoundChangeCol.Play();

	if( row.GetRowDef().m_bExportOnChange )
	{
		vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		ExportOptions( iCurRow, vpns );
	}
}


/* Up/down */
void ScreenOptions::MoveRow( PlayerNumber pn, int dir, bool Repeat ) 
{
	const int iCurrentRow = m_iCurrentRow[pn];
	OptionRow &row = *m_pRows[iCurrentRow];

	if( row.GetFirstItemGoesDown() )
	{
		// If moving from a bFirstChoiceGoesDown row, put focus back on 
		// the first choice before moving.
		row.SetChoiceInRowWithFocus( pn, 0 );
	}

	LOG->Trace("move pn %i, dir %i, rep %i", pn, dir, Repeat);
	bool changed = false;
	FOREACH_PlayerNumber( p )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL && p != pn )
			continue;	// skip

		int r = m_iCurrentRow[p] + dir;
		if( Repeat && ( r == -1 || r == (int) m_pRows.size() ) )
			continue; // don't wrap while repeating

		wrap( r, m_pRows.size() );

		const unsigned iOldSelection = row.GetChoiceInRowWithFocus(p);
		if( m_iCurrentRow[p] == r )
			continue;

		m_iCurrentRow[p] = r;
		ASSERT( r >= 0 && r < (int)m_pRows.size() );

		OptionRow &row = *m_pRows[r];

		switch( m_OptionsNavigation )
		{
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( row.GetRowDef().layoutType != LAYOUT_SHOW_ONE_IN_ROW )
			{
				int iSelectionDist = -1;
				for( unsigned i = 0; i < row.GetTextItemsSize(); ++i )
				{
					int iWidth, iX, iY;
					GetWidthXY( p, m_iCurrentRow[p], i, iWidth, iX, iY );
					const int iDist = abs( iX-m_iFocusX[p] );
					if( iSelectionDist == -1 || iDist < iSelectionDist )
					{
						iSelectionDist = iDist;
						row.SetChoiceInRowWithFocus( p, i );
					}
				}
				row.SetChoiceInRowWithFocus( p, min( iOldSelection, row.GetTextItemsSize()-1 ) );
			}
		}

		OnChange( p );
		changed = true;
	}
	if( changed )
	{
		if( dir < 0 )
			m_SoundPrevRow.Play();
		else
			m_SoundNextRow.Play();
	}
}

void ScreenOptions::MenuUp( PlayerNumber pn, const InputEventType type )
{
	MenuUpDown( pn, type, -1 );
}

void ScreenOptions::MenuDown( PlayerNumber pn, const InputEventType type )
{
	MenuUpDown( pn, type, +1 );
}

void ScreenOptions::MenuSelect( PlayerNumber pn, const InputEventType type )
{
	MenuUpDown( pn, type, -1 );
}

void ScreenOptions::MenuUpDown( PlayerNumber pn, const InputEventType type, int iDir )
{
	ASSERT( iDir == -1 || iDir == +1 );

	if( type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT )
	{
		/* If down is pressed, don't allow up to repeat, and vice versa.  This prevents
		 * holding both up and down from toggling repeatedly in-place. */
		if( iDir == +1 )
		{
			if( INPUTMAPPER->IsButtonDown(MenuInput(pn, MENU_BUTTON_UP)) ||
				INPUTMAPPER->IsButtonDown(MenuInput(pn, MENU_BUTTON_SELECT)) )
				return;
		}
		else
		{
			if( INPUTMAPPER->IsButtonDown(MenuInput(pn, MENU_BUTTON_DOWN)) ||
				INPUTMAPPER->IsButtonDown(MenuInput(pn, MENU_BUTTON_START)) )
				return;
		}
	}

	bool bRepeat = type != IET_FIRST_PRESS;

	int iDest = -1;
	for( int r=1; r<(int)m_pRows.size(); r++ )
	{
		int iDelta = r*iDir;
		iDest = m_iCurrentRow[pn] + iDelta;
		int iOldDest = iDest;
		ASSERT( m_pRows.size() );
		wrap( iDest, m_pRows.size() );

		// Don't wrap on repeating inputs.
		bool bWrapped = iOldDest != iDest;
		if( bRepeat && bWrapped )
			return;

		OptionRow &row = *m_pRows[iDest];

		if( row.GetRowDef().IsEnabledForPlayer(pn) )
		{
			MoveRow( pn, iDelta, bRepeat );
			return;
		}
	}
}


int ScreenOptions::GetCurrentRow( PlayerNumber pn ) const
{
	const int r = m_iCurrentRow[pn];
	return r;
}


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
