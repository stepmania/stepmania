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

#define ONE( arr ) { for( unsigned Z = 0; Z < ARRAYSIZE(arr); ++Z ) arr[Z]=1.0f; }

void PlayerOptions::Init()
{
	m_bTimeSpacing = false;
	m_fScrollSpeed = 1.0f;		m_SpeedfScrollSpeed = 1.0f;
	m_fScrollBPM = 200;			m_SpeedfScrollBPM = 1.0f;
	ZERO( m_fAccels );			ONE( m_SpeedfAccels );
	ZERO( m_fEffects );			ONE( m_SpeedfEffects );
	ZERO( m_fAppearances );		ONE( m_SpeedfAppearances );
	ZERO( m_fScrolls );			ONE( m_SpeedfScrolls );
	m_fDark = 0;				m_SpeedfDark = 1.0f;
	m_fBlind = 0;				m_SpeedfBlind = 1.0f;
	m_fPerspectiveTilt = 0;		m_SpeedfPerspectiveTilt = 1.0f;
	m_fSkew = 0;				m_SpeedfSkew = 1.0f;
	m_fPassmark = 0;			m_SpeedfPassmark = 1.0f;
	m_Turn = TURN_NONE;
	ZERO( m_bTransforms );
	m_bTimingAssist = false;
	m_bProTiming = false;
	m_sPositioning = "";	// "null"
	m_sNoteSkin = "default";
}

void PlayerOptions::Approach( const PlayerOptions& other, float fDeltaSeconds )
{
#define APP( opt ) \
	fapproach( m_ ## opt, other.m_ ## opt, fDeltaSeconds * other.m_Speed ## opt );

	int i;
	for( i=0; i<NUM_ACCELS; i++ )
		APP( fAccels[i] );
	for( i=0; i<NUM_EFFECTS; i++ )
		APP( fEffects[i] );
	for( i=0; i<NUM_APPEARANCES; i++ )
		APP( fAppearances[i] );
	for( i=0; i<NUM_SCROLLS; i++ )
		APP( fScrolls[i] );
	APP( fScrollSpeed );
	APP( fDark );
	APP( fBlind );
	APP( fPerspectiveTilt );
	APP( fSkew );
	APP( fPassmark );
}

static CString AddPart( float level, CString name )
{
	if( level == 0 )
		return "";

	const CString LevelStr = (level == 1)? "": ssprintf( "%i%% ", (int) roundf(level*100) );

	return LevelStr + name + ", ";
}

CString PlayerOptions::GetString() const
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

	sReturn += AddPart( m_fPassmark, "Passmark");

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

	if( m_bTransforms[TRANSFORM_NOHOLDS] )	sReturn += "NoHolds, ";
	if( m_bTransforms[TRANSFORM_NOMINES] )	sReturn += "NoMines, ";
	if( m_bTransforms[TRANSFORM_LITTLE] )	sReturn += "Little, ";
	if( m_bTransforms[TRANSFORM_WIDE] )		sReturn += "Wide, ";
	if( m_bTransforms[TRANSFORM_BIG] )		sReturn += "Big, ";
	if( m_bTransforms[TRANSFORM_QUICK] )	sReturn += "Quick, ";
	if( m_bTransforms[TRANSFORM_SKIPPY] )	sReturn += "Skippy, ";
	if( m_bTransforms[TRANSFORM_MINES] )	sReturn += "Mines, ";
	if( m_bTransforms[TRANSFORM_ECHO] )		sReturn += "Echo, ";
	if( m_bTransforms[TRANSFORM_PLANTED] )	sReturn += "Planted, ";
	if( m_bTransforms[TRANSFORM_STOMP] )	sReturn += "Stomp, ";
	if( m_bTransforms[TRANSFORM_TWISTER] )	sReturn += "Twister, ";
	if( m_bTransforms[TRANSFORM_NOJUMPS] )	sReturn += "NoJumps, ";
	if( m_bTransforms[TRANSFORM_NOHANDS] )	sReturn += "NoHands, ";

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
		 * "150% drunk"
		 * "*2 100% drunk": approach at 2x normal speed */

		float level = 1;
		float speed = 1;
		CStringArray asParts;
		split( sBit, " ", asParts, true );

		for( unsigned j = 0; j < asParts.size()-1; ++j )
		{
			if( asParts[j] == "no" )
				level = 0;
			else if( isdigit(asParts[j][0]) )
			{
				/* If the last character is a *, they probably said "123*" when
				 * they meant "*123". */
				if( asParts[j].Right(1) == "*" )
					RageException::Throw("Invalid player options '%i*'; did you mean '*%i'?", 
						atoi(asParts[j]), atoi(asParts[j]) );
				sscanf( asParts[j], "%f", &level );
				level /= 100.0f;
			}
			else if( asParts[j][0]=='*' )
				sscanf( asParts[j], "*%f", &speed );
		}
		sBit = asParts[asParts.size()-1];

#define SET_FLOAT( opt ) \
		{ m_ ## opt = level; m_Speed ## opt = speed; }
		const bool on = (level > 0.5f);
			 if( sBit == "boost" )		SET_FLOAT( fAccels[ACCEL_BOOST] )
		else if( sBit == "brake" || sBit == "land" ) SET_FLOAT( fAccels[ACCEL_BRAKE] )
		else if( sBit == "wave" )		SET_FLOAT( fAccels[ACCEL_WAVE] )
		else if( sBit == "expand" )		SET_FLOAT( fAccels[ACCEL_EXPAND] )
		else if( sBit == "boomerang" )	SET_FLOAT( fAccels[ACCEL_BOOMERANG] )
		else if( sBit == "drunk" )		SET_FLOAT( fEffects[EFFECT_DRUNK] )
		else if( sBit == "dizzy" )		SET_FLOAT( fEffects[EFFECT_DIZZY] )
		else if( sBit == "mini" )		SET_FLOAT( fEffects[EFFECT_MINI] )
		else if( sBit == "flip" )		SET_FLOAT( fEffects[EFFECT_FLIP] )
		else if( sBit == "tornado" )	SET_FLOAT( fEffects[EFFECT_TORNADO] )
		else if( sBit == "tipsy" )		SET_FLOAT( fEffects[EFFECT_TIPSY] )
		else if( sBit == "bumpy" )		SET_FLOAT( fEffects[EFFECT_BUMPY] )
		else if( sBit == "beat" )		SET_FLOAT( fEffects[EFFECT_BEAT] )
		else if( sBit == "hidden" )		SET_FLOAT( fAppearances[APPEARANCE_HIDDEN] )
		else if( sBit == "sudden" )		SET_FLOAT( fAppearances[APPEARANCE_SUDDEN] )
		else if( sBit == "stealth" )	SET_FLOAT( fAppearances[APPEARANCE_STEALTH] )
		else if( sBit == "blink" )		SET_FLOAT( fAppearances[APPEARANCE_BLINK] )
		else if( sBit == "randomvanish" ) SET_FLOAT( fAppearances[APPEARANCE_RANDOMVANISH] )
		else if( sBit == "turn" && !on )m_Turn = TURN_NONE; /* "no turn" */
		else if( sBit == "mirror" )		m_Turn = TURN_MIRROR;
		else if( sBit == "left" )		m_Turn = TURN_LEFT;
		else if( sBit == "right" )		m_Turn = TURN_RIGHT;
		else if( sBit == "shuffle" )	m_Turn = TURN_SHUFFLE;
		else if( sBit == "supershuffle" )m_Turn = TURN_SUPER_SHUFFLE;
		else if( sBit == "little" )		m_bTransforms[TRANSFORM_LITTLE] = on;
		else if( sBit == "wide" )		m_bTransforms[TRANSFORM_WIDE] = on;
		else if( sBit == "big" )		m_bTransforms[TRANSFORM_BIG] = on;
		else if( sBit == "quick" )		m_bTransforms[TRANSFORM_QUICK] = on;
		else if( sBit == "skippy" )		m_bTransforms[TRANSFORM_SKIPPY] = on;
		else if( sBit == "mines" )		m_bTransforms[TRANSFORM_MINES] = on;
		else if( sBit == "echo" )		m_bTransforms[TRANSFORM_ECHO] = on;
		else if( sBit == "planted" )	m_bTransforms[TRANSFORM_PLANTED] = on;
		else if( sBit == "stomp" )		m_bTransforms[TRANSFORM_STOMP] = on;
		else if( sBit == "twister" )	m_bTransforms[TRANSFORM_TWISTER] = on;
		else if( sBit == "nojumps" )	m_bTransforms[TRANSFORM_NOJUMPS] = on;
		else if( sBit == "nohands" )	m_bTransforms[TRANSFORM_NOHANDS] = on;
		else if( sBit == "reverse" )	SET_FLOAT( fScrolls[SCROLL_REVERSE] )
		else if( sBit == "split" )		SET_FLOAT( fScrolls[SCROLL_SPLIT] )
		else if( sBit == "alternate" )	SET_FLOAT( fScrolls[SCROLL_ALTERNATE] )
		else if( sBit == "noholds" || sBit == "nofreeze" )	m_bTransforms[TRANSFORM_NOHOLDS] = on;
		else if( sBit == "nomines" )	m_bTransforms[TRANSFORM_NOMINES] = on;
		else if( sBit == "dark" )		SET_FLOAT( fDark )
		else if( sBit == "blind" )		SET_FLOAT( fBlind )
		else if( sBit == "passmark" )	SET_FLOAT( fPassmark )
		else if( sBit == "timingassist")m_bTimingAssist = on;
		else if( sBit == "protiming")	m_bProTiming = on;
		else if( sBit == "overhead" )	{ m_fSkew = 0; m_fPerspectiveTilt = 0;				m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "incoming" )	{ m_fSkew = level; m_fPerspectiveTilt = -level;		m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "space" )		{ m_fSkew = level; m_fPerspectiveTilt = +level;		m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "hallway" )	{ m_fSkew = 0; m_fPerspectiveTilt = -level;			m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "distant" )	{ m_fSkew = 0; m_fPerspectiveTilt = +level;			m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( GAMESTATE->m_pPosition->IsValidModeForAnyStyle(sBit) )
			m_sPositioning = sBit;
		else if( sBit == "nopositioning" )
			m_sPositioning = "";
		else if( NOTESKIN->DoesNoteSkinExist(sBit) )
			m_sNoteSkin = sBit;
		else if( sBit == "noteskin" && !on ) /* "no noteskin" */
			m_sNoteSkin = "default";
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
	// It's dumb to use a code to change transforms because there are so many of them. -Chris
	//m_Transforms = (Transform) ((m_Transform+1)%NUM_TRANSFORMS);
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

bool PlayerOptions::operator==( const PlayerOptions &other )
{
#define COMPARE(x) { if( x != other.x ) return false; }
	COMPARE(m_bTimeSpacing);
	COMPARE(m_fScrollSpeed);
	COMPARE(m_fScrollBPM);
	COMPARE(m_fDark);
	COMPARE(m_fBlind);
	COMPARE(m_Turn);
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
	for( i = 0; i < PlayerOptions::NUM_TRANSFORMS; ++i )
		COMPARE(m_bTransforms[i]);
#undef COMPARE
	return true;
}
