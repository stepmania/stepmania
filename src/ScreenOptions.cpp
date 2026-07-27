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
#include "GameCommand.h"
#include "OptionRowHandler.h"
#include "LuaBinding.h"
#include "InputEventPlus.h"

#include <cmath>
#include <vector>


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
 * NAV_THREE_KEY_ALT:
 *  left, right -> next, prev row
 *  start -> change option
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
 * are used when PREFSMAN->m_iArcadeOptionsNavigation is enabled.  However, they can
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

static RString OPTION_EXPLANATION( RString s )
{
	return THEME->GetString("OptionExplanations",s);
}

static const char *InputModeNames[] = {
	"Individual",
	"Together"
};
StringToX( InputMode );

//REGISTER_SCREEN_CLASS( ScreenOptions );	// can't be instantiated
ScreenOptions::ScreenOptions()
{
	// These can be overridden in a derived Init().
	switch( PREFSMAN->m_iArcadeOptionsNavigation )
	{
	case 0: SetNavigation( NAV_FIVE_KEY ); break;
	case 1: SetNavigation( NAV_THREE_KEY ); break;
	case 2: SetNavigation( NAV_THREE_KEY_ALT ); break;
	}
	m_InputMode = INPUTMODE_SHARE_CURSOR;
}


void ScreenOptions::Init()
{
	NUM_ROWS_SHOWN.Load( m_sName, "NumRowsShown" );
	ROW_INIT_COMMAND.Load( m_sName, "RowInitCommand" );
	ROW_ON_COMMAND.Load( m_sName, "RowOnCommand" );
	ROW_OFF_COMMAND.Load( m_sName, "RowOffCommand" );
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
	OPTION_ROW_NORMAL_METRICS_GROUP.Load( m_sName, "OptionRowNormalMetricsGroup" );
	OPTION_ROW_EXIT_METRICS_GROUP.Load( m_sName, "OptionRowExitMetricsGroup" );

	m_exprRowPositionTransformFunction.SetFromReference( THEME->GetMetricR(m_sName,"RowPositionTransformFunction") );

	ScreenWithMenuElements::Init();

	m_SoundChangeCol.Load( THEME->GetPathS(m_sName,"change"), true );
	m_SoundNextRow.Load( THEME->GetPathS(m_sName,"next"), true );
	m_SoundPrevRow.Load( THEME->GetPathS(m_sName,"prev"), true );
	m_SoundToggleOn.Load( THEME->GetPathS(m_sName,"toggle on"), true );
	m_SoundToggleOff.Load( THEME->GetPathS(m_sName,"toggle off"), true );
	m_SoundStart.Load( THEME->GetPathS(m_sName,"start"), true );

	// add everything to m_frameContainer so we can animate everything at once
	m_frameContainer.SetName( "Container" );
	LOAD_ALL_COMMANDS( m_frameContainer );
	this->AddChild( &m_frameContainer );

	m_sprPage.Load( THEME->GetPathG(m_sName,"page") );
	m_sprPage->SetName( "Page" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_sprPage );
	m_frameContainer.AddChild( m_sprPage );

	// init line highlights
	FOREACH_PlayerNumber( p )
	{
		m_sprLineHighlight[p].Load( THEME->GetPathG(m_sName, ssprintf("LineHighlight P%d",p+1)) );
		m_sprLineHighlight[p]->SetName( ssprintf("LineHighlightP%d",p+1) );
		m_sprLineHighlight[p]->SetX( LINE_HIGHLIGHT_X );
		LOAD_ALL_COMMANDS( m_sprLineHighlight[p] );
		m_frameContainer.AddChild( m_sprLineHighlight[p] );
	}

	// init cursors
	FOREACH_PlayerNumber( p )
	{
		m_Cursor[p].Load( "OptionsCursor" + PlayerNumberToString(p), true );
		m_Cursor[p].SetName( "Cursor" );
		LOAD_ALL_COMMANDS( m_Cursor[p] );
		m_frameContainer.AddChild( &m_Cursor[p] );
	}

	switch( m_InputMode )
	{
	case INPUTMODE_INDIVIDUAL:
		FOREACH_PlayerNumber( p )
		{
			m_textExplanation[p].LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
			m_textExplanation[p].SetDrawOrder( 2 );
			m_textExplanation[p].SetName( "Explanation" + PlayerNumberToString(p) );
			LOAD_ALL_COMMANDS_AND_SET_XY( m_textExplanation[p] );
			m_frameContainer.AddChild( &m_textExplanation[p] );

		}
		break;
	case INPUTMODE_SHARE_CURSOR:
		m_textExplanationTogether.LoadFromFont( THEME->GetPathF(m_sName,"explanation") );
		m_textExplanationTogether.SetDrawOrder( 2 );
		m_textExplanationTogether.SetName( "ExplanationTogether" );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_textExplanationTogether );
		m_frameContainer.AddChild( &m_textExplanationTogether );
		break;
	default:
		FAIL_M(ssprintf("Invalid InputMode: %i", m_InputMode));
	}

	if( SHOW_SCROLL_BAR )
	{
		m_ScrollBar.SetName( "ScrollBar" );
		m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT );
		m_ScrollBar.SetBarTime( SCROLL_BAR_TIME );
		m_ScrollBar.Load( "DualScrollBar" );
		FOREACH_PlayerNumber( p )
			m_ScrollBar.EnablePlayer( p, GAMESTATE->IsHumanPlayer(p) );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_ScrollBar );
		m_ScrollBar.SetDrawOrder( 2 );
		m_frameContainer.AddChild( &m_ScrollBar );
	}

	m_sprMore.Load( THEME->GetPathG( m_sName,"more") );
	m_sprMore->SetName( "More" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_sprMore );
	m_sprMore->SetDrawOrder( 2 );
	m_sprMore->PlayCommand( "LoseFocus" );
	m_frameContainer.AddChild( m_sprMore );

	m_OptionRowTypeNormal.Load( OPTION_ROW_NORMAL_METRICS_GROUP, this );
	m_OptionRowTypeExit.Load( OPTION_ROW_EXIT_METRICS_GROUP, this );
}

void ScreenOptions::InitMenu( const std::vector<OptionRowHandler*> &vHands )
{
	LOG->Trace( "ScreenOptions::InitMenu()" );

	for( unsigned i=0; i<m_pRows.size(); i++ )
	{
		m_frameContainer.RemoveChild( m_pRows[i] );
		SAFE_DELETE( m_pRows[i] );
	}
	m_pRows.clear();

	for( unsigned r=0; r<vHands.size(); r++ )		// foreach row
	{
		m_pRows.push_back( new OptionRow(&m_OptionRowTypeNormal) );
		OptionRow &row = *m_pRows.back();
		row.SetDrawOrder( 1 );
		m_frameContainer.AddChild( &row );

		bool bFirstRowGoesDown = m_OptionsNavigation==NAV_TOGGLE_THREE_KEY;

		row.LoadNormal( vHands[r], bFirstRowGoesDown );
	}

	if( SHOW_EXIT_ROW )
	{
		m_pRows.push_back( new OptionRow(&m_OptionRowTypeExit) );
		OptionRow &row = *m_pRows.back();
		row.LoadExit();
		row.SetDrawOrder( 1 );
		m_frameContainer.AddChild( &row );
	}

	m_frameContainer.SortByDrawOrder();

	int iIndex = 0;
	for (OptionRow *p : m_pRows)
	{
		Lua *L = LUA->Get();
		LuaHelpers::Push( L, iIndex++ );
		p->m_pLuaInstance->Set( L, "iIndex" );
		LUA->Release( L );

		p->RunCommands( ROW_INIT_COMMAND );
	}

	// poke once at all the explanation metrics so that we catch missing ones early
	for( int r=0; r<(int)m_pRows.size(); r++ )		// foreach row
	{
		GetExplanationText( r );
	}
}

/* Call when option rows have been re-initialized. */
void ScreenOptions::RestartOptions()
{
	m_exprRowPositionTransformFunction.ClearCache();
	std::vector<PlayerNumber> vpns;
	FOREACH_HumanPlayer( p )
		vpns.push_back( p );

	for( unsigned r=0; r<m_pRows.size(); r++ )		// foreach row
	{
		OptionRow *pRow = m_pRows[r];
		pRow->Reload();

		this->ImportOptions( r, vpns );

		// HACK: Process disabled players, too, to hide inactive underlines.
		FOREACH_PlayerNumber( p )
			pRow->AfterImportOptions( p );
	}


	FOREACH_PlayerNumber( p )
	{
		m_iCurrentRow[p] = -1;
		m_iFocusX[p] = -1;
		m_bWasOnExit[p] = false;

		// put focus on the first enabled row
		for( unsigned r=0; r<m_pRows.size(); r++ )
		{
			const OptionRow &row = *m_pRows[r];
			if( row.GetRowDef().IsEnabledForPlayer(p) )
			{
				m_iCurrentRow[p] = r;
				break;
			}
		}

		// Hide the highlight if no rows are enabled.
		m_sprLineHighlight[p]->SetVisible( m_iCurrentRow[p] != -1 && GAMESTATE->IsHumanPlayer(p) );
	}


	CHECKPOINT_M("About to get the rows positioned right.");

	PositionRows( false );
	FOREACH_HumanPlayer( pn )
	{
		for( unsigned r=0; r<m_pRows.size(); ++r )
		{
			this->RefreshIcons( r, pn );
		}
		PositionCursor( pn );
	}

	FOREACH_PlayerNumber( p )
	{
		AfterChangeRow( p );
	}
	CHECKPOINT_M("Rows positioned.");
}

void ScreenOptions::BeginScreen()
{
	ScreenWithMenuElements::BeginScreen();

	RestartOptions();

	FOREACH_PlayerNumber( p )
		m_bGotAtLeastOneStartPressed[p] = false;

	ON_COMMAND( m_frameContainer );

	FOREACH_PlayerNumber( p )
	{
		m_Cursor[p].SetVisible( GAMESTATE->IsHumanPlayer(p) );
		ON_COMMAND( m_Cursor[p] );
	}

	this->SortByDrawOrder();
}

void ScreenOptions::TweenOnScreen()
{
	ScreenWithMenuElements::TweenOnScreen();

	for (OptionRow *p : m_pRows)
		p->RunCommands( ROW_ON_COMMAND );

	m_frameContainer.SortByDrawOrder();
}

void ScreenOptions::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	for (OptionRow *p : m_pRows)
		p->RunCommands( ROW_OFF_COMMAND );
}

ScreenOptions::~ScreenOptions()
{
	LOG->Trace( "ScreenOptions::~ScreenOptions()" );
	for( unsigned i=0; i<m_pRows.size(); i++ )
		SAFE_DELETE( m_pRows[i] );
}

RString ScreenOptions::GetExplanationText( int iRow ) const
{
	const OptionRow &row = *m_pRows[iRow];

	bool bAllowExplanation = row.GetRowDef().m_bAllowExplanation;
	bool bShowExplanations = bAllowExplanation && SHOW_EXPLANATIONS.GetValue();
	if( !bShowExplanations )
		return RString();

	RString sExplanationName = row.GetRowDef().m_sExplanationName;
	if( sExplanationName.empty() )
		sExplanationName = row.GetRowDef().m_sName;
	ASSERT( !sExplanationName.empty() );

	return OPTION_EXPLANATION(sExplanationName);
}

void ScreenOptions::GetWidthXY( PlayerNumber pn, int iRow, int iChoiceOnRow, int &iWidthOut, int &iXOut, int &iYOut ) const
{
	ASSERT_M( iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );
	const OptionRow &row = *m_pRows[iRow];
	row.GetWidthXY( pn, iChoiceOnRow, iWidthOut, iXOut, iYOut );
}

void ScreenOptions::RefreshIcons( int iRow, PlayerNumber pn )
{
	OptionRow &row = *m_pRows[iRow];

	const OptionRowDefinition &def = row.GetRowDef();

	// find first selection and whether multiple are selected
	int iFirstSelection = row.GetOneSelection( pn, true );

	// set icon name and bullet
	RString sIcon;
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
			pHand->GetIconTextAndGameCommand( iSelection, sIcon, gc );
		}
	}

	// XXX: hack to not display text in the song options menu
	if( def.m_bOneChoiceForAllPlayers )
		sIcon = "";

	m_pRows[iRow]->SetModIcon( pn, sIcon, gc );
}

void ScreenOptions::PositionCursor( PlayerNumber pn )
{
	// Set the position of the cursor showing the current option the user is changing.
	const int iRow = m_iCurrentRow[pn];
	if( iRow == -1 )
		return;

	ASSERT_M( iRow >= 0 && iRow < (int)m_pRows.size(), ssprintf("%i < %i", iRow, (int)m_pRows.size() ) );
	const OptionRow &row = *m_pRows[iRow];

	const int iChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
	if( iChoiceWithFocus == -1 )
		return;

	int iWidth, iX, iY;
	GetWidthXY( pn, iRow, iChoiceWithFocus, iWidth, iX, iY );

	OptionsCursor &cursor = m_Cursor[pn];
	cursor.SetBarWidth( iWidth );
	cursor.SetXY( (float)iX, (float)iY );
	bool bCanGoLeft = iChoiceWithFocus > 0;
	bool bCanGoRight = iChoiceWithFocus >= 0 && iChoiceWithFocus < (int) row.GetRowDef().m_vsChoices.size()-1;
	cursor.SetCanGo( bCanGoLeft, bCanGoRight );
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

	OptionsCursor &cursor = m_Cursor[pn];
	if( cursor.GetDestX() != (float) iX  ||
		cursor.GetDestY() != (float) iY  ||
		cursor.GetBarWidth() != iWidth )
	{
		cursor.StopTweening();
		cursor.BeginTweening( CURSOR_TWEEN_SECONDS );
		cursor.SetXY( (float)iX, (float)iY );
		cursor.SetBarWidth( iWidth );
	}

	bool bCanGoLeft = false;
	bool bCanGoRight = false;
	switch( row.GetRowDef().m_layoutType  )
	{
		case LAYOUT_SHOW_ONE_IN_ROW:
			bCanGoLeft = iChoiceWithFocus > 0;
			bCanGoRight = iChoiceWithFocus >= 0 && iChoiceWithFocus < (int) row.GetRowDef().m_vsChoices.size()-1;
			break;
		case LAYOUT_SHOW_ALL_IN_ROW:
			break;
		default: break;
	}
	cursor.SetCanGo( bCanGoLeft, bCanGoRight );

	if( GAMESTATE->IsHumanPlayer(pn) )
	{
		COMMAND( m_sprLineHighlight[pn], "Change" );
		if( row.GetRowType() == OptionRow::RowType_Exit )
			COMMAND( m_sprLineHighlight[pn], "ChangeToExit" );

		m_sprLineHighlight[pn]->SetY( (float)iY );
	}
}

void ScreenOptions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenOptions::Update(%f)", fDeltaTime );

	ScreenWithMenuElements::Update( fDeltaTime );
}

bool ScreenOptions::Input( const InputEventPlus &input )
{
	// HACK: This screen eats mouse inputs if we don't check for them first.
	bool mouse_evt = false;
	for (int i = MOUSE_LEFT; i <= MOUSE_WHEELDOWN; i++)
	{
		if (input.DeviceI == DeviceInput( DEVICE_MOUSE, (DeviceButton)i ))
			mouse_evt = true;
	}
	if (mouse_evt)
	{
		return ScreenWithMenuElements::Input(input);
	}

	/* Allow input when transitioning in (m_In.IsTransitioning()), but ignore it
	 * when we're transitioning out. */
	if( m_Cancel.IsTransitioning() || m_Out.IsTransitioning() || m_fLockInputSecs > 0 )
		return false;

	if( !GAMESTATE->IsHumanPlayer(input.pn) )
		return false;

	if( input.type == IET_RELEASE )
	{
		switch( input.MenuI )
		{
			case GAME_BUTTON_START:
			case GAME_BUTTON_SELECT:
			case GAME_BUTTON_MENURIGHT:
			case GAME_BUTTON_MENULEFT:
				INPUTMAPPER->ResetKeyRepeat( GAME_BUTTON_START, input.pn );
				INPUTMAPPER->ResetKeyRepeat( GAME_BUTTON_RIGHT, input.pn );
				INPUTMAPPER->ResetKeyRepeat( GAME_BUTTON_LEFT, input.pn );
			default:
				break;
		}
	}

	// default input handler
	return Screen::Input( input );
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
			return; // already transitioning

		// If the selected option sets a screen, honor it.
		RString sThisScreen = GetNextScreenForFocusedItem( GAMESTATE->GetMasterPlayerNumber() );
		if( sThisScreen != "" )
			m_sNextScreen = sThisScreen;

		// If options set a NextScreen or one is specified in metrics, then fade out
		if( GetNextScreenName() == "" )
		{
			LuaHelpers::ReportScriptErrorFmt( "%s::HandleScreenMessage: Tried to fade out, but we have no next screen", m_sName.c_str() );
			return;
		}

		StartTransitioningScreen( SM_ExportOptions );
	}
	else if( SM == SM_ExportOptions )
	{
		std::vector<PlayerNumber> vpns;
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
		for( unsigned r=0; r<m_pRows.size(); r++ ) // foreach row
		{
			if( m_pRows[r]->GetRowType() == OptionRow::RowType_Exit )
				continue;
			this->ExportOptions( r, vpns );
		}

		this->HandleScreenMessage( SM_GoToNextScreen );
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}


void ScreenOptions::PositionRows( bool bTween )
{
	const int total = NUM_ROWS_SHOWN;
	const int halfsize = total / 2;

	int first_start, first_end, second_start, second_end;

	// Choices for each player. If only one player is active, it's the same for both.
	int P1Choice = GAMESTATE->IsHumanPlayer(PLAYER_1)? m_iCurrentRow[PLAYER_1]: m_iCurrentRow[PLAYER_2];
	int P2Choice = GAMESTATE->IsHumanPlayer(PLAYER_2)? m_iCurrentRow[PLAYER_2]: m_iCurrentRow[PLAYER_1];

	std::vector<OptionRow*> Rows( m_pRows );
	OptionRow *pSeparateExitRow = nullptr;

	if( (bool)SEPARATE_EXIT_ROW && !Rows.empty() && Rows.back()->GetRowType() == OptionRow::RowType_Exit )
	{
		pSeparateExitRow = Rows.back();

		// Remove the exit row for purposes of positioning everything else.
		if( P1Choice == (int) Rows.size()-1 )
			--P1Choice;
		if( P2Choice == (int) Rows.size()-1 )
			--P2Choice;

		Rows.pop_back();
	}

	const bool BothPlayersActivated = GAMESTATE->IsHumanPlayer(PLAYER_1) && GAMESTATE->IsHumanPlayer(PLAYER_2);
	if( m_InputMode == INPUTMODE_SHARE_CURSOR || !BothPlayersActivated )
	{
		// Simply center the cursor.
		first_start = std::max( P1Choice - halfsize, 0 );
		first_end = first_start + total;
		second_start = second_end = first_end;
	}
	else
	{
		// First half:
		const int earliest = std::min( P1Choice, P2Choice );
		first_start = std::max( earliest - halfsize/2, 0 );
		first_end = first_start + halfsize;

		// Second half:
		const int latest = std::max( P1Choice, P2Choice );

		second_start = std::max( latest - halfsize/2, 0 );

		// Don't overlap.
		second_start = std::max( second_start, first_end );

		second_end = second_start + halfsize;
	}

	first_end = std::min( first_end, (int) Rows.size() );
	second_end = std::min( second_end, (int) Rows.size() );

	/* If less than total (and Rows.size()) are displayed, fill in the empty
	 * space intelligently. */
	for(;;)
	{
		const int sum = (first_end - first_start) + (second_end - second_start);
		if( sum >= (int) Rows.size() || sum >= total)
		{
			break; // nothing more to display, or no room
		}
		/* First priority: expand the top of the second half until it meets
		 * the first half. */
		if( second_start > first_end )
		{
			second_start--;
		}
		// Otherwise, expand either end.
		else if( first_start > 0 )
		{
			first_start--;
		}
		else if( second_end < (int) Rows.size() )
		{
			second_end++;
		}
		else
		{
			FAIL_M("Do we have room to grow or don't we?");
		}
	}

	int pos = 0;
	for( int i=0; i<(int) Rows.size(); i++ )		// foreach row
	{
		OptionRow &row = *Rows[i];

		float fPos = (float) pos;

		if( i < first_start )				fPos = -0.5f;
		else if( i >= first_end && i < second_start )	fPos = ((int)NUM_ROWS_SHOWN)/2-0.5f;
		else if( i >= second_end )			fPos = ((int)NUM_ROWS_SHOWN)-0.5f;

		Actor::TweenState tsDestination = m_exprRowPositionTransformFunction.GetTransformCached( fPos, i, std::min( (int)Rows.size(), (int)NUM_ROWS_SHOWN ) );

		bool bHidden =
			i < first_start ||
			(i >= first_end && i < second_start) ||
			i >= second_end;
		for( int j=0; j<NUM_DIFFUSE_COLORS; j++ )
			tsDestination.diffuse[j].a = bHidden? 0.0f:1.0f;
		if( !bHidden )
			pos++;
		row.SetDestination( tsDestination, bTween );
	}

	if( pSeparateExitRow )
	{
		Actor::TweenState tsDestination;
		tsDestination.Init();
		tsDestination.pos.y = SEPARATE_EXIT_ROW_Y;

		for( int j=0; j<NUM_DIFFUSE_COLORS; j++ )
			tsDestination.diffuse[j].a = 1.0f;
		pSeparateExitRow->SetDestination( tsDestination, bTween );
	}
}


void ScreenOptions::AfterChangeValueOrRow( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	const int iCurRow = m_iCurrentRow[pn];

	if( iCurRow == -1 )
		return;

	// Update m_fY and m_bHidden[].
	PositionRows( true );

	// Do positioning.
	RefreshIcons( iCurRow, pn );
	for( unsigned r=0; r<m_pRows.size(); r++ )
	{
		/* After changing a value, position underlines. Do this for both players,
		 * since underlines for both players will change with m_bOneChoiceForAllPlayers. */
		FOREACH_HumanPlayer( p )
			m_pRows[r]->PositionUnderlines( p );
		m_pRows[r]->PositionIcons( pn );
		m_pRows[r]->SetRowHasFocus( pn, GAMESTATE->IsHumanPlayer(pn) && iCurRow == (int)r );
		m_pRows[r]->UpdateEnabledDisabled();
	}

	if( SHOW_SCROLL_BAR )
	{
		float fPercent = 0;
		if( m_pRows.size() > 1 )
			fPercent = iCurRow / float(m_pRows.size()-1);
		m_ScrollBar.SetPercentage( pn, fPercent );
	}

	// Update all players, since changing one player can move both cursors.
	FOREACH_HumanPlayer( p )
		TweenCursor( p );

	FOREACH_PlayerNumber( p )
	{
		OptionRow &row = *m_pRows[iCurRow];
		const bool bExitSelected = row.GetRowType() == OptionRow::RowType_Exit;
		if( GAMESTATE->GetNumHumanPlayers() != 1 && p != pn )
			continue;
		if( m_bWasOnExit[p] != bExitSelected )
		{
			m_bWasOnExit[p] = bExitSelected;
			COMMAND( m_sprMore, ssprintf("Exit%sP%i", bExitSelected? "Selected":"Unselected", p+1) );
			m_sprMore->PlayCommand( bExitSelected ? "GainFocus":"LoseFocus" );
		}
	}

	const RString text = GetExplanationText( iCurRow );
	BitmapText *pText = nullptr;
	switch( m_InputMode )
	{
		case INPUTMODE_INDIVIDUAL:
			pText = &m_textExplanation[pn];
			break;
		case INPUTMODE_SHARE_CURSOR:
			pText = &m_textExplanationTogether;
			break;
		default: break;
	}
	if( pText->GetText() != text )
	{
		pText->FinishTweening();
		ON_COMMAND( pText );
		pText->SetText( text );
	}
}


bool ScreenOptions::MenuBack( const InputEventPlus & )
{
	Cancel( SM_GoToPrevScreen );
	return true;
}

bool ScreenOptions::AllAreOnLastRow() const
{
	FOREACH_HumanPlayer( p )
	{
		if( m_iCurrentRow[p] != (int)(m_pRows.size()-1) )
			return false;
	}
	return true;
}

bool ScreenOptions::MenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	switch( input.type )
	{
	case IET_FIRST_PRESS:
		m_bGotAtLeastOneStartPressed[pn] = true;
		break;
	case IET_RELEASE:
		return false;	// ignore
	default:	// repeat type
		if( !m_bGotAtLeastOneStartPressed[pn] )
			return false;	// don't allow repeat
		break;
	}

	/* If we are in a three-button mode, check to see if GAME_BUTTON_LEFT and
	 * GAME_BUTTON_RIGHT are being held. */
	switch( m_OptionsNavigation )
	{
		case NAV_THREE_KEY:
		case NAV_TOGGLE_THREE_KEY:
		{
			bool bHoldingLeftAndRight =
				INPUTMAPPER->IsBeingPressed( GAME_BUTTON_RIGHT, pn ) &&
				INPUTMAPPER->IsBeingPressed( GAME_BUTTON_LEFT, pn );
			if( bHoldingLeftAndRight )
			{
				if( MoveRowRelative(pn, -1, input.type != IET_FIRST_PRESS) )
					m_SoundPrevRow.Play(true);
				return true;
			}
		}
		default: break;
	}

	this->ProcessMenuStart( input );
	return true;
}

void ScreenOptions::ProcessMenuStart( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;

	int iCurRow = m_iCurrentRow[pn];

	if( iCurRow < 0 )
	{
		// this shouldn't be happening, but it is, so we need to bail out. -aj
		m_SoundStart.PlayCopy(true);
		this->BeginFadingOut();
		return;
	}

	OptionRow &row = *m_pRows[iCurRow];

	if( m_OptionsNavigation == NAV_THREE_KEY_MENU && row.GetRowType() != OptionRow::RowType_Exit )
	{
		/* In NAV_THREE_KEY_MENU mode, if a row doesn't set a screen, it does
		 * something.  Apply it now, and don't go to the next screen. */
		if( !FocusedItemEndsScreen(input.pn) )
		{
			std::vector<PlayerNumber> vpns;
			vpns.push_back( input.pn );
			ExportOptions( iCurRow, vpns );
			return;
		}
	}

	// Check whether Start ends this screen.
	{
		bool bEndThisScreen = false;

		// If we didn't apply and return above in NAV_THREE_KEY_MENU, then the
		// selection sets a screen.
		if( m_OptionsNavigation == NAV_THREE_KEY_MENU )
			bEndThisScreen = true;

		// If there's no exit row, then pressing Start on any row ends the screen.
		if( !SHOW_EXIT_ROW )
			bEndThisScreen = true;

		// If all players are on "Exit"
		if( AllAreOnLastRow() )
			bEndThisScreen = true;

		// Don't accept START to go to the next screen if we're still transitioning in.
		if( bEndThisScreen &&
			(input.type != IET_FIRST_PRESS || IsTransitioning()) )
			return;

		if( bEndThisScreen )
		{
			m_SoundStart.PlayCopy(true);
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
		bool changed= row.SetSelected( pn, iChoiceInRow, bSelected );
		if(changed)
		{
			AfterChangeValueOrRow(pn);
		}

		if( bSelected )
			m_SoundToggleOn.Play(true);
		else
			m_SoundToggleOff.Play(true);

		m_pRows[iCurRow]->PositionUnderlines( pn );
		RefreshIcons( iCurRow, pn );

		Message msg( "SelectMultiple" );
		msg.SetParam( "PlayerNumber", pn );
		msg.SetParam( "RowIndex", iCurRow );
		msg.SetParam( "ChoiceInRow", iChoiceInRow );
		msg.SetParam( "Selected", bSelected );
		MESSAGEMAN->Broadcast( msg );

		if(row.GetFirstItemGoesDown() && row.GoToFirstOnStart())
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
		case NAV_THREE_KEY_ALT:
			ChangeValueInRowRelative( m_iCurrentRow[input.pn], input.pn, +1, input.type != IET_FIRST_PRESS );
			break;

		case NAV_TOGGLE_THREE_KEY:
		case NAV_TOGGLE_FIVE_KEY:
		{
			int iChoiceInRow = row.GetChoiceInRowWithFocus(pn);
			if( row.GetRowDef().m_bOneChoiceForAllPlayers )
				row.SetOneSharedSelection( iChoiceInRow );
			else
				row.SetOneSelection( pn, iChoiceInRow );

			if( row.GetFirstItemGoesDown() )
				ChangeValueInRowRelative( m_iCurrentRow[pn], pn, -row.GetChoiceInRowWithFocus(pn), input.type != IET_FIRST_PRESS );	// move to the first choice
			else
				ChangeValueInRowRelative( m_iCurrentRow[pn], pn, 0, input.type != IET_FIRST_PRESS );
			break;
		}
		case NAV_THREE_KEY_MENU:
			FAIL_M("NAV_THREE_KEY_MENU should be unreachable");
		case NAV_FIVE_KEY:
			/* Jump to the exit row.  (If everyone's already on the exit row, then
			 * we'll have already gone to the next screen above.) */
			if( MoveRowAbsolute(pn, m_pRows.size()-1) )
				m_SoundNextRow.Play(true);

			break;
		}
	}
}

void ScreenOptions::StoreFocus( PlayerNumber pn )
{
	// Long rows always put us in the center, so don't update the focus.
	int iCurrentRow = m_iCurrentRow[pn];
	const OptionRow &row = *m_pRows[iCurrentRow];
	if( row.GetRowDef().m_layoutType == LAYOUT_SHOW_ONE_IN_ROW )
		return;

	int iWidth, iY;
	GetWidthXY( pn, m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), iWidth, m_iFocusX[pn], iY );
	LOG->Trace("cur selection %ix%i @ %i",
		m_iCurrentRow[pn], row.GetChoiceInRowWithFocus(pn), m_iFocusX[pn]);
}

bool ScreenOptions::FocusedItemEndsScreen( PlayerNumber pn ) const
{
	RString sScreen = GetNextScreenForFocusedItem( pn );
	return !sScreen.empty();
}

RString ScreenOptions::GetNextScreenForFocusedItem( PlayerNumber pn ) const
{
	int iCurRow = this->GetCurrentRow( pn );

	if( iCurRow == -1 )
		return RString();

	ASSERT( iCurRow >= 0 && iCurRow < (int)m_pRows.size() );
	const OptionRow *pRow = m_pRows[iCurRow];

	int iChoice = pRow->GetChoiceInRowWithFocus( pn );
	if( pRow->GetFirstItemGoesDown() )
		iChoice--;

	// not the "goes down" item
	if( iChoice == -1 )
		return RString();

	const OptionRowHandler *pHand = pRow->GetHandler();
	if( pHand == nullptr )
		return RString();
	return pHand->GetScreen( iChoice );
}

void ScreenOptions::BeginFadingOut()
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}

// Left/right
void ScreenOptions::ChangeValueInRowAbsolute( int iRow, PlayerNumber pn, int iChoiceIndex, bool bRepeat )
{
	if( iRow == -1	)	// no row selected
		return;		// don't allow a move

	OptionRow &row = *m_pRows[iRow];

	const int iNumChoices = row.GetRowDef().m_vsChoices.size();
	ASSERT( iNumChoices >= 0 && iChoiceIndex < iNumChoices );

	int iCurrentChoiceWithFocus = row.GetChoiceInRowWithFocus(pn);
	int iDelta = iChoiceIndex - iCurrentChoiceWithFocus;

	Message msg( "ChangeValue" );
	msg.SetParam( "PlayerNumber", pn );
	msg.SetParam( "RowIndex", iRow );
	MESSAGEMAN->Broadcast( msg );

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
		/* There are no other options on the row; move up or down instead of
		 * left and right. This allows navigating the options menu with
		 * left/right/start.
		 * XXX: Only allow repeats if the opposite key isn't pressed; otherwise,
		 * holding both directions will repeat in place continuously, which is weird. */
		if( MoveRowRelative(pn, iDelta, bRepeat) )
		{
			if( iDelta < 0 )
				m_SoundPrevRow.Play(true);
			else
				m_SoundNextRow.Play(true);
		}
		return;
	}

	if( iNumChoices <= 1 ) // nowhere to move
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
		 * for this row. Don't do this in toggle modes, since the current selection
		 * and the current focus are detached. */
		bool bForceFocusedChoiceTogether = false;
		if( m_OptionsNavigation!=NAV_TOGGLE_THREE_KEY &&
			m_OptionsNavigation!=NAV_TOGGLE_FIVE_KEY &&
			row.GetRowDef().m_bOneChoiceForAllPlayers )
		{
			bForceFocusedChoiceTogether = true;
		}

		// Also lock focus if the screen is explicitly set to share cursors.
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
	}

	FOREACH_PlayerNumber( p )
	{
		if( !row.GetRowDef().m_bOneChoiceForAllPlayers && p != pn )
			continue;

		if( m_OptionsNavigation==NAV_TOGGLE_THREE_KEY || m_OptionsNavigation==NAV_TOGGLE_FIVE_KEY )
		{
			;	// do nothing
		}
		else
		{
			if( row.GetRowDef().m_selectType == SELECT_MULTIPLE )
				;	// do nothing. User must press Start to toggle the selection.
			else
				row.SetOneSelection( p, iNewChoiceWithFocus );
		}
	}

	if( bOneChanged )
		m_SoundChangeCol.Play(true);

	if( row.GetRowDef().m_bExportOnChange )
	{
		std::vector<PlayerNumber> vpns;
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

// Move up/down. Returns true if we actually moved.
bool ScreenOptions::MoveRowRelative( PlayerNumber pn, int iDir, bool bRepeat )
{
	//LOG->Trace( "MoveRowRelative(pn %i, dir %i, rep %i)", pn, iDir, bRepeat );

	int iDest = -1;
	ASSERT( m_pRows.size() != 0 );
	for( int r=1; r<(int)m_pRows.size(); r++ )
	{
		int iDelta = r*iDir;
		iDest = m_iCurrentRow[pn] + iDelta;
		wrap( iDest, m_pRows.size() );

		OptionRow &row = *m_pRows[iDest];
		if( row.GetRowDef().IsEnabledForPlayer(pn) )
			break;

		iDest = -1;
	}

	if( iDest == -1 )
		return false;
	if( bRepeat )
	{
		// Don't wrap on repeating inputs.
		if( iDir > 0 && iDest < m_iCurrentRow[pn] )
			return false;
		if( iDir < 0 && iDest > m_iCurrentRow[pn] )
			return false;
	}

	return MoveRowAbsolute( pn, iDest );
}

void ScreenOptions::AfterChangeRow( PlayerNumber pn )
{
	const int iRow = m_iCurrentRow[pn];
	if( iRow != -1 )
	{
		// In FIVE_KEY, keep the selection in the row near the focus.
		OptionRow &row = *m_pRows[iRow];
		switch( m_OptionsNavigation )
		{
			case NAV_TOGGLE_FIVE_KEY:
			{
				if( row.GetRowDef().m_layoutType != LAYOUT_SHOW_ONE_IN_ROW )
				{
					int iSelectionDist = -1;
					for( unsigned i = 0; i < row.GetTextItemsSize(); ++i )
					{
						int iWidth, iX, iY;
						GetWidthXY( pn, m_iCurrentRow[pn], i, iWidth, iX, iY );
						const int iDist = std::abs( iX-m_iFocusX[pn] );
						if( iSelectionDist == -1 || iDist < iSelectionDist )
						{
							iSelectionDist = iDist;
							row.SetChoiceInRowWithFocus( pn, i );
						}
					}
				}
				break;
			}
			default: break;
		}

		if( row.GetFirstItemGoesDown() )
		{
			// If moving to a bFirstChoiceGoesDown row, put focus back on
			// the first choice.
			row.SetChoiceInRowWithFocus( pn, 0 );
		}
	}

	AfterChangeValueOrRow( pn );
}

bool ScreenOptions::MoveRowAbsolute( PlayerNumber pn, int iRow )
{
	bool bChanged = false;
	FOREACH_PlayerNumber( p )
	{
		if( m_InputMode == INPUTMODE_INDIVIDUAL && p != pn )
			continue;	// skip

		if( m_iCurrentRow[p] == iRow )
			continue;

		m_iCurrentRow[p] = iRow;

		AfterChangeRow( p );
		bChanged = true;

		const OptionRow &row = *m_pRows[iRow];
		Message msg( "ChangeRow" );
		msg.SetParam( "PlayerNumber", p );
		msg.SetParam( "RowIndex", GetCurrentRow(p) );
		msg.SetParam( "ChangedToExit", row.GetRowType() == OptionRow::RowType_Exit );
		MESSAGEMAN->Broadcast( msg );
	}

	return bChanged;
}

bool ScreenOptions::MenuLeft( const InputEventPlus &input )
{
	if( m_OptionsNavigation == NAV_THREE_KEY_ALT )
		MenuUpDown( input, -1 );
	else
		ChangeValueInRowRelative(m_iCurrentRow[input.pn],input.pn,-1, input.type != IET_FIRST_PRESS);

	PlayerNumber pn = input.pn;
	MESSAGEMAN->Broadcast( (MessageID)(Message_MenuLeftP1+Enum::to_integral(pn)) );
	return true;
}

bool ScreenOptions::MenuRight( const InputEventPlus &input )
{
	if( m_OptionsNavigation == NAV_THREE_KEY_ALT )
		MenuUpDown( input, +1 );
	else
		ChangeValueInRowRelative(m_iCurrentRow[input.pn], input.pn,+1, input.type != IET_FIRST_PRESS);

	PlayerNumber pn = input.pn;
	MESSAGEMAN->Broadcast( (MessageID)(Message_MenuRightP1+Enum::to_integral(pn)) );
	return true;
}

bool ScreenOptions::MenuUp( const InputEventPlus &input )
{
	MenuUpDown( input, -1 );
	PlayerNumber pn = input.pn;
	MESSAGEMAN->Broadcast( (MessageID)(Message_MenuUpP1+Enum::to_integral(pn)) );
	return true;
}

bool ScreenOptions::MenuDown( const InputEventPlus &input )
{
	MenuUpDown( input, +1 );
	PlayerNumber pn = input.pn;
	MESSAGEMAN->Broadcast( (MessageID)(Message_MenuDownP1+Enum::to_integral(pn)) );
	return true;
}

bool ScreenOptions::MenuSelect( const InputEventPlus &input )
{
	MenuUpDown( input, -1 );
	return true;
}

void ScreenOptions::MenuUpDown( const InputEventPlus &input, int iDir )
{
	ASSERT( iDir == -1 || iDir == +1 );
	PlayerNumber pn = input.pn;

	if( input.type == IET_REPEAT )
	{
		/* If down is pressed, don't allow up to repeat, and vice versa. This
		 * prevents holding both up and down from toggling repeatedly in-place. */
		if( iDir == +1 )
		{
			if( INPUTMAPPER->IsBeingPressed(GAME_BUTTON_MENUUP, pn) ||
				INPUTMAPPER->IsBeingPressed(GAME_BUTTON_SELECT, pn) )
				return;
		}
		else
		{
			if( INPUTMAPPER->IsBeingPressed(GAME_BUTTON_MENUDOWN, pn) ||
				INPUTMAPPER->IsBeingPressed(GAME_BUTTON_START, pn) )
				return;
		}
	}

	if( MoveRowRelative(pn, iDir, input.type != IET_FIRST_PRESS) )
	{
		if( iDir < 0 )
			m_SoundPrevRow.Play(true);
		else
			m_SoundNextRow.Play(true);
	}
}

/*
void ScreenOptions::SetOptionRowFromName( const RString& nombre )
	{
		FOREACH_PlayerNumber( pn )
		{
			for( unsigned i=0; i<m_pRows.size(); i++ )
			{
				if( m_pRows[i]->GetRowTitle() == nombre) && m_pRows[i]->GetRowDef().IsEnabledForPlayer(p) )
					MoveRowAbsolute(pn,i)
			}
		}
	}
*/
// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to ScreenOptions. */
class LunaScreenOptions: public Luna<ScreenOptions>
{
public:
	static int AllAreOnLastRow( T* p, lua_State *L ) { lua_pushboolean( L, p->AllAreOnLastRow() ); return 1; }
	static int FocusedItemEndsScreen( T* p, lua_State *L ) { lua_pushboolean( L, p->FocusedItemEndsScreen(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int GetCurrentRowIndex( T* p, lua_State *L ) { lua_pushnumber( L, p->GetCurrentRow(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int GetOptionRow( T* p, lua_State *L ) {
		int row_index= IArg(1);
		// TODO:  Change row indices to be 1-indexed when breaking compatibility
		// is allowed. -Kyz
		if(row_index < 0 || row_index >= p->GetNumRows())
		{
			luaL_error(L, "Row index %d is invalid.", row_index);
		}
		OptionRow* pOptRow = p->GetRow(row_index);
		if( pOptRow )
			pOptRow->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}
	DEFINE_METHOD(GetNumRows, GetNumRows());
   //static int SetOptionRowFromName( T* p, lua_State *L ) { p->SetOptionRowFromName( SArg(1) ); return 0; }

	LunaScreenOptions()
	{
		ADD_METHOD( AllAreOnLastRow );
		ADD_METHOD( FocusedItemEndsScreen );
		ADD_METHOD( GetCurrentRowIndex );
		ADD_METHOD( GetOptionRow );
		ADD_METHOD( GetNumRows );
        //ADD_METHOD( SetOptionRowFromName );
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
