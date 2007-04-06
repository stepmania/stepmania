#include "global.h"
#include "OptionsList.h"
#include "GameState.h"
#include "RageLog.h"
#include "Course.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Style.h"
#include "InputEventPlus.h"
#include "CodeDetector.h"
#include "InputMapper.h"
#include "PlayerState.h"

#define LINE(sLineName)				THEME->GetMetric (m_sName,ssprintf("Line%s",sLineName.c_str()))
#define MAX_ITEMS_BEFORE_SPLIT			THEME->GetMetricI(m_sName,"MaxItemsBeforeSplit")
#define ITEMS_SPLIT_WIDTH			THEME->GetMetricF(m_sName,"ItemsSplitWidth")
#define DIRECT_LINES				THEME->GetMetric (m_sName,"DirectLines")

static const RString RESET_ROW = "ResetOptions";

void OptionListRow::Load( OptionsList *pOptions, const RString &sType )
{
	m_pOptions = pOptions;
	ITEMS_SPACING_Y	.Load(sType,"ItemsSpacingY");

	m_Text.resize( 1 );
	m_Text[0].SetName( "Text" );
	m_Text[0].LoadFromFont( THEME->GetPathF(sType, "normal") );
	ActorUtil::LoadAllCommands( m_Text[0], sType );

	m_Underlines.resize( 1 );
	m_Underlines[0].Load( THEME->GetPathG(sType, "underline") );
	m_Underlines[0]->SetName( "Underline" );
	ActorUtil::LoadAllCommands( *m_Underlines[0], sType );

	m_Text[0].PlayCommand( "On" );
	m_Underlines[0]->PlayCommand( "On" );
}

// GetTitleForHandler
// can always use the title
// can have special speed row titles, note skin, or any SELECT_ONE

void OptionListRow::SetFromHandler( const OptionRowHandler *pHandler )
{
	this->RemoveAllChildren();

	if( pHandler == NULL )
		return;

	int iNum = max( pHandler->m_Def.m_vsChoices.size(), m_Text.size() )+1;
	m_Text.resize( iNum, m_Text[0] );
	m_Underlines.resize( iNum, m_Underlines[0] );

	for( unsigned i = 0; i < pHandler->m_Def.m_vsChoices.size(); ++i )
	{
		// init underlines
		this->AddChild( m_Underlines[i] );

		// init text
		this->AddChild( &m_Text[i] );
	}

	SetTextFromHandler( pHandler );

	const unsigned iCnt = pHandler->m_Def.m_vsChoices.size();
	m_bItemsInTwoRows = (int) iCnt > MAX_ITEMS_BEFORE_SPLIT;
	const float fWidth = ITEMS_SPLIT_WIDTH;
	float fY = 0;
	for( unsigned i = 0; i < iCnt; ++i )
	{
		float fX = 0;
		if( m_bItemsInTwoRows )
		{
			if( (i % 2) == 0 )
				fX = -fWidth/2;
			else
				fX = +fWidth/2;
		}

		// set the Y position of each item in the line
		m_Text[i].SetXY( fX, fY );
		m_Underlines[i]->SetXY( fX, fY );

		if( m_bItemsInTwoRows )
			m_Underlines[i]->PlayCommand( "SetTwoRows" );
		else
			m_Underlines[i]->PlayCommand( "SetOneRow" );

		if( !m_bItemsInTwoRows || (i % 2) == 1 || i+1 == iCnt )
			fY += ITEMS_SPACING_Y;
	}

	int iExit = pHandler->m_Def.m_vsChoices.size();
	m_Text[iExit].SetText( "Exit" ); // XXX localize
	m_Text[iExit].SetXY( 0, fY );
	this->AddChild( &m_Text[iExit] );
}

void OptionListRow::SetTextFromHandler( const OptionRowHandler *pHandler )
{
	ASSERT( pHandler );
	for( unsigned i = 0; i < pHandler->m_Def.m_vsChoices.size(); ++i )
	{
		// init text
		RString sText = pHandler->GetThemedItemText( i );

		RString sDest = pHandler->GetScreen( i );
		if( m_pOptions->m_setDirectRows.find(sDest) != m_pOptions->m_setDirectRows.end() && sDest.size() )
		{
			const OptionRowHandler *pTarget = m_pOptions->m_Rows[sDest];
			if( pTarget->m_Def.m_selectType == SELECT_ONE )
			{
				int iSelection = m_pOptions->GetOneSelection(sDest);
				sText += ": " + pTarget->GetThemedItemText( iSelection );
			}
		}

		m_Text[i].SetText( sText );
	}
}

void OptionListRow::SetUnderlines( const vector<bool> &aSelections, const OptionRowHandler *pHandler )
{
	for( unsigned i = 0; i < aSelections.size(); ++i )
	{
		Actor *pActor = m_Underlines[i];

		bool bSelected = aSelections[i];
		RString sDest = pHandler->GetScreen( i );
		if( sDest.size() )
		{
			/* This is a submenu.  Underline the row if its options have been changed
			 * from the default. */
			const OptionRowHandler *pTarget = m_pOptions->m_Rows[sDest];
			if( pTarget->m_Def.m_selectType == SELECT_ONE )
			{
				int iSelection = m_pOptions->GetOneSelection(sDest);
				const OptionRowHandler *pHandler = m_pOptions->m_Rows.find(sDest)->second;
				int iDefault = pHandler->GetDefaultOption();
				if( iDefault != -1 && iSelection != iDefault )
					bSelected |= true;
			}
			else if( pTarget->m_Def.m_selectType == SELECT_MULTIPLE )
			{
				const vector<bool> &bTargetSelections = m_pOptions->m_bSelections.find(sDest)->second;
				for( unsigned i=0; i<bTargetSelections.size(); i++ )
				{
					if( bTargetSelections[i] )
						bSelected = true;
				}
			}
		}

		pActor->PlayCommand( bSelected?"Show":"Hide" );
	}
}

void OptionListRow::PositionCursor( Actor *pCursor, int iSelection )
{
	float fX = m_Text[iSelection].GetDestX();
	float fY = m_Text[iSelection].GetDestY();
	if( m_bItemsInTwoRows )
		pCursor->PlayCommand( "PositionTwoRows" );
	else
		pCursor->PlayCommand( "PositionOneRow" );
	pCursor->SetXY( fX, fY );
}

OptionsList::OptionsList()
{
	m_iCurrentRow = 0;
	m_pLinked = NULL;
}

OptionsList::~OptionsList()
{
	FOREACHM( RString, OptionRowHandler *, m_Rows, hand )
		delete hand->second;
}

void OptionsList::Load( RString sType, PlayerNumber pn )
{
	m_pn = pn;
	m_bStartIsDown = false;

	m_Codes.Load( sType );

	m_Cursor.Load( THEME->GetPathG(sType, "cursor") );
	m_Cursor->SetName( "Cursor" );
	ActorUtil::LoadAllCommands( *m_Cursor, sType );
	this->AddChild( m_Cursor );

	vector<RString> asDirectLines;
	split( DIRECT_LINES, ",", asDirectLines, true );
	FOREACH( RString, asDirectLines, s )
		m_setDirectRows.insert( *s );

	vector<RString> setToLoad;
	setToLoad.push_back( "Main" );

	while( !setToLoad.empty() )
	{
		RString sLineName = *setToLoad.begin();
		setToLoad.erase( setToLoad.begin() );

		if( m_Rows.find(sLineName) != m_Rows.end() )
			continue;

		RString sRowCommands = LINE(sLineName);
		Commands cmds;
		ParseCommands( sRowCommands, cmds );

		OptionRowHandler *pHand = OptionRowHandlerUtil::Make( cmds );
		if( pHand == NULL )
			RageException::Throw( "Invalid OptionRowHandler '%s' in %s::Line%s", cmds.GetOriginalCommandString().c_str(), m_sName.c_str(), sLineName.c_str() );

		m_Rows[sLineName] = pHand;
		m_asLoadedRows.push_back( sLineName );

		for( size_t i = 0; i < pHand->m_Def.m_vsChoices.size(); ++i )
		{
			RString sScreen = pHand->GetScreen(i);
			if( !sScreen.empty() )
				setToLoad.push_back( sScreen );
		}
	}

	for( int i = 0; i < 2; ++i )
	{
		m_Row[i].SetName( "OptionsList" );
		m_Row[i].Load( this, "OptionsList" );
		ActorUtil::LoadAllCommands( m_Row[i], sType );
		this->AddChild( &m_Row[i] );
	}

	this->PlayCommand( "TweenOff" );
	this->FinishTweening();

	/* Import options. */
	FOREACHM( RString, OptionRowHandler *, m_Rows, hand )
	{
		RString sLineName = hand->first;
		ImportRow( sLineName );
	}
}

void OptionsList::Open()
{	
	this->PlayCommand( "Reset" );

	/* Push the initial menu. */
	ASSERT( m_asMenuStack.size() == 0 );
	Push( "Main" );

	this->FinishTweening();
	m_Row[!m_iCurrentRow].SetFromHandler( NULL );
	this->PlayCommand( "TweenOn" );
}

void OptionsList::Close()
{
	m_bStartIsDown = false;
	m_asMenuStack.clear();
	this->PlayCommand( "TweenOff" );
}

RString OptionsList::GetCurrentRow() const
{
	ASSERT( !m_asMenuStack.empty() ); // called while the menu was closed
	return m_asMenuStack.back();
}

const OptionRowHandler *OptionsList::GetCurrentHandler()
{
	RString sCurrentRow = GetCurrentRow();
	return m_Rows[sCurrentRow];
}

int OptionsList::GetOneSelection( RString sRow, bool bAllowFail ) const
{
	const vector<bool> &bSelections = m_bSelections.find(sRow)->second;
	for( unsigned i=0; i<bSelections.size(); i++ )
	{
		if( bSelections[i] )
			return i;
	}

	ASSERT( bAllowFail );	// shouldn't call this if not expecting one to be selected
	return -1;
}

void OptionsList::PositionCursor()
{
	m_Row[m_iCurrentRow].PositionCursor( m_Cursor, m_iMenuStackSelection );
}

/* Toggle to the next menu.  This is used to switch quickly through option submenus,
 * to choose many options or to find the one you're looking for.  For that goal,
 * it's not helpful to switch only through the options listed in the current parent
 * menu; always toggle through the whole set.  Skip menus that only contain other
 * menus. */
void OptionsList::SwitchMenu( int iDir )
{
	int iCurrentRow = 0;
	for( size_t i = 0; i < m_asLoadedRows.size(); ++i )
		if( m_asLoadedRows[i] == m_asMenuStack.back() )
			iCurrentRow = i;

	const int iCurrentRowStart = iCurrentRow;
	RString sDest;
	do
	{
		iCurrentRow += iDir;
		wrap( iCurrentRow, m_asLoadedRows.size() );
		sDest = m_asLoadedRows[iCurrentRow];
	}
	while( sDest == "Main" && iCurrentRow != iCurrentRowStart );

	ASSERT( !sDest.empty() );
	if( m_asMenuStack.size() == 1 )
		m_asMenuStack.push_back( sDest );
	else
		m_asMenuStack.back() = sDest;

	SetDefaultCurrentRow();
	SwitchToCurrentRow();
	TweenOnCurrentRow( iDir > 0 );
}

void OptionsList::MoveItem( const RString &sRowName, int iMove )
{
}

void OptionsList::Input( const InputEventPlus &input )
{
	Message msg("");
	if( m_Codes.InputMessage(input, msg) )
		this->HandleMessage( msg );

	const OptionRowHandler *pHandler = GetCurrentHandler();

	PlayerNumber pn = input.pn;
	if( m_bStartIsDown )
	{
		if( input.MenuI == MENU_BUTTON_LEFT || input.MenuI == MENU_BUTTON_RIGHT )
		{
			if( input.type != IET_FIRST_PRESS )
				return;

			m_bAcceptStartRelease = false;

			const RString &sCurrentRow = m_asMenuStack.back();
			vector<bool> &bSelections = m_bSelections[sCurrentRow];
			if( m_iMenuStackSelection == (int)bSelections.size() )
				return;

			RString sDest = pHandler->GetScreen( m_iMenuStackSelection );
			if( m_setDirectRows.find(sDest) != m_setDirectRows.end() && sDest.size() )
			{
				const OptionRowHandler *pTarget = m_Rows[sDest];
				vector<bool> &bTargetSelections = m_bSelections[sDest];

				if( pTarget->m_Def.m_selectType == SELECT_ONE )
				{
					int iSelection = GetOneSelection(sDest);
					int iDir = (input.MenuI == MENU_BUTTON_RIGHT? +1:-1);
					iSelection += iDir;
					wrap( iSelection, bTargetSelections.size() );
					SelectItem( sDest, iSelection );

					Message msg("OptionsListQuickChange");
					msg.SetParam( "Player", pn );
					msg.SetParam( "Direction", iDir );
					MESSAGEMAN->Broadcast( msg );
				}
			}
			return;
		}
	}

	if( input.MenuI == MENU_BUTTON_LEFT )
	{
		if( input.type == IET_RELEASE )
			return;

		if( INPUTMAPPER->IsBeingPressed(MENU_BUTTON_RIGHT, pn) )
		{
			if( input.type == IET_FIRST_PRESS )
				SwitchMenu( -1 );
			return;
		}

		--m_iMenuStackSelection;
		wrap( m_iMenuStackSelection, pHandler->m_Def.m_vsChoices.size()+1 ); // +1 for exit row
		PositionCursor();

		Message msg("OptionsListLeft");
		msg.SetParam( "Player", input.pn );
		MESSAGEMAN->Broadcast( msg );
		return;
	}
	else if( input.MenuI == MENU_BUTTON_RIGHT )
	{
		if( input.type == IET_RELEASE )
			return;

		if( INPUTMAPPER->IsBeingPressed(MENU_BUTTON_LEFT, pn) )
		{
			if( input.type == IET_FIRST_PRESS )
				SwitchMenu( +1 );
			return;
		}

		++m_iMenuStackSelection;
		wrap( m_iMenuStackSelection, pHandler->m_Def.m_vsChoices.size()+1 ); // +1 for exit row
		PositionCursor();

		Message msg("OptionsListRight");
		msg.SetParam( "Player", input.pn );
		MESSAGEMAN->Broadcast( msg );
		return;
	}
	else if( input.MenuI == MENU_BUTTON_START )
	{
		if( input.type == IET_FIRST_PRESS )
		{
			m_bStartIsDown = true;
			m_bAcceptStartRelease = true;
			return;
		}
		if( input.type == IET_RELEASE )
		{
			if( m_bAcceptStartRelease )
				Start();
			m_bStartIsDown = false;
		}

		return;
	}
	else if( input.MenuI == MENU_BUTTON_SELECT )
	{
		if( input.type != IET_FIRST_PRESS )
			return;
//		if( input.type == IET_RELEASE )
		{
			Close();
			return;
		}
		return;
	}
}

void OptionsList::SwitchToCurrentRow()
{
	m_iCurrentRow = !m_iCurrentRow;

	/* Set up the new row. */
	m_Row[m_iCurrentRow].SetFromHandler( GetCurrentHandler() );
	m_Row[m_iCurrentRow].SetUnderlines( m_bSelections[m_asMenuStack.back()], GetCurrentHandler() );
	PositionCursor();

	Message msg("OptionsMenuChanged");
	msg.SetParam( "Player", m_pn );
	msg.SetParam( "Menu", m_asMenuStack.back() );
	MESSAGEMAN->Broadcast( msg );
}

/* After setting up a new row, tween it on. */
void OptionsList::TweenOnCurrentRow( bool bForward )
{
	OptionListRow &OldRow = m_Row[!m_iCurrentRow];
	OptionListRow &NewRow = m_Row[m_iCurrentRow];

	/* Tween out the old row. */
	if( bForward )
		OldRow.PlayCommand( "TweenOutForward" );
	else
		OldRow.PlayCommand( "TweenOutBackward" );

	/* Tween in the old row. */
	if( bForward )
		NewRow.PlayCommand( "TweenInForward" );
	else
		NewRow.PlayCommand( "TweenInBackward" );
}

void OptionsList::ImportRow( RString sRow )
{
	vector<bool> aSelections[NUM_PLAYERS];
	vector<PlayerNumber> vpns;
	vpns.push_back( m_pn );
	OptionRowHandler *pHandler = m_Rows[sRow];
	aSelections[ m_pn ].resize( pHandler->m_Def.m_vsChoices.size() );
	pHandler->ImportOption( vpns, aSelections );
	m_bSelections[sRow] = aSelections[ m_pn ];

	if( sRow == "Main" )
		fill( m_bSelections[sRow].begin(), m_bSelections[sRow].end(), false );
}

void OptionsList::ExportRow( RString sRow )
{
	if( sRow == "Main" )
		return;

	vector<bool> aSelections[NUM_PLAYERS];
	aSelections[m_pn] = m_bSelections[sRow];

	vector<PlayerNumber> vpns;
	vpns.push_back( m_pn );

	m_Rows[sRow]->ExportOption( vpns, aSelections );
}

void OptionsList::SetDefaultCurrentRow()
{
	/* If all items on the row just point to other menus, default to 0. */
	m_iMenuStackSelection = 0;

	const RString &sCurrentRow = m_asMenuStack.back();
	const OptionRowHandler *pHandler = m_Rows.find(sCurrentRow)->second;
	if( pHandler->m_Def.m_selectType == SELECT_ONE )
	{
		/* One item is selected, so position the cursor on it. */
		m_iMenuStackSelection = GetOneSelection( sCurrentRow, true );
		if( m_iMenuStackSelection == -1 )
			m_iMenuStackSelection = 0;
	}
}

void OptionsList::Pop()
{
	if( m_asMenuStack.size() == 1 )
	{
		Close();
		return;
	}

	RString sLastMenu = m_asMenuStack.back();

	m_asMenuStack.pop_back();

	/* Choose the default option. */
	SetDefaultCurrentRow();

	/* If the old menu exists as a target from the new menu, switch to it. */
	const OptionRowHandler *pHandler = GetCurrentHandler();
	RString sDest;
	for( size_t i = 0; i < pHandler->m_Def.m_vsChoices.size(); ++i )
	{
		if( pHandler->GetScreen(i) == sLastMenu )
		{
			m_iMenuStackSelection = i;
			break;
		}
	}

	SwitchToCurrentRow();
	TweenOnCurrentRow( false );
}

void OptionsList::Push( RString sDest )
{
	m_asMenuStack.push_back( sDest );
	SetDefaultCurrentRow();
	SwitchToCurrentRow();
}

void OptionsList::SelectItem( const RString &sRowName, int iMenuItem )
{
	const OptionRowHandler *pHandler = m_Rows[sRowName];
	vector<bool> &bSelections = m_bSelections[sRowName];

	if( pHandler->m_Def.m_selectType == SELECT_MULTIPLE )
	{
		bool bSelected = !bSelections[iMenuItem];
		bSelections[iMenuItem] = bSelected;

//		if( bSelected )
//			m_SoundToggleOn.Play();
//		else
//			m_SoundToggleOff.Play();
	}
	else	// data.selectType != SELECT_MULTIPLE
	{
		fill( bSelections.begin(), bSelections.end(), false ); 
		bSelections[iMenuItem] = true;
	}

	ExportRow( sRowName );

	UpdateMenuFromSelections();

	if( pHandler->m_Def.m_bOneChoiceForAllPlayers && m_pLinked != NULL )
	{
		vector<bool> &bLinkedSelections = m_pLinked->m_bSelections[sRowName];
		bLinkedSelections = bSelections;

		if( m_pLinked->IsOpened() )
			m_pLinked->UpdateMenuFromSelections();
	}
}

void OptionsList::UpdateMenuFromSelections()
{
	const vector<bool> &bCurrentSelections = m_bSelections.find(GetCurrentRow())->second;
	m_Row[m_iCurrentRow].SetUnderlines( bCurrentSelections, GetCurrentHandler() );
	m_Row[m_iCurrentRow].SetTextFromHandler( GetCurrentHandler() );
}

bool OptionsList::Start()
{
	const OptionRowHandler *pHandler = GetCurrentHandler();
	const RString &sCurrentRow = m_asMenuStack.back();
	vector<bool> &bSelections = m_bSelections[sCurrentRow];
	if( m_iMenuStackSelection == (int)bSelections.size() )
	{
		Pop();

		Message msg("OptionsListPop");
		msg.SetParam( "Player", m_pn );
		MESSAGEMAN->Broadcast( msg );

		return m_asMenuStack.empty();
	}

	{
		RString sIconText;
		GameCommand gc;
		pHandler->GetIconTextAndGameCommand( m_iMenuStackSelection, sIconText, gc );
		if( gc.m_sName == RESET_ROW )
		{
			GAMESTATE->m_pPlayerState[m_pn]->ResetToDefaultPlayerOptions( ModsLevel_Preferred );
			GAMESTATE->ResetToDefaultSongOptions( ModsLevel_Preferred );

			/* Import options. */
			FOREACHM( RString, OptionRowHandler *, m_Rows, hand )
				ImportRow( hand->first );

			UpdateMenuFromSelections();

			Message msg("OptionsListReset");
			msg.SetParam( "Player", m_pn );
			MESSAGEMAN->Broadcast( msg );

			return false;
		}
	}

	RString sDest = pHandler->GetScreen( m_iMenuStackSelection );
	if( sDest.size() )
	{
		Push( sDest );
		TweenOnCurrentRow( true );

		Message msg("OptionsListPush");
		msg.SetParam( "Player", m_pn );
		MESSAGEMAN->Broadcast( msg );

		return false;
	}

	SelectItem( GetCurrentRow(), m_iMenuStackSelection );

	if( pHandler->m_Def.m_selectType == SELECT_ONE )
	{
		/* Move to the exit row.  Don't exit, so different entries don't have widely
		 * different types, with some exiting the menu level and some not; this also
		 * makes selection using L+R and R+L faster. */
		m_iMenuStackSelection = (int)bSelections.size();
		PositionCursor();
	}

	Message msg("OptionsListStart");
	msg.SetParam( "Player", m_pn );
	MESSAGEMAN->Broadcast( msg );

	return false;
}

/*
 * Copyright (c) 2006 Glenn Maynard
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

