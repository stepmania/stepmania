#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenLogo

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenLogo.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "GameDef.h"
#include "RageLog.h"
#include "SongManager.h"

#define LOGO_ON_COMMAND			THEME->GetMetric("ScreenLogo","LogoOnCommand")
#define VERSION_ON_COMMAND		THEME->GetMetric("ScreenLogo","VersionOnCommand")
#define SONGS_ON_COMMAND		THEME->GetMetric("ScreenLogo","SongsOnCommand")


ScreenLogo::ScreenLogo() : ScreenAttract("ScreenLogo")
{
	m_sprLogo.Load( THEME->GetPathTo("Graphics",ssprintf("ScreenLogo %s",GAMESTATE->GetCurrentGameDef()->m_szName)) );
	m_sprLogo.Command( LOGO_ON_COMMAND );
	this->AddChild( &m_sprLogo );

	m_textVersion.LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
	m_textVersion.Command( VERSION_ON_COMMAND );
	m_textVersion.SetText( "CVS" );
	this->AddChild( &m_textVersion );


	m_textSongs.LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
	m_textSongs.Command( SONGS_ON_COMMAND );
	m_textSongs.SetText( ssprintf("%d songs in %d groups, %d courses", SONGMAN->GetNumSongs(), SONGMAN->GetNumGroups(), SONGMAN->GetNumCourses()) );
	this->AddChild( &m_textSongs );

	this->MoveToTail( &m_In );	// put it in the back so it covers up the stuff we just added
	this->MoveToTail( &m_Out );	// put it in the back so it covers up the stuff we just added
}
