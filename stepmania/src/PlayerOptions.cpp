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

#include "GameState.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"

void PlayerOptions::Init()
{
//	m_bUseScrollBPM = false;
	m_fScrollSpeed = 1.0f;
//	m_fScrollBPM = 200;
	ZERO( m_fAccels );
	ZERO( m_fEffects );
	ZERO( m_fAppearances );
	m_fReverseScroll = 0;
	m_fDark = 0;
	m_Turn = TURN_NONE;
	m_Transform = TRANSFORM_NONE;
	m_bHoldNotes = true;
	m_bTimingAssist = false;
	m_fPerspectiveTilt = 0;
	m_bTimeSpacing = false;
	m_sPositioning = "";	// "null"
	m_sNoteSkin = "default";

}

void PlayerOptions::Approach( const PlayerOptions& other, float fDeltaSeconds )
{
	int i;

	fapproach( m_fScrollSpeed, other.m_fScrollSpeed, fDeltaSeconds*(0.2f+fabsf(m_fScrollSpeed-other.m_fScrollSpeed)) );	// make big jumps in scroll speed move faster
	for( i=0; i<NUM_ACCELS; i++ )
		fapproach( m_fAccels[i], other.m_fAccels[i], fDeltaSeconds );
	for( i=0; i<NUM_EFFECTS; i++ )
		fapproach( m_fEffects[i], other.m_fEffects[i], fDeltaSeconds );
	for( i=0; i<NUM_APPEARANCES; i++ )
		fapproach( m_fAppearances[i], other.m_fAppearances[i], fDeltaSeconds );
	fapproach( m_fReverseScroll, other.m_fReverseScroll, fDeltaSeconds );
	fapproach( m_fDark, other.m_fDark, fDeltaSeconds );
	fapproach( m_fPerspectiveTilt, other.m_fPerspectiveTilt, fDeltaSeconds );
}

CString PlayerOptions::GetString()
{
	CString sReturn;

//	if( !m_bUseScrollBPM )
//	{
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
//	}
//	else
//	{
//		CString s = ssprintf( "%.0f", m_fScrollBPM );
//		sReturn += s + "V, ";
//	}

	if( m_fAccels[ACCEL_BOOST]==1 )		sReturn += "Boost, ";
	if( m_fAccels[ACCEL_BRAKE]==1 )		sReturn += "Brake, ";
	if( m_fAccels[ACCEL_WAVE]==1 )		sReturn += "Wave, ";
	if( m_fAccels[ACCEL_EXPAND]==1 )	sReturn += "Expand, ";
	if( m_fAccels[ACCEL_BOOMERANG]==1 )	sReturn += "Boomerang, ";

	if( m_fEffects[EFFECT_DRUNK]==1 )	sReturn += "Drunk, ";
	if( m_fEffects[EFFECT_DIZZY]==1 )	sReturn += "Dizzy, ";
	if( m_fEffects[EFFECT_MINI]==1 )	sReturn += "Mini, ";
	if( m_fEffects[EFFECT_FLIP]==1 )	sReturn += "Flip, ";
	if( m_fEffects[EFFECT_TORNADO]==1 )	sReturn += "Tornado, ";

	if( m_fAppearances[APPEARANCE_HIDDEN]==1 )	sReturn += "Hidden, ";
	if( m_fAppearances[APPEARANCE_SUDDEN]==1 )	sReturn += "Sudden, ";
	if( m_fAppearances[APPEARANCE_STEALTH]==1 )	sReturn += "Stealth, ";
	if( m_fAppearances[APPEARANCE_BLINK]==1 )	sReturn += "Blink, ";

	if( m_fReverseScroll == 1 )		sReturn += "Reverse, ";

	if( m_fDark == 1)				sReturn += "Dark, ";

	if( m_bTimeSpacing )			sReturn += "TimeSpacing, ";

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
	case TRANSFORM_SKIPPY:	sReturn += "Skippy, ";	break;
	default:	ASSERT(0);	// invalid
	}

	if( !m_bHoldNotes )		sReturn += "NoHolds, ";
	if( m_bTimingAssist )	sReturn += "TimingAssist, ";

	switch( (int)m_fPerspectiveTilt )
	{
	case -1:	sReturn += "Incoming, ";	break;
	case +1:	sReturn += "Space, ";		break;
	}

	if( !m_sPositioning.empty() )
		sReturn += m_sPositioning + ", ";
	if( !m_sNoteSkin.empty()  &&  m_sNoteSkin.CompareNoCase("default")!=0 )
		sReturn += m_sNoteSkin + ", ";

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
		
		if(	     sBit == "0.25x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 0.25f;	}
		else if( sBit == "0.5x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 0.50f;	}
		else if( sBit == "0.75x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 0.75f;	}
		else if( sBit == "1.5x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 1.00f;	}
		else if( sBit == "2.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 2.00f;	}
		else if( sBit == "3.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 3.00f;	}
		else if( sBit == "4.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 4.00f;	}
		else if( sBit == "5.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 5.00f;	}
		else if( sBit == "8.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 8.00f;	}
		else if( sBit == "12.0x" )		{ /*m_bUseScrollBPM=false;*/	m_fScrollSpeed = 12.00f;	}
//		else if( sBit == "200v" )		{ m_bUseScrollBPM=true;		m_fScrollBPM = 200;	}
//		else if( sBit == "300v" )		{ m_bUseScrollBPM=true;		m_fScrollBPM = 300;	}
//		else if( sBit == "450v" )		{ m_bUseScrollBPM=true;		m_fScrollBPM = 450;	}
		else if( sBit == "boost" )		m_fAccels[ACCEL_BOOST] = 1;
		else if( sBit == "brake" )		m_fAccels[ACCEL_BRAKE] = 1;
		else if( sBit == "wave" )		m_fAccels[ACCEL_WAVE] = 1;
		else if( sBit == "expand" )		m_fAccels[ACCEL_EXPAND] = 1;
		else if( sBit == "boomerang" )	m_fAccels[ACCEL_BOOMERANG] = 1;
		else if( sBit == "drunk" )		m_fEffects[EFFECT_DRUNK] = 1;
		else if( sBit == "dizzy" )		m_fEffects[EFFECT_DIZZY] = 1;
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
		else if( sBit == "skippy" )		m_Transform = TRANSFORM_SKIPPY;
		else if( sBit == "reverse" )	m_fReverseScroll = 1;
		else if( sBit == "noholds" )	m_bHoldNotes = false;
		else if( sBit == "nofreeze" )	m_bHoldNotes = false;
		else if( sBit == "dark" )		m_fDark = 1;
		else if( sBit == "timingassist")m_bTimingAssist = true;
		else if( sBit == "incoming" )	m_fPerspectiveTilt = -1;
		else if( sBit == "space" )		m_fPerspectiveTilt = +1;
		else if( GAMESTATE->m_pPosition->IsValidModeForCurrentGame(sBit) )
			m_sPositioning = sBit;
		else if( NOTESKIN->DoesNoteSkinExist(sBit) )
			m_sNoteSkin = sBit;
	}
}


void NextFloat( float fValues[], int size )
{
	int index = -1;
	int i;
	for( i=0; i<size; i++ )
	{
		if( fValues[i] == 1 )
		{
			index = i;
			break;
		}
	}

	for( i=0; i<size; i++ )
		fValues[i] = 0;

	index;
	if( index == size-1 )	// if true, then the last float in the list was selected
		;	// leave all off
	else
		fValues[index+1] = 1;
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

void PlayerOptions::NextPerspective()
{
	switch( (int)m_fPerspectiveTilt )
	{
	case -1:			m_fPerspectiveTilt =  0;	break;
	case  0:			m_fPerspectiveTilt = +1;	break;
	case +1: default:	m_fPerspectiveTilt = -1;	break;
	}
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
