#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ModeSelectorMaxType1

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ModeSelectorMaxType1.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"


#define JOINT_PREMIUM_BANNER_X	THEME->GetMetricF("ModeSelectorMaxType1","JointPremiumBannerX")
#define JOINT_PREMIUM_BANNER_Y	THEME->GetMetricF("ModeSelectorMaxType1","JointPremiumBannerY")
#define ICONS_START_X			THEME->GetMetricF("ModeSelectorMaxType1","IconsStartX")
#define ICONS_SPACING_X			THEME->GetMetricF("ModeSelectorMaxType1","IconsSpacingX")
#define ICONS_START_Y			THEME->GetMetricF("ModeSelectorMaxType1","IconsStartY")
#define ICONS_SPACING_Y			THEME->GetMetricF("ModeSelectorMaxType1","IconsSpacingY")
#define ICONS_DISABLED_COLOR	THEME->GetMetricC("ModeSelectorMaxType1","IconsDisabledColor")
#define EXPLANATION_X			THEME->GetMetricF("ModeSelectorMaxType1","ExplanationX")
#define EXPLANATION_Y			THEME->GetMetricF("ModeSelectorMaxType1","ExplanationY")
#define INFO_X					THEME->GetMetricF("ModeSelectorMaxType1","InfoX")
#define INFO_Y					THEME->GetMetricF("ModeSelectorMaxType1","InfoY")
#define PREVIEW_X				THEME->GetMetricF("ModeSelectorMaxType1","PreviewX")
#define PREVIEW_Y				THEME->GetMetricF("ModeSelectorMaxType1","PreviewY")
#define FADE_SECONDS			THEME->GetMetricF("ModeSelectorMaxType1","FadeSeconds")


ModeSelectorMaxType1::ModeSelectorMaxType1()
{
	m_iSelection = 0;
}

void ModeSelectorMaxType1::Init( const vector<ModeChoice>& choices, CString sClassName, CString sThemeElementPrefix )
{
	m_aModeChoices = choices;

	unsigned i;
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		const ModeChoice& mc = m_aModeChoices[i];
		CString sGameName = GAMEMAN->GetGameDefForGame(mc.game)->m_szName;


		//
		// Load icon
		//
		float fIconX = ICONS_START_X + i*ICONS_SPACING_X;
		float fIconY = ICONS_START_Y + i*ICONS_SPACING_Y;
		CString sIconElementName = ssprintf("%s icon %s %s", sThemeElementPrefix.GetString(), sGameName.GetString(), mc.name );
		CString sIconPath = THEME->GetPathTo("Graphics",sIconElementName);
		if( sIconPath.empty() )	// element doesn't exist
		{
			m_textIcon[i].LoadFromFont( THEME->GetPathTo("Fonts","normal") );
			m_textIcon[i].SetXY( fIconX, fIconY );
			m_textIcon[i].SetText( mc.name );
			m_textIcon[i].SetZoom(0.5f);
			this->AddChild( &m_textIcon[i] );
		}
		else
		{
			m_sprIcon[i].Load( sIconPath );
			m_sprIcon[i].SetXY( fIconX, fIconY );
			this->AddChild( &m_sprIcon[i] );
		}

	
		//
		// Load preview
		//
		CString sPreviewElementName = ssprintf("%s preview %s %s", sThemeElementPrefix.GetString(), sGameName.GetString(), mc.name );
		CString sPreviewPath = THEME->GetPathTo("Graphics",sPreviewElementName);
		if( sPreviewPath != "" )
		{
			m_sprPreview[i].Load( sPreviewPath );
			m_sprPreview[i].SetXY( PREVIEW_X, PREVIEW_Y );
			m_sprPreview[i].SetDiffuse( RageColor(1,1,1,0) );
			this->AddChild( &m_sprPreview[i] );
		}


		//
		// Load info
		//
		CString sInfoElementName = ssprintf("%s info %s %s", sThemeElementPrefix.GetString(), sGameName.GetString(), mc.name );
		CString sInfoPath = THEME->GetPathTo("Graphics",sInfoElementName);
		if( sInfoPath != "" )
		{
			m_sprInfo[i].Load( sInfoPath );
			m_sprInfo[i].SetXY( INFO_X, INFO_Y );
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

	m_sprExplanation.Load( THEME->GetPathTo("Graphics","select style explanation") );
	m_sprExplanation.SetXY( EXPLANATION_X, EXPLANATION_Y );
	this->AddChild( &m_sprExplanation );
		
	if( PREFSMAN->m_bJointPremium )
	{
		m_sprJointPremium.Load( THEME->GetPathTo("Graphics","select style joint premium banner") );
		m_sprJointPremium.SetXY( JOINT_PREMIUM_BANNER_X, JOINT_PREMIUM_BANNER_Y );
		this->AddChild( &m_sprJointPremium );
	}


	m_soundChange.Load( THEME->GetPathTo("Sounds",sThemeElementPrefix + " change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	TweenOnScreen();
}

void ModeSelectorMaxType1::MenuLeft( PlayerNumber pn )
{
	int iSwitchToIndex = -1;	// -1 means none found
	for( int i=m_iSelection-1; i>=0; i-- )
	{
		if( IsSelectable(m_aModeChoices[i]) )
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

void ModeSelectorMaxType1::MenuRight( PlayerNumber pn )
{
	int iSwitchToIndex = -1;	// -1 means none found
	for( unsigned i=m_iSelection+1; i<m_aModeChoices.size(); i++ )	
	{
		if( IsSelectable(m_aModeChoices[i]) )
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

void ModeSelectorMaxType1::MenuStart( PlayerNumber pn )
{
	m_soundSelect.Play();
	SCREENMAN->SendMessageToTopScreen( SM_BeginFadingOut, 0 );
}

void ModeSelectorMaxType1::MenuBack( PlayerNumber pn )
{

}

void ModeSelectorMaxType1::TweenOffScreen()
{
	for( unsigned i=0; i<m_aModeChoices.size(); i++ )
	{
		m_sprIcon[i].FadeOff( 0, "FoldY", FADE_SECONDS );
		m_textIcon[i].FadeOff( 0, "FoldY", FADE_SECONDS );
	}
	m_sprExplanation.FadeOff( 0, "FoldY", FADE_SECONDS );
	m_sprPreview[m_iSelection].FadeOff( 0, "FoldY", FADE_SECONDS );
	m_sprInfo[m_iSelection].FadeOff( 0, "FoldY", FADE_SECONDS );
	m_sprJointPremium.FadeOff( 0, "fade", FADE_SECONDS );
}

void ModeSelectorMaxType1::TweenOnScreen()
{
	for( unsigned i=0; i<m_aModeChoices.size(); i++ )
	{
		m_sprIcon[i].FadeOn( (m_aModeChoices.size()-i)*0.05f, "Left Accelerate", FADE_SECONDS );
		m_textIcon[i].FadeOn( (m_aModeChoices.size()-i)*0.05f, "Left Accelerate", FADE_SECONDS );
	}
	m_sprExplanation.FadeOn( 0, "Right Accelerate", FADE_SECONDS );
	m_sprJointPremium.FadeOn( 0, "fade", FADE_SECONDS );

	// let AfterChange tween Preview and Info
}

void ModeSelectorMaxType1::GetSelectedModeChoice( PlayerNumber pn, ModeChoice* pModeChoiceOut )
{
	*pModeChoiceOut = m_aModeChoices[m_iSelection];	// doesn't depend on pn
}

void ModeSelectorMaxType1::UpdateSelectableChoices()
{
	unsigned i;
	/* XXX: If a player joins during the tween-in, this diffuse change
	 * will be undone by the tween.  Hmm. */
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		/* If the icon is text, use a dimmer diffuse, or we won't be
		 * able to see the glow. */
		if( IsSelectable(m_aModeChoices[i]) )
		{
			m_sprIcon[i].SetDiffuse( RageColor(1,1,1,1) );
			m_textIcon[i].SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );	// gray so glow is visible
		}
		else
		{
			m_sprIcon[i].SetDiffuse( ICONS_DISABLED_COLOR );
			m_textIcon[i].SetDiffuse( ICONS_DISABLED_COLOR );
		}
	}

	// Select first enabled choie
	BeforeChange();

	int iSwitchToStyleIndex = -1;	// -1 means none found
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		if( IsSelectable(m_aModeChoices[i]) )
		{
			iSwitchToStyleIndex = i;
			break;
		}
	}
	ASSERT( iSwitchToStyleIndex != -1 );	// no styles are enabled.  We're stuck!  This should never happen

	m_iSelection = iSwitchToStyleIndex;
	AfterChange();
}

void ModeSelectorMaxType1::BeforeChange()
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

void ModeSelectorMaxType1::AfterChange()
{
	m_sprIcon[m_iSelection].SetEffectGlowShift();
	m_textIcon[m_iSelection].SetEffectGlowShift();
	m_sprPreview[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprInfo[m_iSelection].SetDiffuse( RageColor(1,1,1,1) );
	m_sprPreview[m_iSelection].SetZoom( 1 );
	m_sprInfo[m_iSelection].SetZoom( 1 );
	m_sprPreview[m_iSelection].FadeOn( 0, "ghost", FADE_SECONDS );
	m_sprInfo[m_iSelection].FadeOn( 0, "FoldY bounce", FADE_SECONDS );
}
