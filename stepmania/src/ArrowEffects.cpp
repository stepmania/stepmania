#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ArrowEffects.cpp

 Desc: Functions that return properties of arrows based on StyleDef and PlayerOptions

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "ArrowEffects.h"
#include "NoteMetadata.h"
#include "ColorNote.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"


float ArrowGetYOffset( const PlayerOptions& po, float fStepIndex, float fSongBeat )
{
	float fBeatsUntilStep = NoteIndexToBeat( fStepIndex ) - fSongBeat;
	float fYOffset = fBeatsUntilStep * ARROW_GAP;
	switch( po.m_EffectType )
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

float ArrowGetXPos( const PlayerOptions& po, int iColNum, float fYOffset, float fSongBeat ) 
{
	float fColOffsetFromCenter = iColNum - (GAME->GetCurrentStyleDef()->m_iColsPerPlayer-1)/2.0f;
	float fPixelOffsetFromCenter = fColOffsetFromCenter * ARROW_SIZE;
	
	switch( po.m_EffectType )
	{
	case PlayerOptions::EFFECT_DRUNK:
		fPixelOffsetFromCenter += cosf( (GetTickCount()%1000000)/250.0f + iColNum*0.4f + fYOffset/SCREEN_HEIGHT*4) * ARROW_SIZE/3; 
		break;
	}
	return fPixelOffsetFromCenter;
}

float ArrowGetRotation( const PlayerOptions& po, int iColNum, float fYOffset ) 
{
	float fRotation = 0; //StyleDef.m_ColumnToRotation[iColNum];

	switch( po.m_EffectType )
	{
	case PlayerOptions::EFFECT_DIZZY:
		fRotation += fYOffset/SCREEN_HEIGHT*6; 
		break;
	}
	
	return fRotation;
}

float ArrowGetYPos( const PlayerOptions& po, float fYOffset )
{
	return fYOffset * po.m_fArrowScrollSpeed * (po.m_bReverseScroll ? -1 : 1 );
}

float ArrowGetAlpha( const PlayerOptions& po, float fYPos )
{
	float fAlpha;
	switch( po.m_AppearanceType )
	{ 
	case PlayerOptions::APPEARANCE_VISIBLE:
		fAlpha = 1;
		break;
	case PlayerOptions::APPEARANCE_HIDDEN:
		fAlpha = (fYPos-100)/200;
		break;
	case PlayerOptions::APPEARANCE_SUDDEN:
		fAlpha = ((SCREEN_HEIGHT-fYPos)-280)/200;
		break;
	case PlayerOptions::APPEARANCE_STEALTH:
		fAlpha = 0;
		break;
	default:
		ASSERT( false );
		fAlpha = 0;
	};
	if( fYPos < 0 )
		fAlpha = 1;

	return fAlpha;
};


