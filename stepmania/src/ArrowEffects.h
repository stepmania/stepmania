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


//	fYOffset is a vertical position in pixels relative to the center.
//	(positive if has not yet been stepped on, negative if has already passed).
//	The ArrowEffect and ScrollSpeed is applied in this stage.
float ArrowGetYOffset( PlayerNumber pn, int iCol, float fNoteBeat );

//	Actual display position, with reverse factored in.
float ArrowGetYPos(	PlayerNumber pn, int iCol, float fYOffset, float fYReverseOffsetPixels );


//	fRotation is Z rotation of an arrow.  This will depend on the column of 
//	the arrow and possibly the Arrow effect and the fYOffset (in the case of 
//	EFFECT_DIZZY).
float ArrowGetRotation(	PlayerNumber pn, float fNoteBeat );


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
float ArrowGetAlpha( PlayerNumber pn, int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels );


//	fAlpha is the transparency of the arrow.  It depends on fYPos and the 
//	AppearanceType.
float ArrowGetGlow( PlayerNumber pn, int iCol, float fYPos, float fPercentFadeToFail, float fYReverseOffsetPixels );


//	Depends on fYOffset.
float ArrowGetBrightness( PlayerNumber pn, float fNoteBeat );


#endif
