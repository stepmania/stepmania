#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectGroup

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


#define FRAME_X				THEME->GetMetricF("SelectGroup","FrameX")
#define FRAME_Y				THEME->GetMetricF("SelectGroup","FrameY")
#define BANNER_X			THEME->GetMetricF("SelectGroup","BannerX")
#define BANNER_Y			THEME->GetMetricF("SelectGroup","BannerY")
#define BANNER_WIDTH		THEME->GetMetricF("SelectGroup","BannerWidth")
#define BANNER_HEIGHT		THEME->GetMetricF("SelectGroup","BannerHeight")
#define NUMBER_X			THEME->GetMetricF("SelectGroup","NumberX")
#define NUMBER_Y			THEME->GetMetricF("SelectGroup","NumberY")
#define EXPLANATION_X		THEME->GetMetricF("SelectGroup","ExplanationX")
#define EXPLANATION_Y		THEME->GetMetricF("SelectGroup","ExplanationY")
#define BUTTON_X			THEME->GetMetricF("SelectGroup","ButtonX")
#define BUTTON_SELECTED_X	THEME->GetMetricF("SelectGroup","ButtonSelectedX")
#define BUTTON_START_Y		THEME->GetMetricF("SelectGroup","ButtonStartY")
#define BUTTON_SPACING_Y	THEME->GetMetricF("SelectGroup","ButtonSpacingY")
#define CONTENTS_X			THEME->GetMetricF("SelectGroup","ContentsX")
#define CONTENTS_Y			THEME->GetMetricF("SelectGroup","ContentsY")
#define TITLES_START_X		THEME->GetMetricF("SelectGroup","TitlesStartX")
#define TITLES_SPACING_X	THEME->GetMetricF("SelectGroup","TitlesSpacingX")
#define TITLES_START_Y		THEME->GetMetricF("SelectGroup","TitlesStartY")
#define TITLES_COLUMNS		THEME->GetMetricI("SelectGroup","TitlesColumns")
#define TITLES_ROWS			THEME->GetMetricI("SelectGroup","TitlesRows")


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);
const ScreenMessage SM_StartFadingOut		=	ScreenMessage(SM_User + 3);


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
		THEME->GetPathTo("Graphics","select group background") , 
		THEME->GetPathTo("Graphics","select group top edge"),
		ssprintf("Use %c %c to select, then press START", char(1), char(2)),
		false, true, 40 
		);
	this->AddSubActor( &m_Menu );

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select group explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddSubActor( &m_sprExplanation );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetXY( BANNER_X, BANNER_Y );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddSubActor( &m_Banner );

	m_sprFrame.Load( THEME->GetPathTo("Graphics","select group info frame") );
	m_sprFrame.SetXY( FRAME_X, FRAME_Y );
	this->AddSubActor( &m_sprFrame );

	m_textNumber.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textNumber.SetHorizAlign( Actor::align_right );
	m_textNumber.TurnShadowOff();
	this->AddSubActor( &m_textNumber );
	
	m_sprContents.Load( THEME->GetPathTo("Graphics","select group contents header") );
	m_sprContents.SetXY( CONTENTS_X, CONTENTS_Y );
	this->AddSubActor( &m_sprContents );

	for( i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textTitles[i].SetXY( TITLES_START_X + i*TITLES_SPACING_X, TITLES_START_Y );
		m_textTitles[i].SetHorizAlign( Actor::align_left );
		m_textTitles[i].SetVertAlign( Actor::align_bottom );
		m_textTitles[i].SetZoom( 0.5f );
		m_textTitles[i].SetShadowLength( 2 );
		this->AddSubActor( &m_textTitles[i] );
	}
	

	// This part is still compatible with the new pare routine.
	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		CString sGroupName = m_arrayGroupNames[i];

		m_sprButton[i].Load( THEME->GetPathTo("Graphics","select group button") );
		m_sprButton[i].SetXY( BUTTON_X, BUTTON_START_Y + i*BUTTON_SPACING_Y );
		this->AddSubActor( &m_sprButton[i] );

		m_textLabel[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
		m_textLabel[i].SetXY( BUTTON_X, BUTTON_START_Y + i*BUTTON_SPACING_Y );
		m_textLabel[i].SetText( SONGMAN->ShortenGroupName( sGroupName ) );
		m_textLabel[i].SetZoom( 0.8f );
		m_textLabel[i].SetShadowLength( 2 );

		if( i == 0 )	m_textLabel[i].TurnRainbowOn();
		else			m_textLabel[i].SetDiffuseColor( SONGMAN->GetGroupColor(sGroupName) );

		this->AddSubActor( &m_textLabel[i] );
	}



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

		for( int c=0; c<TITLES_COLUMNS; c++ )
		{
			CString sText;
			for( int j=c*TITLES_ROWS; j<(c+1)*TITLES_ROWS; j++ )
			{
				if( j < p_arraySongs->GetSize() )
				{
					if( j == TITLES_COLUMNS * TITLES_ROWS - 1 )
						sText += ssprintf( "%d more.....", p_arraySongs->GetSize() - TITLES_COLUMNS * TITLES_ROWS - 1 );
					else
						sText += (*p_arraySongs)[j]->GetFullTitle() + "\n";
				}
			}
			m_sContentsText[i][c] = sText;
		}
	}

	m_soundChange.Load( THEME->GetPathTo("Sounds","select group change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_GROUP_INTRO) );


	if( !MUSIC->IsPlaying()  ||  MUSIC->GetLoadedFilePath() != THEME->GetPathTo("Sounds","select group music") )
	{
		MUSIC->Load( THEME->GetPathTo("Sounds","select group music") );
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

	m_sprButton[iSel].BeginTweening( 0.2f );
	m_sprButton[iSel].SetTweenX( BUTTON_X );
	m_sprButton[iSel].SetEffectNone();

	m_textLabel[iSel].BeginTweening( 0.2f );
	m_textLabel[iSel].SetTweenX( BUTTON_X );
	m_textLabel[iSel].SetEffectNone();
}

void ScreenSelectGroup::AfterChange()
{
	int iSel = m_iSelection;

	m_sprButton[iSel].BeginTweening( 0.2f );
	m_sprButton[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_sprButton[iSel].SetEffectGlowing();

	m_textLabel[iSel].BeginTweening( 0.2f );
	m_textLabel[iSel].SetTweenX( BUTTON_SELECTED_X );
	m_textLabel[iSel].SetEffectGlowing();

	for( int c=0; c<TITLES_COLUMNS; c++ )
		m_textTitles[c].SetText( m_sContentsText[m_iSelection][c] );

	CString sSelectedGroupName = m_arrayGroupNames[m_iSelection];

	CString sGroupBannerPath;
	if( 0 == stricmp(sSelectedGroupName, "ALL MUSIC") )
		sGroupBannerPath = THEME->GetPathTo("Graphics","all music banner");
	else if( SONGMAN->GetGroupBannerPath(sSelectedGroupName) != "" )
		sGroupBannerPath = SONGMAN->GetGroupBannerPath(sSelectedGroupName);
	else
		sGroupBannerPath = THEME->GetPathTo("Graphics","fallback banner");

	const int iNumSongs = m_iNumSongsInGroup[m_iSelection];
	m_textNumber.SetText( ssprintf("%d", iNumSongs) );

	m_Banner.SetFromGroup( sSelectedGroupName );
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
		m_iSelection += min( m_arrayGroupNames.GetSize(), MAX_GROUPS );

	AfterChange();

	m_soundChange.PlayRandom();
}


void ScreenSelectGroup::MenuDown( const PlayerNumber p )
{
	if( m_bChosen )
		return;

	BeforeChange();

	m_iSelection = (m_iSelection+1) % min( m_arrayGroupNames.GetSize(), MAX_GROUPS );
	
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

	Actor* pActorsInGroupInfoFrame[] = { &m_sprFrame, &m_Banner, &m_textNumber };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	for( int i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		pActorsInGroupInfoFrame[i]->BeginTweeningQueued( 0.9f );
		pActorsInGroupInfoFrame[i]->BeginTweeningQueued( 0.5f, TWEEN_BOUNCE_BEGIN );
		pActorsInGroupInfoFrame[i]->SetTweenX( pActorsInGroupInfoFrame[i]->GetX()-400 );
	}

	m_sprContents.BeginTweeningQueued( 0.7f );
	m_sprContents.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContents.SetTweenY( CONTENTS_Y+400 );
	for( i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].BeginTweeningQueued( 0.7f );
		m_textTitles[i].BeginTweeningQueued( 0.5f );
		m_textTitles[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		if( i == m_iSelection )
			m_sprButton[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_sprButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_sprButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_sprButton[i].SetTweenX( BUTTON_X+400 );

		if( i == m_iSelection )
			m_textLabel[i].BeginTweeningQueued( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_textLabel[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_textLabel[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_textLabel[i].SetTweenX( BUTTON_X+400 );
	}
}

void ScreenSelectGroup::TweenOnScreen() 
{
	m_sprExplanation.SetX( EXPLANATION_X-400 );
	m_sprExplanation.BeginTweening( 0.5f, TWEEN_BOUNCE_END );
	m_sprExplanation.SetTweenX( EXPLANATION_X );

	Actor* pActorsInGroupInfoFrame[] = { &m_sprFrame, &m_Banner, &m_textNumber };
	const int iNumActorsInGroupInfoFrame = sizeof(pActorsInGroupInfoFrame) / sizeof(Actor*);
	for( int i=0; i<iNumActorsInGroupInfoFrame; i++ )
	{
		float fOriginalX = pActorsInGroupInfoFrame[i]->GetX();
		pActorsInGroupInfoFrame[i]->SetX( fOriginalX-400 );
		pActorsInGroupInfoFrame[i]->BeginTweening( 0.5f, TWEEN_BOUNCE_END );
		pActorsInGroupInfoFrame[i]->SetTweenX( fOriginalX );
	}

	m_sprContents.SetY( CONTENTS_Y+400 );
	m_sprContents.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );	// sleep
	m_sprContents.BeginTweeningQueued( 0.5f, TWEEN_BIAS_END );
	m_sprContents.SetTweenY( CONTENTS_Y );

	for( i=0; i<TITLES_COLUMNS; i++ )
	{
		m_textTitles[i].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_textTitles[i].BeginTweeningQueued( 0.5f );
		m_textTitles[i].BeginTweeningQueued( 0.5f );
		m_textTitles[i].SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
	}
	

	for( i=0; i<min(m_arrayGroupNames.GetSize(), MAX_GROUPS); i++ )
	{
		m_sprButton[i].SetX( BUTTON_X+400 );
		m_sprButton[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_sprButton[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_sprButton[i].SetTweenX( BUTTON_X );

		m_textLabel[i].SetX( BUTTON_X+400 );
		m_textLabel[i].BeginTweeningQueued( 0.1f*i, TWEEN_BOUNCE_END );
		m_textLabel[i].BeginTweeningQueued( 0.2f, TWEEN_BOUNCE_END );
		m_textLabel[i].SetTweenX( BUTTON_X );
	}
}
