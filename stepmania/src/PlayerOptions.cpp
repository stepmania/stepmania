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
#include "math.h"


void PlayerOptions::Init()
{
	m_fScrollSpeed = 1.0f;
	ZERO( m_fAccels );
	ZERO( m_fEffects );
	ZERO( m_fAppearances );
	m_fReverseScroll = 0;
	m_fDark = 0;
	m_Turn = TURN_NONE;
	m_Transform = TRANSFORM_NONE;
	m_bHoldNotes = true;
}

void FLOAT_APPROACH( float& val, float other_val, float deltaPercent )
{
	if( val == other_val )
		return;
	float fDelta = other_val - val;
	float fSign = fDelta / fabsf( fDelta );
	float fToMove = fSign*deltaPercent;
	if( fabsf(fToMove) > fabsf(fDelta) )
		fToMove = fDelta;	// snap
	val += fToMove;
}

void PlayerOptions::Approach( const PlayerOptions& other, float fDeltaSeconds )
{
	int i;

	FLOAT_APPROACH( m_fScrollSpeed, other.m_fScrollSpeed, fDeltaSeconds*max(m_fScrollSpeed,other.m_fScrollSpeed) );	// make big jumps in scroll speed move faster
	for( i=0; i<NUM_ACCELS; i++ )
		FLOAT_APPROACH( m_fAccels[i], other.m_fAccels[i], fDeltaSeconds );
	for( i=0; i<NUM_EFFECTS; i++ )
		FLOAT_APPROACH( m_fEffects[i], other.m_fEffects[i], fDeltaSeconds );
	for( i=0; i<NUM_APPEARANCES; i++ )
		FLOAT_APPROACH( m_fAppearances[i], other.m_fAppearances[i], fDeltaSeconds );
	FLOAT_APPROACH( m_fReverseScroll, other.m_fReverseScroll, fDeltaSeconds );
	FLOAT_APPROACH( m_fDark, other.m_fDark, fDeltaSeconds );
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

	if( m_fAccels[ACCEL_BOOST]==1 )		sReturn += "Boost, ";
	if( m_fAccels[ACCEL_LAND]==1 )		sReturn += "Land, ";
	if( m_fAccels[ACCEL_WAVE]==1 )		sReturn += "Wave, ";
	if( m_fAccels[ACCEL_EXPAND]==1 )	sReturn += "Expand, ";
	if( m_fAccels[ACCEL_BOOMERANG]==1 )	sReturn += "Boomerang, ";

	if( m_fEffects[EFFECT_DRUNK]==1 )	sReturn += "Drunk, ";
	if( m_fEffects[EFFECT_DIZZY]==1 )	sReturn += "Dizzy, ";
	if( m_fEffects[EFFECT_SPACE]==1 )	sReturn += "Space, ";
	if( m_fEffects[EFFECT_MINI]==1 )	sReturn += "Mini, ";
	if( m_fEffects[EFFECT_FLIP]==1 )	sReturn += "Flip, ";
	if( m_fEffects[EFFECT_TORNADO]==1 )	sReturn += "Tornado, ";

	if( m_fAppearances[APPEARANCE_HIDDEN]==1 )	sReturn += "Hidden, ";
	if( m_fAppearances[APPEARANCE_SUDDEN]==1 )	sReturn += "Sudden, ";
	if( m_fAppearances[APPEARANCE_STEALTH]==1 )	sReturn += "Stealth, ";
	if( m_fAppearances[APPEARANCE_BLINK]==1 )	sReturn += "Blink, ";

	if( m_fReverseScroll == 1 )		sReturn += "Reverse, ";

	if( m_fDark == 1)				sReturn += "Dark, ";


	switch( m_Turn )
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
	case TRANSFORM_NONE:							break;
	case TRANSFORM_LITTLE:	sReturn += "Little, ";	break;
	case TRANSFORM_WIDE:	sReturn += "Wide, ";	break;
	case TRANSFORM_BIG:		sReturn += "Big, ";		break;
	case TRANSFORM_QUICK:	sReturn += "Quick, ";	break;
	default:	ASSERT(0);	// invalid
	}

	if( !m_bHoldNotes )		sReturn += "NoHolds, ";


	if( sReturn.GetLength() > 2 )
		sReturn.erase( sReturn.GetLength()-2 );	// delete the trailing ", "
	return sReturn;
}

/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void PlayerOptions::FromString( CString sOptions )
{
//	Init();
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
		else if( sBit == "boost" )		m_fAccels[ACCEL_BOOST] = 1;
		else if( sBit == "land" )		m_fAccels[ACCEL_LAND] = 1;
		else if( sBit == "wave" )		m_fAccels[ACCEL_WAVE] = 1;
		else if( sBit == "expand" )		m_fAccels[ACCEL_EXPAND] = 1;
		else if( sBit == "boomerang" )	m_fAccels[ACCEL_BOOMERANG] = 1;
		else if( sBit == "drunk" )		m_fEffects[EFFECT_DRUNK] = 1;
		else if( sBit == "dizzy" )		m_fEffects[EFFECT_DIZZY] = 1;
		else if( sBit == "space" )		m_fEffects[EFFECT_SPACE] = 1;
		else if( sBit == "mini" )		m_fEffects[EFFECT_MINI] = 1;
		else if( sBit == "flip" )		m_fEffects[EFFECT_FLIP] = 1;
		else if( sBit == "tornado" )	m_fEffects[EFFECT_TORNADO] = 1;
		else if( sBit == "hidden" )		m_fAppearances[APPEARANCE_HIDDEN] = 1;
		else if( sBit == "sudden" )		m_fAppearances[APPEARANCE_SUDDEN] = 1;
		else if( sBit == "stealth" )	m_fAppearances[APPEARANCE_STEALTH] = 1;
		else if( sBit == "blink" )		m_fAppearances[APPEARANCE_BLINK] = 1;
		else if( sBit == "mirror" )		m_Turn = TURN_MIRROR;
		else if( sBit == "left" )		m_Turn = TURN_LEFT;
		else if( sBit == "right" )		m_Turn = TURN_RIGHT;
		else if( sBit == "shuffle" )	m_Turn = TURN_SHUFFLE;
		else if( sBit == "supershuffle" )m_Turn = TURN_SUPER_SHUFFLE;
		else if( sBit == "little" )		m_Transform = TRANSFORM_LITTLE;
		else if( sBit == "wide" )		m_Transform = TRANSFORM_WIDE;
		else if( sBit == "big" )		m_Transform = TRANSFORM_BIG;
		else if( sBit == "quick" )		m_Transform = TRANSFORM_QUICK;
		else if( sBit == "reverse" )	m_fReverseScroll = 1;
		else if( sBit == "noholds" )	m_bHoldNotes = false;
		else if( sBit == "nofreeze" )	m_bHoldNotes = false;
		else if( sBit == "dark" )		m_fDark = 1;
	}
}


void NextFloat( float fValues[], int size )
{
	int index = -1;
	for( int i=0; i<size; i++ )
	{
		if( fValues[i] == 1 )
		{
			index = i;
			break;
		}
	}

	memset( fValues, 0, size );

	index++;
	if( index == size )	// if true, then the last float in the list was selected
		;	// leave all off
	else
		fValues[index] = 1;
}

void PlayerOptions::NextAccel()
{
	NextFloat( m_fAccels, NUM_ACCELS );
}

void PlayerOptions::NextEffect()
{
	NextFloat( m_fEffects, NUM_EFFECTS );
}

void PlayerOptions::NextAppearance()
{
	NextFloat( m_fAppearances, NUM_APPEARANCES );
}

void PlayerOptions::NextTurn()
{
	m_Turn = (Turn) ((m_Turn+1)%NUM_TURNS);
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
		m_fReverseScroll = 1;
	if( RandomFloat(0,1)>0.9f )
		m_fDark = 1;
	float f;
	f = RandomFloat(0,1);
	if( f>0.66f )
		m_fAccels[rand()%NUM_ACCELS] = 1;
	else if( f>0.33f )
		m_fEffects[rand()%NUM_EFFECTS] = 1;
	f = RandomFloat(0,1);
	if( f>0.95f )
		m_fAppearances[APPEARANCE_HIDDEN] = 1;
	else if( f>0.9f )
		m_fAppearances[APPEARANCE_SUDDEN] = 1;
}

PlayerOptions::Accel PlayerOptions::GetFirstAccel()
{
	for( int i=0; i<NUM_ACCELS; i++ )
		if( m_fAccels[i] == 1.f )
			return (Accel)i;
	return (Accel)-1;
}

PlayerOptions::Effect PlayerOptions::GetFirstEffect()
{
	for( int i=0; i<NUM_EFFECTS; i++ )
		if( m_fEffects[i] == 1.f )
			return (Effect)i;
	return (Effect)-1;
}

PlayerOptions::Appearance PlayerOptions::GetFirstAppearance()
{
	for( int i=0; i<NUM_APPEARANCES; i++ )
		if( m_fAppearances[i] == 1.f )
			return (Appearance)i;
	return (Appearance)-1;
}

void PlayerOptions::SetOneAccel( Accel a )
{
	ZERO( m_fAccels );
	m_fAccels[a] = 1;
}

void PlayerOptions::SetOneEffect( Effect e )
{
	ZERO( m_fEffects );
	m_fEffects[e] = 1;
}

void PlayerOptions::SetOneAppearance( Appearance a )
{
	ZERO( m_fAppearances );
	m_fAppearances[a] = 1;
}
