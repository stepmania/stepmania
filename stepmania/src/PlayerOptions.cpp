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
	{
		CString s = ssprintf( "%2.2f", m_fArrowScrollSpeed );
		if( s[s.GetLength()-1] == '0' )
			s.Delete(s.GetLength()-1);
		sReturn += s + "X, ";
	}

	if( m_EffectType & EFFECT_BOOST ) sReturn += "Boost, ";
	if( m_EffectType & EFFECT_WAVE ) sReturn += "Wave, ";
	if( m_EffectType & EFFECT_DRUNK ) sReturn += "Drunk, ";
	if( m_EffectType & EFFECT_DIZZY ) sReturn += "Dizzy, ";
	if( m_EffectType & EFFECT_SPACE ) sReturn += "Space, ";
	if( m_EffectType & EFFECT_MINI ) sReturn += "Mini, ";
	if( m_EffectType & EFFECT_FLIP ) sReturn += "Flip, ";
	if( m_EffectType & EFFECT_TORNADO ) sReturn += "Tornado, ";

	switch( m_AppearanceType )
	{
	case APPEARANCE_VISIBLE:							break;
	case APPEARANCE_HIDDEN:		sReturn += "Hidden, ";	break;
	case APPEARANCE_SUDDEN:		sReturn += "Sudden, ";	break;
	case APPEARANCE_STEALTH:	sReturn += "Stealth, ";	break;
	case APPEARANCE_BLINK:		sReturn += "Blink, ";	break;
	default:	ASSERT(0);	// invalid EFFECT
	}

	switch( m_TurnType )
	{
	case TURN_NONE:										break;
	case TURN_MIRROR:		sReturn += "Mirror, ";		break;
	case TURN_LEFT:			sReturn += "Left, ";		break;
	case TURN_RIGHT:		sReturn += "Right, ";		break;
	case TURN_SHUFFLE:		sReturn += "Shuffle, ";		break;
	case TURN_SUPER_SHUFFLE:sReturn += "SuperShuffle, ";break;
	default:	ASSERT(0);	// invalid EFFECT
	}

	if( m_bLittle )
		sReturn += "Little, ";

	if( m_bReverseScroll )
		sReturn += "Reverse, ";

	switch( m_ColorType )
	{
	case COLOR_VIVID:							break;
	case COLOR_NOTE:	sReturn += "Note, ";	break;
	case COLOR_FLAT:	sReturn += "Flat, ";	break;
	case COLOR_PLAIN:	sReturn += "Plain, ";	break;
	default:	ASSERT(0);	// invalid COLOR
	};

	if( !m_bHoldNotes )
		sReturn += "NoHolds, ";

	if( m_bDark )
		sReturn += "Dark, ";

	if( sReturn.GetLength() > 2 )
		sReturn.Delete( sReturn.GetLength()-2, 2 );	// delete the trailing ", "
	return sReturn;
}

void PlayerOptions::FromString( CString sOptions )
{
	Init();
	sOptions.MakeLower();
	CStringArray asBits;
	split( sOptions, ",", asBits, true );

	for( int i=0; i<asBits.GetSize(); i++ )
	{
		CString& sBit = asBits[i];
		sBit.TrimLeft();
		sBit.TrimRight();
		
		if(	     sBit == "0.5x" )		m_fArrowScrollSpeed = 0.5f;
		else if( sBit == "0.75x" )		m_fArrowScrollSpeed = 0.75f;
		else if( sBit == "1.5x" )		m_fArrowScrollSpeed = 1.5f;
		else if( sBit == "2.0x" )		m_fArrowScrollSpeed = 2.0f;
		else if( sBit == "3.0x" )		m_fArrowScrollSpeed = 3.0f;
		else if( sBit == "4.0x" )		m_fArrowScrollSpeed = 4.0f;
		else if( sBit == "5.0x" )		m_fArrowScrollSpeed = 5.0f;
		else if( sBit == "8.0x" )		m_fArrowScrollSpeed = 8.0f;
		else if( sBit == "boost" )		m_EffectType |= EFFECT_BOOST;
		else if( sBit == "wave" )		m_EffectType |= EFFECT_WAVE;
		else if( sBit == "drunk" )		m_EffectType |= EFFECT_DRUNK;
		else if( sBit == "dizzy" )		m_EffectType |= EFFECT_DIZZY;
		else if( sBit == "space" )		m_EffectType |= EFFECT_SPACE;
		else if( sBit == "mini" )		m_EffectType |= EFFECT_MINI;
		else if( sBit == "flip" )		m_EffectType |= EFFECT_FLIP;
		else if( sBit == "tornado" )	m_EffectType |= EFFECT_TORNADO;
		else if( sBit == "hidden" )		m_AppearanceType = APPEARANCE_HIDDEN;
		else if( sBit == "sudden" )		m_AppearanceType = APPEARANCE_SUDDEN;
		else if( sBit == "stealth" )	m_AppearanceType = APPEARANCE_STEALTH;
		else if( sBit == "blink" )		m_AppearanceType = APPEARANCE_BLINK;
		else if( sBit == "mirror" )		m_TurnType = TURN_MIRROR;
		else if( sBit == "left" )		m_TurnType = TURN_LEFT;
		else if( sBit == "right" )		m_TurnType = TURN_RIGHT;
		else if( sBit == "shuffle" )	m_TurnType = TURN_SHUFFLE;
		else if( sBit == "supershuffle" )m_TurnType = TURN_SUPER_SHUFFLE;
		else if( sBit == "little" )		m_bLittle = true;
		else if( sBit == "reverse" )	m_bReverseScroll = true;
		else if( sBit == "note" )		m_ColorType = COLOR_NOTE;
		else if( sBit == "flat" )		m_ColorType = COLOR_FLAT;
		else if( sBit == "plain" )		m_ColorType = COLOR_PLAIN;
		else if( sBit == "noholds" )	m_bHoldNotes = false;
		else if( sBit == "nofreeze" )	m_bHoldNotes = false;
		else if( sBit == "dark" )		m_bDark = true;
	}
}