#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: PlayerOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerOptions.h"
#include "RageUtil.h"


CString PlayerOptions::GetString()
{
	CString sReturn;
	if( m_fArrowScrollSpeed != 1 )
		sReturn += ssprintf( "%2.1fX, ", m_fArrowScrollSpeed );
	switch( m_EffectType )
	{
	case EFFECT_NONE:							break;
	case EFFECT_BOOST:	sReturn += "Boost, ";	break;
	case EFFECT_WAVE:	sReturn += "Wave, ";	break;
	case EFFECT_DRUNK:	sReturn += "Drunk, ";	break;
	case EFFECT_DIZZY:	sReturn += "Dizzy, ";	break;
	case EFFECT_SPACE:	sReturn += "Space, ";	break;
	case EFFECT_MINI:	sReturn += "Mini, ";	break;
	default:	ASSERT(0);	// invalid EFFECT
	}
	switch( m_AppearanceType )
	{
	case APPEARANCE_VISIBLE:							break;
	case APPEARANCE_HIDDEN:		sReturn += "Hidden, ";	break;
	case APPEARANCE_SUDDEN:		sReturn += "Sudden, ";	break;
	case APPEARANCE_STEALTH:	sReturn += "Stealth, ";	break;
	case APPEARANCE_BLINK:	sReturn += "Blink, ";	break;
	default:	ASSERT(0);	// invalid EFFECT
	}
	switch( m_TurnType )
	{
	case TURN_NONE:								break;
	case TURN_MIRROR:	sReturn += "Mirror, ";	break;
	case TURN_LEFT:		sReturn += "Left, ";	break;
	case TURN_RIGHT:	sReturn += "Right, ";	break;
	case TURN_SHUFFLE:	sReturn += "Shuffle, ";	break;
	default:	ASSERT(0);	// invalid EFFECT
	}
	if( m_bLittle )
		sReturn += "Little, ";
	if( m_bReverseScroll )
		sReturn += "Reverse, ";

	if( sReturn.GetLength() > 2 )
		sReturn.Delete( sReturn.GetLength()-2, 2 );	// delete the trailing ", "
	return sReturn;
}

void PlayerOptions::FromString( CString sOptions )
{
}