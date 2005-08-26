#include "global.h"
#include "ScreenSelectStyle.h"
#include "GameManager.h"
#include "GameSoundManager.h"
#include "NetworkSyncManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "ActorUtil.h"
#include "LightsManager.h"
#include "CommonMetrics.h"
#include "Command.h"


#define ICON_GAIN_FOCUS_COMMAND		THEME->GetMetricA(m_sName,"IconGainFocusCommand")
#define ICON_LOSE_FOCUS_COMMAND		THEME->GetMetricA(m_sName,"IconLoseFocusCommand")
#define DISABLED_COMMAND			THEME->GetMetricA(m_sName,"DisabledCommand")


REGISTER_SCREEN_CLASS( ScreenSelectStyle );
ScreenSelectStyle::ScreenSelectStyle( CString sClassName ) : ScreenSelect( sClassName )
{
	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );
}

void ScreenSelectStyle::Init()
{
	ScreenSelect::Init();

	m_iSelection = 0;

	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		const GameCommand& mc = m_aGameCommands[i];

		//
		// Load icon
		//
		CString sIconPath = THEME->GetPathG(m_sName,ssprintf("icon%d",i+1));

		m_textIcon[i].SetName( ssprintf("Icon%d",i+1) );
		m_sprIcon[i].SetName( ssprintf("Icon%d",i+1) );

		if( sIconPath.empty() )	// element doesn't exist
		{
			m_textIcon[i].LoadFromFont( THEME->GetPathF("Common","normal") );
			m_textIcon[i].SetText( mc.m_sName );
			m_textIcon[i].SetZoom(0.5f);
			this->AddChild( &m_textIcon[i] );
		}
		else
		{
			m_sprIcon[i].Load( sIconPath );
			this->AddChild( &m_sprIcon[i] );
		}
	

		//
		// Load Picture
		//
		CString sPicturePath = THEME->GetPathG(m_sName, ssprintf("picture%d",i+1));
		if( sPicturePath != "" )
		{
			m_sprPicture[i].SetName( "Picture" );
			m_sprPicture[i].Load( sPicturePath );
			m_sprPicture[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprPicture[i] );
		}


		//
		// Load info
		//
		CString sInfoPath = THEME->GetPathG(m_sName,ssprintf("info%d",i+1));
		if( sInfoPath != "" )
		{
			m_sprInfo[i].SetName( "Info" );
			m_sprInfo[i].Load( sInfoPath );
			m_sprInfo[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprInfo[i] );
		}
	}


	m_sprWarning.SetName( "Warning" );
	m_sprWarning.Load( THEME->GetPathG(m_sName,"warning") );
	this->AddChild( &m_sprWarning );
		
	m_sprExplanation.SetName( "Explanation" );
	m_sprExplanation.Load( THEME->GetPathG(m_sName,"explanation") );
	this->AddChild( &m_sprExplanation );
		


	// fix Z ordering of Picture and Info so that they draw on top
	for( unsigned i=0; i<this->m_aGameCommands.size(); i++ )
		this->MoveToTail( &m_sprPicture[i] );
	for( unsigned i=0; i<this->m_aGameCommands.size(); i++ )
		this->MoveToTail( &m_sprInfo[i] );


	this->UpdateSelectableChoices();

	m_sprPremium.SetName( "Premium" );

	switch( GAMESTATE->GetPremium() )
	{
	case PREMIUM_DOUBLE:
		m_sprPremium.Load( THEME->GetPathG(m_sName,"doubles premium") );
		this->AddChild( &m_sprPremium );
		break;
	case PREMIUM_JOINT:
		m_sprPremium.Load( THEME->GetPathG(m_sName,"joint premium") );
		this->AddChild( &m_sprPremium );
		break;
	}


	m_soundChange.Load( THEME->GetPathS(m_sName,"change"), true );


	//
	// TweenOnScreen
	//
	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		SET_XY_AND_ON_COMMAND( m_textIcon[i] );
		SET_XY_AND_ON_COMMAND( m_sprIcon[i] );
	}
	SET_XY_AND_ON_COMMAND( m_sprExplanation );
	SET_XY_AND_ON_COMMAND( m_sprWarning );
	SET_XY_AND_ON_COMMAND( m_sprPremium );

	// let AfterChange tween Picture and Info
	this->SortByDrawOrder();
}

void ScreenSelectStyle::MenuLeft( PlayerNumber pn )
{
	int iSwitchToIndex = -1;	// -1 means none found
	for( int i=m_iSelection-1; i>=0; i-- )
	{
		if( m_aGameCommands[i].IsPlayable() )
		{
			iSwitchToIndex = i;
			break;
		}
	}

	if( iSwitchToIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToIndex;
	m_soundChange.Play();
	AfterChange();
}

void ScreenSelectStyle::MenuRight( PlayerNumber pn )
{
	int iSwitchToIndex = -1;	// -1 means none found
	for( unsigned i=m_iSelection+1; i<m_aGameCommands.size(); i++ )	
	{
		if( m_aGameCommands[i].IsPlayable() )
		{
			iSwitchToIndex = i;
			break;
		}
	}

	if( iSwitchToIndex == -1 )
		return;

	BeforeChange();
	m_iSelection = iSwitchToIndex;
	m_soundChange.Play();
	AfterChange();
}

void ScreenSelectStyle::MenuStart( PlayerNumber pn )
{
	/* Stop all tweens where they are, since we might have selected before
	 * we finished tweening in. */
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->StopTweening();

	SCREENMAN->PlayStartSound();
	SCREENMAN->SendMessageToTopScreen( SM_BeginFadingOut );
	SCREENMAN->SendMessageToTopScreen( SM_AllDoneChoosing );

	const GameCommand& mc = m_aGameCommands[GetSelectionIndex(pn)];
	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("%s comment %s",m_sName.c_str(),mc.m_sName.c_str())) );

	//
	// TweenOffScreen
	//

	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		OFF_COMMAND( m_sprIcon[i] );
		OFF_COMMAND( m_textIcon[i] );
	}
	OFF_COMMAND( m_sprExplanation );
	OFF_COMMAND( m_sprWarning );
	OFF_COMMAND( m_sprPicture[m_iSelection] );
	OFF_COMMAND( m_sprInfo[m_iSelection] );
	OFF_COMMAND( m_sprPremium );
}

int ScreenSelectStyle::GetSelectionIndex( PlayerNumber pn )
{
	return m_iSelection;
}

void ScreenSelectStyle::UpdateSelectableChoices()
{
	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		/* If the icon is text, use a dimmer diffuse, or we won't be
		 * able to see the glow. */
		if( m_aGameCommands[i].IsPlayable() )
		{
			m_sprIcon[i].SetDiffuse( RageColor(1,1,1,1) );
			m_textIcon[i].SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );	// gray so glow is visible
		}
		else
		{
			m_sprIcon[i].RunCommands( DISABLED_COMMAND );
			m_textIcon[i].RunCommands( DISABLED_COMMAND );
		}
	}

	// Select the first enabled choice.
	BeforeChange();

	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		const GameCommand& mc = m_aGameCommands[i];
		if( mc.IsPlayable() )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}

	if( iSwitchToStyleIndex == -1 )// no styles are enabled.  We're stuck!
	{
		DEBUG_ASSERT(0);
		SCREENMAN->SystemMessage( "No Styles are selectable." );
		this->PostScreenMessage( SM_GoToPrevScreen, 0 );
		return;
	}
	

	m_iSelection = iSwitchToStyleIndex;
	AfterChange();
}

void ScreenSelectStyle::BeforeChange()
{
	// dim/hide old selection
	m_sprIcon[m_iSelection].RunCommands( ICON_LOSE_FOCUS_COMMAND );
	m_textIcon[m_iSelection].RunCommands( ICON_LOSE_FOCUS_COMMAND );
	m_sprPicture[m_iSelection].StopTweening();
	m_sprInfo[m_iSelection].StopTweening();
	m_sprPicture[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprPicture[m_iSelection].SetGlow( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetGlow( RageColor(1,1,1,0) );
}

void ScreenSelectStyle::AfterChange()
{
	m_sprIcon[m_iSelection].RunCommands( ICON_GAIN_FOCUS_COMMAND );
	m_textIcon[m_iSelection].RunCommands( ICON_GAIN_FOCUS_COMMAND );
	m_sprPicture[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprPicture[m_iSelection].SetZoom( 1 );
	m_sprInfo[m_iSelection].SetZoom( 1 );
	SET_XY_AND_ON_COMMAND( m_sprPicture[m_iSelection] );
	SET_XY_AND_ON_COMMAND( m_sprInfo[m_iSelection] );
}

/*
 * (c) 2001-2004 Chris Danford
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
