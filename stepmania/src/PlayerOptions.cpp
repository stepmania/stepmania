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
#include "RageLog.h"

#include "GameState.h"
#include "NoteFieldPositioning.h"
#include "NoteSkinManager.h"

void PlayerOptions::Init()
{
	m_bTimeSpacing = false;
	m_fScrollSpeed = 1.0f;
	m_fScrollBPM = 200;
	ZERO( m_fAccels );
	ZERO( m_fEffects );
	ZERO( m_fAppearances );
	m_fReverseScroll = 0;
	m_fDark = 0;
	m_Turn = TURN_NONE;
	m_Transform = TRANSFORM_NONE;
	m_bHoldNotes = true;
	m_bTimingAssist = false;
	m_bProTiming = false;
	m_fPerspectiveTilt = 0;
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

	if( !m_bTimeSpacing )
	{
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
	}
	else
	{
		CString s = ssprintf( "C%.0f", m_fScrollBPM );
		sReturn += s + ", ";
	}

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
	if( m_fEffects[EFFECT_BUMPY]==1 )	sReturn += "Bumpy, ";
	if( m_fEffects[EFFECT_BEAT]==1 )	sReturn += "Beat, ";

	if( m_fAppearances[APPEARANCE_HIDDEN]==1 )	sReturn += "Hidden, ";
	if( m_fAppearances[APPEARANCE_SUDDEN]==1 )	sReturn += "Sudden, ";
	if( m_fAppearances[APPEARANCE_STEALTH]==1 )	sReturn += "Stealth, ";
	if( m_fAppearances[APPEARANCE_BLINK]==1 )	sReturn += "Blink, ";
	if( m_fAppearances[APPEARANCE_RANDOMVANISH]==1) sReturn += "RandomVanish, ";

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
	case TRANSFORM_SKIPPY:	sReturn += "Skippy, ";	break;
	default:	ASSERT(0);	// invalid
	}

	if( !m_bHoldNotes )		sReturn += "NoHolds, ";
	if( m_bTimingAssist )	sReturn += "TimingAssist, ";
	if( m_bProTiming )		sReturn += "ProTiming, ";

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
	ASSERT( GAMESTATE->m_pPosition );
	ASSERT( NOTESKIN );
//	Init();
	sOptions.MakeLower();
	CStringArray asBits;
	split( sOptions, ",", asBits, true );

	for( unsigned i=0; i<asBits.size(); i++ )
	{
		CString& sBit = asBits[i];
		TrimLeft(sBit);
		TrimRight(sBit);

		int i1;

		Regex mult("^([0-9]+(\\.[0-9]+)?)x$");
		vector<CString> matches;
		if( mult.Compare(sBit, matches) )
		{
			m_bTimeSpacing = false;
			int ret = sscanf( matches[0], "%f", &m_fScrollSpeed );
			ASSERT( ret == 1 );
		}
		/* This has two problems: it misparses .75 (anything where the fractional
		 * part is >= 10), and sscanf doesn't actually care about input after the
		 * last placeholder: "5" will match, the "x" won't be required. */
/*

		int i1, i2;
		if(	sscanf( sBit, "%d.%dx", &i1, &i2 ) == 2 )
		{
			m_bTimeSpacing = false;
			m_fScrollSpeed = i1 + (i2 / 10.f);
		}
		else if( sscanf( sBit, "%dx", &i1 ) == 1 )
		{
			m_bTimeSpacing = false;
			m_fScrollSpeed = i1;
		}
		else if( sscanf( sBit, ".%dx", &i1 ) == 1 )
		{
			m_bTimeSpacing = false;
			m_fScrollSpeed = i1 / 10.f;
		}
*/
		else if( sscanf( sBit, "C%d", &i1 ) == 1 )
		{
			m_bTimeSpacing = true;
			m_fScrollBPM = (float) i1;
		}
		else if( sBit == "boost" )		m_fAccels[ACCEL_BOOST] = 1;
		else if( sBit == "brake" || sBit == "land" )		m_fAccels[ACCEL_BRAKE] = 1;
		else if( sBit == "wave" )		m_fAccels[ACCEL_WAVE] = 1;
		else if( sBit == "expand" )		m_fAccels[ACCEL_EXPAND] = 1;
		else if( sBit == "boomerang" )	m_fAccels[ACCEL_BOOMERANG] = 1;
		else if( sBit == "drunk" )		m_fEffects[EFFECT_DRUNK] = 1;
		else if( sBit == "dizzy" )		m_fEffects[EFFECT_DIZZY] = 1;
		else if( sBit == "mini" )		m_fEffects[EFFECT_MINI] = 1;
		else if( sBit == "flip" )		m_fEffects[EFFECT_FLIP] = 1;
		else if( sBit == "tornado" )	m_fEffects[EFFECT_TORNADO] = 1;
		else if( sBit == "bumpy" )		m_fEffects[EFFECT_BUMPY] = 1;
		else if( sBit == "beat" )		m_fEffects[EFFECT_BEAT] = 1;
		else if( sBit == "hidden" )		m_fAppearances[APPEARANCE_HIDDEN] = 1;
		else if( sBit == "sudden" )		m_fAppearances[APPEARANCE_SUDDEN] = 1;
		else if( sBit == "stealth" )	m_fAppearances[APPEARANCE_STEALTH] = 1;
		else if( sBit == "blink" )		m_fAppearances[APPEARANCE_BLINK] = 1;
		else if( sBit == "randomvanish" ) m_fAppearances[APPEARANCE_RANDOMVANISH] = 1;
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
		else if( sBit == "protiming")	m_bProTiming = true;
		else if( sBit == "incoming" )	m_fPerspectiveTilt = -1;
		else if( sBit == "space" )		m_fPerspectiveTilt = +1;
		else if( GAMESTATE->m_pPosition->IsValidModeForAnyStyle(sBit) )
			m_sPositioning = sBit;
		else if( NOTESKIN->DoesNoteSkinExist(sBit) )
			m_sNoteSkin = sBit;
		// XXX: this warns about song options
		//else
		//	LOG->Warn( "Modifier '%s' not recognized.", sBit.c_str() );
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
