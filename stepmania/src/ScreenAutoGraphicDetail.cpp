#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenAutoGraphicDetail

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenAutoGraphicDetail.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameDef.h"
#include "RageLog.h"
#include "SongManager.h"
#include "NoteFieldPositioning.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "RageTimer.h"
#include "RageDisplay.h"

#define SONG_BPM			THEME->GetMetricF("ScreenAutoGraphicDetail","SongBPM")
#define SECONDS_TO_SHOW		THEME->GetMetricF("ScreenAutoGraphicDetail","SecondsToShow")

enum Detail { low, medium, high, NUM_DETAIL_SETTINGS };
struct DetailSettings
{
	int width, height, displaybpp, texturebpp;
};

/* XXX: It'd be nice to set 800x600 or 1024x768 (instead of increasing the framebuffer
 * bit depth), and to increase the refresh rate to 70 or so; it makes a huge difference,
 * and many people I see playing games play them at the default resolution and refresh,
 * never changing anything.
 *
 * However, on some systems the change will succeed but the monitor won't actually
 * be able to handle it.  Should we do the "press a key in 15 seconds" deal like
 * Windows does? */
const DetailSettings g_DetailSettings[NUM_DETAIL_SETTINGS] = {
	{ 320, 240, 16, 16 },
	{ 640, 480, 16, 16 },
	{ 640, 480, 32, 32 },
};

void ApplyDetailSetting( Detail detail )
{
	PREFSMAN->m_bVsync = false;	// TODO:  preserve vsync pref
	PREFSMAN->m_iDisplayWidth = g_DetailSettings[detail].width;
	PREFSMAN->m_iDisplayHeight = g_DetailSettings[detail].height;
	PREFSMAN->m_iDisplayColorDepth = g_DetailSettings[detail].displaybpp;
	PREFSMAN->m_iTextureColorDepth = g_DetailSettings[detail].texturebpp;

	ApplyGraphicOptions();
}

ScreenAutoGraphicDetail::ScreenAutoGraphicDetail() : Screen("ScreenAutoGraphicDetail")
{
	ApplyDetailSetting( medium );

	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenAutoGraphicDetail background") );
	this->AddChild( &m_Background );

	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;
	NotesType nt = GAMESTATE->GetCurrentStyleDef()->m_NotesType;
	int iNumOfTracks = GAMEMAN->NotesTypeToNumTracks( nt );

	ASSERT(iNumOfTracks > 0); // crazy to have less than 1 track....

	NoteData* pND = new NoteData;
	pND->SetNumTracks( iNumOfTracks );

	float fSongBPM = SONG_BPM;
	float fSongBPS = fSongBPM / 60.f;
	int iBeatsToPlay = int(fSongBPS * SECONDS_TO_SHOW);

	for( int i=0; i<iBeatsToPlay; i++ )
	{
		float fBeat = float(i)+2;
		int iTrack = i % iNumOfTracks;
		bool bIsHold = ((i / iNumOfTracks) % 2) == 1;
		if( bIsHold )
			pND->AddHoldNote( HoldNote(iTrack, fBeat, fBeat+1) );
		else
		{
			pND->SetTapNote( iTrack, BeatToNoteRow(fBeat), TAP_TAP );
			pND->SetTapNote( iTrack, BeatToNoteRow(fBeat+0.5f), TAP_TAP );
		}
	}

	m_pSong = new Song;
	m_pSong->AddBPMSegment( BPMSegment(0,fSongBPM) );

	GAMESTATE->m_pCurSong = m_pSong;
	GAMESTATE->m_bPastHereWeGo = true;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsPlayerEnabled(p) )
			continue;	// skip

		GAMESTATE->m_PlayerController[p] = PC_AUTOPLAY;
		m_Player[p].Load( (PlayerNumber)p, pND, NULL, NULL, NULL, NULL );
		m_Player[p].SetX( (float) GAMESTATE->GetCurrentStyleDef()->m_iCenterX[p] );
		this->AddChild( &m_Player[p] );
	}

	delete pND;


	m_Overlay.LoadFromAniDir( THEME->GetPathToB("ScreenAutoGraphicDetail overlay") );
	this->AddChild( &m_Overlay );

	m_textMessage.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textMessage.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_textMessage );

	m_fFakeSecondsIntoSong = 0;
	this->ClearMessageQueue();
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );
}

ScreenAutoGraphicDetail::~ScreenAutoGraphicDetail()
{
	delete m_pSong;
}

void ScreenAutoGraphicDetail::Update( float fDelta )
{
	m_fFakeSecondsIntoSong += fDelta;
	GAMESTATE->UpdateSongPosition( m_fFakeSecondsIntoSong );

	int iThisSecond = (int)RageTimer::GetTimeSinceStart();
	int iLastSecond = (int)(RageTimer::GetTimeSinceStart()-fDelta);
	if( iThisSecond != iLastSecond )
		m_textMessage.SetText( ssprintf("Analyzing peformance... (FPS: %d)", DISPLAY->GetCumFPS()) );

	Screen::Update( fDelta );
}

void ScreenAutoGraphicDetail::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		int cumFPS = DISPLAY->GetCumFPS();
		if( cumFPS > 120 )
			ApplyDetailSetting( high );
		else if( cumFPS < 60 )
			ApplyDetailSetting( low );
		else 
			ApplyDetailSetting( medium );
		GAMESTATE->m_pCurSong = NULL;
		SCREENMAN->SetNewScreen( THEME->GetMetric("Common","InitialScreen") );
		break;
	}
}
