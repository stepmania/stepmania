#ifndef ARROWEFFECTS_H
#define ARROWEFFECTS_H

/*
-----------------------------------------------------------------------------
 File: ArrowEffects.h

 Desc: Functions that return properties of arrows based on StyleDef and PlayerOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "GameConstantsAndTypes.h"
#include "StyleDef.h"

const int	ARROW_SIZE	= 64;
const float ARROW_SPACING	= ARROW_SIZE;// + 2;


//	fYOffset is a vertical position in pixels relative to the center.
//	(positive if has not yet been stepped on, negative if has already passed).
//	The ArrowEffect is applied in this stage.
float ArrowGetYOffset( PlayerNumber pn, int iCol, float fNoteBeat );


//	fRotation is Z rotation of an arrow.  This will depend on the column of 
//	the arrow and possibly the Arrow effect and the fYOffset (in the case of 
//	EFFECT_DIZZY).
float ArrowGetRotation(	PlayerNumber pn, float fNoteBeat );


//	fYPos is the position of the note in pixels relative to the center.
//	(positive if has not yet been stepped on, negative if has already passed).
//	This value is fYOffset with bReverseScroll and fScrollSpeed factored in.
float ArrowGetYPosWithoutReverse( PlayerNumber pn, int iCol, float fYOffset );
float ArrowGetYPos(	PlayerNumber pn, int iCol, float fYOffset, float fYReverseOffsetPixels );


//	fXPos is a horizontal position in pixels relative to the center of the field.
//	This depends on the column of the arrow and possibly the Arrow effect and
//	fYPos (in the case of EFFECT_DRUNK).
float ArrowGetXPos( PlayerNumber pn, int iCol, float fYPos );

//  Z position; normally 0.  Only visible in perspective modes.
float ArrowGetZPos( PlayerNumber pn, int iCol, float fYPos );

// Enable this if any ZPos effects are enabled.
bool ArrowsNeedZBuffer( PlayerNumber pn );

//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
//	AppearanceType.
float ArrowGetAlpha( PlayerNumber pn, float fYPos, float fPercentFadeToFail );


//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
//	AppearanceType.
float ArrowGetGlow( PlayerNumber pn, float fYPos, float fPercentFadeToFail );


//	Depends on fYOffset.
float ArrowGetBrightness( PlayerNumber pn, float fNoteBeat );


#endif
