#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMaxType1

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenSelectMaxType1.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameState.h"


#define JOINT_PREMIUM_ON_COMMAND	THEME->GetMetric ("ScreenSelectMaxType1","JointPremiumOnCommand")
#define JOINT_PREMIUM_OFF_COMMAND	THEME->GetMetric ("ScreenSelectMaxType1","JointPremiumOffCommand")
#define ICON_ON_COMMAND( i )		THEME->GetMetric ("ScreenSelectMaxType1",ssprintf("Icon%dOnCommand",i+1))
#define ICON_OFF_COMMAND( i )		THEME->GetMetric ("ScreenSelectMaxType1",ssprintf("Icon%dOffCommand",i+1))
#define DISABLED_COLOR				THEME->GetMetricC("ScreenSelectMaxType1","DisabledColor")
#define EXPLANATION_ON_COMMAND		THEME->GetMetric ("ScreenSelectMaxType1","ExplanationOnCommand")
#define EXPLANATION_OFF_COMMAND		THEME->GetMetric ("ScreenSelectMaxType1","ExplanationOffCommand")
#define INFO_ON_COMMAND				THEME->GetMetric ("ScreenSelectMaxType1","InfoOnCommand")
#define INFO_OFF_COMMAND			THEME->GetMetric ("ScreenSelectMaxType1","InfoOffCommand")
#define PREVIEW_ON_COMMAND			THEME->GetMetric ("ScreenSelectMaxType1","PreviewOnCommand")
#define PREVIEW_OFF_COMMAND			THEME->GetMetric ("ScreenSelectMaxType1","PreviewOffCommand")


ScreenSelectMaxType1::ScreenSelectMaxType1() : ScreenSelect( "ScreenSelectMaxType1" )
{
	m_iSelection = 0;


	unsigned i;
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		const ModeChoice& mc = m_aModeChoices[i];
		CString sGameName = GAMEMAN->GetGameDefForGame(mc.game)->m_szName;


		//
		// Load icon
		//
		CString sIconElementName = ssprintf("ScreenSelectMaxType1 icon %s %s", sGameName.GetString(), mc.name );
		CString sIconPath = THEME->GetPathTo("Graphics",sIconElementName);
		if( sIconPath.empty() )	// element doesn't exist
		{
			m_textIcon[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
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
		// Load preview
		//
		CString sPreviewElementName = ssprintf("ScreenSelectMaxType1 preview %s %s", sGameName.GetString(), mc.name );
		CString sPreviewPath = THEME->GetPathTo("Graphics",sPreviewElementName);
		if( sPreviewPath != "" )
		{
			m_sprPreview[i].Load( sPreviewPath );
//			m_sprPreview[i].Command( PREVIEW_X, PREVIEW_Y );
			m_sprPreview[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprPreview[i] );
		}


		//
		// Load info
		//
		CString sInfoElementName = ssprintf("ScreenSelectMaxType1 info %s %s", sGameName.GetString(), mc.name );
		CString sInfoPath = THEME->GetPathTo("Graphics",sInfoElementName);
		if( sInfoPath != "" )
		{
			m_sprInfo[i].Load( sInfoPath );
//			m_sprInfo[i].SetXY( INFO_X, INFO_Y );
			m_sprInfo[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprInfo[i] );
		}
	}

	// fix Z ordering of Preview and Info so that they draw on top
	for( i=0; i<this->m_aModeChoices.size(); i++ )
		this->MoveToTail( &m_sprPreview[i] );
	for( i=0; i<this->m_aModeChoices.size(); i++ )
		this->MoveToTail( &m_sprInfo[i] );


	this->UpdateSelectableChoices();

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","ScreenSelectMaxType1 explanation") );
//	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );
		
	if( PREFSMAN->m_bJointPremium )
	{
		m_sprJointPremium.Load( THEME->GetPathTo("Graphics","ScreenSelectMaxType1 joint premium") );
//		m_sprJointPremium.SetXY( JOINT_PREMIUM_BANNER_X, JOINT_PREMIUM_BANNER_Y );
		this->AddChild( &m_sprJointPremium );
	}


	m_soundChange.Load( THEME->GetPathTo("Sounds","ScreenSelectMaxType1 change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );


	//
	// TweenOnScreen
	//
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		m_textIcon[i].Command( ICON_ON_COMMAND(i) );
		m_sprIcon[i].Command( ICON_ON_COMMAND(i) );
	}
	m_sprExplanation.Command( EXPLANATION_ON_COMMAND );
	m_sprJointPremium.Command( JOINT_PREMIUM_ON_COMMAND );

	// let AfterChange tween Preview and Info
}

void ScreenSelectMaxType1::MenuLeft( PlayerNumber pn )
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

void ScreenSelectMaxType1::MenuRight( PlayerNumber pn )
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

void ScreenSelectMaxType1::MenuStart( PlayerNumber pn )
{
	m_soundSelect.Play();
	SCREENMAN->SendMessageToTopScreen( SM_AllDoneChoosing, 0 );

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
	m_sprPreview[m_iSelection].Command( PREVIEW_OFF_COMMAND );
	m_sprInfo[m_iSelection].Command( INFO_OFF_COMMAND );
	m_sprJointPremium.Command( JOINT_PREMIUM_OFF_COMMAND );
}

int ScreenSelectMaxType1::GetSelectionIndex( PlayerNumber pn )
{
	return m_iSelection;
}

void ScreenSelectMaxType1::UpdateSelectableChoices()
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

void ScreenSelectMaxType1::BeforeChange()
{
	// dim/hide old selection
	m_sprIcon[m_iSelection].SetEffectNone();
	m_textIcon[m_iSelection].SetEffectNone();
	m_sprPreview[m_iSelection].StopTweening();
	m_sprInfo[m_iSelection].StopTweening();
	m_sprPreview[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,0) );
	m_sprPreview[m_iSelection].SetGlow( RageColor(1,1,1,0) );
	m_sprInfo[m_iSelection].SetGlow( RageColor(1,1,1,0) );
}

void ScreenSelectMaxType1::AfterChange()
{
	m_sprIcon[m_iSelection].SetEffectGlowShift();
	m_textIcon[m_iSelection].SetEffectGlowShift();
	m_sprPreview[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprPreview[m_iSelection].SetZoom( 1 );
	m_sprInfo[m_iSelection].SetZoom( 1 );
	m_sprPreview[m_iSelection].Command( PREVIEW_ON_COMMAND );
	m_sprInfo[m_iSelection].Command( INFO_ON_COMMAND );
}
