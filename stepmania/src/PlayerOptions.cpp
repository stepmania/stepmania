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
	ZERO( m_fScrolls );
	m_fDark = 0;
	m_fBlind = 0;
	m_Turn = TURN_NONE;
	m_Transform = TRANSFORM_NONE;
	m_bHoldNotes = true;
	m_bTimingAssist = false;
	m_bProTiming = false;
	m_fPerspectiveTilt = 0;
	m_fSkew = 0;
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
	for( i=0; i<NUM_SCROLLS; i++ )
		fapproach( m_fScrolls[i], other.m_fScrolls[i], fDeltaSeconds );
	fapproach( m_fDark, other.m_fDark, fDeltaSeconds );
	fapproach( m_fBlind, other.m_fBlind, fDeltaSeconds );
	fapproach( m_fPerspectiveTilt, other.m_fPerspectiveTilt, fDeltaSeconds );
	fapproach( m_fSkew, other.m_fSkew, fDeltaSeconds );
}

static CString AddPart( float level, CString name )
{
	if( level == 0 )
		return "";

	const CString LevelStr = (level == 1)? "": ssprintf( "%i%% ", int(level*100) );

	return LevelStr + name + ", ";
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

	sReturn += AddPart( m_fAccels[ACCEL_BOOST],		"Boost" );
	sReturn += AddPart( m_fAccels[ACCEL_BRAKE],		"Brake" );
	sReturn += AddPart( m_fAccels[ACCEL_WAVE],		"Wave" );
	sReturn += AddPart( m_fAccels[ACCEL_EXPAND],	"Expand" );
	sReturn += AddPart( m_fAccels[ACCEL_BOOMERANG],	"Boomerang" );

	sReturn += AddPart( m_fEffects[EFFECT_DRUNK],	"Drunk" );
	sReturn += AddPart( m_fEffects[EFFECT_DIZZY],	"Dizzy" );
	sReturn += AddPart( m_fEffects[EFFECT_MINI],	"Mini" );
	sReturn += AddPart( m_fEffects[EFFECT_FLIP],	"Flip" );
	sReturn += AddPart( m_fEffects[EFFECT_TORNADO],	"Tornado" );
	sReturn += AddPart( m_fEffects[EFFECT_TIPSY],	"Tipsy" );
	sReturn += AddPart( m_fEffects[EFFECT_BUMPY],	"Bumpy" );
	sReturn += AddPart( m_fEffects[EFFECT_BEAT],	"Beat" );

	sReturn += AddPart( m_fAppearances[APPEARANCE_HIDDEN],	"Hidden" );
	sReturn += AddPart( m_fAppearances[APPEARANCE_SUDDEN],	"Sudden" );
	sReturn += AddPart( m_fAppearances[APPEARANCE_STEALTH],	"Stealth" );
	sReturn += AddPart( m_fAppearances[APPEARANCE_BLINK],	"Blink" );
	sReturn += AddPart( m_fAppearances[APPEARANCE_RANDOMVANISH],	"RandomVanish" );

	sReturn += AddPart( m_fScrolls[SCROLL_REVERSE],	"Reverse" );
	sReturn += AddPart( m_fScrolls[SCROLL_SPLIT],	"Split" );
	sReturn += AddPart( m_fScrolls[SCROLL_ALTERNATE],	"Alternate" );

	sReturn += AddPart( m_fDark, "Dark");

	sReturn += AddPart( m_fBlind,	"Blind");

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
	case TRANSFORM_MINES:	sReturn += "Mines, ";	break;
	default:	ASSERT(0);	// invalid
	}

	if( !m_bHoldNotes )		sReturn += "NoHolds, ";
	if( m_bTimingAssist )	sReturn += "TimingAssist, ";
	if( m_bProTiming )		sReturn += "ProTiming, ";

	if( m_fSkew==1 && m_fPerspectiveTilt==-1 )
		sReturn += "Incoming, ";
	else if( m_fSkew==1 && m_fPerspectiveTilt==+1 )
		sReturn += "Space, ";
	else if( m_fSkew==0 && m_fPerspectiveTilt==-1 )
		sReturn += "Hallway, ";
	else if( m_fSkew==0 && m_fPerspectiveTilt==+1 )
		sReturn += "Distant, ";

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
			continue;
		}

		else if( sscanf( sBit, "c%d", &i1 ) == 1 )
		{
			m_bTimeSpacing = true;
			m_fScrollBPM = (float) i1;
			continue;
		}

		/* "drunk"
		 * "no drunk"
		 * "150% drunk" */

		float level = 1;
		CStringArray asParts;
		split( sBit, " ", asParts, true );

		if( asParts.size() > 1 )
		{
			sBit = asParts[1];

			if( asParts[0] == "no" )
				level = 0;
			else
			{
				sscanf( asParts[0], "%f", &level );
				level /= 100.0f;
			}
		}

		const bool on = (level > 0.5f);
			if( sBit == "boost" )		m_fAccels[ACCEL_BOOST] = level;
		else if( sBit == "brake" || sBit == "land" )		m_fAccels[ACCEL_BRAKE] = level;
		else if( sBit == "wave" )		m_fAccels[ACCEL_WAVE] = level;
		else if( sBit == "expand" )		m_fAccels[ACCEL_EXPAND] = level;
		else if( sBit == "boomerang" )	m_fAccels[ACCEL_BOOMERANG] = level;
		else if( sBit == "drunk" )		m_fEffects[EFFECT_DRUNK] = level;
		else if( sBit == "dizzy" )		m_fEffects[EFFECT_DIZZY] = level;
		else if( sBit == "mini" )		m_fEffects[EFFECT_MINI] = level;
		else if( sBit == "flip" )		m_fEffects[EFFECT_FLIP] = level;
		else if( sBit == "tornado" )	m_fEffects[EFFECT_TORNADO] = level;
		else if( sBit == "tipsy" )		m_fEffects[EFFECT_TIPSY] = level;
		else if( sBit == "bumpy" )		m_fEffects[EFFECT_BUMPY] = level;
		else if( sBit == "beat" )		m_fEffects[EFFECT_BEAT] = level;
		else if( sBit == "hidden" )		m_fAppearances[APPEARANCE_HIDDEN] = level;
		else if( sBit == "sudden" )		m_fAppearances[APPEARANCE_SUDDEN] = level;
		else if( sBit == "stealth" )	m_fAppearances[APPEARANCE_STEALTH] = level;
		else if( sBit == "blink" )		m_fAppearances[APPEARANCE_BLINK] = level;
		else if( sBit == "randomvanish" ) m_fAppearances[APPEARANCE_RANDOMVANISH] = level;
		else if( sBit == "turn" && !on )m_Turn = TURN_NONE; /* "no turn" */
		else if( sBit == "mirror" )		m_Turn = TURN_MIRROR;
		else if( sBit == "left" )		m_Turn = TURN_LEFT;
		else if( sBit == "right" )		m_Turn = TURN_RIGHT;
		else if( sBit == "shuffle" )	m_Turn = TURN_SHUFFLE;
		else if( sBit == "supershuffle" )m_Turn = TURN_SUPER_SHUFFLE;
		else if( sBit == "transform" && !on )	m_Transform = TRANSFORM_NONE; /* "no transform" */
		else if( sBit == "little" )		m_Transform = TRANSFORM_LITTLE;
		else if( sBit == "wide" )		m_Transform = TRANSFORM_WIDE;
		else if( sBit == "big" )		m_Transform = TRANSFORM_BIG;
		else if( sBit == "quick" )		m_Transform = TRANSFORM_QUICK;
		else if( sBit == "skippy" )		m_Transform = TRANSFORM_SKIPPY;
		else if( sBit == "mines" )		m_Transform = TRANSFORM_MINES;
		else if( sBit == "reverse" )	m_fScrolls[SCROLL_REVERSE] = level;
		else if( sBit == "split" )		m_fScrolls[SCROLL_SPLIT] = level;
		else if( sBit == "alternate" )	m_fScrolls[SCROLL_ALTERNATE] = level;
		else if( sBit == "noholds" )	m_bHoldNotes = !on;
		else if( sBit == "nofreeze" )	m_bHoldNotes = !on;
		else if( sBit == "dark" )		m_fDark = level;
		else if( sBit == "blind" )		m_fBlind = level;
		else if( sBit == "timingassist")m_bTimingAssist = on;
		else if( sBit == "protiming")	m_bProTiming = on;
		else if( sBit == "incoming" )	{ m_fSkew = 1; m_fPerspectiveTilt = -1; }
		else if( sBit == "space" )		{ m_fSkew = 1; m_fPerspectiveTilt = +1; }
		else if( sBit == "hallway" )	{ m_fSkew = 0; m_fPerspectiveTilt = -1; }
		else if( sBit == "distant" )	{ m_fSkew = 0; m_fPerspectiveTilt = +1; }
		else if( GAMESTATE->m_pPosition->IsValidModeForAnyStyle(sBit) )
			m_sPositioning = sBit;
		else if( sBit == "nopositioning" )
			m_sPositioning = "";
		else if( NOTESKIN->DoesNoteSkinExist(sBit) )
			m_sNoteSkin = sBit;
		else if( sBit == "noteskin" && !on ) /* "no noteskin" */
			m_sNoteSkin = "default";

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

void PlayerOptions::NextScroll()
{
	NextFloat( m_fScrolls, NUM_SCROLLS );
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
		m_fScrolls[SCROLL_REVERSE] = 1;
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

PlayerOptions::Scroll PlayerOptions::GetFirstScroll()
{
	for( int i=0; i<NUM_SCROLLS; i++ )
		if( m_fScrolls[i] == 1.f )
			return (Scroll)i;
	return (Scroll)-1;
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

void PlayerOptions::SetOneScroll( Scroll s )
{
	ZERO( m_fScrolls );
	m_fScrolls[s] = 1;
}

float PlayerOptions::GetReversePercentForColumn( int iCol )
{
	float f = 0;
	f += m_fScrolls[SCROLL_REVERSE];
	if( iCol >= GAMESTATE->GetCurrentStyleDef()->m_iColsPerPlayer/2 )
		f += m_fScrolls[SCROLL_SPLIT];
	if( (iCol%2)==1 )
		f += m_fScrolls[SCROLL_ALTERNATE];
	if( f > 2 )
		f = fmodf( f, 2 );
	if( f > 1 )
		f -= 1;
	return f;
}

bool ComparePlayerOptions( const PlayerOptions &po1, const PlayerOptions &po2 )
{
#define COMPARE(x) { if( po1.x != po2.x ) return false; }
	COMPARE(m_bTimeSpacing);
	COMPARE(m_fScrollSpeed);
	COMPARE(m_fScrollBPM);
	COMPARE(m_fDark);
	COMPARE(m_fBlind);
	COMPARE(m_Turn);
	COMPARE(m_Transform);
	COMPARE(m_bHoldNotes);
	COMPARE(m_bTimingAssist);
	COMPARE(m_bProTiming);
	COMPARE(m_fPerspectiveTilt);
	COMPARE(m_fSkew);
	COMPARE(m_sPositioning);
	COMPARE(m_sNoteSkin);
	int i;
	for( i = 0; i < PlayerOptions::NUM_ACCELS; ++i )
		COMPARE(m_fAccels[i]);
	for( i = 0; i < PlayerOptions::NUM_EFFECTS; ++i )
		COMPARE(m_fEffects[i]);
	for( i = 0; i < PlayerOptions::NUM_APPEARANCES; ++i )
		COMPARE(m_fAppearances[i]);
	for( i = 0; i < PlayerOptions::NUM_SCROLLS; ++i )
		COMPARE(m_fScrolls[i]);
#undef COMPARE

	return true;
}
