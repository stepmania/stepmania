#include "global.h"

#include "ScreenExit.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "RageSounds.h"
#include "RageSound.h"
#include "RageLog.h"
#include "StepMania.h"

ScreenExit::ScreenExit( CString sName ): Screen( sName )
{
	m_Exited = false;

	/* It'd be better for any previous screen playing music to fade it out as it fades
	 * out the screen.  XXX: Check to see if it's fading out; if it'll stop playing in
	 * reasonable time, let it. */
	SOUND->StopMusic();
}

void ScreenExit::Update( float fDelta )
{
	if( m_Exited )
		return;

	/* Grab the list of playing sounds, and see if it's empty. */
	const set<RageSound *> &PlayingSounds = SOUNDMAN->GetPlayingSounds();
	bool DoQuit = PlayingSounds.empty();

	/* As a safety precaution, don't wait indefinitely, in case some sound was
	 * inadvertently set to play too long. */
	if( !DoQuit && m_ShutdownTimer.PeekDeltaTime() > 3 )
	{
		DoQuit = true;
		CString warn = ssprintf("ScreenExit: %i sound%s failed to finish playing quickly: ",
			PlayingSounds.size(), PlayingSounds.size()==1?"":"s" );
		for( set<RageSound *>::const_iterator i = PlayingSounds.begin();
			i != PlayingSounds.end(); ++i )
		{
			warn += (*i)->GetLoadedFilePath() + "; ";
		}
			
		LOG->Warn("%s", warn.c_str() );
	}

	if( DoQuit )
	{
		m_Exited = true;
		LOG->Trace("ScreenExit: shutting down");
		ExitGame();
	}
}
