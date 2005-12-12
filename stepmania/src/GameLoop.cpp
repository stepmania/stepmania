#include "global.h"
#include "GameLoop.h"
#include "RageLog.h"
#include "RageTextureManager.h"
#include "RageSoundManager.h"
#include "PrefsManager.h"
#include "RageDisplay.h"

#include "arch/ArchHooks/ArchHooks.h"

#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "MemoryCardManager.h"
#include "ScreenManager.h"
#include "InputFilter.h"
#include "RageFileManager.h"
#include "LightsManager.h"
#include "NetworkSyncManager.h"
#include "RageTimer.h"

#include "StepMania.h"

static bool g_bQuitting = false;
static RageTimer g_GameplayTimer;

void ExitGame()
{
	g_bQuitting = true;
}

static bool UserQuit()
{
	return g_bQuitting || HOOKS->UserQuit();
}

void HandleInputEvents( float fDeltaTime );



static void CheckGameLoopTimerSkips( float fDeltaTime )
{
	if( !PREFSMAN->m_bLogSkips )
		return;

	static int iLastFPS = 0;
	int iThisFPS = DISPLAY->GetFPS();

	/* If vsync is on, and we have a solid framerate (vsync == refresh and we've sustained this
	 * for at least one second), we expect the amount of time for the last frame to be 1/FPS. */
	if( iThisFPS != DISPLAY->GetActualVideoModeParams().rate || iThisFPS != iLastFPS )
	{
		iLastFPS = iThisFPS;
		return;
	}

	const float fExpectedTime = 1.0f / iThisFPS;
	const float fDifference = fDeltaTime - fExpectedTime;
	if( fabsf(fDifference) > 0.002f && fabsf(fDifference) < 0.100f )
		LOG->Trace( "GameLoop timer skip: %i FPS, expected %.3f, got %.3f (%.3f difference)",
			iThisFPS, fExpectedTime, fDeltaTime, fDifference );
}

void GameLoop()
{
	while( !UserQuit() )
	{
		/*
		 * Update
		 */
		float fDeltaTime = g_GameplayTimer.GetDeltaTime();

		if( PREFSMAN->m_fConstantUpdateDeltaSeconds > 0 )
			fDeltaTime = PREFSMAN->m_fConstantUpdateDeltaSeconds;
		
		CheckGameLoopTimerSkips( fDeltaTime );

		if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_TAB) ) ) {
			if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT) ) )
				fDeltaTime = 0; /* both; stop time */
			else
				fDeltaTime *= 4;
		}
		else if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_ACCENT) ) )
		{
			fDeltaTime /= 4;
		}

		/* Update SOUNDMAN early (before any RageSound::GetPosition calls), to flush position data. */
		SOUNDMAN->Update( fDeltaTime );

		/* Update song beat information -before- calling update on all the classes that
		 * depend on it.  If you don't do this first, the classes are all acting on old 
		 * information and will lag.  (but no longer fatally, due to timestamping -glenn) */
		SOUND->Update( fDeltaTime );
		TEXTUREMAN->Update( fDeltaTime );
		GAMESTATE->Update( fDeltaTime );
		SCREENMAN->Update( fDeltaTime );
		MEMCARDMAN->Update( fDeltaTime );
		NSMAN->Update( fDeltaTime );

		/* Important:  Process input AFTER updating game logic, or input will be acting on song beat from last frame */
		HandleInputEvents( fDeltaTime );

		LIGHTSMAN->Update( fDeltaTime );

		/*
		 * Render
		 */
		SCREENMAN->Draw();

		/* If we don't have focus, give up lots of CPU. */
		// XXX: do this in DISPLAY EndFrame?
		if( !StepMania::AppHasFocus() )
			usleep( 10000 );// give some time to other processes and threads
#if defined(_WINDOWS)
		/* In Windows, we want to give up some CPU for other threads.  Most OS's do
		 * this more intelligently. */
		else
			usleep( 1000 );	// give some time to other processes and threads
#endif
	}
}

class ConcurrentRenderer
{
public:
	ConcurrentRenderer();
	~ConcurrentRenderer();

private:
	RageThread m_Thread;
	bool m_bShutdown;
	void RenderThread();
	static int StartRenderThread( void *p );
};
static ConcurrentRenderer *g_pConcurrentRenderer = NULL;

ConcurrentRenderer::ConcurrentRenderer()
{
	m_bShutdown = false;

	m_Thread.SetName( "ConcurrentRenderer" );
	m_Thread.Create( StartRenderThread, this );
}

ConcurrentRenderer::~ConcurrentRenderer()
{
	m_bShutdown = true;
	m_Thread.Wait();
}

void ConcurrentRenderer::RenderThread()
{
	ASSERT( SCREENMAN != NULL );

	HOOKS->SetupConcurrentRenderingThread();

	LOG->Trace( "ConcurrentRenderer::RenderThread start" );

	/* This is called during Update().  The next thing the game loop
	 * will do is Draw, so shift operations around to put Draw at the
	 * top.  This makes sure updates are seamless. */
	while( !m_bShutdown )
	{
		SCREENMAN->Draw();

		float fDeltaTime = g_GameplayTimer.GetDeltaTime();
		SCREENMAN->Update( fDeltaTime );
	}

	LOG->Trace( "ConcurrentRenderer::RenderThread done" );
}

int ConcurrentRenderer::StartRenderThread( void *p )
{
	((ConcurrentRenderer *) p)->RenderThread();
	return 0;
}

void StartConcurrentRendering()
{
	ASSERT( g_pConcurrentRenderer == NULL );
	g_pConcurrentRenderer = new ConcurrentRenderer;
}

void FinishConcurrentRendering()
{
	SAFE_DELETE( g_pConcurrentRenderer );
}

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
