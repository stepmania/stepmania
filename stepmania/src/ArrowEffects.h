/*
-----------------------------------------------------------------------------
 File: ArrowEffects.h

 Desc: Functions that return properties of arrows based on StyleDef and PlayerOptions

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#ifndef _ArrowEffects_H_
#define _ArrowEffects_H_


#include "GameConstantsAndTypes.h"
#include "StyleDef.h"


//	fYOffset is a vertical position in pixels relative to the center.
//	(positive if has not yet been stepped on, negative if has already passed).
//	The ArrowEffect is applied in this stage.
float ArrowGetYOffset( const PlayerOptions& po, float fStepIndex, float fSongBeat );


//	fXPos is a horizontal position in pixels relative to the center of the field.
//	This depends on the column of the arrow and possibly the Arrow effect and
//	fYOffset (in the case of EFFECT_DRUNK).
float ArrowGetXPos(	const PlayerOptions& po, int iCol, float fYOffset, float fSongBeat );


//	fRotation is Z rotation of an arrow.  This will depend on the column of 
//	the arrow and possibly the Arrow effect and the fYOffset (in the case of 
//	EFFECT_DIZZY).
float ArrowGetRotation(	const PlayerOptions& po, int iCol, float fYOffset );


//	fYPos is the position of the note in pixels relative to the center.
//	(positive if has not yet been stepped on, negative if has already passed).
//	This value is fYOffset with bReverseScroll and fScrollSpeed factored in.
float ArrowGetYPos(	const PlayerOptions& po, float fYOffset );


//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
//	ArrowAppearance.
float ArrowGetAlpha( const PlayerOptions& po, float fYPos );


#endif 
