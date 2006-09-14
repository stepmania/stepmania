#include "global.h"
#include "ScreenSelectMaster.h"
#include "ScreenManager.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "GameCommand.h"
#include "ActorUtil.h"
#include "RageLog.h"
#include <set>
#include "Foreach.h"
#include "RageSoundManager.h"
#include "InputEventPlus.h"

static const char *MenuDirNames[] = {
	"Up",
	"Down",
	"Left",
	"Right",
	"Auto",
};
XToString( MenuDir, NUM_MenuDir );

AutoScreenMessage( SM_PlayPostSwitchPage )

RString CURSOR_OFFSET_X_FROM_ICON_NAME( size_t p ) { return ssprintf("CursorP%dOffsetXFromIcon",int(p+1)); }
RString CURSOR_OFFSET_Y_FROM_ICON_NAME( size_t p ) { return ssprintf("CursorP%dOffsetYFromIcon",int(p+1)); }
/* e.g. "OptionOrderLeft=0:1,1:2,2:3,3:4" */
RString OPTION_ORDER_NAME( size_t dir )							{ return "OptionOrder"+MenuDirToString((MenuDir)dir); }

REGISTER_SCREEN_CLASS( ScreenSelectMaster );

ScreenSelectMaster::ScreenSelectMaster()
{
	ZERO( m_iChoice );
	ZERO( m_bChosen );
}

void ScreenSelectMaster::Init()
{
	SHOW_ICON.Load( m_sName, "ShowIcon" );
	SHOW_SCROLLER.Load( m_sName, "ShowScroller" );
	SHOW_CURSOR.Load( m_sName, "ShowCursor" );
	SHARED_SELECTION.Load( m_sName, "SharedSelection" );
	USE_ICON_METRICS.Load( m_sName, "UseIconMetrics" );
	NUM_CHOICES_ON_PAGE_1.Load( m_sName, "NumChoicesOnPage1" );
	CURSOR_OFFSET_X_FROM_ICON.Load( m_sName, CURSOR_OFFSET_X_FROM_ICON_NAME, NUM_PLAYERS );
	CURSOR_OFFSET_Y_FROM_ICON.Load( m_sName, CURSOR_OFFSET_Y_FROM_ICON_NAME, NUM_PLAYERS );
	PER_CHOICE_ICON_ELEMENT.Load( m_sName, "PerChoiceIconElement" );
	PRE_SWITCH_PAGE_SECONDS.Load( m_sName, "PreSwitchPageSeconds" );
	POST_SWITCH_PAGE_SECONDS.Load( m_sName, "PostSwitchPageSeconds" );
	OPTION_ORDER.Load( m_sName, OPTION_ORDER_NAME, NUM_MenuDir );
	WRAP_CURSOR.Load( m_sName, "WrapCursor" );
	WRAP_SCROLLER.Load( m_sName, "WrapScroller" );
	LOOP_SCROLLER.Load( m_sName, "LoopScroller" );
	PER_CHOICE_SCROLL_ELEMENT.Load( m_sName, "PerChoiceScrollElement" );
	ALLOW_REPEATING_INPUT.Load( m_sName, "AllowRepeatingInput" );
	SCROLLER_SECONDS_PER_ITEM.Load( m_sName, "ScrollerSecondsPerItem" );
	SCROLLER_NUM_ITEMS_TO_DRAW.Load( m_sName, "ScrollerNumItemsToDraw" );
	SCROLLER_TRANSFORM.Load( m_sName, "ScrollerTransform" );
	SCROLLER_SUBDIVISIONS.Load( m_sName, "ScrollerSubdivisions" );
	DEFAULT_CHOICE.Load( m_sName, "DefaultChoice" );

	ScreenSelect::Init();

	m_TrackingRepeatingInput.MakeInvalid();

	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}

#define PLAYER_APPEND_NO_SPACE(p)	(SHARED_SELECTION ? RString() : ssprintf("P%d",(p)+1))
	
	// init cursor
	if( SHOW_CURSOR )
	{
		FOREACH( PlayerNumber, vpns, p )
		{
			RString sElement = "Cursor " + PLAYER_APPEND_NO_SPACE(*p);
			m_sprCursor[*p].Load( THEME->GetPathG(m_sName,sElement) );
			sElement.Replace( " ", "" );
			m_sprCursor[*p]->SetName( sElement );
			this->AddChild( m_sprCursor[*p] );
		}
	}

	// Resize vectors depending on how many choices there are
	m_vsprIcon.resize( m_aGameCommands.size() );
	FOREACH( PlayerNumber, vpns, p )
		m_vsprScroll[*p].resize( m_aGameCommands.size() );


	for( unsigned c=0; c<m_aGameCommands.size(); c++ )
	{
		GameCommand& mc = m_aGameCommands[c];

		Lua *L = LUA->Get();
		mc.PushSelf( L );
		lua_setglobal( L, "ThisGameCommand" );
		LUA->Release( L );

		// init icon
		if( SHOW_ICON )
		{
			vector<RString> vs;
			vs.push_back( "Icon" );
			if( PER_CHOICE_ICON_ELEMENT )
				vs.push_back( "Choice" + mc.m_sName );
			RString sElement = join( " ", vs );
			m_vsprIcon[c].Load( THEME->GetPathG(m_sName,sElement) );
			RString sName = "Icon" "Choice" + mc.m_sName;
			m_vsprIcon[c]->SetName( sName );
			this->AddChild( m_vsprIcon[c] );
		}

		// init scroll
		if( SHOW_SCROLLER )
		{
			FOREACH( PlayerNumber, vpns, p )
			{
				vector<RString> vs;
				vs.push_back( "Scroll" );
				if( PER_CHOICE_SCROLL_ELEMENT )
					vs.push_back( "Choice" + mc.m_sName );
				if( !SHARED_SELECTION )
					vs.push_back( PLAYER_APPEND_NO_SPACE(*p) );
				RString sElement = join( " ", vs );
				m_vsprScroll[*p][c].Load( THEME->GetPathG(m_sName,sElement) );
				RString sName = "Scroll" "Choice" + mc.m_sName;
				if( !SHARED_SELECTION )
					sName += PLAYER_APPEND_NO_SPACE(*p);
				m_vsprScroll[*p][c]->SetName( sName );
				m_Scroller[*p].AddChild( m_vsprScroll[*p][c] );
			}

		}

		LUA->UnsetGlobal( "ThisGameCommand" );
	}

	// init scroll
	if( SHOW_SCROLLER )
	{
		FOREACH( PlayerNumber, vpns, p )
		{
			m_Scroller[*p].SetLoop( LOOP_SCROLLER );
			m_Scroller[*p].SetNumItemsToDraw( SCROLLER_NUM_ITEMS_TO_DRAW );
			m_Scroller[*p].Load2();
			m_Scroller[*p].SetTransformFromExpression( SCROLLER_TRANSFORM );
			m_Scroller[*p].SetSecondsPerItem( SCROLLER_SECONDS_PER_ITEM );
			m_Scroller[*p].SetNumSubdivisions( SCROLLER_SUBDIVISIONS );
			m_Scroller[*p].SetName( "Scroller"+PLAYER_APPEND_NO_SPACE(*p) );
			this->AddChild( &m_Scroller[*p] );
		}
	}

	for( int page=0; page<NUM_PAGES; page++ )
	{
		m_sprMore[page].Load( THEME->GetPathG(m_sName, ssprintf("more page%d",page+1)) );
		m_sprMore[page]->SetName( ssprintf("MorePage%d",page+1) );
		ActorUtil::LoadAllCommands( *m_sprMore[page], m_sName );
		this->AddChild( m_sprMore[page] );

		m_sprExplanation[page].Load( THEME->GetPathG(m_sName, ssprintf("explanation page%d",page+1)) );
		m_sprExplanation[page]->SetName( ssprintf("ExplanationPage%d",page+1) );
		ActorUtil::LoadAllCommands( *m_sprExplanation[page], m_sName );
		this->AddChild( m_sprExplanation[page] );
	}


	m_soundChange.Load( THEME->GetPathS(m_sName,"change"), true );
	m_soundDifficult.Load( ANNOUNCER->GetPathTo("select difficulty challenge") );
	m_soundStart.Load( THEME->GetPathS(m_sName,"start") );

	// init m_Next order info
	FOREACH_MenuDir( dir )
	{
		const RString order = OPTION_ORDER.GetValue( dir );
		vector<RString> parts;
		split( order, ",", parts, true );

		for( unsigned part = 0; part < parts.size(); ++part )
		{
			int from, to;
			if( sscanf( parts[part], "%d:%d", &from, &to ) != 2 )
			{
				LOG->Warn( "%s::OptionOrder%s parse error", m_sName.c_str(), MenuDirToString(dir).c_str() );
				continue;
			}

			--from;
			--to;

			m_mapCurrentChoiceToNextChoice[dir][from] = to;
		}

		if( m_mapCurrentChoiceToNextChoice[dir].empty() )	// Didn't specify any mappings
		{
			// Fill with reasonable defaults
			for( unsigned c = 0; c < m_aGameCommands.size(); ++c )
			{
				int add;
				switch( dir )
				{
				case MenuDir_Up:
				case MenuDir_Left:
					add = -1;
					break;
				default:
					add = +1;
					break;
				}

				m_mapCurrentChoiceToNextChoice[dir][c] = c + add;
				/* Always wrap around MenuDir_Auto. */
				if( dir == MenuDir_Auto || (bool)WRAP_CURSOR )
					wrap( m_mapCurrentChoiceToNextChoice[dir][c], m_aGameCommands.size() );
				else
					m_mapCurrentChoiceToNextChoice[dir][c] = clamp( m_mapCurrentChoiceToNextChoice[dir][c], 0, (int)m_aGameCommands.size()-1 );
			}
		}
	}
}

RString ScreenSelectMaster::GetDefaultChoice()
{
	return DEFAULT_CHOICE.GetValue();
}

void ScreenSelectMaster::BeginScreen()
{
	// TODO: Move default choice to ScreenSelect
	int iDefaultChoice = -1;
	for( unsigned c=0; c<m_aGameCommands.size(); c++ )
	{
		const GameCommand& mc = m_aGameCommands[c];
		if( mc.m_sName == (RString) DEFAULT_CHOICE )
		{
			iDefaultChoice = c;
			break;
		}
	}

	FOREACH_PlayerNumber( p )
	{
		m_iChoice[p] = (iDefaultChoice!=-1) ? iDefaultChoice : 0;
		CLAMP( m_iChoice[p], 0, (int)m_aGameCommands.size()-1 );
		m_bChosen[p] = false;
	}

	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}

	this->UpdateSelectableChoices();

	m_fLockInputSecs = this->GetTweenTimeLeft();

	ScreenSelect::BeginScreen();
}

void ScreenSelectMaster::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
}

void ScreenSelectMaster::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenSelect::HandleScreenMessage( SM );


	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}

	
	if( SM == SM_PlayPostSwitchPage )
	{
		if( SHOW_CURSOR )
		{
			FOREACH( PlayerNumber, vpns, p )
			{
				m_sprCursor[*p]->SetXY( GetCursorX(*p), GetCursorY(*p) );
				COMMAND( m_sprCursor[*p], "PostSwitchPage" );
			}
		}

		if( SHOW_SCROLLER )
		{
			FOREACH( PlayerNumber, vpns, p )
			{
				int iChoice = m_iChoice[*p];
				COMMAND( m_vsprScroll[*p][iChoice], "PostSwitchPage" );
			}
		}

		m_fLockInputSecs = POST_SWITCH_PAGE_SECONDS;
	}
}

int ScreenSelectMaster::GetSelectionIndex( PlayerNumber pn )
{
	return m_iChoice[pn];
}

void ScreenSelectMaster::UpdateSelectableChoices()
{
	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}


	for( unsigned c=0; c<m_aGameCommands.size(); c++ )
	{
		if( SHOW_ICON )
			m_vsprIcon[c]->PlayCommand( m_aGameCommands[c].IsPlayable()? "Enabled":"Disabled" );
		
		FOREACH( PlayerNumber, vpns, p )
			if( m_vsprScroll[*p][c].IsLoaded() )
				m_vsprScroll[*p][c]->PlayCommand( m_aGameCommands[c].IsPlayable()? "Enabled":"Disabled" );
	}

	/*
	 * If no options are playable at all, just wait.  Some external
	 * stimulus may make options available (such as coin insertion).
	 *
	 * If any options are playable, make sure one is selected.
	 */
	FOREACH_HumanPlayer( p )
		if( !m_aGameCommands[m_iChoice[p]].IsPlayable() )
			Move( p, MenuDir_Auto );
}

bool ScreenSelectMaster::AnyOptionsArePlayable() const
{
	for( unsigned i = 0; i < m_aGameCommands.size(); ++i )
		if( m_aGameCommands[i].IsPlayable() )
			return true;

	return false;
}

bool ScreenSelectMaster::Move( PlayerNumber pn, MenuDir dir )
{
	if( !AnyOptionsArePlayable() )
		return false;

	int iSwitchToIndex = m_iChoice[pn];
	set<int> seen;
try_again:
	map<int,int>::const_iterator iter = m_mapCurrentChoiceToNextChoice[dir].find( iSwitchToIndex );
	if( iter != m_mapCurrentChoiceToNextChoice[dir].end() )
		iSwitchToIndex = iter->second;

	if( iSwitchToIndex < 0 || iSwitchToIndex >= (int) m_aGameCommands.size() ) // out of choice range
		return false; // can't go that way
	if( seen.find(iSwitchToIndex) != seen.end() )
		return false; // went full circle and none found
	seen.insert( iSwitchToIndex );

	if( !m_aGameCommands[iSwitchToIndex].IsPlayable() )
		goto try_again;

	return ChangeSelection( pn, dir, iSwitchToIndex );
}

void ScreenSelectMaster::MenuLeft( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	if( m_fLockInputSecs > 0 || m_bChosen[pn] )
		return;
	if( input.type == IET_RELEASE )
		return;
	if( input.type != IET_FIRST_PRESS )
	{
		if( !ALLOW_REPEATING_INPUT )
			return;
		if( m_TrackingRepeatingInput != input.MenuI )
			return;
	}
	if( Move(pn, MenuDir_Left) )
	{
		m_TrackingRepeatingInput = input.MenuI;
		m_soundChange.Play();
		MESSAGEMAN->Broadcast( (Message)(Message_MenuLeftP1+pn) );
		MESSAGEMAN->Broadcast( (Message)(Message_MenuSelectionChanged) );
	}
}

void ScreenSelectMaster::MenuRight( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	if( m_fLockInputSecs > 0 || m_bChosen[pn] )
		return;
	if( input.type == IET_RELEASE )
		return;
	if( input.type != IET_FIRST_PRESS )
	{
		if( !ALLOW_REPEATING_INPUT )
			return;
		if( m_TrackingRepeatingInput != input.MenuI )
			return;
	}
	if( Move(pn, MenuDir_Right) )
	{
		m_TrackingRepeatingInput = input.MenuI;
		m_soundChange.Play();
		MESSAGEMAN->Broadcast( (Message)(Message_MenuRightP1+pn) );
		MESSAGEMAN->Broadcast( (Message)(Message_MenuSelectionChanged) );
	}
}

void ScreenSelectMaster::MenuUp( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	if( m_fLockInputSecs > 0 || m_bChosen[pn] )
		return;
	if( input.type == IET_RELEASE )
		return;
	if( input.type != IET_FIRST_PRESS )
	{
		if( !ALLOW_REPEATING_INPUT )
			return;
		if( m_TrackingRepeatingInput != input.MenuI )
			return;
	}
	if( Move(pn, MenuDir_Up) )
	{
		m_TrackingRepeatingInput = input.MenuI;
		m_soundChange.Play();
		MESSAGEMAN->Broadcast( (Message)(Message_MenuUpP1+pn) );
		MESSAGEMAN->Broadcast( (Message)(Message_MenuSelectionChanged) );
	}
}

void ScreenSelectMaster::MenuDown( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;
	if( m_fLockInputSecs > 0 || m_bChosen[pn] )
		return;
	if( input.type == IET_RELEASE )
		return;
	if( input.type != IET_FIRST_PRESS )
	{
		if( !ALLOW_REPEATING_INPUT )
			return;
		if( m_TrackingRepeatingInput != input.MenuI )
			return;
	}
	if( Move(pn, MenuDir_Down) )
	{
		m_TrackingRepeatingInput = input.MenuI;
		m_soundChange.Play();
		MESSAGEMAN->Broadcast( (Message)(Message_MenuDownP1+pn) );
		MESSAGEMAN->Broadcast( (Message)(Message_MenuSelectionChanged) );
	}
}

bool ScreenSelectMaster::ChangePage( int iNewChoice )
{
	Page newPage = GetPage(iNewChoice);

	// If anyone has already chosen, don't allow changing of pages
	FOREACH_PlayerNumber( p )
	{
		if( GAMESTATE->IsHumanPlayer(p) && m_bChosen[p] )
			return false;
	}


	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}


	if( SHOW_CURSOR )
	{
		FOREACH( PlayerNumber, vpns, p )
			if( GAMESTATE->IsHumanPlayer(*p) )
				COMMAND( m_sprCursor[*p], "PreSwitchPage" );
	}

	const RString sIconAndExplanationCommand = ssprintf( "SwitchToPage%d", newPage+1 );

	if( SHOW_ICON )
	{
		for( unsigned c=0; c<m_aGameCommands.size(); c++ )
			COMMAND( m_vsprIcon[c], sIconAndExplanationCommand );
	}

	if( SHOW_SCROLLER )
	{
		if( SHARED_SELECTION )
		{
			int iChoice = m_iChoice[GetSharedPlayer()];
			COMMAND( m_vsprScroll[0][iChoice], "PreSwitchPage" );
		}
		else
		{
			FOREACH_HumanPlayer( p )
			{
				int iChoice = m_iChoice[p];
				COMMAND( m_vsprScroll[p][iChoice], "PreSwitchPage" );
			}
		}
	}

	for( int page=0; page<NUM_PAGES; page++ )
	{
		COMMAND( m_sprExplanation[page], sIconAndExplanationCommand );
		COMMAND( m_sprMore[page], sIconAndExplanationCommand );
	}



	if( newPage == PAGE_2 )
	{
		// XXX: only play this once (I thought we already did that?)
		// Play it on every change to page 2.  -Chris
		/* That sounds ugly if you go back and forth quickly. -g */
		// Should we lock input while it's scrolling? -Chris
		m_soundDifficult.Stop();
		m_soundDifficult.PlayRandom();
	}

	// change both players
	FOREACH_PlayerNumber( p )
		m_iChoice[p] = iNewChoice;

	m_fLockInputSecs = PRE_SWITCH_PAGE_SECONDS;
	this->PostScreenMessage( SM_PlayPostSwitchPage, PRE_SWITCH_PAGE_SECONDS );
	return true;
}

bool ScreenSelectMaster::ChangeSelection( PlayerNumber pn, MenuDir dir, int iNewChoice )
{
	if( iNewChoice == m_iChoice[pn] )
		return false; // already there

	if( GetPage(m_iChoice[pn]) != GetPage(iNewChoice) )
		return ChangePage( iNewChoice );

	FOREACH_PlayerNumber( p )
	{
		const int iOldChoice = m_iChoice[p];

		/* Set the new m_iChoice even for disabled players, since a player might
		 * join on a SHARED_SELECTION after the cursor has been moved. */
		m_iChoice[p] = iNewChoice;

		if( p!=pn )
			continue;	// skip

		if( SHOW_ICON )
		{
			/* XXX: If !SharedPreviewAndCursor, this is incorrect.  (Nothing uses
			 * both icon focus and !SharedPreviewAndCursor right now.) */
			m_vsprIcon[iOldChoice]->PlayCommand( "LoseFocus" );
			m_vsprIcon[iNewChoice]->PlayCommand( "GainFocus" );
		}

		if( SHOW_CURSOR )
		{
			if( SHARED_SELECTION )
			{
				COMMAND( m_sprCursor[0], "Change" );
				m_sprCursor[0]->SetXY( GetCursorX(PLAYER_1), GetCursorY(PLAYER_1) );
			}
			else
			{
				COMMAND( m_sprCursor[p], "Change" );
				m_sprCursor[p]->SetXY( GetCursorX(p), GetCursorY(p) );
			}
		}

		if( SHOW_SCROLLER )
		{
			if( WRAP_SCROLLER )
			{
				// HACK: We can't tell from the option orders whether or not we wrapped.
				// For now, assume that the order is increasing left to right.
				int iPressedDir = (dir == MenuDir_Left) ? -1 : +1;
				int iActualDir = (iOldChoice < iNewChoice) ? +1 : -1;

				if( iPressedDir != iActualDir )	// wrapped
				{
					ActorScroller &scroller = SHARED_SELECTION ? m_Scroller[0] : m_Scroller[p];
					float fItem = scroller.GetCurrentItem();
					int iNumChoices = m_aGameCommands.size();
					fItem += iActualDir * iNumChoices;
					scroller.SetCurrentAndDestinationItem( fItem );
				}
			}

			if( SHARED_SELECTION )
				m_Scroller[0].SetDestinationItem( (float)iNewChoice );
			else
				m_Scroller[p].SetDestinationItem( (float)iNewChoice );
			
			if( SHARED_SELECTION )
			{
				m_vsprScroll[0][iOldChoice]->PlayCommand( "LoseFocus" );
				m_vsprScroll[0][iNewChoice]->PlayCommand( "GainFocus" );
			}
			else
			{
				m_vsprScroll[p][iOldChoice]->PlayCommand( "LoseFocus" );
				m_vsprScroll[p][iNewChoice]->PlayCommand( "GainFocus" );
			}
		}
	}


	return true;
}

PlayerNumber ScreenSelectMaster::GetSharedPlayer()
{
	if( GAMESTATE->m_MasterPlayerNumber != PLAYER_INVALID )
		return GAMESTATE->m_MasterPlayerNumber;

	return PLAYER_1;
}

ScreenSelectMaster::Page ScreenSelectMaster::GetPage( int iChoiceIndex ) const
{
	return iChoiceIndex < NUM_CHOICES_ON_PAGE_1? PAGE_1:PAGE_2;
}

ScreenSelectMaster::Page ScreenSelectMaster::GetCurrentPage() const
{
	// Both players are guaranteed to be on the same page.
	return GetPage( m_iChoice[GetSharedPlayer()] );
}


float ScreenSelectMaster::DoMenuStart( PlayerNumber pn )
{
	if( m_bChosen[pn] )
		return 0;

	bool bAnyChosen = false;
	FOREACH_PlayerNumber( p )
		bAnyChosen |= m_bChosen[p];

	m_bChosen[pn] = true;

	this->PlayCommand( "MadeChoice"+PlayerNumberToString(pn) );

	bool bIsFirstToChoose = bAnyChosen;

	float fSecs = 0;

	if( bIsFirstToChoose )
	{
		for( int page=0; page<NUM_PAGES; page++ )
		{
			OFF_COMMAND( m_sprMore[page] );
			fSecs = max( fSecs, m_sprMore[page]->GetTweenTimeLeft() );
		}

		int iIndex = SHARED_SELECTION ? 0 : pn;

		if( SHOW_CURSOR )
		{
			COMMAND( m_sprCursor[pn], "Choose");
			fSecs = max( fSecs, m_sprCursor[iIndex]->GetTweenTimeLeft() );
		}
	}

	return fSecs;
}

void ScreenSelectMaster::MenuStart( PlayerNumber pn )
{
	// If the player isn't already joined, try to join them.
	// Allow a player to join even if input is locked or someone has already already chosen.
	InputEventPlus input;
	input.pn = pn;
	input.MenuI = MENU_BUTTON_START;
	Screen::JoinInput( input );

	if( m_fLockInputSecs > 0 )
		return;
	if( m_bChosen[pn] )
		return;

	if( !ProcessMenuStart( pn ) )
		return;

	const GameCommand &mc = m_aGameCommands[m_iChoice[pn]];

	/* If no options are playable, then we're just waiting for one to become available.
	 * If any options are playable, then the selection must be playable. */
	if( !AnyOptionsArePlayable() )
		return;
		
	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("%s comment %s",m_sName.c_str(), mc.m_sName.c_str())) );
	
	/* Play a copy of the sound, so it'll finish playing even if we leave the screen immediately. */
	if( mc.m_sSoundPath.empty() )
		SOUNDMAN->PlayCopyOfSound( m_soundStart );

	if( mc.m_sScreen.empty() )
	{
		mc.ApplyToAllPlayers();
		return;
	}
	
	float fSecs = 0;
	bool bAllDone = true;
	if( (bool)SHARED_SELECTION || GetCurrentPage() == PAGE_2 )
	{
		/* Only one player has to pick.  Choose this for all the other players, too. */
		FOREACH_PlayerNumber( p )
		{
			ASSERT( !m_bChosen[p] );
			fSecs = max( fSecs, DoMenuStart(p) );	// no harm in calling this for an unjoined player
		}
	}
	else
	{
		fSecs = max( fSecs, DoMenuStart(pn) );
		// check to see if everyone has chosen
		FOREACH_HumanPlayer( p )
			bAllDone &= m_bChosen[p];
	}

	if( bAllDone )
		this->PostScreenMessage( SM_BeginFadingOut, fSecs );// tell our owner it's time to move on
}

/*
 * We want all items to always run OnCommand and either GainFocus or LoseFocus on
 * tween-in.  If we only run OnCommand, then it has to contain a copy of either
 * GainFocus or LoseFocus, which implies that the default setting is hard-coded in
 * the theme.  Always run both.
 *
 * However, the actual tween-in is OnCommand; we don't always want to actually run
 * through the Gain/LoseFocus tweens during initial tween-in.  So, we run the focus
 * command first, do a FinishTweening to pop it in place, and then run OnCommand.
 * This means that the focus command should be position neutral; eg. only use "addx",
 * not "x".
 */
void ScreenSelectMaster::TweenOnScreen()
{
	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}

	if( SHOW_ICON )
	{
		for( unsigned c=0; c<m_aGameCommands.size(); c++ )
		{
			m_vsprIcon[c]->PlayCommand( (int(c) == m_iChoice[0])? "GainFocus":"LoseFocus" );
			m_vsprIcon[c]->FinishTweening();
			if( USE_ICON_METRICS )
				SET_XY_AND_ON_COMMAND( m_vsprIcon[c] );
			else
				m_vsprIcon[c]->PlayCommand( "On" );
		}
	}

	if( SHOW_SCROLLER )
	{
		FOREACH( PlayerNumber, vpns, p )
		{
			// Play Gain/LoseFocus before playing the on command.  Gain/Lose will 
			// often stop tweening, which ruins the OnCommand.
			for( unsigned c=0; c<m_aGameCommands.size(); c++ )
			{
				m_vsprScroll[*p][c]->PlayCommand( int(c) == m_iChoice[*p]? "GainFocus":"LoseFocus" );
				m_vsprScroll[*p][c]->FinishTweening();
			}

			m_Scroller[*p].SetCurrentAndDestinationItem( (float)m_iChoice[*p] );
			SET_XY_AND_ON_COMMAND( m_Scroller[*p] );
		}
	}

	// Need to SetXY of Cursor after Icons since it depends on the Icons' positions.
	if( SHOW_CURSOR )
	{
		FOREACH( PlayerNumber, vpns, p )
		{
			m_sprCursor[*p]->SetXY( GetCursorX(*p), GetCursorY(*p) );
			ON_COMMAND( m_sprCursor[*p] );
		}
	}

	//We have to move page two's explanation and more off the screen
	//so it doesn't just sit there on page one.  (Thanks Zhek)
	
	for (int page=0;page<NUM_PAGES;page++)
	{
		m_sprMore[page]->SetXY(999,999);
		m_sprExplanation[page]->SetXY(999,999);
	}

	SET_XY_AND_ON_COMMAND( m_sprExplanation[GetCurrentPage()] );
	SET_XY_AND_ON_COMMAND( m_sprMore[GetCurrentPage()] );

	ScreenSelect::TweenOnScreen();
}

void ScreenSelectMaster::TweenOffScreen()
{
	ScreenSelect::TweenOffScreen();

	vector<PlayerNumber> vpns;
	if( SHARED_SELECTION )
	{
		vpns.push_back( PLAYER_1 );
	}
	else
	{
		FOREACH_HumanPlayer( p )
			vpns.push_back( p );
	}


	if( SHOW_CURSOR )
	{
		FOREACH( PlayerNumber, vpns, p )
			OFF_COMMAND( m_sprCursor[*p] );
	}

	for( unsigned c=0; c<m_aGameCommands.size(); c++ )
	{
		if( GetPage(c) != GetCurrentPage() )
			continue;	// skip

		bool bSelectedByEitherPlayer = false;
		FOREACH( PlayerNumber, vpns, p )
		{
			if( m_iChoice[*p] == (int)c )
				bSelectedByEitherPlayer = true;
		}

		if( SHOW_ICON )
		{
			if( USE_ICON_METRICS )
				OFF_COMMAND( m_vsprIcon[c] );
			else
				m_vsprIcon[c]->PlayCommand( "Off" );

			m_vsprIcon[c]->PlayCommand( bSelectedByEitherPlayer? "OffFocused":"OffUnfocused" );
		}

		if( SHOW_SCROLLER )
		{
			FOREACH( PlayerNumber, vpns, p )
				m_vsprScroll[*p][c]->PlayCommand( bSelectedByEitherPlayer? "OffFocused":"OffUnfocused" );
		}
	}

	if( SHOW_SCROLLER )
	{
		FOREACH( PlayerNumber, vpns, p )
			OFF_COMMAND( m_Scroller[*p] );
	}

	OFF_COMMAND( m_sprExplanation[GetCurrentPage()] );
	OFF_COMMAND( m_sprMore[GetCurrentPage()] );
}


float ScreenSelectMaster::GetCursorX( PlayerNumber pn )
{
	int iChoice = m_iChoice[pn];
	AutoActor spr = m_vsprIcon[iChoice];
	return spr->GetX() + CURSOR_OFFSET_X_FROM_ICON.GetValue(pn);
}

float ScreenSelectMaster::GetCursorY( PlayerNumber pn )
{
	int iChoice = m_iChoice[pn];
	AutoActor &spr = m_vsprIcon[iChoice];
	return spr->GetY() + CURSOR_OFFSET_Y_FROM_ICON.GetValue(pn);
}

/*
 * (c) 2003-2004 Chris Danford
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
