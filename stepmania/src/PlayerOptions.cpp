#include "global.h"
#include "PlayerOptions.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "song.h"
#include "Course.h"
#include "Steps.h"
#include "ThemeManager.h"
#include "Foreach.h"
#include "Style.h"
#include "CommonMetrics.h"
#include "arch/Dialog/Dialog.h"

#define ONE( arr ) { for( unsigned Z = 0; Z < ARRAYSIZE(arr); ++Z ) arr[Z]=1.0f; }

void PlayerOptions::Init()
{
	m_bSetScrollSpeed = false;
	m_fTimeSpacing = 0;			m_SpeedfTimeSpacing = 1.0f;
	m_fScrollSpeed = 1.0f;		m_SpeedfScrollSpeed = 1.0f;
	m_fScrollBPM = 200;			m_SpeedfScrollBPM = 1.0f;
	ZERO( m_fAccels );			ONE( m_SpeedfAccels );
	ZERO( m_fEffects );			ONE( m_SpeedfEffects );
	ZERO( m_fAppearances );		ONE( m_SpeedfAppearances );
	ZERO( m_fScrolls );			ONE( m_SpeedfScrolls );
	m_fDark = 0;				m_SpeedfDark = 1.0f;
	m_fBlind = 0;				m_SpeedfBlind = 1.0f;
	m_fCover = 0;				m_SpeedfCover = 1.0f;
	m_bSetTiltOrSkew = false;
	m_fPerspectiveTilt = 0;		m_SpeedfPerspectiveTilt = 1.0f;
	m_fSkew = 0;				m_SpeedfSkew = 1.0f;
	m_fPassmark = 0;			m_SpeedfPassmark = 1.0f;
	m_fRandomSpeed = 0;			m_SpeedfRandomSpeed = 1.0f;
	ZERO( m_bTurns );
	ZERO( m_bTransforms );
	m_ScoreDisplay = SCORING_ADD;
	m_sNoteSkin = "default";
}

void PlayerOptions::Approach( const PlayerOptions& other, float fDeltaSeconds )
{
#define APP( opt ) \
	fapproach( m_ ## opt, other.m_ ## opt, fDeltaSeconds * other.m_Speed ## opt );

	APP( fTimeSpacing );
	APP( fScrollSpeed );
	fapproach( m_fScrollBPM, other.m_fScrollBPM, fDeltaSeconds * other.m_SpeedfScrollBPM*150 );
	for( int i=0; i<NUM_ACCELS; i++ )
		APP( fAccels[i] );
	for( int i=0; i<NUM_EFFECTS; i++ )
		APP( fEffects[i] );
	for( int i=0; i<NUM_APPEARANCES; i++ )
		APP( fAppearances[i] );
	for( int i=0; i<NUM_SCROLLS; i++ )
		APP( fScrolls[i] );
	APP( fDark );
	APP( fBlind );
	APP( fCover );
	APP( fPerspectiveTilt );
	APP( fSkew );
	APP( fPassmark );
	APP( fRandomSpeed );
}

static void AddPart( vector<CString> &AddTo, float level, CString name )
{
	if( level == 0 )
		return;

	const CString LevelStr = (level == 1)? CString(""): ssprintf( "%i%% ", (int) roundf(level*100) );

	AddTo.push_back( LevelStr + name );
}

CString PlayerOptions::GetString() const
{
	vector<CString> v;
	GetMods( v );
	return join( ", ", v );
}

void PlayerOptions::GetMods( vector<CString> &AddTo ) const
{
	CString sReturn;

	if( !m_fTimeSpacing )
	{
		if( m_bSetScrollSpeed || m_fScrollSpeed != 1 )
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
			AddTo.push_back( s + "x" );
		}
	}
	else
	{
		CString s = ssprintf( "C%.0f", m_fScrollBPM );
		AddTo.push_back( s );
	}

	AddPart( AddTo, m_fAccels[ACCEL_BOOST],		"Boost" );
	AddPart( AddTo, m_fAccels[ACCEL_BRAKE],		"Brake" );
	AddPart( AddTo, m_fAccels[ACCEL_WAVE],		"Wave" );
	AddPart( AddTo, m_fAccels[ACCEL_EXPAND],	"Expand" );
	AddPart( AddTo, m_fAccels[ACCEL_BOOMERANG],	"Boomerang" );

	AddPart( AddTo, m_fEffects[EFFECT_DRUNK],	"Drunk" );
	AddPart( AddTo, m_fEffects[EFFECT_DIZZY],	"Dizzy" );
	AddPart( AddTo, m_fEffects[EFFECT_MINI],	"Mini" );
	AddPart( AddTo, m_fEffects[EFFECT_FLIP],	"Flip" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO],	"Tornado" );
	AddPart( AddTo, m_fEffects[EFFECT_TIPSY],	"Tipsy" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY],	"Bumpy" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT],	"Beat" );

	AddPart( AddTo, m_fAppearances[APPEARANCE_HIDDEN],			"Hidden" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_HIDDEN_OFFSET],	"HiddenOffset" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_SUDDEN],			"Sudden" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_SUDDEN_OFFSET],	"SuddenOffset" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_STEALTH],			"Stealth" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_BLINK],			"Blink" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_RANDOMVANISH],	"RandomVanish" );

	AddPart( AddTo, m_fScrolls[SCROLL_REVERSE],		"Reverse" );
	AddPart( AddTo, m_fScrolls[SCROLL_SPLIT],		"Split" );
	AddPart( AddTo, m_fScrolls[SCROLL_ALTERNATE],	"Alternate" );
	AddPart( AddTo, m_fScrolls[SCROLL_CROSS],		"Cross" );
	AddPart( AddTo, m_fScrolls[SCROLL_CENTERED],	"Centered" );

	AddPart( AddTo, m_fDark, "Dark" );

	AddPart( AddTo, m_fBlind,	"Blind" );
	AddPart( AddTo, m_fCover,	"Cover" );

	AddPart( AddTo, m_fPassmark, "Passmark" );

	AddPart( AddTo, m_fRandomSpeed, "RandomSpeed" );

	if( m_bTurns[TURN_MIRROR] )			AddTo.push_back( "Mirror" );
	if( m_bTurns[TURN_LEFT] )			AddTo.push_back( "Left" );
	if( m_bTurns[TURN_RIGHT] )			AddTo.push_back( "Right" );
	if( m_bTurns[TURN_SHUFFLE] )		AddTo.push_back( "Shuffle" );
	if( m_bTurns[TURN_SUPER_SHUFFLE] )	AddTo.push_back( "SuperShuffle" );

	if( m_bTransforms[TRANSFORM_NOHOLDS] )	AddTo.push_back( "NoHolds" );
	if( m_bTransforms[TRANSFORM_NOROLLS] )	AddTo.push_back( "NoRolls" );
	if( m_bTransforms[TRANSFORM_NOMINES] )	AddTo.push_back( "NoMines" );
	if( m_bTransforms[TRANSFORM_LITTLE] )	AddTo.push_back( "Little" );
	if( m_bTransforms[TRANSFORM_WIDE] )		AddTo.push_back( "Wide" );
	if( m_bTransforms[TRANSFORM_BIG] )		AddTo.push_back( "Big" );
	if( m_bTransforms[TRANSFORM_QUICK] )	AddTo.push_back( "Quick" );
	if( m_bTransforms[TRANSFORM_BMRIZE] )	AddTo.push_back( "BMRize" );
	if( m_bTransforms[TRANSFORM_SKIPPY] )	AddTo.push_back( "Skippy" );
	if( m_bTransforms[TRANSFORM_MINES] )	AddTo.push_back( "Mines" );
	if( m_bTransforms[TRANSFORM_ECHO] )		AddTo.push_back( "Echo" );
	if( m_bTransforms[TRANSFORM_STOMP] )	AddTo.push_back( "Stomp" );
	if( m_bTransforms[TRANSFORM_PLANTED] )	AddTo.push_back( "Planted" );
	if( m_bTransforms[TRANSFORM_FLOORED] )	AddTo.push_back( "Floored" );
	if( m_bTransforms[TRANSFORM_TWISTER] )	AddTo.push_back( "Twister" );
	if( m_bTransforms[TRANSFORM_NOJUMPS] )	AddTo.push_back( "NoJumps" );
	if( m_bTransforms[TRANSFORM_NOHANDS] )	AddTo.push_back( "NoHands" );
	if( m_bTransforms[TRANSFORM_NOQUADS] )	AddTo.push_back( "NoQuads" );

	if( m_fSkew==0 && m_fPerspectiveTilt==0 )		{ if( m_bSetTiltOrSkew ) AddTo.push_back( "Overhead" ); }
	else if( m_fSkew==1 && m_fPerspectiveTilt==-1 )	AddTo.push_back( "Incoming" );
	else if( m_fSkew==1 && m_fPerspectiveTilt==+1 )	AddTo.push_back( "Space" );
	else if( m_fSkew==0 && m_fPerspectiveTilt==-1 )	AddTo.push_back( "Hallway" );
	else if( m_fSkew==0 && m_fPerspectiveTilt==+1 )	AddTo.push_back( "Distant" );

	if( !m_sNoteSkin.empty()  &&  m_sNoteSkin.CompareNoCase("default")!=0 )
	{
		CString s = m_sNoteSkin;
		Capitalize( s );
		AddTo.push_back( s );
	}
}

/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void PlayerOptions::FromString( CString sOptions, bool bWarnOnInvalid )
{
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
			m_fTimeSpacing = 0.0f;
			char *p = NULL;
			m_fScrollSpeed = strtof( matches[0], &p );
			ASSERT( p != matches[0] );
			continue;
		}

		else if( sscanf( sBit, "c%d", &i1 ) == 1 )
		{
			m_fTimeSpacing = 1.0f;
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

		FOREACH_CONST( CString, asParts, s )
		{
			if( *s == "no" )
			{
				level = 0;
			}
			else if( isdigit((*s)[0]) || (*s)[0] == '-' )
			{
				/* If the last character is a *, they probably said "123*" when
				 * they meant "*123". */
				if( s->Right(1) == "*" )
					RageException::Throw("Invalid player options '%i*'; did you mean '*%i'?", 
						atoi(*s), atoi(*s) );
				level = strtof( *s, NULL ) / 100.0f;
			}
			else if( *s[0]=='*' )
			{
				sscanf( *s, "*%f", &speed );
			}
		}
		sBit = asParts.back();

#define SET_FLOAT( opt ) { m_ ## opt = level; m_Speed ## opt = speed; }
		const bool on = (level > 0.5f);
			 if( sBit == "clearall" )	Init();
		else if( sBit == "boost" )		SET_FLOAT( fAccels[ACCEL_BOOST] )
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
		else if( sBit == "hiddenoffset" )	SET_FLOAT( fAppearances[APPEARANCE_HIDDEN_OFFSET] )
		else if( sBit == "sudden" )		SET_FLOAT( fAppearances[APPEARANCE_SUDDEN] )
		else if( sBit == "suddenoffset" )	SET_FLOAT( fAppearances[APPEARANCE_SUDDEN_OFFSET] )
		else if( sBit == "stealth" )	SET_FLOAT( fAppearances[APPEARANCE_STEALTH] )
		else if( sBit == "blink" )		SET_FLOAT( fAppearances[APPEARANCE_BLINK] )
		else if( sBit == "randomvanish" ) SET_FLOAT( fAppearances[APPEARANCE_RANDOMVANISH] )
		else if( sBit == "turn" && !on ) ZERO( m_bTurns ); /* "no turn" */
		else if( sBit == "mirror" )		m_bTurns[TURN_MIRROR] = on;
		else if( sBit == "left" )		m_bTurns[TURN_LEFT] = on;
		else if( sBit == "right" )		m_bTurns[TURN_RIGHT] = on;
		else if( sBit == "shuffle" )	m_bTurns[TURN_SHUFFLE] = on;
		else if( sBit == "supershuffle" )m_bTurns[TURN_SUPER_SHUFFLE] = on;
		else if( sBit == "little" )		m_bTransforms[TRANSFORM_LITTLE] = on;
		else if( sBit == "wide" )		m_bTransforms[TRANSFORM_WIDE] = on;
		else if( sBit == "big" )		m_bTransforms[TRANSFORM_BIG] = on;
		else if( sBit == "quick" )		m_bTransforms[TRANSFORM_QUICK] = on;
		else if( sBit == "bmrize" )		m_bTransforms[TRANSFORM_BMRIZE] = on;
		else if( sBit == "skippy" )		m_bTransforms[TRANSFORM_SKIPPY] = on;
		else if( sBit == "mines" )		m_bTransforms[TRANSFORM_MINES] = on;
		else if( sBit == "echo" )		m_bTransforms[TRANSFORM_ECHO] = on;
		else if( sBit == "stomp" )		m_bTransforms[TRANSFORM_STOMP] = on;
		else if( sBit == "planted" )	m_bTransforms[TRANSFORM_PLANTED] = on;
		else if( sBit == "floored" )	m_bTransforms[TRANSFORM_FLOORED] = on;
		else if( sBit == "twister" )	m_bTransforms[TRANSFORM_TWISTER] = on;
		else if( sBit == "nojumps" )	m_bTransforms[TRANSFORM_NOJUMPS] = on;
		else if( sBit == "nohands" )	m_bTransforms[TRANSFORM_NOHANDS] = on;
		else if( sBit == "noquads" )	m_bTransforms[TRANSFORM_NOQUADS] = on;
		else if( sBit == "reverse" )	SET_FLOAT( fScrolls[SCROLL_REVERSE] )
		else if( sBit == "split" )		SET_FLOAT( fScrolls[SCROLL_SPLIT] )
		else if( sBit == "alternate" )	SET_FLOAT( fScrolls[SCROLL_ALTERNATE] )
		else if( sBit == "cross" )		SET_FLOAT( fScrolls[SCROLL_CROSS] )
		else if( sBit == "centered" || sBit == "converge" )	SET_FLOAT( fScrolls[SCROLL_CENTERED] )
		else if( sBit == "noholds" || sBit == "nofreeze" )	m_bTransforms[TRANSFORM_NOHOLDS] = on;
		else if( sBit == "norolls" )	m_bTransforms[TRANSFORM_NOROLLS] = on;
		else if( sBit == "nomines" )	m_bTransforms[TRANSFORM_NOMINES] = on;
		else if( sBit == "dark" )		SET_FLOAT( fDark )
		else if( sBit == "blind" )		SET_FLOAT( fBlind )
		else if( sBit == "cover" )		SET_FLOAT( fCover )
		else if( sBit == "passmark" )	SET_FLOAT( fPassmark )
		else if( sBit == "overhead" )	{ m_bSetTiltOrSkew = true; m_fSkew = 0;		m_fPerspectiveTilt = 0;			m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "incoming" )	{ m_bSetTiltOrSkew = true; m_fSkew = level; m_fPerspectiveTilt = -level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "space" )		{ m_bSetTiltOrSkew = true; m_fSkew = level; m_fPerspectiveTilt = +level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "hallway" )	{ m_bSetTiltOrSkew = true; m_fSkew = 0;		m_fPerspectiveTilt = -level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( sBit == "distant" )	{ m_bSetTiltOrSkew = true; m_fSkew = 0;		m_fPerspectiveTilt = +level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
		else if( NOTESKIN && NOTESKIN->DoesNoteSkinExist(sBit) )	m_sNoteSkin = sBit;
		else if( sBit == "noteskin" && !on ) /* "no noteskin" */	m_sNoteSkin = "default";
		else if( sBit == "randomspeed" ) 		SET_FLOAT( fRandomSpeed )
		else if( sBit == "addscore" )			m_ScoreDisplay = SCORING_ADD;
		else if( sBit == "subtractscore" )		m_ScoreDisplay = SCORING_SUBTRACT;
		else if( sBit == "averagescore" )		m_ScoreDisplay = SCORING_AVERAGE;
		else if( sBit == "random" )				ChooseRandomMofifiers();
		else
		{
			if( bWarnOnInvalid )
			{
				CString sWarning = ssprintf( "The options string '%s' contains an invalid mod name '%s'", sOptions.c_str(), sBit.c_str() );
				LOG->Warn( sWarning );
				Dialog::OK( sWarning, "INVALID_PLAYER_OPTION_WANRING" );
			}
		}
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

	for( int i=0; i<size; i++ )
		fValues[i] = 0;

	if( index == size-1 )	// if true, then the last float in the list was selected
		;	// leave all off
	else
		fValues[index+1] = 1;
}

void NextBool( bool bValues[], int size )
{
	int index = -1;
	for( int i=0; i<size; i++ )
	{
		if( bValues[i] )
		{
			index = i;
			break;
		}
	}

	for( int i=0; i<size; i++ )
		bValues[i] = false;

	if( index == size-1 )	// if true, then the last float in the list was selected
		;	// leave all off
	else
		bValues[index+1] = 1;
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
	NextBool( m_bTurns, NUM_TURNS );
}

void PlayerOptions::NextTransform()
{
	NextBool( m_bTransforms, NUM_TRANSFORMS );
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

void PlayerOptions::ToggleOneTurn( Turn t )
{
	bool bWasOn = m_bTurns[t];
	ZERO( m_bTurns );
	m_bTurns[t] = !bWasOn;
}

float PlayerOptions::GetReversePercentForColumn( int iCol ) const
{
	float f = 0;
	int iNumCols = GAMESTATE->GetCurrentStyle()->m_iColsPerPlayer;

	f += m_fScrolls[SCROLL_REVERSE];
	
	if( iCol >= iNumCols/2 )
		f += m_fScrolls[SCROLL_SPLIT];

	if( (iCol%2)==1 )
		f += m_fScrolls[SCROLL_ALTERNATE];
	
	int iFirstCrossCol = iNumCols/4;
	int iLastCrossCol = iNumCols-1-iFirstCrossCol;
	if( iCol>=iFirstCrossCol && iCol<=iLastCrossCol )
		f += m_fScrolls[SCROLL_CROSS];
	
	if( f > 2 )
		f = fmodf( f, 2 );
	if( f > 1 )
		f = SCALE( f, 1.f, 2.f, 1.f, 0.f );
	return f;
}

bool PlayerOptions::operator==( const PlayerOptions &other ) const
{
#define COMPARE(x) { if( x != other.x ) return false; }
	COMPARE(m_fTimeSpacing);
	COMPARE(m_fScrollSpeed);
	COMPARE(m_fScrollBPM);
	COMPARE(m_fRandomSpeed);
	COMPARE(m_ScoreDisplay);
	COMPARE(m_fDark);
	COMPARE(m_fBlind);
	COMPARE(m_fCover);
	COMPARE(m_fPerspectiveTilt);
	COMPARE(m_fSkew);
	COMPARE(m_sNoteSkin);
	for( int i = 0; i < PlayerOptions::NUM_ACCELS; ++i )
		COMPARE(m_fAccels[i]);
	for( int i = 0; i < PlayerOptions::NUM_EFFECTS; ++i )
		COMPARE(m_fEffects[i]);
	for( int i = 0; i < PlayerOptions::NUM_APPEARANCES; ++i )
		COMPARE(m_fAppearances[i]);
	for( int i = 0; i < PlayerOptions::NUM_SCROLLS; ++i )
		COMPARE(m_fScrolls[i]);
	for( int i = 0; i < PlayerOptions::NUM_TURNS; ++i )
		COMPARE(m_bTurns[i]);
	for( int i = 0; i < PlayerOptions::NUM_TRANSFORMS; ++i )
		COMPARE(m_bTransforms[i]);
#undef COMPARE
	return true;
}

bool PlayerOptions::IsEasierForSongAndSteps( Song* pSong, Steps* pSteps )
{
	if( m_fTimeSpacing && pSong->HasSignificantBpmChangesOrStops() )
		return true;
	if( m_bTransforms[TRANSFORM_NOHOLDS] && pSteps->GetRadarValues()[RADAR_NUM_HOLDS]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOROLLS] && pSteps->GetRadarValues()[RADAR_NUM_ROLLS]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOMINES] && pSteps->GetRadarValues()[RADAR_NUM_MINES]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOHANDS] && pSteps->GetRadarValues()[RADAR_NUM_HANDS]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOQUADS] && pSteps->GetRadarValues()[RADAR_NUM_HANDS]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOJUMPS] && pSteps->GetRadarValues()[RADAR_NUM_JUMPS]>0 )
		return true;

	// Inserted holds can be really easy on some songs, and scores will be 
	// highly hold-weighted, and very little tap score weighted.
	if( m_bTransforms[TRANSFORM_PLANTED] )	return true;
	if( m_bTransforms[TRANSFORM_FLOORED] )	return true;
	if( m_bTransforms[TRANSFORM_TWISTER] )	return true;

	// This makes songs with sparse notes easier.
	if( m_bTransforms[TRANSFORM_ECHO] )	return true;
	
	if( m_fCover )	return true;
	return false;
}

bool PlayerOptions::IsEasierForCourseAndTrail( Course* pCourse, Trail* pTrail )
{
	ASSERT( pCourse );
	ASSERT( pTrail );

	FOREACH_CONST( TrailEntry, pTrail->m_vEntries, e )
	{
		if( e->pSong && IsEasierForSongAndSteps(e->pSong, e->pSteps) )
			return true;
	}
	return false;
}

void PlayerOptions::GetThemedMods( vector<CString> &AddTo ) const
{
	vector<CString> vMods;
	GetMods( vMods );
	FOREACH_CONST( CString, vMods, s )
	{
		const CString& sOneMod = *s;

		ASSERT( !sOneMod.empty() );

		CStringArray asTokens;
		split( sOneMod, " ", asTokens );

		if( asTokens.empty() )
			continue;

		// Strip the approach speed token, if any
		if( asTokens[0][0] == '*' )
			asTokens.erase( asTokens.begin() );

		// capitalize NoteSkin names
		asTokens.back() = Capitalize( asTokens.back() );

		/* Theme the mod name (the last string).  Allow this to not exist, since
		 * characters might use modifiers that don't exist in the theme. */
		asTokens.back() = THEME_OPTION_ITEM( asTokens.back(), true );

		CString sThemedMod = join( " ", asTokens );

		AddTo.push_back( sThemedMod );
	}
}

bool PlayerOptions::ContainsTransformOrTurn() const
{
	for( int i=0; i<NUM_TRANSFORMS; i++ )
	{
		if( m_bTransforms[i] )
			return true;
	}
	for( int i=0; i<NUM_TURNS; i++ )
	{
		if( m_bTurns[i] )
			return true;
	}
	return false;
}

CString PlayerOptions::GetSavedPrefsString() const
{
	PlayerOptions po_prefs;
#define SAVE(x) po_prefs.x = this->x;
	SAVE( m_fTimeSpacing );
	SAVE( m_fScrollSpeed );
	SAVE( m_fScrollBPM );
	SAVE( m_fScrolls[SCROLL_REVERSE] );
	SAVE( m_fPerspectiveTilt );
	SAVE( m_bTransforms[TRANSFORM_NOHOLDS] );
	SAVE( m_bTransforms[TRANSFORM_NOROLLS] );
	SAVE( m_bTransforms[TRANSFORM_NOMINES] );
	SAVE( m_bTransforms[TRANSFORM_NOJUMPS] );
	SAVE( m_bTransforms[TRANSFORM_NOHANDS] );
	SAVE( m_bTransforms[TRANSFORM_NOQUADS] );
	SAVE( m_ScoreDisplay );
	SAVE( m_sNoteSkin );
#undef SAVE
	return po_prefs.GetString();
}

void PlayerOptions::ResetSavedPrefs()
{
	PlayerOptions defaults;
#define CPY(x) this->x = defaults.x;
	CPY( m_fTimeSpacing );
	CPY( m_fScrollSpeed );
	CPY( m_fScrollBPM );
	CPY( m_fScrolls[SCROLL_REVERSE] );
	CPY( m_fPerspectiveTilt );
	CPY( m_bTransforms[TRANSFORM_NOHOLDS] );
	CPY( m_bTransforms[TRANSFORM_NOROLLS] );
	CPY( m_bTransforms[TRANSFORM_NOMINES] );
	CPY( m_bTransforms[TRANSFORM_NOJUMPS] );
	CPY( m_bTransforms[TRANSFORM_NOHANDS] );
	CPY( m_bTransforms[TRANSFORM_NOQUADS] );
	CPY( m_sNoteSkin );
#undef CPY
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
