#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectStyle.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"


#define JOINT_PREMIUM_ON_COMMAND	THEME->GetMetric ("ScreenSelectStyle","JointPremiumOnCommand")
#define JOINT_PREMIUM_OFF_COMMAND	THEME->GetMetric ("ScreenSelectStyle","JointPremiumOffCommand")
#define ICON_ON_COMMAND( i )		THEME->GetMetric ("ScreenSelectStyle",ssprintf("Icon%dOnCommand",i+1))
#define ICON_OFF_COMMAND( i )		THEME->GetMetric ("ScreenSelectStyle",ssprintf("Icon%dOffCommand",i+1))
#define ICON_GAIN_FOCUS_COMMAND		THEME->GetMetric ("ScreenSelectStyle","IconGainFocusCommand")
#define ICON_LOSE_FOCUS_COMMAND		THEME->GetMetric ("ScreenSelectStyle","IconLoseFocusCommand")
#define DISABLED_COLOR				THEME->GetMetricC("ScreenSelectStyle","DisabledColor")
#define EXPLANATION_ON_COMMAND		THEME->GetMetric ("ScreenSelectStyle","ExplanationOnCommand")
#define EXPLANATION_OFF_COMMAND		THEME->GetMetric ("ScreenSelectStyle","ExplanationOffCommand")
#define WARNING_ON_COMMAND			THEME->GetMetric ("ScreenSelectStyle","WarningOnCommand")
#define WARNING_OFF_COMMAND			THEME->GetMetric ("ScreenSelectStyle","WarningOffCommand")
#define INFO_ON_COMMAND				THEME->GetMetric ("ScreenSelectStyle","InfoOnCommand")
#define INFO_OFF_COMMAND			THEME->GetMetric ("ScreenSelectStyle","InfoOffCommand")
#define PICTURE_ON_COMMAND			THEME->GetMetric ("ScreenSelectStyle","PictureOnCommand")
#define PICTURE_OFF_COMMAND			THEME->GetMetric ("ScreenSelectStyle","PictureOffCommand")


ScreenSelectStyle::ScreenSelectStyle() : ScreenSelect( "ScreenSelectStyle" )
{
	m_iSelection = 0;


	unsigned i;
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		const ModeChoice& mc = m_aModeChoices[i];

		//
		// Load icon
		//
		CString sIconElementName = ssprintf("ScreenSelectStyle icon %s", mc.name );
		CString sIconPath = THEME->GetPathTo("Graphics",sIconElementName);
		if( sIconPath.empty() )	// element doesn't exist
		{
			m_textIcon[i].LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
			m_textIcon[i].SetText( mc.name );
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
		CString sPictureElementName = ssprintf("ScreenSelectStyle picture %s", mc.name );
		CString sPicturePath = THEME->GetPathTo("Graphics",sPictureElementName);
		if( sPicturePath != "" )
		{
			m_sprPicture[i].Load( sPicturePath );
//			m_sprPicture[i].Command( PREVIEW_X, PREVIEW_Y );
			m_sprPicture[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprPicture[i] );
		}


		//
		// Load info
		//
		CString sInfoElementName = ssprintf("ScreenSelectStyle info %s", mc.name );
		CString sInfoPath = THEME->GetPathTo("Graphics",sInfoElementName);
		if( sInfoPath != "" )
		{
			m_sprInfo[i].Load( sInfoPath );
//			m_sprInfo[i].SetXY( INFO_X, INFO_Y );
			m_sprInfo[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprInfo[i] );
		}
	}


	m_sprWarning.Load( THEME->GetPathTo("Graphics","ScreenSelectStyle warning") );
	this->AddChild( &m_sprWarning );
		
	m_sprExplanation.Load( THEME->GetPathTo("Graphics","ScreenSelectStyle explanation") );
	this->AddChild( &m_sprExplanation );
		


	// fix Z ordering of Picture and Info so that they draw on top
	for( i=0; i<this->m_aModeChoices.size(); i++ )
		this->MoveToTail( &m_sprPicture[i] );
	for( i=0; i<this->m_aModeChoices.size(); i++ )
		this->MoveToTail( &m_sprInfo[i] );


	this->UpdateSelectableChoices();

	if( PREFSMAN->m_bJointPremium )
	{
		m_sprJointPremium.Load( THEME->GetPathTo("Graphics","ScreenSelectStyle joint premium") );
		this->AddChild( &m_sprJointPremium );
	}


	m_soundChange.Load( THEME->GetPathTo("Sounds","ScreenSelectStyle change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","Common start") );


	//
	// TweenOnScreen
	//
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		m_textIcon[i].Command( ICON_ON_COMMAND(i) );
		m_sprIcon[i].Command( ICON_ON_COMMAND(i) );
	}
	m_sprExplanation.Command( EXPLANATION_ON_COMMAND );
	m_sprWarning.Command( WARNING_ON_COMMAND );
	m_sprJointPremium.Command( JOINT_PREMIUM_ON_COMMAND );

	// let AfterChange tween Picture and Info
}

void ScreenSelectStyle::MenuLeft( PlayerNumber pn )
{
	int iSwitchToIndex = -1;	// -1 means none found
	for( int i=m_iSelection-1; i>=0; i-- )
	{
		if( GAMESTATE->IsPlayable(m_aModeChoices[i]) )
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
	for( unsigned i=m_iSelection+1; i<m_aModeChoices.size(); i++ )	
	{
		if( GAMESTATE->IsPlayable(m_aModeChoices[i]) )
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
	m_soundSelect.Play();
	SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, 0 );

	const ModeChoice& mc = m_aModeChoices[GetSelectionIndex(pn)];
	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo(ssprintf("ScreenSelectStyle comment %s",mc.name)) );

	//
	// TweenOffScreen
	//
	unsigned i;

	/* Stop all tweens where they are, since we might have selected before
	 * we finished tweening in. */
	for( i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->StopTweening();

	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		m_sprIcon[i].Command( ICON_OFF_COMMAND(i) );
		m_textIcon[i].Command( ICON_OFF_COMMAND(i) );
	}
	m_sprExplanation.Command( EXPLANATION_OFF_COMMAND );
	m_sprWarning.Command( WARNING_OFF_COMMAND );
	m_sprPicture[m_iSelection].Command( PICTURE_OFF_COMMAND );
	m_sprInfo[m_iSelection].Command( INFO_OFF_COMMAND );
	m_sprJointPremium.Command( JOINT_PREMIUM_OFF_COMMAND );
}

int ScreenSelectStyle::GetSelectionIndex( PlayerNumber pn )
{
	return m_iSelection;
}

void ScreenSelectStyle::UpdateSelectableChoices()
{
	unsigned i;
	/* If a player joins during the tween-in, this diffuse change
	 * will be undone by the tween.  Hmm. 
	 *
	 * This is fixed now, since SetDiffuse() will affect the latest
	 * tween, if we're currently tweening. */
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		/* If the icon is text, use a dimmer diffuse, or we won't be
		 * able to see the glow. */
		if( GAMESTATE->IsPlayable(m_aModeChoices[i]) )
		{
			m_sprIcon[i].SetDiffuse( RageColor(1,1,1,1) );
			m_textIcon[i].SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );	// gray so glow is visible
		}
		else
		{
			m_sprIcon[i].SetDiffuse( DISABLED_COLOR );
			m_textIcon[i].SetDiffuse( DISABLED_COLOR );
		}
	}

	// Select first enabled choie
	BeforeChange();

	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		if( GAMESTATE->IsPlayable(m_aModeChoices[i]) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}
	ASSERT( iSwitchToStyleIndex != -1 );	// no styles are enabled.  We're stuck!  This should never happen

	m_iSelection = iSwitchToStyleIndex;
	AfterChange();
}

void ScreenSelectStyle::BeforeChange()
{
	// dim/hide old selection
	m_sprIcon[m_iSelection].Command( ICON_LOSE_FOCUS_COMMAND );
	m_textIcon[m_iSelection].Command( ICON_LOSE_FOCUS_COMMAND );
	m_sprPicture[m_iSelection].StopTweening();
	m_sprInfo[m_iSelection].StopTweening();
	m_sprPicture[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprPicture[m_iSelection].SetGlow( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetGlow( RageColor(1,1,1,0) );
}

void ScreenSelectStyle::AfterChange()
{
	m_sprIcon[m_iSelection].Command( ICON_GAIN_FOCUS_COMMAND );
	m_textIcon[m_iSelection].Command( ICON_GAIN_FOCUS_COMMAND );
	m_sprPicture[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprPicture[m_iSelection].SetZoom( 1 );
	m_sprInfo[m_iSelection].SetZoom( 1 );
	m_sprPicture[m_iSelection].Command( PICTURE_ON_COMMAND );
	m_sprInfo[m_iSelection].Command( INFO_ON_COMMAND );
}
