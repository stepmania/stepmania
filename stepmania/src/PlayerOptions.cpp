#include "global.h"
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


void PlayerOptions::Init()
{
	m_fScrollSpeed = 1.0f;
	m_AccelType = ACCEL_OFF;
	ZERO( m_bEffects );
	m_AppearanceType = APPEARANCE_VISIBLE;
	m_TurnType = TURN_NONE;
	m_Transform = TRANSFORM_NONE;
	m_bReverseScroll = false;
	m_bHoldNotes = true;
	m_bDark = false;
}


CString PlayerOptions::GetString()
{
	CString sReturn;

	if( m_fScrollSpeed != 1 )
	{
		/* -> 1.00 */
		CString s = ssprintf( "%2.2f", m_fScrollSpeed );
		if( s[s.GetLength()-1] == '0' ) {
			/* -> 1.0 */
			s.erase(s.GetLength()-1);	// delete last char
			if( s[s.GetLength()-1] == '0' ) {
				/* -> 1 */
				s.erase(s.GetLength()-2);	// delete last 2 chars
			}
		}
		sReturn += s + "X, ";
	}

	switch( m_AccelType )
	{
	case ACCEL_OFF:									break;
	case ACCEL_BOOST:		sReturn += "Boost, ";	break;
	case ACCEL_LAND:		sReturn += "Land, ";	break;
	case ACCEL_WAVE:		sReturn += "Wave, ";	break;
	case ACCEL_EXPAND:		sReturn += "Expand, ";	break;
	case ACCEL_BOOMERANG:	sReturn += "Boomerang, ";	break;
	default:	ASSERT(0);
	}

	if( m_bEffects[EFFECT_DRUNK] )	sReturn += "Drunk, ";
	if( m_bEffects[EFFECT_DIZZY] )	sReturn += "Dizzy, ";
	if( m_bEffects[EFFECT_SPACE] )	sReturn += "Space, ";
	if( m_bEffects[EFFECT_MINI] )	sReturn += "Mini, ";
	if( m_bEffects[EFFECT_FLIP] )	sReturn += "Flip, ";
	if( m_bEffects[EFFECT_TORNADO] )sReturn += "Tornado, ";

	switch( m_AppearanceType )
	{
	case APPEARANCE_VISIBLE:							break;
	case APPEARANCE_HIDDEN:		sReturn += "Hidden, ";	break;
	case APPEARANCE_SUDDEN:		sReturn += "Sudden, ";	break;
	case APPEARANCE_STEALTH:	sReturn += "Stealth, ";	break;
	case APPEARANCE_BLINK:		sReturn += "Blink, ";	break;
	default:	ASSERT(0);	// invalid
	}

	switch( m_TurnType )
	{
	case TURN_NONE:										break;
	case TURN_MIRROR:		sReturn += "Mirror, ";		break;
	case TURN_LEFT:			sReturn += "Left, ";		break;
	case TURN_RIGHT:		sReturn += "Right, ";		break;
	case TURN_SHUFFLE:		sReturn += "Shuffle, ";		break;
	case TURN_SUPER_SHUFFLE:sReturn += "SuperShuffle, ";break;
	default:	ASSERT(0);	// invalid
	}

	switch( m_Transform )
	{
	case TRANSFORM_NONE:								break;
	case TRANSFORM_LITTLE:	sReturn += "Little, ";		break;
	case TRANSFORM_WIDE:	sReturn += "Wide, ";		break;
	case TRANSFORM_TALL:	sReturn += "Tall, ";		break;
	default:	ASSERT(0);	// invalid
	}

	if( m_bReverseScroll )
		sReturn += "Reverse, ";

	if( !m_bHoldNotes )
		sReturn += "NoHolds, ";

	if( m_bDark )
		sReturn += "Dark, ";

	if( sReturn.GetLength() > 2 )
		sReturn.erase( sReturn.GetLength()-2 );	// delete the trailing ", "
	return sReturn;
}

void PlayerOptions::FromString( CString sOptions )
{
	Init();
	sOptions.MakeLower();
	CStringArray asBits;
	split( sOptions, ",", asBits, true );

	for( unsigned i=0; i<asBits.size(); i++ )
	{
		CString& sBit = asBits[i];
		TrimLeft(sBit);
		TrimRight(sBit);
		
		if(	     sBit == "0.25x" )		m_fScrollSpeed = 0.25f;
		else if( sBit == "0.5x" )		m_fScrollSpeed = 0.5f;
		else if( sBit == "0.75x" )		m_fScrollSpeed = 0.75f;
		else if( sBit == "1.5x" )		m_fScrollSpeed = 1.5f;
		else if( sBit == "2.0x" )		m_fScrollSpeed = 2.0f;
		else if( sBit == "3.0x" )		m_fScrollSpeed = 3.0f;
		else if( sBit == "4.0x" )		m_fScrollSpeed = 4.0f;
		else if( sBit == "5.0x" )		m_fScrollSpeed = 5.0f;
		else if( sBit == "8.0x" )		m_fScrollSpeed = 8.0f;
		else if( sBit == "boost" )		m_AccelType = ACCEL_BOOST;
		else if( sBit == "land" )		m_AccelType = ACCEL_LAND;
		else if( sBit == "wave" )		m_AccelType = ACCEL_WAVE;
		else if( sBit == "expand" )		m_AccelType = ACCEL_EXPAND;
		else if( sBit == "boomerang" )	m_AccelType = ACCEL_BOOMERANG;
		else if( sBit == "drunk" )		m_bEffects[EFFECT_DRUNK] = true;
		else if( sBit == "dizzy" )		m_bEffects[EFFECT_DIZZY] = true;
		else if( sBit == "space" )		m_bEffects[EFFECT_SPACE] = true;
		else if( sBit == "mini" )		m_bEffects[EFFECT_MINI] = true;
		else if( sBit == "flip" )		m_bEffects[EFFECT_FLIP] = true;
		else if( sBit == "tornado" )	m_bEffects[EFFECT_TORNADO] = true;
		else if( sBit == "hidden" )		m_AppearanceType = APPEARANCE_HIDDEN;
		else if( sBit == "sudden" )		m_AppearanceType = APPEARANCE_SUDDEN;
		else if( sBit == "stealth" )	m_AppearanceType = APPEARANCE_STEALTH;
		else if( sBit == "blink" )		m_AppearanceType = APPEARANCE_BLINK;
		else if( sBit == "mirror" )		m_TurnType = TURN_MIRROR;
		else if( sBit == "left" )		m_TurnType = TURN_LEFT;
		else if( sBit == "right" )		m_TurnType = TURN_RIGHT;
		else if( sBit == "shuffle" )	m_TurnType = TURN_SHUFFLE;
		else if( sBit == "supershuffle" )m_TurnType = TURN_SUPER_SHUFFLE;
		else if( sBit == "little" )		m_Transform = TRANSFORM_LITTLE;
		else if( sBit == "wide" )		m_Transform = TRANSFORM_WIDE;
		else if( sBit == "tall" )		m_Transform = TRANSFORM_TALL;
		else if( sBit == "reverse" )	m_bReverseScroll = true;
		else if( sBit == "noholds" )	m_bHoldNotes = false;
		else if( sBit == "nofreeze" )	m_bHoldNotes = false;
		else if( sBit == "dark" )		m_bDark = true;
	}
}


void PlayerOptions::NextAccel()
{
	m_AccelType = (AccelType) ((m_AccelType+1)%NUM_ACCEL_TYPES);
}

void PlayerOptions::NextEffect()
{
	if( m_bEffects[NUM_EFFECT_TYPES-1] )	// last effect is on
	{
		ZERO( m_bEffects );
		return;
	}

	for( int i=0; i<NUM_EFFECT_TYPES-1; i++ )
	{
		if( m_bEffects[i] )
		{
			ZERO( m_bEffects );
			m_bEffects[i+1] = true;
			return;
		}
	}

	// if we get here, no effects are on
	m_bEffects[0] = true;
}

void PlayerOptions::NextAppearance()
{
	m_AppearanceType = (AppearanceType) ((m_AppearanceType+1)%NUM_APPEARANCE_TYPES);
}

void PlayerOptions::NextTurn()
{
	m_TurnType = (TurnType) ((m_TurnType+1)%NUM_TURN_TYPES);
}

void PlayerOptions::NextTransform()
{
	m_Transform = (Transform) ((m_Transform+1)%NUM_TRANSFORMS);
}

void PlayerOptions::ChooseRandomMofifiers()
{
	if( RandomFloat(0,1)>0.8f )
		m_fScrollSpeed = 1.5f;
	if( RandomFloat(0,1)>0.8f )
		m_bReverseScroll = true;
	if( RandomFloat(0,1)>0.9f )
		m_bDark = true;
	float f;
	f = RandomFloat(0,1);
	if( f>0.66f )
		m_AccelType = (AccelType)(rand()%NUM_ACCEL_TYPES);
	else if( f>0.33f )
		m_bEffects[ rand()%NUM_EFFECT_TYPES ] = true;
	f = RandomFloat(0,1);
	if( f>0.95f )
		m_AppearanceType = APPEARANCE_HIDDEN;
	else if( f>0.9f )
		m_AppearanceType = APPEARANCE_SUDDEN;
}
