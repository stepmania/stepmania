#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ArrowEffects.cpp

 Desc: Functions that return properties of arrows based on StyleDef and PlayerOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ArrowEffects.h"
#include "Notes.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "NoteDisplay.h"


float ArrowGetYOffset( PlayerNumber pn, float fNoteBeat )
{
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP;
	switch( GAMESTATE->m_PlayerOptions[pn].m_EffectType )
	{
	case PlayerOptions::EFFECT_BOOST:
		fYOffset *= 1.4f / ((fYOffset+SCREEN_HEIGHT/1.6f)/SCREEN_HEIGHT); 
		break;
	case PlayerOptions::EFFECT_WAVE:
		fYOffset += 15.0f*sinf( fYOffset/38.0f ); 
		break;
	}
	return fYOffset;
}

float ArrowGetXPos2( PlayerNumber pn, int iColNum, float fYPos ) 
{
	float fSongBeat = GAMESTATE->m_fSongBeat;
	float fPixelOffsetFromCenter = GAMESTATE->GetCurrentStyleDef()->m_ColumnInfo[PLAYER_1][iColNum].fXOffset;
	
	switch( GAMESTATE->m_PlayerOptions[pn].m_EffectType )
	{
	case PlayerOptions::EFFECT_DRUNK:
		fPixelOffsetFromCenter += cosf( TIMER->GetTimeSinceStart() + iColNum*0.2f + fYPos*6/SCREEN_HEIGHT) * ARROW_SIZE*0.5f; 
		break;
	}
	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( PlayerNumber pn, int iColNum, float fYOffset ) 
{
	float fRotation = 0; //StyleDef.m_ColumnToRotation[iColNum];

	switch( GAMESTATE->m_PlayerOptions[pn].m_EffectType )
	{
	case PlayerOptions::EFFECT_DIZZY:
		fRotation += fYOffset/SCREEN_HEIGHT*6; 
		break;
	}
	
	return fRotation;
}

float ArrowGetYPos( PlayerNumber pn, float fYOffset )
{
	return fYOffset * GAMESTATE->m_PlayerOptions[pn].m_fArrowScrollSpeed * (GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll ? -1 : 1 );
}

float ArrowGetAlpha( PlayerNumber pn, float fYPos )
{
	float fAlpha;
	static	float blinktimer=0;
	if (blinktimer == 0)
		blinktimer = TIMER->GetTimeSinceStart();
	
	const bool bReverse = GAMESTATE->m_PlayerOptions[pn].m_bReverseScroll;

	static int blinkstate=2;
	switch( GAMESTATE->m_PlayerOptions[pn].m_AppearanceType )
	{ 
	case PlayerOptions::APPEARANCE_VISIBLE:
		fAlpha = 1;
		break;
	case PlayerOptions::APPEARANCE_HIDDEN:
		fAlpha = ((bReverse?-fYPos:fYPos)-100)/100;
		break;
	case PlayerOptions::APPEARANCE_SUDDEN:
		fAlpha = ((SCREEN_HEIGHT-(bReverse?-fYPos:fYPos))-260)/100;
		break;
	case PlayerOptions::APPEARANCE_STEALTH:
		fAlpha = 0;
		break;
	case PlayerOptions::APPEARANCE_BLINK: // this is an Ez2dancer Appearance Mode
		if (TIMER->GetTimeSinceStart() > blinktimer + 0.20f)
		{
			blinktimer = TIMER->GetTimeSinceStart();
			if (blinkstate == 1)
			{
				blinkstate = 2;
			}
			else if (blinkstate == 0)
			{
				blinkstate = 3;
			}
			else if (blinkstate == 2)
			{
				blinkstate = 0;
			}
			else
			{
				blinkstate = 1;
			}
		}

		if (blinkstate == 1)
		{
			fAlpha = 1;			
		}
		else if (blinkstate == 0)
		{				
			fAlpha = 0;
		}
		else
		{
			fAlpha = ((fYPos-200)/200) + (((SCREEN_HEIGHT-fYPos)-260)/200);	
		}

		break;
	default:
		ASSERT( false );
		fAlpha = 0;
	};
	if( !bReverse  &&  fYPos < 0 )
		fAlpha = 1;
	else if( bReverse  &&  fYPos > 0 )
		fAlpha = 1;

	return clamp( fAlpha, 0, 1 );
};


