#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSelectGroup.cpp

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectGroup.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "PrefsManager.h"
#include "ScreenSelectMusic.h"
#include "ScreenTitleMenu.h"
#include "GameManager.h"
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "AnnouncerManager.h"
#include "GameState.h"


const float GROUP_INFO_FRAME_X	=	SCREEN_LEFT + 180;
const float GROUP_INFO_FRAME_Y	=	SCREEN_TOP + 160;

const float EXPLANATION_X	=	SCREEN_LEFT + 110;
const float EXPLANATION_Y	=	SCREEN_TOP + 65;

const float BUTTON_X			=	SCREEN_RIGHT - 135;
const float BUTTON_SELECTED_X	=	BUTTON_X - 30;
const float BUTTON_START_Y		=	SCREEN_TOP + 65;
const float BUTTON_GAP_Y		=	26;

const float CONTENTS_X			=	CENTER_X;
const float CONTENTS_START_Y	=	SCREEN_TOP + SCREEN_HEIGHT*3/5 - 15;

const int TITLES_PER_COLUMN = 10;

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_StartFadingOut		 =	ScreenMessage(SM_User + 3);


ScreenSelectGroup::ScreenSelectGroup()
{
	
	LOG->Trace( "ScreenSelectGroup::ScreenSelectGroup()" );	


	int i;


	// The new process by which the group and song lists are formed
	// is bizarre and complex but yields the end result that songs
	// and groups that do not contain any steps for the current
	// style (such as solo) are omitted. Bear with me!
	// -- dro kulix
	
	// m_arrayGroupNames will contain all relevant group names
	CMap<int, int, CArray<Song*, Song*>*, CArray<Song*, Song*>*>
		mapGroupToSongArray; // Will contain all relevant songs

	{	// Let's get local!
		// (There are some variables we want deallocated before continuing.)

		// Retrieve list of ALL song groups and ALL songs

		SONGMAN->GetGroupNames( m_arrayGroupNames );

		CArray<Song*, Song*> arrayAllSongs;
		arrayAllSongs.Copy( SONGMAN->m_pSongs );

		// Retrieve the identities (Song*) of all songs under these groups

		for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
		{
			CArray<Song*, Song*> *p_arrayGroupSongs = new CArray<Song*, Song*>;
			// WARNING! We are creating new objects that must be destroyed.
			const CString &sCurGroupName = m_arrayGroupNames[i];
			
			SONGMAN->GetSongsInGroup( sCurGroupName, *p_arrayGroupSongs );
			mapGroupToSongArray.SetAt(GetHashForString(sCurGroupName), p_arrayGroupSongs);
		}

		{
			// Add "ALL MUSIC" group
			CArray<Song*, Song*> *p_arrayGroupSongs = new CArray<Song*, Song*>;
			// WARNING! (Same as last.)
			m_arrayGroupNames.InsertAt(0, "ALL MUSIC");
			
			p_arrayGroupSongs->Copy( arrayAllSongs );
			mapGroupToSongArray.SetAt(GetHashForString("ALL MUSIC"), p_arrayGroupSongs);
		}

		// Now, create a map of all songs to their support for the current style.
		// (Song *) -> (Support current style?)
		CMap<Song*, Song*, bool, bool> mapStyleSupport;
		mapStyleSupport.InitHashTable(arrayAllSongs.GetSize());

		// The following loop is functionally identical to one used in MusicWheel
		NotesType curNotesType = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
		for( i=0; i<arrayAllSongs.GetSize(); i++ )
		{
			CArray<Notes*, Notes*> arraySteps;

			arrayAllSongs.GetAt(i)->GetNotesThatMatch( curNotesType, arraySteps );

				if( arraySteps.GetSize() > 0 )
					mapStyleSupport.SetAt(arrayAllSongs.GetAt(i), true);
				else
					mapStyleSupport.SetAt(arrayAllSongs.GetAt(i), false);
		}

		// Here's the fun part!

		// Recurse each group of songs, removing the ones unsupported by this style.
		// If a group ends up empty, remove it from the list.

		{
			int gi = 0;

			// Group iteration
			while (gi < m_arrayGroupNames.GetSize())
			{

				int si = 0;

				int curKey = GetHashForString(m_arrayGroupNames.GetAt(gi));
				CArray <Song*, Song*>* p_arrayGroupSongs = 0;
				mapGroupToSongArray.Lookup(curKey, p_arrayGroupSongs);

				// Song iteration
				while (si < p_arrayGroupSongs->GetSize())
				{
					Song* curSong = p_arrayGroupSongs->GetAt(si);
					bool bSongSupported = false;

					mapStyleSupport.Lookup(curSong, bSongSupported);

					if (bSongSupported)
					{
						// Increment only if no remove
						si++;
					}
					else
					{
						// Remove
						p_arrayGroupSongs->RemoveAt(si);
					}
				}


				// Do we remove group?
				// Never remove "ALL MUSIC".
				if ( (gi > 0) && (p_arrayGroupSongs->GetSize() <= 0) )
				{
					// Yes, remove group, since it is empty and is not "ALL MUSIC".
					// Deallocate empty song array
					delete p_arrayGroupSongs;
					// Remove from groups array and map
					mapGroupToSongArray.RemoveKey(curKey);
					m_arrayGroupNames.RemoveAt(gi);
				}
				else
				{
					// Keep this group; move on.
					gi++;
				}

			}
		}	// End recurse
	}	// Ends local song-paring block.


	// Retrieve list of song groups
	//SONGMAN->GetGroupNames( m_arrayGroupNames );
	//m_arrayGroupNames.InsertAt( 0, "ALL MUSIC" );

	

	m_iSelection = 0;
	m_bChosen = false;

	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_GROUP_BACKGROUND) , 
		THEME->GetPathTo(GRAPHIC_SELECT_GROUP_TOP_EDGE),
		ssprintf("Use %c %c to select, then press START", char(1), char(2)),
		false, true, 40 
		);
	this->AddSubActor( &m_Menu );

	m_sprExplanation.Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_EXPLANATION) );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddSubActor( &m_sprExplanation );

	m_GroupInfoFrame.SetXY( GROUP_INFO_FRAME_X, GROUP_INFO_FRAME_Y );
	this->AddSubActor( &m_GroupInfoFrame );

	m_sprContentsHeader.Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_CONTENTS_HEADER) );
	m_sprContentsHeader.SetVertAlign( Actor::align_top );
	m_sprContentsHeader.SetXY( CONTENTS_X, CONTENTS_START_Y );
	this->AddSubActor( &m_sprContentsHeader );

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		const float fX = SCREEN_WIDTH * i / (float)NUM_CONTENTS_COLUMNS + 15;
		m_textContents[i].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textContents[i].SetXY( fX, CONTENTS_START_Y+15 );
		m_textContents[i].SetHorizAlign( Actor::align_left );
		m_textContents[i].SetVertAlign( Actor::align_bottom );
		m_textContents[i].SetZoom( 0.5f );
		m_textContents[i].SetShadowLength( 2 );
		this->AddSubActor( &m_textContents[i] );
	}
	

	// This part is still compatible with the new pare routine.
	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		CString sGroupName = m_arrayGroupNames[i];

		m_sprGroupButton[i].Load( THEME->GetPathTo(GRAPHIC_SELECT_GROUP_BUTTON) );
		m_sprGroupButton[i].SetXY( BUTTON_X, BUTTON_START_Y + BUTTON_GAP_Y*i );
		this->AddSubActor( &m_sprGroupButton[i] );

		m_textGroup[i].Load( THEME->GetPathTo(FONT_NORMAL) );
		m_textGroup[i].SetXY( BUTTON_X, BUTTON_START_Y + BUTTON_GAP_Y*i );
		m_textGroup[i].SetText( SONGMAN->ShortenGroupName( sGroupName ) );
		m_textGroup[i].SetZoom( 0.8f );
		m_textGroup[i].SetShadowLength( 2 );

		if( i == 0 )	m_textGroup[i].TurnRainbowOn();
		else			m_textGroup[i].SetDiffuseColor( SONGMAN->GetGroupColor(sGroupName) );

		this->AddSubActor( &m_textGroup[i] );
	}

	//
	// Generate what text will show in the contents for each group
	//

	/*
	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		CArray<Song*, Song*> arraySongs;
		const CString &sCurGroupName = m_arrayGroupNames[i];
		
		if( i == 0 )	arraySongs.Copy( SONGMAN->m_pSongs );
		else			SONGMAN->GetSongsInGroup( sCurGroupName, arraySongs );

		m_iNumSongsInGroup[i] = arraySongs.GetSize();

		SortSongPointerArrayByTitle( arraySongs );

		for( int c=0; c<NUM_CONTENTS_COLUMNS; c++ )
		{
			CString sText;
			for( int j=c*TITLES_PER_COLUMN; j<(c+1)*TITLES_PER_COLUMN; j++ )
			{
				if( j < arraySongs.GetSize() )
				{
					if( j == NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 1 )
						sText += ssprintf( "%d more.....", arraySongs.GetSize() - NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 1 );
					else
						sText += arraySongs[j]->GetFullTitle() + "\n";
				}
			}
			m_sContentsText[i][c] = sText;
		}
	}
	*/

	//
	// Generate what text will show in the contents for each group
	//
	// This block is modified for the new paring routine
	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		CArray<Song*, Song*>* p_arraySongs;
		const CString &sCurGroupName = m_arrayGroupNames[i];
		
		mapGroupToSongArray.Lookup( GetHashForString(sCurGroupName), p_arraySongs);

		m_iNumSongsInGroup[i] = p_arraySongs->GetSize();

		SortSongPointerArrayByTitle( *p_arraySongs );

		for( int c=0; c<NUM_CONTENTS_COLUMNS; c++ )
		{
			CString sText;
			for( int j=c*TITLES_PER_COLUMN; j<(c+1)*TITLES_PER_COLUMN; j++ )
			{
				if( j < p_arraySongs->GetSize() )
				{
					if( j == NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 1 )
						sText += ssprintf( "%d more.....", p_arraySongs->GetSize() - NUM_CONTENTS_COLUMNS * TITLES_PER_COLUMN - 1 );
					else
						sText += (*p_arraySongs)[j]->GetFullTitle() + "\n";
				}
			}
			m_sContentsText[i][c] = sText;
		}
	}

	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_GROUP_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

	m_Menu.TweenOnScreenFromMenu( SM_None );
	TweenOnScreen();
	AfterChange();


	// Deallocate remaining song arrays in map
	{
		int csKey;
		CArray<Song*, Song*>* pSongArray;

		POSITION Pos;
		Pos = mapGroupToSongArray.GetStartPosition();


		while (Pos)
		{
			mapGroupToSongArray.GetNextAssoc(Pos, csKey, pSongArray);
			delete pSongArray;
		}
	}
}


ScreenSelectGroup::~ScreenSelectGroup()
{
	LOG->Trace( "ScreenSelectGroup::~ScreenSelectGroup()" );

}


void ScreenSelectGroup::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenSelectGroup::Input()" );

	if( m_Menu.IsClosing()  ||  m_bChosen )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

void ScreenSelectGroup::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectGroup::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenSelectMusic );
		break;
	case SM_StartFadingOut:
		m_Menu.TweenOffScreenToMenu( SM_GoToNextState );
		break;
	}
}

void ScreenSelectGroup::BeforeChange()
{
	int iSel = m_iSelection;

	m_sprGroupButton[iSel].BeginTweening( 0.2f );
	m_sprGroupButton[iSel].SetTweenX( BUTTON_X );
	m_sprGroupButton[iSel].SetEffectNone();

	m_textGroup[iSel].BeginTweening( 0.2f );
	m_textGroup[iSel].SetTweenX( BUTTON_X );
	m_textGroup[iSel].SetEffectNone();
}

void ScreenSelectGroup::AfterChange()
{
	int iSel = m_iSelection;

	m_sprGroupButton[iSel].BeginTweening( 0.2f );
	m_sprGroupButton[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_sprGroupButton[iSel].SetEffectGlowing();

	m_textGroup[iSel].BeginTweening( 0.2f );
	m_textGroup[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_textGroup[iSel].SetEffectGlowing();

	for( int c=0; c<NUM_CONTENTS_COLUMNS; c++ )
		m_textContents[c].SetText( m_sContentsText[m_iSelection][c] );

	CString sSelectedGroupName = m_arrayGroupNames[m_iSelection];

	CString sGroupBannerPath;
	if( 0 == stricmp(sSelectedGroupName, "ALL MUSIC") )
		sGroupBannerPath = THEME->GetPathTo(GRAPHIC_ALL_MUSIC_BANNER);
	else if( SONGMAN->GetGroupBannerPath(sSelectedGroupName) != "" )
		sGroupBannerPath = SONGMAN->GetGroupBannerPath(sSelectedGroupName);
	else
		sGroupBannerPath = THEME->GetPathTo(GRAPHIC_FALLBACK_BANNER);

	// There is too much Z-fighting when we rotate this, so fake a rotation with a squash
	m_GroupInfoFrame.Set( sGroupBannerPath, m_iNumSongsInGroup[m_iSelection] );
}


void ScreenSelectGroup::MenuLeft( const PlayerNumber p )
{
	MenuUp( p );
}

void ScreenSelectGroup::MenuRight( const PlayerNumber p )
{
	MenuDown( p );
}

void ScreenSelectGroup::MenuUp( const PlayerNumber p )
{
	if( m_bChosen )
		return;

	BeforeChange();

	m_iSelection = m_iSelection-1 % m_arrayGroupNames.GetSize();
	if( m_iSelection < 0 )
		m_iSelection += m_arrayGroupNames.GetSize();

	AfterChange();

	m_soundChange.PlayRandom();
}


void ScreenSelectGroup::MenuDown( const PlayerNumber p )
{
	if( m_bChosen )
		return;

	BeforeChange();

	m_iSelection = (m_iSelection+1) % m_arrayGroupNames.GetSize();
	
	AfterChange();

	m_soundChange.PlayRandom();
}

void ScreenSelectGroup::MenuStart( const PlayerNumber p )
{
	m_soundSelect.PlayRandom();
	m_bChosen = true;

	GAMESTATE->m_pCurSong = NULL;
	GAMESTATE->m_sPreferredGroup = m_arrayGroupNames[m_iSelection];

	if( 0 == stricmp(GAMESTATE->m_sPreferredGroup, "All Music") )
        SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_COMMENT_ALL_MUSIC) );
	else
        SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_COMMENT_GENERAL) );


	TweenOffScreen();

	this->SendScreenMessage( SM_StartFadingOut, 0.8f );
}

void ScreenSelectGroup::MenuBack( const PlayerNumber p )
{
	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
}

void ScreenSelectGroup::TweenOffScreen()
{
	m_sprExplanation.BeginTweeningQueued( 0.8f );
	m_sprExplanation.BeginTweeningQueued( 0.5f, TWEEN_BOUNCE_BEGIN );
	m_sprExplanation.SetTweenX( EXPLANATION_X-400 );

	m_GroupInfoFrame.BeginTweeningQueued( 0.9f );
	m_GroupInfoFrame.BeginTweeningQueued( 0.5f, TWEEN_BOUNCE_BEGIN );
	m_GroupInfoFrame.SetTweenX( GROUP_INFO_FRAME_X-400 );

	m_sprContentsHeader.BeginTweeningQueued( 0.7f );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.SetTweenY( CONTENTS_START_Y+400 );

	int i;

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		m_textContents[i].BeginTweeningQueued( 0.7f );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		if( i == m_iSelection )
			m_sprGroupButton[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_sprGroupButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_sprGroupButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_sprGroupButton[i].SetTweenX( BUTTON_X+400 );

		if( i == m_iSelection )
			m_textGroup[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_textGroup[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_textGroup[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_textGroup[i].SetTweenX( BUTTON_X+400 );
	}
}

void ScreenSelectGroup::TweenOnScreen() 
{
	m_sprExplanation.SetX( EXPLANATION_X-400 );
	m_sprExplanation.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_sprExplanation.SetTweenX( EXPLANATION_X );

	m_GroupInfoFrame.SetX( GROUP_INFO_FRAME_X-400 );
	m_GroupInfoFrame.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_GroupInfoFrame.SetTweenX( GROUP_INFO_FRAME_X );

	m_sprContentsHeader.SetY( CONTENTS_START_Y+400 );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContentsHeader.SetTweenY( CONTENTS_START_Y );

	int i;

	for( i=0; i<NUM_CONTENTS_COLUMNS; i++ )
	{
		m_textContents[i].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].BeginTweeningQueued( 0.5f );
		m_textContents[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		m_sprGroupButton[i].SetX( BUTTON_X+400 );
		m_sprGroupButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_sprGroupButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_sprGroupButton[i].SetTweenX( BUTTON_X );

		m_textGroup[i].SetX( BUTTON_X+400 );
		m_textGroup[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_textGroup[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_textGroup[i].SetTweenX( BUTTON_X );
	}
}
