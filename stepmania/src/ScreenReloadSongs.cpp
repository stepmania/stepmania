#include "global.h"

#include "ScreenReloadSongs.h"
#include "SongManager.h"
#include "UnlockSystem.h"
#include "ScreenManager.h"

/* This could be cleaned up: show progress, for example.  Let's not use
 * this for the initial load, since we don't want to start up the display
 * until we finish loading songs; that way, people can continue to use their
 * computer while songs load. */
ScreenReloadSongs::ScreenReloadSongs( CString sClassName ): Screen(sClassName)
{
	m_FirstUpdate = true;
}

void ScreenReloadSongs::Update( float fDeltaTime )
{
	if( !m_FirstUpdate )
		return;

	m_FirstUpdate = false;
	SONGMAN->Reload();
	UNLOCKMAN->UpdateSongs();
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
}



