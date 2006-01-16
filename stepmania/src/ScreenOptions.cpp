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
#include "ScreenDimensions.h"
#include "Command.h"
#include "GameCommand.h"
#include "OptionRowHandler.h"
#include "LuaBinding.h"
#include "InputEventPlus.h"


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
 * This is a specialized navigation for ScreenOptionsService.  It must be enabled to
 * allow screens that use rows to select other screens to work with only three
 * buttons.  (It's also used when in five-key mode.)
 *
 * We don't want to simply allow left/right to move up and down on single-entry
 * rows when in NAV_THREE_KEY, becasue left and right shouldn't exit the "exit" row
 * in player options menus, but it should in the options menu.
 */

static CString EXPLANATION_X_NAME( size_t p )			{ return ssprintf("ExplanationP%dX",int(p+1)); }
static CString EXPLANATION_Y_NAME( size_t p )			{ return ssprintf("ExplanationP%dY",int(p+1)); }
static CString EXPLANATION_ON_COMMAND_NAME( size_t p )	{ return ssprintf("ExplanationP%dOnCommand",int(p+1)); }

static CString OPTION_EXPLANATION( CString s )
{
	return THEME->GetString("OptionExplanations",s);
}

//REGISTER_SCREEN_CLASS( ScreenOptions );	// can't be instantiated
ScreenOptions::ScreenOptions()
{
	m_fLockInputSecs = 0.0001f;	// always lock for a tiny amount of time so that we throw away any queued inputs during the load.
	
	// These can be overridden in a derived Init().
	m_OptionsNavigation = PREFSMAN->m_bArcadeOptionsNavigation? NAV_THREE_KEY:NAV_FIVE_KEY;
	m_InputMode = INPUTMODE_SHARE_CURSOR;
}


void ScreenOptions::Init()
{
	NUM_ROWS_SHOWN.Load( m_sName, "NumRowsShown" );
	ROW_INIT_COMMAND.Load( m_sName, "RowInitCommand" );
	ROW_ON_COMMAND.Load( m_sName, "RowOnCommand" );
	ROW_OFF_COMMAND.Load( m_sName, "RowOffCommand" );
	EXPLANATION_X.Load( m_sName, EXPLANATION_X_NAME, NUM_PLAYERS );
	EXPLANATION_Y.Load( m_sName, EXPLANATION_Y_NAME, NUM_PLAYERS );
	EXPLANATION_ON_COMMAND.Load( m_sName, EXPLANATION_ON_COMMAND_NAME, NUM_PLAYERS );
	EXPLANATION_TOGETHER_X.Load( m_sName, "ExplanationTogetherX" );
	EXPLANATION_TOGETHER_Y.Load( m_sName, "ExplanationTogetherY" );
	EXPLANATION_TOGETHER_ON_COMMAND.Load( m_sName, "ExplanationTogetherOnCommand" );
	SHOW_SCROLL_BAR.Load( m_sName, "ShowScrollBar" );
	SCROLL_BAR_HEIGHT.Load( m_sName, "ScrollBarHeight" );
	SCROLL_BAR_TIME.Load( m_sName, "ScrollBarTime" );
	LINE_HIGHLIGHT_X.Load( m_sName, "LineHighlightX" );
	SHOW_EXIT_ROW.Load( m_sName, "ShowExitRow" );
	SEPARATE_EXIT_ROW.Load( m_sName, "SeparateExitRow" );
	SEPARATE_EXIT_ROW_Y.Load( m_sName, "SeparateExitRowY" );
	SHOW_EXPLANATIONS.Load( m_sName, "ShowExplanations" );
	ALLOW_REPEATING_CHANGE_VALUE_INPUT.Load( m_sName, "AllowRepeatingChangeValueInput" );
	CURSOR_TWEEN_SECONDS.Load( m_sName, "CursorTweenSeconds" );
	WRAP_VALUE_IN_ROW.Load( m_sName, "WrapValueInRow" );

	m_exprRowPositionTransformFunction.SetFromExpression( THEME->GetMetric(m_sName,"RowPositionTransformFunction") );

	ScreenWithMenuElements::Init();

	m_SoundChangeCol.Load( THEME->GetPathS(m_sName,"change"), true );
	m_SoundNextRow.Load( THEME->GetPathS(m_sName,"next"), true );
	m_SoundPrevRow.Load( THEME->GetPathS(m_sName,"prev"), true );
	m_SoundToggleOn.Load( THEME->GetPathS(m_sName,"toggle on"), true );
	m_SoundToggleOff.Load( THEME->GetPathS(m_sName,"toggle off"), true );

	// add everything to m_framePage so we can animate everything at once
	m_framePage.SetName( "Frame" );
	this->AddChild( &m_framePage );

	m_sprPage.Load( THEME->GetPathG(m_sName,"page") );
	m_sprPage->SetName( "Page" );
	SET_XY( m_sprPage );
	m_framePage.AddChild( m_sprPage );

	// init line line highlights
	FOREACH_PlayerNumber( p )
	{
		m_sprLineHighlight[p].Load( THEME->GetPathG(m_sName,"line highlight") );
		m_sprLineHighlight[p]->SetName( "LineHighlight" );
		m_sprLineHighlight[p]->SetX( LINE_HIGHLIGHT_X );
		m_framePage.AddChild( m_sprLineHighlight[p] );
	}

	// init cursors
	FOREACH_PlayerNumber( p )
	{
		m_Cursor[p].Load( m_sName, OptionsCursorPlus::cursor );
		m_Cursor[p].Set( p );
		m_Cursor[p].SetName( "Cursor" );
		m_framePage.AddChild( &m_Cursor[p] );
	}
	
	// on top:
	FOREACH_PlayerNumber( p )
	{
		m_textExplanation[p].LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
		m_textExplanation[p].SetShadowLength( 0 );
		m_textExplanation[p].SetDrawOrder( 2 );
		m_framePage.AddChild( &m_textExplanation[p] );
	}

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

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		m_ScrollBar.Load( "DualScrollBar" );
		FOREACH_PlayerNumber( p )
			m_ScrollBar.EnablePlayer( p, GAMESTATE->IsHumanPlayer(p) );
		SET_XY( m_ScrollBar );
		m_ScrollBar.SetDrawOrder( 2 );
		m_framePage.AddChild( &m_ScrollBar );
	}

	m_sprMore.Load( THEME->GetPathG( m_sName,"more") );
	m_sprMore->SetName( "More" );
	SET_XY( m_sprMore );
	m_sprMore->SetDrawOrder( 2 );
	m_framePage.AddChild( m_sprMore );

	FOREACH_PlayerNumber( p )
	{
		m_sprDisqualify[p].Load( THEME->GetPathG(m_sName,"disqualify") );
		m_sprDisqualify[p]->SetName( ssprintf("DisqualifyP%i",p+1) );
		SET_XY( m_sprDisqualify[p] );
		m_sprDisqualify[p]->SetHidden( true );	// unhide later if handicapping options are discovered
		m_sprDisqualify[p]->SetDrawOrder( 2 );
		m_framePage.AddChild( m_sprDisqualify[p] );
	}

	m_OptionRowType.Load( m_sName );
}

void ScreenOptions::SetOptionIcon( PlayerNumber pn, int iRow, CString sText, GameCommand &gc )
{
	m_pRows[iRow]->SetOptionIcon( pn, sText, gc );
}

void ScreenOptions::InitMenu( const vector<OptionRowDefinition> &vDefs, const vector<OptionRowHandler*> &vHands )
{
	LOG->Trace( "ScreenOptions::InitMenu()" );

	ASSERT( vDefs.size() == vHands.size() );

	for( unsigned i=0; i<m_pRows.size(); i++ )
	{
		m_framePage.RemoveChild( m_pRows[i] );
		m_pRows[i]->DetachHandler();
		SAFE_DELETE( m_pRows[i] );
	}
	m_pRows.clear();

	for( unsigned r=0; r<vDefs.size(); r++ )		// foreach row
	{
		m_pRows.push_back( new OptionRow(&m_OptionRowType) );
		OptionRow &row = *m_pRows.back();
		row.SetDrawOrder( 1 );
		m_framePage.AddChild( &row );

		const OptionRowDefinition &def = vDefs[r];
		OptionRowHandler* hand = vHands[r];
		
		bool bFirstRowGoesDown = m_OptionsNavigation==NAV_TOGGLE_THREE_KEY;

		row.LoadNormal( def, hand, bFirstRowGoesDown );
		row.InitText();
	}

	if( SHOW_EXIT_ROW )
	{
		m_pRows.push_back( new OptionRow(&m_OptionRowType) );
		OptionRow &row = *m_pRows.back();
		row.LoadExit();
		row.InitText();
		row.SetDrawOrder( 1 );
		m_framePage.AddChild( &row );
	}

	m_framePage.SortByDrawOrder();

	FOREACH( OptionRow*, m_pRows, p )
	{
		int iIndex = p - m_pRows.begin();

		Lua *L = LUA->Get();
		LuaHelpers::Push( iIndex, L );
		(*p)->m_pLuaInstance->Set( L, "iIndex" );
		LUA->Release( L );

		(*p)->RunCommands( ROW_INIT_COMMAND );
	}

	// poke once at all the explanation metrics so that we catch missing ones early
	for( int r=0; r<(int)m_pRows.size(); r++ )		// foreach row
	{
		GetExplanationText( r );
	}
}

void ScreenOptions::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	for( unsigned r=0; r<m_pRows.size(); r++ )		// foreach row
	{
		vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		this->ImportOptions( r, vpns );
	
		OptionRow *pRow = m_pRows[r];
		pRow->AfterImportOptions();
	}

	ON_COMMAND( m_framePage );

	FOREACH_PlayerNumber( p )
	{
		m_iCurrentRow[p] = -1;
		m_iFocusX[p] = -1;
		m_bWasOnExit[p] = false;
		m_bGotAtLeastOneStartPressed[p] = false;
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

	FOREACH_PlayerNumber( p )
	{
		m_sprLineHighlight[p]->SetHidden( !GAMESTATE->IsHumanPlayer(p) );
		m_Cursor[p].SetHidden( !GAMESTATE->IsHumanPlayer(p) );
	}

	// Hide highlight if no rows are enabled.
	FOREACH_HumanPlayer( p )
		m_sprLineHighlight[p]->SetHidden( m_iCurrentRow[p] == -1 );


	CHECKPOINT;

	PositionRows();
	PositionAllUnderlines();
	PositionIcons();
	RefreshAllIcons();
	PositionCursors();
	UpdateEnabledDisabled();

	FOREACH_PlayerNumber( p )
		AfterChangeValueOrRow( p );

	CHECKPOINT;
}

void ScreenOptions::TweenOnScreen()
{
	ScreenWithMenuElements::TweenOnScreen();

	FOREACH( OptionRow*, m_pRows, p )
		(*p)->RunCommands( ROW_ON_COMMAND );

	ON_COMMAND( m_sprPage );
	FOREACH_HumanPlayer( p )
	{
		ON_COMMAND( m_Cursor[p] );
		ON_COMMAND( m_sprLineHighlight[p] );
	}

	ON_COMMAND( m_sprMore );

	FOREACH_PlayerNumber( p )
		ON_COMMAND( m_sprDisqualify[p] );

	m_framePage.SortByDrawOrder();
}

void ScreenOptions::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	FOREACH( OptionRow*, m_pRows, p )
		(*p)->RunCommands( ROW_OFF_COMMAND );

	OFF_COMMAND( m_framePage );
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

	bool bAllowExplanation = row.GetRowDef().m_bAllowExplanation;
	bool bShowExplanations = bAllowExplanation && SHOW_EXPLANATIONS.GetValue();
	if( !bShowExplanations )
		return CString();

	CString sExplanationName = row.GetRowDef().m_sExplanationName;
	if( sExplanationName.empty() )
		sExplanationName = row.GetRowDef().m_sName;
	ASSERT( !sExplanationName.empty() );

	return OPTION_EXPLANATION(sExplanationName);
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

void ScreenOptions::RefreshIcons( int iRow, PlayerNumber pn )
{
	OptionRow &row = *m_pRows[iRow];
	
	const OptionRowDefinition &def = row.GetRowDef();

	// find first selection and whether multiple are selected
	int iFirstSelection = row.GetOneSelection( pn, true );

	// set icon name and bullet
	CString sIcon;
	GameCommand gc;

	if( iFirstSelection == -1 )
	{
		sIcon = "Multi";
	}
	else if( iFirstSelection != -1 )
	{
		const OptionRowHandler *pHand = row.GetHandler();
		if( pHand )
		{
			int iSelection = iFirstSelection+(m_OptionsNavigation==NAV_TOGGLE_THREE_KEY?-1:0);
			pHand->GetIconTextAndGameCommand( def, iSelection, sIcon, gc );
		}
	}
	

	/* XXX: hack to not display text in the song options menu */
	if( def.m_bOneChoiceForAllPlayers )
		sIcon = "";

	SetOptionIcon( pn, iRow, sIcon, gc );
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
		const OptionRow &row = *m_pRows[iRow];
		OptionsCursorPlus &cursor = m_Cursor[pn];

		const int iChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
		if( iChoiceWithFocus == -1 )
			continue;	// skip

		int iWidth, iX, iY;
		GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );
		cursor.SetBarWidth( iWidth );
		cursor.SetXY( (float)iX, (float)iY );
		bool bCanGoLeft = iChoiceWithFocus > 0;
		bool bCanGoRight = iChoiceWithFocus >= 0 && iChoiceWithFocus < (int) row.GetRowDef().m_vsChoices.size()-1;
		cursor.SetCanGo( bCanGoLeft, bCanGoRight );
	}
}

void ScreenOptions::TweenCursor( PlayerNumber pn )
{
	// Set the position of the cursor showing the current option the user is changing.
	const int iRow = m_iCurrentRow[pn];
	ASSERT_M( iRow >= 0  &&  iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );

	const OptionRow &row = *m_pRows[iRow];
	const int iChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);

	int iWidth, iX, iY;
	GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );

	OptionsCursorPlus &cursor = m_Cursor[pn];
	if( cursor.GetDestX() != (float) iX  || 
		cursor.GetDestY() != (float) iY  || 
		cursor.GetBarWidth() != iWidth )
	{
		cursor.StopTweening();
		cursor.BeginTweening( CURSOR_TWEEN_SECONDS );
		cursor.SetXY( (float)iX, (float)iY );
		cursor.SetBarWidth( iWidth );
	}


	bool bCanGoLeft = iChoiceWithFocus > 0;
	bool bCanGoRight = iChoiceWithFocus >= 0 && iChoiceWithFocus < (int) row.GetRowDef().m_vsChoices.size()-1;
	cursor.SetCanGo( bCanGoLeft, bCanGoRight );

	if( GAMESTATE->IsHumanPlayer(pn) )  
	{
		COMMAND( m_sprLineHighlight[pn], "Change" );
		if( row.GetRowType() == OptionRow::RowType_Exit )
			COMMAND( m_sprLineHighlight[pn], "ChangeToExit" );
		m_sprLineHighlight[pn]->SetY( (float)iY );
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

	FOREACH_PlayerNumber( pn )
		row.SetRowHasFocus( pn, GAMESTATE->IsHumanPlayer(pn) && m_iCurrentRow[pn] == (int)r );

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

void ScreenOptions::Input( const InputEventPlus &input )
{
	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Cancel.IsTransitioning() || m_Out.IsTransitioning() || m_fLockInputSecs > 0 )
		return;

	if( !GAMESTATE->IsHumanPlayer(input.MenuI.player) )
		return;

	if( input.type == IET_RELEASE )
	{
		switch( input.MenuI.button )
		{
		case MENU_BUTTON_START:
		case MENU_BUTTON_SELECT:
		case MENU_BUTTON_RIGHT:
		case MENU_BUTTON_LEFT:
			INPUTMAPPER->ResetKeyRepeat( MenuInput(input.MenuI.player, MENU_BUTTON_START) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(input.MenuI.player, MENU_BUTTON_RIGHT) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(input.MenuI.player, MENU_BUTTON_LEFT) );
		}
	}

	// default input handler
	Screen::Input( input );
}

void ScreenOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer )
	{
		this->BeginFadingOut();
	}
	else if( SM == SM_BeginFadingOut )
	{
		if( IsTransitioning() )
			return; /* already transitioning */
		StartTransitioningScreen( SM_ExportOptions );

		SCREENMAN->PlayStartSound();
	}
	else if( SM == SM_ExportOptions )
	{
		vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		for( unsigned r=0; r<m_pRows.size(); r++ )		// foreach row
			this->ExportOptions( r, vpns );

		this->HandleScreenMessage( SM_GoToNextScreen );
	}
	else if( SM == SM_GainFocus )
	{
		INPUTFILTER->SetRepeatRate( 0.25f, 0.25f, 12 );
	}
	else if( SM == SM_LoseFocus )
	{
		INPUTFILTER->ResetRepeatRate();
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}


void ScreenOptions::PositionRows()
{
	const int total = NUM_ROWS_SHOWN;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	/* Choices for each player.  If only one player is active, it's the same for both. */
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	vector<OptionRow*> Rows( m_pRows );
	OptionRow *pSeparateExitRow = NULL;

	if( (bool)SEPARATE_EXIT_ROW && !Rows.empty() && Rows.back()->GetRowType() == OptionRow::RowType_Exit )
	{
		pSeparateExitRow = &*Rows.back();

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
	}
	else
	{
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
		OptionRow &row = *Rows[i];

		float fPos = (float) pos;

		LuaExpressionTransform *pExpr = NULL;
		pExpr = &m_exprRowPositionTransformFunction;

		if( i < first_start )				fPos = -0.5f;
		else if( i >= first_end && i < second_start )	fPos = ((int)NUM_ROWS_SHOWN)/2-0.5f;
		else if( i >= second_end )			fPos = ((int)NUM_ROWS_SHOWN)-0.5f;

		row.m_tsDestination = m_exprRowPositionTransformFunction.GetPosition( (float) fPos, i, min( (int)Rows.size(), (int)NUM_ROWS_SHOWN ) );

		bool bHidden = 
			i < first_start ||
			(i >= first_end && i < second_start) ||
			i >= second_end;
		if( !bHidden )
			pos++;
		row.SetRowHidden( bHidden );
	}

	if( pSeparateExitRow )
	{
		pSeparateExitRow->m_tsDestination.pos.y = SEPARATE_EXIT_ROW_Y;
		pSeparateExitRow->SetRowHidden( second_end != (int) Rows.size() );
	}
}


void ScreenOptions::AfterChangeValueOrRow( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	const int iCurRow = m_iCurrentRow[pn];

	if( iCurRow == -1 )
		return;
	
	/* Update m_fY and m_bHidden[]. */
	PositionRows();

	/* Do positioning. */
	PositionAllUnderlines();
	this->RefreshIcons( iCurRow, pn );
	PositionIcons();
	UpdateEnabledDisabled();

	if( SHOW_SCROLL_BAR )
	{
		float fPercent = 0;
		if( m_pRows.size() > 1 )
			fPercent = m_iCurrentRow[pn] / float(m_pRows.size()-1);
		m_ScrollBar.SetPercentage( pn, fPercent );
	}

	/* Update all players, since changing one player can move both cursors. */
	FOREACH_HumanPlayer( p )
		TweenCursor( p );

	FOREACH_PlayerNumber( p )
	{
		int iCurrentRow = m_iCurrentRow[pn];
		OptionRow &row = *m_pRows[iCurrentRow];
		const bool bExitSelected = row.GetRowType() == OptionRow::RowType_Exit;
		if( GAMESTATE->GetNumHumanPlayers() != 1 && p != pn )
			continue;
		if( m_bWasOnExit[p] != bExitSelected )
		{
			m_bWasOnExit[p] = bExitSelected;
			COMMAND( m_sprMore, ssprintf("Exit%sP%i", bExitSelected? "Selected":"Unselected", p+1) );
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

void ScreenOptions::MenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = input.MenuI.player;
	switch( input.type )
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
				MoveRowRelative( pn, -1, input.type != IET_FIRST_PRESS );		
				return;
			}
		}
	}
	
	this->ProcessMenuStart( input );
}

void ScreenOptions::ProcessMenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = input.MenuI.player;
	int iCurRow = m_iCurrentRow[pn];
	OptionRow &row = *m_pRows[iCurRow];

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
		if( input.type != IET_FIRST_PRESS || IsTransitioning() )
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
			MenuDown( input );
			return;
		}
	}
	
	if( row.GetRowDef().m_selectType == SELECT_MULTIPLE )
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
			ChangeValueInRowRelative( m_iCurrentRow[pn], pn, -row.GetChoiceInRowWithFocus(pn), input.type != IET_FIRST_PRESS );
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
			MenuDown( input );
			break;
		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
			if( row.GetRowDef().m_selectType != SELECT_MULTIPLE )
			{
				int iChoiceInRow = row.GetChoiceInRowWithFocus(pn);
				if( row.GetRowDef().m_bOneChoiceForAllPlayers )
					row.SetOneSharedSelection( iChoiceInRow );
				else
					row.SetOneSelection( pn, iChoiceInRow );
			}
			if( row.GetFirstItemGoesDown() )
				ChangeValueInRowRelative( m_iCurrentRow[pn], pn, -row.GetChoiceInRowWithFocus(pn), input.type != IET_FIRST_PRESS );	// move to the first choice
			else
				ChangeValueInRowRelative( m_iCurrentRow[pn], pn, 0, input.type != IET_FIRST_PRESS );
			break;
		case NAV_THREE_KEY_MENU:
			/* Don't accept START to go to the next screen if we're still transitioning in. */
			if( input.type == IET_FIRST_PRESS && !IsTransitioning() )
				this->BeginFadingOut();
			break;
		case NAV_FIVE_KEY:
			/* Jump to the exit row.  (If everyone's already on the exit row, then
			 * we'll have already gone to the next screen above.) */
			MoveRowRelative( pn, m_pRows.size()-m_iCurrentRow[pn]-1, input.type != IET_FIRST_PRESS );
			break;
		}
	}
}

void ScreenOptions::StoreFocus( PlayerNumber pn )
{
	/* Long rows always put us in the center, so don't update the focus. */
	int iCurrentRow = m_iCurrentRow[pn];
	const OptionRow &row = *m_pRows[iCurrentRow];
	if( row.GetRowDef().m_layoutType == LAYOUT_SHOW_ONE_IN_ROW )
		return;

	int iWidth, iY;
	GetWidthXY( pn, m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), iWidth, m_iFocusX[pn], iY );
	LOG->Trace("cur selection %ix%i @ %i", 
		m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), m_iFocusX[pn]);
}

/* Left/right */
void ScreenOptions::ChangeValueInRowAbsolute( int iRow, PlayerNumber pn, int iChoiceIndex, bool bRepeat )
{
	if( iRow == -1	)	// no row selected
		return;		// don't allow a move

	OptionRow &row = *m_pRows[iRow];
	
	const int iNumChoices = row.GetRowDef().m_vsChoices.size();
	ASSERT( iNumChoices >= 0 && iChoiceIndex < iNumChoices );

	int iCurrentChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
	int iDelta = iChoiceIndex - iCurrentChoiceWithFocus;

	ChangeValueInRowRelative( iRow, pn, iDelta, bRepeat );
}

void ScreenOptions::ChangeValueInRowRelative( int iRow, PlayerNumber pn, int iDelta, bool bRepeat )
{
	if( iRow == -1	)	// no row selected
		return;		// don't allow a move

	OptionRow &row = *m_pRows[iRow];
	
	const int iNumChoices = row.GetRowDef().m_vsChoices.size();

	if( m_OptionsNavigation == NAV_THREE_KEY_MENU && iNumChoices <= 1 )	// 1 or 0
	{
		/* There are no other options on the row; move up or down instead of left and right.
		 * This allows navigating the options menu with left/right/start. 
		 *
		 * XXX: Only allow repeats if the opposite key isn't pressed; otherwise, holding both
		 * directions will repeat in place continuously, which is weird. */
		MoveRowRelative( pn, iDelta, bRepeat );
		return;
	}

	if( iNumChoices <= 1 )	// nowhere to move
		return;

	if( bRepeat && !ALLOW_REPEATING_CHANGE_VALUE_INPUT )
		return;

	bool bOneChanged = false;


	int iCurrentChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
	int iNewChoiceWithFocus = iCurrentChoiceWithFocus + iDelta;
	if( !bRepeat  &&  WRAP_VALUE_IN_ROW.GetValue() )
		wrap( iNewChoiceWithFocus, iNumChoices );
	else
		CLAMP( iNewChoiceWithFocus, 0, iNumChoices-1 );
	
	if( iCurrentChoiceWithFocus != iNewChoiceWithFocus )
		bOneChanged = true;

	row.SetChoiceInRowWithFocus( pn, iNewChoiceWithFocus );
	StoreFocus( pn );

	if( row.GetRowDef().m_bOneChoiceForAllPlayers )
	{
		/* If this row is bOneChoiceForAllPlayers, then lock the cursors together
		 * for this row.  Don't do this in toggle modes, since the current selection
		 * and the current focus are detached. */
		bool bForceFocusedChoiceTogether = false;
		if( m_OptionsNavigation!=NAV_TOGGLE_THREE_KEY &&
			m_OptionsNavigation!=NAV_TOGGLE_FIVE_KEY &&
			row.GetRowDef().m_bOneChoiceForAllPlayers )
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
				if( row.GetRowDef().m_selectType == SELECT_MULTIPLE )
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
			if( row.GetRowDef().m_selectType == SELECT_MULTIPLE )
				;	// do nothing.  User must press Start to toggle the selection.
			else
				row.SetOneSelection( pn, iNewChoiceWithFocus );
		}
	}

	UpdateText( iRow );

	// XXX: What was this NAV_THREE_KEY_MENU special case for?
	if( bOneChanged )
		// if( m_OptionsNavigation != NAV_THREE_KEY_MENU )
			m_SoundChangeCol.Play();

	if( row.GetRowDef().m_bExportOnChange )
	{
		vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		ExportOptions( iRow, vpns );
	}

	this->AfterChangeValueInRow( iRow, pn );
}

void ScreenOptions::AfterChangeValueInRow( int iRow, PlayerNumber pn ) 
{
	AfterChangeValueOrRow( pn );
}

/* Up/down */
void ScreenOptions::MoveRowRelative( PlayerNumber pn, int iDir, bool Repeat ) 
{
	const int iCurrentRow = m_iCurrentRow[pn];
	OptionRow &row = *m_pRows[iCurrentRow];

	if( row.GetFirstItemGoesDown() )
	{
		// If moving from a bFirstChoiceGoesDown row, put focus back on 
		// the first choice before moving.
		row.SetChoiceInRowWithFocus( pn, 0 );
	}

	LOG->Trace("move pn %i, dir %i, rep %i", pn, iDir, Repeat);
	bool changed = false;
	FOREACH_PlayerNumber( p )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL && p != pn )
			continue;	// skip

		int r = m_iCurrentRow[p] + iDir;
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
			if( row.GetRowDef().m_layoutType != LAYOUT_SHOW_ONE_IN_ROW )
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

		AfterChangeRow( p );
		changed = true;
	}
	if( changed )
	{
		if( iDir < 0 )
			m_SoundPrevRow.Play();
		else
			m_SoundNextRow.Play();
	}
}

void ScreenOptions::AfterChangeRow( PlayerNumber pn ) 
{
	AfterChangeValueOrRow( pn );
}

void ScreenOptions::MoveRowAbsolute( PlayerNumber pn, int iRow, bool bRepeat ) 
{
	int iDir = iRow - m_iCurrentRow[pn];
	MoveRowRelative( pn, iDir, bRepeat );
}

void ScreenOptions::MenuUp( const InputEventPlus &input )
{
	MenuUpDown( input, -1 );
}

void ScreenOptions::MenuDown( const InputEventPlus &input )
{
	MenuUpDown( input, +1 );
}

void ScreenOptions::MenuSelect( const InputEventPlus &input )
{
	MenuUpDown( input, -1 );
}

void ScreenOptions::MenuUpDown( const InputEventPlus &input, int iDir )
{
	ASSERT( iDir == -1 || iDir == +1 );
	PlayerNumber pn = input.MenuI.player;

	if( input.type == IET_SLOW_REPEAT || input.type == IET_FAST_REPEAT )
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

	bool bRepeat = input.type != IET_FIRST_PRESS;

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
			MoveRowRelative( pn, iDelta, bRepeat );
			return;
		}
	}
}


int ScreenOptions::GetCurrentRow( PlayerNumber pn ) const
{
	const int r = m_iCurrentRow[pn];
	return r;
}

// lua start
#include "LuaBinding.h"

class LunaScreenOptions: public Luna<ScreenOptions>
{
public:
	LunaScreenOptions() { LUA->Register( Register ); }

	static int GetCurrentRow( T* p, lua_State *L ) { lua_pushnumber( L, p->GetCurrentRow((PlayerNumber) IArg(1)) ); return 1; }
	static void Register( Lua *L )
	{
  		ADD_METHOD( GetCurrentRow );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenOptions, ScreenWithMenuElements )
// lua end


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
