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

#define LINE(sLineName)				THEME->GetMetric (m_sName,ssprintf("Line%s",sLineName.c_str()))
#define MAX_ITEMS_BEFORE_SPLIT			THEME->GetMetricI(m_sName,"MaxItemsBeforeSplit")
#define ITEMS_SPLIT_WIDTH			THEME->GetMetricF(m_sName,"ItemsSplitWidth")

void OptionListRow::Load( const RString &sType )
{
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
		RString sText = pHandler->GetThemedItemText( i );
		m_Text[i].SetText( sText );
		this->AddChild( &m_Text[i] );
	}

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

void OptionListRow::SetUnderlines( const vector<bool> &aSelections )
{
	for( unsigned i = 0; i < aSelections.size(); ++i )
	{
		Actor *pActor = m_Underlines[i];
		pActor->PlayCommand( aSelections[i]?"Show":"Hide" );
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
}

OptionsList::~OptionsList()
{
	FOREACHM( RString, OptionRowHandler *, m_Rows, hand )
		delete hand->second;
}

void OptionsList::Load( RString sType )
{
	m_Cursor.Load( THEME->GetPathG(sType, "cursor") );
	m_Cursor->SetName( "Cursor" );
	ActorUtil::LoadAllCommands( *m_Cursor, sType );
	this->AddChild( m_Cursor );

	set<RString> setToLoad;
	setToLoad.insert( "Main" );

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
				setToLoad.insert( sScreen );
		}
	}

	for( int i = 0; i < 2; ++i )
	{
		m_Row[i].SetName( "OptionsList" );
		m_Row[i].Load( "OptionsList" );
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
	m_asMenuStack.clear();
	this->PlayCommand( "TweenOff" );
}

const OptionRowHandler *OptionsList::GetCurrentHandler()
{
	ASSERT( !m_asMenuStack.empty() ); // called while the menu was closed
	const RString &sCurrentRow = m_asMenuStack.back();
	return m_Rows[sCurrentRow];
}


void OptionsList::PositionCursor()
{
	m_Row[m_iCurrentRow].PositionCursor( m_Cursor, m_iMenuStackSelection );
}

bool OptionsList::RowIsMenusOnly( RString sRow ) const
{
	map<RString, OptionRowHandler *>::const_iterator it = m_Rows.find( sRow );
	ASSERT( it != m_Rows.end() );
	const OptionRowHandler *pHandler = it->second;

	for( size_t i = 0; i < pHandler->m_Def.m_vsChoices.size(); ++i )
	{
		if( pHandler->GetScreen(i).empty() )
			return false;
	}

	return true;
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
	while( RowIsMenusOnly(sDest) && iCurrentRow != iCurrentRowStart );

	ASSERT( !sDest.empty() );
	if( m_asMenuStack.size() == 1 )
		m_asMenuStack.push_back( sDest );
	else
		m_asMenuStack.back() = sDest;

	SetDefaultCurrentRow();
	SwitchToCurrentRow();
	TweenOnCurrentRow( iDir > 0 );
}

void OptionsList::Input( const InputEventPlus &input )
{
	if( input.type == IET_RELEASE )
		return;

	const OptionRowHandler *pHandler = GetCurrentHandler();

	PlayerNumber pn = input.pn;
	if( input.MenuI == MENU_BUTTON_LEFT )
	{
		if( INPUTMAPPER->IsBeingPressed(MENU_BUTTON_RIGHT, pn) )
		{
			LOG->Trace( "X1" );
			SwitchMenu( -1 );
			return;
		}

		LOG->Trace( "X2" );
		--m_iMenuStackSelection;
		wrap( m_iMenuStackSelection, pHandler->m_Def.m_vsChoices.size()+1 ); // +1 for exit row
		PositionCursor();

		Message msg("OptionsListLeft");
		msg.SetParam( "Player", input.pn );
		MESSAGEMAN->Broadcast( msg );
	}
	else if( input.MenuI == MENU_BUTTON_RIGHT )
	{
		if( INPUTMAPPER->IsBeingPressed(MENU_BUTTON_LEFT, pn) )
		{
			LOG->Trace( "X3" );
			SwitchMenu( +1 );
			return;
		}

		LOG->Trace( "X4" );
		++m_iMenuStackSelection;
		wrap( m_iMenuStackSelection, pHandler->m_Def.m_vsChoices.size()+1 ); // +1 for exit row
		PositionCursor();

		Message msg("OptionsListRight");
		msg.SetParam( "Player", input.pn );
		MESSAGEMAN->Broadcast( msg );
	}
	else if( input.MenuI == MENU_BUTTON_START )
	{
		LOG->Trace( "X5" );
		Start( input.pn );
	}
}

void OptionsList::SwitchToCurrentRow()
{
	m_iCurrentRow = !m_iCurrentRow;

	/* Set up the new row. */
	m_Row[m_iCurrentRow].SetFromHandler( GetCurrentHandler() );
	m_Row[m_iCurrentRow].SetUnderlines( m_bSelections[m_asMenuStack.back()] );
	PositionCursor();
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
	vpns.push_back(PLAYER_1); // XXX
	OptionRowHandler *pHandler = m_Rows[sRow];
	aSelections[PLAYER_1].resize( pHandler->m_Def.m_vsChoices.size() );
	pHandler->ImportOption( vpns, aSelections );
	m_bSelections[sRow] = aSelections[PLAYER_1];

	if( RowIsMenusOnly(sRow) )
		fill( m_bSelections[sRow].begin(), m_bSelections[sRow].end(), false );
}

void OptionsList::ExportRow( RString sRow )
{
	if( RowIsMenusOnly(sRow) )
		return;

	vector<bool> aSelections[NUM_PLAYERS];
	aSelections[PLAYER_1] = m_bSelections[sRow];

	vector<PlayerNumber> vpns;
	vpns.push_back(PLAYER_1); // XXX

	m_Rows[sRow]->ExportOption( vpns, aSelections );
}

void OptionsList::SetDefaultCurrentRow()
{
	const OptionRowHandler *pHandler = GetCurrentHandler();
	const RString &sCurrentRow = m_asMenuStack.back();
	const vector<bool> &bSelections = m_bSelections[sCurrentRow];

	/* Choose the default cursor position, based on the selection and the contents
	 * of the row.  If */
	/* If all items on the row just point to other menus, default to 0. */

	if( pHandler->m_Def.m_selectType == SELECT_ONE )
	{
		/* One item is selected, so position the cursor on it. */
	}

	m_iMenuStackSelection = 0; // XXX
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

bool OptionsList::Start( PlayerNumber pn )
{
	const OptionRowHandler *pHandler = GetCurrentHandler();
	const RString &sCurrentRow = m_asMenuStack.back();
	vector<bool> &bSelections = m_bSelections[sCurrentRow];
	if( m_iMenuStackSelection == (int)bSelections.size() )
	{
		Pop();
		return m_asMenuStack.empty();
	}

	RString sDest = pHandler->GetScreen( m_iMenuStackSelection );
	if( sDest.size() )
	{
		Push( sDest );
		TweenOnCurrentRow( true );
		return false;
	}

	if( pHandler->m_Def.m_selectType == SELECT_MULTIPLE )
	{
		bool bSelected = !bSelections[m_iMenuStackSelection];
		bSelections[m_iMenuStackSelection] = bSelected;

//		if( bSelected )
//			m_SoundToggleOn.Play();
//		else
//			m_SoundToggleOff.Play();
	}
	else	// data.selectType != SELECT_MULTIPLE
	{
		fill( bSelections.begin(), bSelections.end(), false ); 
		bSelections[m_iMenuStackSelection] = true;
	}

	ExportRow( sCurrentRow );

	m_Row[m_iCurrentRow].SetUnderlines( m_bSelections[m_asMenuStack.back()] );

	if( pHandler->m_Def.m_selectType == SELECT_ONE )
	{
		/* Move to the exit row.  Don't exit, so different entries don't have widely
		 * different types, with some exiting the menu level and some not; this also
		 * makes selection using L+R and R+L faster. */
		m_iMenuStackSelection = (int)bSelections.size();
		PositionCursor();
	}

	Message msg("OptionsListStart");
	msg.SetParam( "Player", pn );
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

