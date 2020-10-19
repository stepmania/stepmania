#include "global.h"
#include "PlayerOptions.h"
#include "RageUtil.h"
#include "GameState.h"
#include "NoteSkinManager.h"
#include "Song.h"
#include "Course.h"
#include "Steps.h"
#include "ThemeManager.h"
#include "Style.h"
#include "CommonMetrics.h"
#include <float.h>

static const char *LifeTypeNames[] = {
	"Bar",
	"Battery",
	"Time",
};
XToString( LifeType );
XToLocalizedString( LifeType );
LuaXType( LifeType );

static const char *DrainTypeNames[] = {
	"Normal",
	"NoRecover",
	"SuddenDeath",
};
XToString( DrainType );
XToLocalizedString( DrainType );
LuaXType( DrainType );

static const char *ModTimerTypeNames[] = {
	"Game",
	"Beat",
	"Song",
	"Default",
};
XToString( ModTimerType );
XToLocalizedString( ModTimerType );
LuaXType( ModTimerType );

void NextFloat( float fValues[], int size );
void NextBool( bool bValues[], int size );

ThemeMetric<float> RANDOM_SPEED_CHANCE		( "PlayerOptions", "RandomSpeedChance" );
ThemeMetric<float> RANDOM_REVERSE_CHANCE	( "PlayerOptions", "RandomReverseChance" );
ThemeMetric<float> RANDOM_DARK_CHANCE		( "PlayerOptions", "RandomDarkChance" );
ThemeMetric<float> RANDOM_ACCEL_CHANCE		( "PlayerOptions", "RandomAccelChance" );
ThemeMetric<float> RANDOM_EFFECT_CHANCE		( "PlayerOptions", "RandomEffectChance" );
ThemeMetric<float> RANDOM_HIDDEN_CHANCE		( "PlayerOptions", "RandomHiddenChance" );
ThemeMetric<float> RANDOM_SUDDEN_CHANCE		( "PlayerOptions", "RandomSuddenChance" );

static const float CMOD_DEFAULT= 200.0f;
// Is there a better place for this?
// It needs to be a named constant because it's used in several places in
// this file, but nothing else has a named constant for its default value.
// -Kyz

void PlayerOptions::Init()
{
	m_LifeType = LifeType_Bar;
	m_DrainType = DrainType_Normal;
	m_ModTimerType = ModTimerType_Default;
	m_BatteryLives = 4;
	m_MinTNSToHideNotes= PREFSMAN->m_MinTNSToHideNotes;
	m_bSetScrollSpeed = false;
	m_fMaxScrollBPM = 0;		m_SpeedfMaxScrollBPM = 1.0f;
	m_fTimeSpacing = 0;		m_SpeedfTimeSpacing = 1.0f;
	m_fScrollSpeed = 1.0f;		m_SpeedfScrollSpeed = 1.0f;
	m_fScrollBPM = CMOD_DEFAULT;		m_SpeedfScrollBPM = 1.0f;
	ZERO( m_fAccels );		ONE( m_SpeedfAccels );
	ZERO( m_fEffects );		ONE( m_SpeedfEffects );
	ZERO( m_fAppearances );		ONE( m_SpeedfAppearances );
	ZERO( m_fScrolls );		ONE( m_SpeedfScrolls );
	m_fDark = 0;			m_SpeedfDark = 1.0f;
	m_fBlind = 0;			m_SpeedfBlind = 1.0f;
	m_fCover = 0;			m_SpeedfCover = 1.0f;
	m_fRandAttack = 0;		m_SpeedfRandAttack = 1.0f;
	m_fNoAttack = 0;		m_SpeedfNoAttack = 1.0f;
	m_fPlayerAutoPlay = 0;		m_SpeedfPlayerAutoPlay = 1.0f;
	m_fPerspectiveTilt = 0;		m_SpeedfPerspectiveTilt = 1.0f;
	m_fSkew = 0;			m_SpeedfSkew = 1.0f;
	m_fPassmark = 0;		m_SpeedfPassmark = 1.0f;
	m_fRandomSpeed = 0;		m_SpeedfRandomSpeed = 1.0f;
	m_fModTimerMult = 0;		m_SpeedfModTimerMult = 1.0f;
	m_fModTimerOffset = 0;		m_SpeedfModTimerOffset = 1.0f;
	m_fDrawSize = 0;		m_SpeedfDrawSize = 1.0f;
	m_fDrawSizeBack = 0;		m_SpeedfDrawSizeBack = 1.0f;
	ZERO( m_bTurns );
	ZERO( m_bTransforms );
	m_bMuteOnError = false;
	m_bStealthType = false;
	m_bStealthPastReceptors = false;
	m_bDizzyHolds = false;
	m_bZBuffer = false;
	m_bCosecant = false;
	m_sNoteSkin = "";
	ZERO( m_fMovesX );		ONE( m_SpeedfMovesX );
	ZERO( m_fMovesY );		ONE( m_SpeedfMovesY );
	ZERO( m_fMovesZ );		ONE( m_SpeedfMovesZ );
	ZERO( m_fConfusionX );		ONE( m_SpeedfConfusionX );
	ZERO( m_fConfusionY );		ONE( m_SpeedfConfusionY );
	ZERO( m_fConfusionZ );		ONE( m_SpeedfConfusionZ );
	ZERO( m_fDarks );		ONE( m_SpeedfDarks );
	ZERO( m_fStealth );		ONE( m_SpeedfStealth );
	ZERO( m_fTiny );		ONE( m_SpeedfTiny );
	ZERO( m_fBumpy );		ONE( m_SpeedfBumpy );
	ZERO( m_fReverse );		ONE( m_SpeedfReverse );

}

void PlayerOptions::Approach( const PlayerOptions& other, float fDeltaSeconds )
{
	const float fRateMult = PREFSMAN->m_bRateModsAffectTweens ? GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate : 1.0f;

#define APPROACH( opt ) \
	fapproach( m_ ## opt, other.m_ ## opt, fRateMult * fDeltaSeconds * other.m_Speed ## opt );
#define DO_COPY( x ) \
	x = other.x;

	DO_COPY( m_LifeType );
	DO_COPY( m_DrainType );
	DO_COPY( m_ModTimerType );
	DO_COPY( m_BatteryLives );
	APPROACH( fModTimerMult );
	APPROACH( fModTimerOffset );
	APPROACH( fDrawSize );
	APPROACH( fDrawSizeBack );
	APPROACH( fTimeSpacing );
	APPROACH( fScrollSpeed );
	APPROACH( fMaxScrollBPM );
	fapproach( m_fScrollBPM, other.m_fScrollBPM, fDeltaSeconds * other.m_SpeedfScrollBPM*150 );
	for( int i=0; i<NUM_ACCELS; i++ )
		APPROACH( fAccels[i] );
	for( int i=0; i<NUM_EFFECTS; i++ )
		APPROACH( fEffects[i] );
	for( int i=0; i<NUM_APPEARANCES; i++ )
		APPROACH( fAppearances[i] );
	for( int i=0; i<NUM_SCROLLS; i++ )
		APPROACH( fScrolls[i] );
	APPROACH( fDark );
	APPROACH( fBlind );
	APPROACH( fCover );
	APPROACH( fRandAttack );
	APPROACH( fNoAttack );
	APPROACH( fPlayerAutoPlay );
	APPROACH( fPerspectiveTilt );
	APPROACH( fSkew );
	APPROACH( fPassmark );
	APPROACH( fRandomSpeed );
	for( int i=0; i<16; i++)
	    APPROACH( fMovesX[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fMovesY[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fMovesZ[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fConfusionX[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fConfusionY[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fConfusionZ[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fDarks[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fStealth[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fTiny[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fBumpy[i] );
	for( int i=0; i<16; i++)
	    APPROACH( fReverse[i] );

	DO_COPY( m_bSetScrollSpeed );
	for( int i=0; i<NUM_TURNS; i++ )
		DO_COPY( m_bTurns[i] );
	for( int i=0; i<NUM_TRANSFORMS; i++ )
		DO_COPY( m_bTransforms[i] );
	DO_COPY( m_bMuteOnError );
	DO_COPY( m_bStealthType );
	DO_COPY( m_bStealthPastReceptors );
	DO_COPY( m_bDizzyHolds );
	DO_COPY( m_bZBuffer );
	DO_COPY( m_bCosecant );
	DO_COPY( m_FailType );
	DO_COPY( m_MinTNSToHideNotes );
	DO_COPY( m_sNoteSkin );
#undef APPROACH
#undef DO_COPY
}

static void AddPart( vector<RString> &AddTo, float level, RString name )
{
	if( level == 0 )
		return;

	const RString LevelStr = (level == 1)? RString(""): ssprintf( "%ld%% ", lrintf(level*100) );

	AddTo.push_back( LevelStr + name );
}

RString PlayerOptions::GetString( bool bForceNoteSkin ) const
{
	vector<RString> v;
	GetMods( v, bForceNoteSkin );
	return join( ", ", v );
}

void PlayerOptions::GetMods( vector<RString> &AddTo, bool bForceNoteSkin ) const
{
	//RString sReturn;

	switch(m_LifeType)
	{
		case LifeType_Bar:
			switch(m_DrainType)
			{
				case DrainType_NoRecover:
					AddTo.push_back("NoRecover");
					break;
				case DrainType_SuddenDeath:
					AddTo.push_back("SuddenDeath");
					break;
				case DrainType_Normal:
				default:
					break;
			}
			break;
		case LifeType_Battery:
			AddTo.push_back(ssprintf("%dLives", m_BatteryLives));
			break;
		case LifeType_Time:
			AddTo.push_back("LifeTime");
			break;
		default:
			FAIL_M(ssprintf("Invalid LifeType: %i", m_LifeType));
	}

	if( !m_fTimeSpacing )
	{
		if( m_fMaxScrollBPM )
		{
			RString s = ssprintf( "m%.0f", m_fMaxScrollBPM );
			AddTo.push_back( s );
		}
		else if( m_bSetScrollSpeed || m_fScrollSpeed != 1 )
		{
			/* -> 1.00 */
			RString s = ssprintf( "%2.2f", m_fScrollSpeed );
			if( s[s.size()-1] == '0' )
			{
				/* -> 1.0 */
				s.erase( s.size()-1 );	// delete last char
				if( s[s.size()-1] == '0' )
				{
					/* -> 1 */
					s.erase( s.size()-2 );	// delete last 2 chars
				}
			}
			AddTo.push_back( s + "x" );
		}
	}
	else
	{
		RString s = ssprintf( "C%.0f", m_fScrollBPM );
		AddTo.push_back( s );
	}

	switch(m_ModTimerType)
	{
		case ModTimerType_Game:
			AddTo.push_back("ModTimerGame");
			break;
		case ModTimerType_Beat:
			AddTo.push_back("ModTimerBeat");
			break;
		case ModTimerType_Song:
			AddTo.push_back("ModTimerSong");
			break;
		case ModTimerType_Default:
			break;
		default:
			FAIL_M(ssprintf("Invalid ModTimerType: %i", m_ModTimerType));
	}

	AddPart( AddTo, m_fAccels[ACCEL_BOOST],		"Boost" );
	AddPart( AddTo, m_fAccels[ACCEL_BRAKE],		"Brake" );
	AddPart( AddTo, m_fAccels[ACCEL_WAVE],			"Wave" );
	AddPart( AddTo, m_fAccels[ACCEL_WAVE_PERIOD],		"WavePeriod" );
	AddPart( AddTo, m_fAccels[ACCEL_EXPAND],		"Expand" );
	AddPart( AddTo, m_fAccels[ACCEL_EXPAND_PERIOD],		"ExpandPeriod" );
	AddPart( AddTo, m_fAccels[ACCEL_TAN_EXPAND],		"TanExpand" );
	AddPart( AddTo, m_fAccels[ACCEL_TAN_EXPAND_PERIOD],	"TanExpandPeriod" );
	AddPart( AddTo, m_fAccels[ACCEL_BOOMERANG],	"Boomerang" );

	AddPart( AddTo, m_fEffects[EFFECT_DRUNK],		"Drunk" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_SPEED],		"DrunkSpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_OFFSET],	"DrunkOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_PERIOD],	"DrunkPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK],		"TanDrunk" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_SPEED],	"TanDrunkSpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_OFFSET],	"TanDrunkOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_PERIOD],	"TanDrunkPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_Z],		"DrunkZ" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_Z_SPEED],	"DrunkZSpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_Z_OFFSET],	"DrunkZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_DRUNK_Z_PERIOD],	"DrunkZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_Z],		"TanDrunkZ" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_Z_SPEED],	"TanDrunkZSpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_Z_OFFSET],	"TanDrunkZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DRUNK_Z_PERIOD],	"TanDrunkZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_SHRINK_TO_LINEAR],	"ShrinkLinear" );
	AddPart( AddTo, m_fEffects[EFFECT_SHRINK_TO_MULT],	"ShrinkMult" );
	AddPart( AddTo, m_fEffects[EFFECT_PULSE_INNER],		"PulseInner" );
	AddPart( AddTo, m_fEffects[EFFECT_PULSE_OUTER],		"PulseOuter" );
	AddPart( AddTo, m_fEffects[EFFECT_PULSE_PERIOD],	"PulsePeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_PULSE_OFFSET],	"PulseOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_ATTENUATE_X],		"AttenuateX" );
	AddPart( AddTo, m_fEffects[EFFECT_ATTENUATE_Y],		"AttenuateY" );
	AddPart( AddTo, m_fEffects[EFFECT_ATTENUATE_Z],		"AttenuateZ" );
	AddPart( AddTo, m_fEffects[EFFECT_DIZZY],		"Dizzy" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION],	"Confusion" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION_OFFSET],	"ConfusionOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION_X],	"ConfusionX" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION_X_OFFSET],	"ConfusionXOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION_Y],	"ConfusionY" );
	AddPart( AddTo, m_fEffects[EFFECT_CONFUSION_Y_OFFSET],	"ConfusionYOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE],		"Bounce" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE_PERIOD],	"BouncePeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE_OFFSET],	"BounceOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE_Z],		"BounceZ" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE_Z_PERIOD],	"BounceZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BOUNCE_Z_OFFSET],	"BounceZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_MINI],		"Mini" );
	AddPart( AddTo, m_fEffects[EFFECT_TINY],		"Tiny" );
	AddPart( AddTo, m_fEffects[EFFECT_FLIP],		"Flip" );
	AddPart( AddTo, m_fEffects[EFFECT_INVERT],		"Invert" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO],	"Tornado" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO_PERIOD],	"TornadoPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO_OFFSET],	"TornadoOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO],	"TanTornado" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO_PERIOD],	"TanTornadoPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO_OFFSET],	"TanTornadoOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO_Z],	"TornadoZ" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO_Z_PERIOD],	"TornadoZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TORNADO_Z_OFFSET],	"TornadoZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO_Z],	"TanTornadoZ" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO_Z_PERIOD],"TanTornadoZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TORNADO_Z_OFFSET],"TanTornadoZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TIPSY],		"Tipsy" );
	AddPart( AddTo, m_fEffects[EFFECT_TIPSY_SPEED],		"TipsySpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_TIPSY_OFFSET],	"TipsyOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TIPSY],		"TanTipsy" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TIPSY_SPEED],	"TanTipsySpeed" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_TIPSY_OFFSET],	"TanTipsyOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY],		"Bumpy" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY_OFFSET],	"BumpyOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY_PERIOD],	"BumpyPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY],		"TanBumpy" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY_OFFSET],	"TanBumpyOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY_PERIOD],	"TanBumpyPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY_X],		"BumpyX" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY_X_OFFSET],	"BumpyXOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BUMPY_X_PERIOD],	"BumpyXPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY_X],		"TanBumpyX" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY_X_OFFSET],	"TanBumpyXOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_BUMPY_X_PERIOD],	"TanBumpyXPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT],		"Beat" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_OFFSET],		"BeatOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_PERIOD],		"BeatPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_MULT],		"BeatMult" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Y],		"BeatY" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Y_OFFSET],	"BeatYOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Y_PERIOD],	"BeatYPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Y_MULT],		"BeatYMult" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Z],		"BeatZ" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Z_OFFSET],	"BeatZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Z_PERIOD],	"BeatZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_BEAT_Z_MULT],		"BeatZMult" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG],		"Zigzag" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG_PERIOD],	"ZigzagPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG_OFFSET],	"ZigzagOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG_Z],		"ZigzagZ" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG_Z_PERIOD],	"ZigzagZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_ZIGZAG_Z_OFFSET],	"ZigzagZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_SAWTOOTH],		"Sawtooth" );
	AddPart( AddTo, m_fEffects[EFFECT_SAWTOOTH_PERIOD],	"SawtoothPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_SAWTOOTH_Z],		"SawtoothZ" );
	AddPart( AddTo, m_fEffects[EFFECT_SAWTOOTH_Z_PERIOD],	"SawtoothZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE],		"Square" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE_OFFSET],	"SquareOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE_PERIOD],	"SquarePeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE_Z],		"SquareZ" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE_Z_OFFSET],	"SquareZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_SQUARE_Z_PERIOD],	"SquareZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL],		"Digital" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_STEPS],	"DigitalSteps" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_PERIOD],	"DigitalPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_OFFSET],	"DigitalOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL],		"TanDigital" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_STEPS],	"TanDigitalSteps" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_PERIOD],	"TanDigitalPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_OFFSET],	"TanDigitalOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_Z],		"DigitalZ" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_Z_STEPS],	"DigitalZSteps" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_Z_PERIOD],	"DigitalZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_DIGITAL_Z_OFFSET],	"DigitalZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_Z],	"TanDigitalZ" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_Z_STEPS],	"TanDigitalZSteps" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_Z_PERIOD],"TanDigitalZPeriod" );
	AddPart( AddTo, m_fEffects[EFFECT_TAN_DIGITAL_Z_OFFSET],"TanDigitalZOffset" );
	AddPart( AddTo, m_fEffects[EFFECT_PARABOLA_X],		"ParabolaX" );
	AddPart( AddTo, m_fEffects[EFFECT_PARABOLA_Y],		"ParabolaY" );
	AddPart( AddTo, m_fEffects[EFFECT_PARABOLA_Z],		"ParabolaZ" );
	AddPart( AddTo, m_fEffects[EFFECT_XMODE],		"XMode" );
	AddPart( AddTo, m_fEffects[EFFECT_TWIRL],		"Twirl" );
	AddPart( AddTo, m_fEffects[EFFECT_ROLL],		"Roll" );
	AddPart( AddTo, m_bStealthType,				"StealthType" );
	AddPart( AddTo, m_bStealthPastReceptors,		"StealthPastReceptors");
	AddPart( AddTo, m_bDizzyHolds,				"DizzyHolds");
	AddPart( AddTo, m_bZBuffer,				"ZBuffer");
	AddPart( AddTo, m_bCosecant,				"Cosecant");

	for( int i=0; i<16; i++)
	{
		RString s = ssprintf( "MoveX%d", i+1 );

		AddPart( AddTo, m_fMovesX[i],				s );
		s = ssprintf( "MoveY%d", i+1 );
		AddPart( AddTo, m_fMovesY[i],				s );
		s = ssprintf( "MoveZ%d", i+1 );
		AddPart( AddTo, m_fMovesZ[i],				s );
		s = ssprintf( "ConfusionOffset%d", i+1);
		AddPart( AddTo, m_fConfusionX[i],				s );
		s = ssprintf( "ConfusionYOffset%d", i+1 );
		AddPart( AddTo, m_fConfusionY[i],				s );
		s = ssprintf( "ConfusionZOffset%d", i+1 );
		AddPart( AddTo, m_fConfusionZ[i],				s );
		s = ssprintf( "Dark%d", i+1 );
		AddPart( AddTo, m_fDarks[i],				s );
		s = ssprintf( "Stealth%d", i+1 );
		AddPart( AddTo, m_fStealth[i],				s );
		s = ssprintf( "Tiny%d", i+1 );
		AddPart( AddTo, m_fTiny[i],				s );
		s = ssprintf( "Bumpy%d", i+1 );
		AddPart( AddTo, m_fBumpy[i],				s );
		s = ssprintf( "Reverse%d", i+1 );
		AddPart( AddTo, m_fReverse[i],				s );
	}

	AddPart( AddTo, m_fAppearances[APPEARANCE_HIDDEN],			"Hidden" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_HIDDEN_OFFSET],	"HiddenOffset" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_SUDDEN],			"Sudden" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_SUDDEN_OFFSET],	"SuddenOffset" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_STEALTH],		"Stealth" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_BLINK],			"Blink" );
	AddPart( AddTo, m_fAppearances[APPEARANCE_RANDOMVANISH],	"RandomVanish" );

	AddPart( AddTo, m_fScrolls[SCROLL_REVERSE],	"Reverse" );
	AddPart( AddTo, m_fScrolls[SCROLL_SPLIT],		"Split" );
	AddPart( AddTo, m_fScrolls[SCROLL_ALTERNATE],	"Alternate" );
	AddPart( AddTo, m_fScrolls[SCROLL_CROSS],		"Cross" );
	AddPart( AddTo, m_fScrolls[SCROLL_CENTERED],	"Centered" );

	AddPart( AddTo, m_fModTimerMult,	"ModTimerMult" );
	AddPart( AddTo, m_fModTimerOffset,	"ModTimerOffset" );
	AddPart( AddTo, m_fDrawSize,		"DrawSize" );
	AddPart( AddTo, m_fDrawSizeBack,	"DrawSizeBack" );

	AddPart( AddTo, m_fDark,	"Dark" );

	AddPart( AddTo, m_fBlind,	"Blind" );
	AddPart( AddTo, m_fCover,	"Cover" );

	AddPart( AddTo, m_fRandAttack,		"RandomAttacks" );
	AddPart( AddTo, m_fNoAttack,		"NoAttacks" );
	AddPart( AddTo, m_fPlayerAutoPlay,	"PlayerAutoPlay" );

	AddPart( AddTo, m_fPassmark,	"Passmark" );

	AddPart( AddTo, m_fRandomSpeed,	"RandomSpeed" );

	if( m_bTurns[TURN_MIRROR] )		AddTo.push_back( "Mirror" );
	if( m_bTurns[TURN_BACKWARDS] )		AddTo.push_back( "Backwards" );
	if( m_bTurns[TURN_LEFT] )			AddTo.push_back( "Left" );
	if( m_bTurns[TURN_RIGHT] )			AddTo.push_back( "Right" );
	if( m_bTurns[TURN_SHUFFLE] )		AddTo.push_back( "Shuffle" );
	if( m_bTurns[TURN_SOFT_SHUFFLE] )	AddTo.push_back( "SoftShuffle" );
	if( m_bTurns[TURN_SUPER_SHUFFLE] )	AddTo.push_back( "SuperShuffle" );

	if( m_bTransforms[TRANSFORM_NOHOLDS] )	AddTo.push_back( "NoHolds" );
	if( m_bTransforms[TRANSFORM_NOROLLS] )	AddTo.push_back( "NoRolls" );
	if( m_bTransforms[TRANSFORM_NOMINES] )	AddTo.push_back( "NoMines" );
	if( m_bTransforms[TRANSFORM_LITTLE] )	AddTo.push_back( "Little" );
	if( m_bTransforms[TRANSFORM_WIDE] )	AddTo.push_back( "Wide" );
	if( m_bTransforms[TRANSFORM_BIG] )		AddTo.push_back( "Big" );
	if( m_bTransforms[TRANSFORM_QUICK] )	AddTo.push_back( "Quick" );
	if( m_bTransforms[TRANSFORM_BMRIZE] )	AddTo.push_back( "BMRize" );
	if( m_bTransforms[TRANSFORM_SKIPPY] )	AddTo.push_back( "Skippy" );
	if( m_bTransforms[TRANSFORM_MINES] )	AddTo.push_back( "Mines" );
	if( m_bTransforms[TRANSFORM_ATTACKMINES] ) AddTo.push_back( "AttackMines" );
	if( m_bTransforms[TRANSFORM_ECHO] )	AddTo.push_back( "Echo" );
	if( m_bTransforms[TRANSFORM_STOMP] )	AddTo.push_back( "Stomp" );
	if( m_bTransforms[TRANSFORM_PLANTED] )	AddTo.push_back( "Planted" );
	if( m_bTransforms[TRANSFORM_FLOORED] )	AddTo.push_back( "Floored" );
	if( m_bTransforms[TRANSFORM_TWISTER] )	AddTo.push_back( "Twister" );
	if( m_bTransforms[TRANSFORM_HOLDROLLS] ) AddTo.push_back( "HoldsToRolls" );
	if( m_bTransforms[TRANSFORM_NOJUMPS] )	AddTo.push_back( "NoJumps" );
	if( m_bTransforms[TRANSFORM_NOHANDS] )	AddTo.push_back( "NoHands" );
	if( m_bTransforms[TRANSFORM_NOLIFTS] ) AddTo.push_back( "NoLifts" );
	if( m_bTransforms[TRANSFORM_NOFAKES] ) AddTo.push_back( "NoFakes" );
	if( m_bTransforms[TRANSFORM_NOQUADS] )	AddTo.push_back( "NoQuads" );
	if( m_bTransforms[TRANSFORM_NOSTRETCH] )AddTo.push_back( "NoStretch" );
	if( m_bMuteOnError )			AddTo.push_back( "MuteOnError" );

	switch( m_FailType )
	{
	case FailType_Immediate:							break;
	case FailType_ImmediateContinue:		AddTo.push_back("FailImmediateContinue");	break;
	case FailType_EndOfSong:			AddTo.push_back("FailAtEnd");	break;
	case FailType_Off:				AddTo.push_back("FailOff");	break;
	default:
		FAIL_M(ssprintf("Invalid FailType: %i", m_FailType));
	}

	if( m_fSkew==0 && m_fPerspectiveTilt==0 )
	{
		AddTo.push_back( "Overhead" );
	}
	else if( m_fSkew == 0 )
	{
		if( m_fPerspectiveTilt > 0 )
			AddPart( AddTo, m_fPerspectiveTilt, "Distant" );
		else
			AddPart( AddTo, -m_fPerspectiveTilt, "Hallway" );
	}
	else if( fabsf(m_fSkew-m_fPerspectiveTilt) < 0.0001f )
	{
		AddPart( AddTo, m_fSkew, "Space" );
	}
	else if( fabsf(m_fSkew+m_fPerspectiveTilt) < 0.0001f )
	{
		AddPart( AddTo, m_fSkew, "Incoming" );
	}
	else
	{
		AddPart( AddTo, m_fSkew, "Skew" );
		AddPart( AddTo, m_fPerspectiveTilt, "Tilt" );
	}

	// Don't display a string if using the default NoteSkin unless we force it.
	if( bForceNoteSkin || (!m_sNoteSkin.empty() && m_sNoteSkin != CommonMetrics::DEFAULT_NOTESKIN_NAME.GetValue()) )
	{
		RString s = m_sNoteSkin;
		Capitalize( s );
		AddTo.push_back( s );
	}
}

/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void PlayerOptions::FromString( const RString &sMultipleMods )
{
	RString sTemp = sMultipleMods;
	vector<RString> vs;
	split( sTemp, ",", vs, true );
	RString sThrowAway;
	for (RString &s : vs)
	{
		if (!FromOneModString( s, sThrowAway ))
		{
			LOG->Trace( "Attempted to load a non-existing mod \'%s\' for the Player. Ignoring.", s.c_str() );
		}
	}
}

bool PlayerOptions::FromOneModString( const RString &sOneMod, RString &sErrorOut )
{
	ASSERT_M( NOTESKIN != nullptr, "The Noteskin Manager must be loaded in order to process mods." );

	RString sBit = sOneMod;
	RString sMod = "";
	sBit.MakeLower();
	Trim( sBit );

	/* "drunk"
	 * "no drunk"
	 * "150% drunk"
	 * "*2 100% drunk": approach at 2x normal speed */

	float level = 1;
	float speed = 1;
	vector<RString> asParts;
	split( sBit, " ", asParts, true );

	for (RString const &s : asParts)
	{
		if( s == "no" )
		{
			level = 0;
		}
		else if( isdigit(s[0]) || s[0] == '-' )
		{
			/* If the last character is a *, they probably said "123*" when
			 * they meant "*123". */
			if( s.Right(1) == "*" )
			{
				// XXX: We know what they want, is there any reason not to handle it?
				// Yes. We should be strict in handling the format. -Chris
				sErrorOut = ssprintf("Invalid player options \"%s\"; did you mean '*%d'?", s.c_str(), std::stoi(s) );
				return false;
			}
			else
			{
				level = StringToFloat( s ) / 100.0f;
			}
		}
		else if( s[0]=='*' )
		{
			sscanf( s, "*%f", &speed );
			if( !isfinite(speed) )
				speed = 1.0f;
		}
	}

	sBit = asParts.back();

#define SET_FLOAT( opt ) { m_ ## opt = level; m_Speed ## opt = speed; }
	const bool on = (level > 0.5f);

	static Regex mult("^([0-9]+(\\.[0-9]+)?)x$");
	vector<RString> matches;
	if( mult.Compare(sBit, matches) )
	{
		StringConversion::FromString( matches[0], level );
		SET_FLOAT( fScrollSpeed )
		SET_FLOAT( fTimeSpacing )
		m_fTimeSpacing = 0;
		m_fMaxScrollBPM = 0;
	}
	else if( sscanf( sBit, "c%f", &level ) == 1 )
	{
		if( !isfinite(level) || level <= 0.0f )
			level = CMOD_DEFAULT;
		SET_FLOAT( fScrollBPM )
		SET_FLOAT( fTimeSpacing )
		m_fTimeSpacing = 1;
		m_fMaxScrollBPM = 0;
	}
	// oITG's m-mods
	else if( sscanf( sBit, "m%f", &level ) == 1 )
	{
		// OpenITG doesn't have this block:
		/*
		if( !isfinite(level) || level <= 0.0f )
			level = CMOD_DEFAULT;
		*/
		SET_FLOAT( fMaxScrollBPM )
		m_fTimeSpacing = 0;
	}

	else if( sBit == "clearall" )
	{
		Init();
		m_sNoteSkin= NOTESKIN->GetDefaultNoteSkinName();
	}
	else if( sBit == "resetspeed" )
	{
		/* level is set to the values from Init() because all speed related
		   fields are being reset to initial values, and they each have different
		   initial values.  -kyz */
		level= 0;
		SET_FLOAT(fMaxScrollBPM);
		SET_FLOAT(fTimeSpacing);
		level= 1.0f;
		SET_FLOAT(fScrollSpeed);
		level= CMOD_DEFAULT;
		SET_FLOAT(fScrollBPM)
	}
	else if( sBit == "life" || sBit == "lives" )
	{
		// level is a percentage for every other option, so multiply by 100. -Kyz
		m_BatteryLives= level * 100.0f;
	}
	else if( sBit.find("modtimer") != sBit.npos)
	{
	    if( sBit == "modtimerdefault" )			{ m_ModTimerType= ModTimerType_Default; }
	    else if( sBit == "modtimersong" )			{ m_ModTimerType= ModTimerType_Song; }
	    else if( sBit == "modtimerbeat" )			{ m_ModTimerType= ModTimerType_Beat; }
	    else if( sBit == "modtimergame" )			{ m_ModTimerType= ModTimerType_Game; }
	    else if( sBit == "modtimermult" )			SET_FLOAT( fModTimerMult )
	    else if( sBit == "modtimeroffset" )			SET_FLOAT( fModTimerOffset )
	}
	else if( sBit.find("drawsize") != sBit.npos)
	{
	    if( sBit == "drawsize" )				SET_FLOAT( fDrawSize )
	    else if( sBit == "drawsizeback" )			SET_FLOAT( fDrawSizeBack )
	}
	else if( sBit == "bar" ) { m_LifeType= LifeType_Bar; }
	else if( sBit == "battery" ) { m_LifeType= LifeType_Battery; }
	else if( sBit == "lifetime" ) { m_LifeType= LifeType_Time; }
	else if( sBit == "norecover" || sBit == "power-drop" ) { m_DrainType= DrainType_NoRecover; }
	else if( sBit == "suddendeath" || sBit == "death" ) { m_DrainType= DrainType_SuddenDeath; }
	else if( sBit == "normal-drain" ) { m_DrainType= DrainType_Normal; }
	else if( sBit == "boost" )				SET_FLOAT( fAccels[ACCEL_BOOST] )
	else if( sBit == "brake" || sBit == "land" )		SET_FLOAT( fAccels[ACCEL_BRAKE] )
	else if( sBit.find("wave") != sBit.npos)
	{
	    if( sBit == "wave" )				SET_FLOAT( fAccels[ACCEL_WAVE] )
	    else if( sBit == "waveperiod" )			SET_FLOAT( fAccels[ACCEL_WAVE_PERIOD] )
	}
	else if( (sBit.find("expand") != sBit.npos) || (sBit.find("dwiwave") != sBit.npos) )
	{
	    if( sBit == "expand" || sBit == "dwiwave" )		SET_FLOAT( fAccels[ACCEL_EXPAND] )
	    else if( sBit == "expandperiod" )			SET_FLOAT( fAccels[ACCEL_EXPAND_PERIOD] )
	    else if( sBit == "tanexpand" )			SET_FLOAT( fAccels[ACCEL_TAN_EXPAND] )
	    else if( sBit == "tanexpandperiod" )		SET_FLOAT( fAccels[ACCEL_TAN_EXPAND_PERIOD] )
	}
	else if( sBit == "boomerang" )				SET_FLOAT( fAccels[ACCEL_BOOMERANG] )
	else if( sBit.find("drunk") != sBit.npos)
	{
	    if( sBit == "drunk" )				SET_FLOAT( fEffects[EFFECT_DRUNK] )
	    else if( sBit == "drunkspeed" )			SET_FLOAT( fEffects[EFFECT_DRUNK_SPEED] )
	    else if( sBit == "drunkoffset" )			SET_FLOAT( fEffects[EFFECT_DRUNK_OFFSET] )
	    else if( sBit == "drunkperiod" )			SET_FLOAT( fEffects[EFFECT_DRUNK_PERIOD] )
	    else if( sBit == "tandrunk" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK] )
	    else if( sBit == "tandrunkspeed" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_SPEED] )
	    else if( sBit == "tandrunkoffset" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_OFFSET] )
	    else if( sBit == "tandrunkperiod" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_PERIOD] )
	    else if( sBit == "drunkz" )				SET_FLOAT( fEffects[EFFECT_DRUNK_Z] )
	    else if( sBit == "drunkzspeed" )			SET_FLOAT( fEffects[EFFECT_DRUNK_Z_SPEED] )
	    else if( sBit == "drunkzoffset" )			SET_FLOAT( fEffects[EFFECT_DRUNK_Z_OFFSET] )
	    else if( sBit == "drunkzperiod" )			SET_FLOAT( fEffects[EFFECT_DRUNK_Z_PERIOD] )
	    else if( sBit == "tandrunkz" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_Z] )
	    else if( sBit == "tandrunkzspeed" )			SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_Z_SPEED] )
	    else if( sBit == "tandrunkzoffset" )		SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_Z_OFFSET] )
	    else if( sBit == "tandrunkzperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_DRUNK_Z_PERIOD] )
	}
	else if( sBit.find("shrink") != sBit.npos)
	{
	    if( sBit == "shrinklinear" )			SET_FLOAT( fEffects[EFFECT_SHRINK_TO_LINEAR] )
	    else if( sBit == "shrinkmult" )			SET_FLOAT( fEffects[EFFECT_SHRINK_TO_MULT] )
	}
	else if( sBit.find("pulse") != sBit.npos)
	{
	    if( sBit == "pulseinner" )				SET_FLOAT( fEffects[EFFECT_PULSE_INNER] )
	    else if( sBit == "pulseouter" )			SET_FLOAT( fEffects[EFFECT_PULSE_OUTER] )
	    else if( sBit == "pulseoffset" )			SET_FLOAT( fEffects[EFFECT_PULSE_OFFSET] )
	    else if( sBit == "pulseperiod" )			SET_FLOAT( fEffects[EFFECT_PULSE_PERIOD] )
	}
	else if( sBit.find("dizzy") != sBit.npos)
	{
	    if( sBit == "dizzy" )				SET_FLOAT( fEffects[EFFECT_DIZZY] )
	    else if( sBit == "dizzyholds" )			m_bDizzyHolds = on;
	}
	else if( sBit.find("confusion") != sBit.npos)
	{
	    if( sBit.find("x") != sBit.npos)
	    {
		if( sBit == "confusionx" )			SET_FLOAT( fEffects[EFFECT_CONFUSION_X] )
		else if( sBit == "confusionxoffset" )		SET_FLOAT( fEffects[EFFECT_CONFUSION_X_OFFSET] )
		else
		{
		    for (int i=0; i<16; i++)
		    {
			sMod = ssprintf( "confusionxoffset%d", i+1 );
			if( sBit == sMod)
			{
			    SET_FLOAT( fConfusionX[i] )
			    break;
			}
		    }
		}
	    }
	    else if( sBit.find("y") != sBit.npos)
	    {
		if( sBit == "confusiony" )			SET_FLOAT( fEffects[EFFECT_CONFUSION_Y] )
		else if( sBit == "confusionyoffset" )		SET_FLOAT( fEffects[EFFECT_CONFUSION_Y_OFFSET] )
		else
		{
		    for (int i=0; i<16; i++)
		    {
			sMod = ssprintf( "confusionyoffset%d", i+1 );
			if( sBit == sMod)
			{
			    SET_FLOAT( fConfusionY[i] )
			    break;
			}
		    }
		}
	    }
	    if( sBit == "confusion" )				SET_FLOAT( fEffects[EFFECT_CONFUSION] )
	    else if( sBit == "confusionoffset" )		SET_FLOAT( fEffects[EFFECT_CONFUSION_OFFSET] )
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "confusionoffset%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fConfusionZ[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit.find("bounce") != sBit.npos)
	{
	    if( sBit == "bounce" )				SET_FLOAT( fEffects[EFFECT_BOUNCE] )
	    else if( sBit == "bounceperiod" )			SET_FLOAT( fEffects[EFFECT_BOUNCE_PERIOD] )
	    else if( sBit == "bounceoffset" )			SET_FLOAT( fEffects[EFFECT_BOUNCE_OFFSET] )
	    else if( sBit == "bouncez" )			SET_FLOAT( fEffects[EFFECT_BOUNCE_Z] )
	    else if( sBit == "bouncezperiod" )			SET_FLOAT( fEffects[EFFECT_BOUNCE_Z_PERIOD] )
	    else if( sBit == "bouncezoffset" )			SET_FLOAT( fEffects[EFFECT_BOUNCE_Z_OFFSET] )
	}
	else if( sBit == "mini" )				SET_FLOAT( fEffects[EFFECT_MINI] )
	else if ( sBit.find("tiny") != sBit.npos)
	{
	    if( sBit == "tiny" )				SET_FLOAT( fEffects[EFFECT_TINY] )
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "tiny%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fTiny[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit == "flip" )				SET_FLOAT( fEffects[EFFECT_FLIP] )
	else if( sBit == "invert" )				SET_FLOAT( fEffects[EFFECT_INVERT] )
	else if( sBit.find("tornado") != sBit.npos)
	{
	    if( sBit == "tornado" )				SET_FLOAT( fEffects[EFFECT_TORNADO] )
	    else if( sBit == "tornadoperiod" )			SET_FLOAT( fEffects[EFFECT_TORNADO_PERIOD] )
	    else if( sBit == "tornadooffset" )			SET_FLOAT( fEffects[EFFECT_TORNADO_OFFSET] )
	    else if( sBit == "tantornado" )			SET_FLOAT( fEffects[EFFECT_TAN_TORNADO] )
	    else if( sBit == "tantornadoperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_TORNADO_PERIOD] )
	    else if( sBit == "tantornadooffset" )		SET_FLOAT( fEffects[EFFECT_TAN_TORNADO_OFFSET] )
	    else if( sBit == "tornadoz" )			SET_FLOAT( fEffects[EFFECT_TORNADO_Z] )
	    else if( sBit == "tornadozperiod" )			SET_FLOAT( fEffects[EFFECT_TORNADO_Z_PERIOD] )
	    else if( sBit == "tornadozoffset" )			SET_FLOAT( fEffects[EFFECT_TORNADO_Z_OFFSET] )
	    else if( sBit == "tantornadoz" )			SET_FLOAT( fEffects[EFFECT_TAN_TORNADO_Z] )
	    else if( sBit == "tantornadozperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_TORNADO_Z_PERIOD] )
	    else if( sBit == "tantornadozoffset" )		SET_FLOAT( fEffects[EFFECT_TAN_TORNADO_Z_OFFSET] )
	}
	else if( sBit.find("tipsy") != sBit.npos)
	{
	    if( sBit == "tipsy" )				SET_FLOAT( fEffects[EFFECT_TIPSY] )
	    else if( sBit == "tipsyspeed" )			SET_FLOAT( fEffects[EFFECT_TIPSY_SPEED] )
	    else if( sBit == "tipsyoffset" )			SET_FLOAT( fEffects[EFFECT_TIPSY_OFFSET] )
	    else if( sBit == "tantipsy" )			SET_FLOAT( fEffects[EFFECT_TAN_TIPSY] )
	    else if( sBit == "tantipsyspeed" )			SET_FLOAT( fEffects[EFFECT_TAN_TIPSY_SPEED] )
	    else if( sBit == "tantipsyoffset" )			SET_FLOAT( fEffects[EFFECT_TAN_TIPSY_OFFSET] )
	}
	else if( sBit.find("bumpy") != sBit.npos)
	{
	    if( sBit == "bumpy" )				SET_FLOAT( fEffects[EFFECT_BUMPY] )
	    else if( sBit == "bumpyoffset" )			SET_FLOAT( fEffects[EFFECT_BUMPY_OFFSET] )
	    else if( sBit == "bumpyperiod" )			SET_FLOAT( fEffects[EFFECT_BUMPY_PERIOD] )
	    else if( sBit == "tanbumpy" )			SET_FLOAT( fEffects[EFFECT_TAN_BUMPY] )
	    else if( sBit == "tanbumpyoffset" )			SET_FLOAT( fEffects[EFFECT_TAN_BUMPY_OFFSET] )
	    else if( sBit == "tanbumpyperiod" )			SET_FLOAT( fEffects[EFFECT_TAN_BUMPY_PERIOD] )
	    else if( sBit == "bumpyx" )				SET_FLOAT( fEffects[EFFECT_BUMPY_X] )
	    else if( sBit == "bumpyxoffset" )			SET_FLOAT( fEffects[EFFECT_BUMPY_X_OFFSET] )
	    else if( sBit == "bumpyxperiod" )			SET_FLOAT( fEffects[EFFECT_BUMPY_X_PERIOD] )
	    else if( sBit == "tanbumpyx" )			SET_FLOAT( fEffects[EFFECT_TAN_BUMPY_X] )
	    else if( sBit == "tanbumpyxoffset" )		SET_FLOAT( fEffects[EFFECT_TAN_BUMPY_X_OFFSET] )
	    else if( sBit == "tanbumpyxperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_BUMPY_X_PERIOD] )
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "bumpy%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fBumpy[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit.find("beat") != sBit.npos)
	{
	    if( sBit == "beat" )				SET_FLOAT( fEffects[EFFECT_BEAT] )
	    else if( sBit == "beatoffset" )			SET_FLOAT( fEffects[EFFECT_BEAT_OFFSET] )
	    else if( sBit == "beatperiod" )			SET_FLOAT( fEffects[EFFECT_BEAT_PERIOD] )
	    else if( sBit == "beatmult" )			SET_FLOAT( fEffects[EFFECT_BEAT_MULT] )
	    else if( sBit == "beaty" )				SET_FLOAT( fEffects[EFFECT_BEAT_Y] )
	    else if( sBit == "beatyoffset" )			SET_FLOAT( fEffects[EFFECT_BEAT_Y_OFFSET] )
	    else if( sBit == "beatyperiod" )			SET_FLOAT( fEffects[EFFECT_BEAT_Y_PERIOD] )
	    else if( sBit == "beatymult" )			SET_FLOAT( fEffects[EFFECT_BEAT_Y_MULT] )
	    else if( sBit == "beatz" )				SET_FLOAT( fEffects[EFFECT_BEAT_Z] )
	    else if( sBit == "beatzoffset" )			SET_FLOAT( fEffects[EFFECT_BEAT_Z_OFFSET] )
	    else if( sBit == "beatzperiod" )			SET_FLOAT( fEffects[EFFECT_BEAT_Z_PERIOD] )
	    else if( sBit == "beatzmult" )			SET_FLOAT( fEffects[EFFECT_BEAT_Z_MULT] )
	}
	else if( sBit.find("digital") != sBit.npos)
	{
	    if( sBit == "digital" )				SET_FLOAT( fEffects[EFFECT_DIGITAL] )
	    else if( sBit == "digitalsteps" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_STEPS] )
	    else if( sBit == "digitalperiod" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_PERIOD] )
	    else if( sBit == "digitaloffset" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_OFFSET] )
	    else if( sBit == "tandigital" )			SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL] )
	    else if( sBit == "tandigitalsteps" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_STEPS] )
	    else if( sBit == "tandigitalperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_PERIOD] )
	    else if( sBit == "tandigitaloffset" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_OFFSET] )
	    else if( sBit == "digitalz" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_Z] )
	    else if( sBit == "digitalzsteps" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_Z_STEPS] )
	    else if( sBit == "digitalzperiod" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_Z_PERIOD] )
	    else if( sBit == "digitalzoffset" )			SET_FLOAT( fEffects[EFFECT_DIGITAL_Z_OFFSET] )
	    else if( sBit == "tandigitalz" )			SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_Z] )
	    else if( sBit == "tandigitalzsteps" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_Z_STEPS] )
	    else if( sBit == "tandigitalzperiod" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_Z_PERIOD] )
	    else if( sBit == "tandigitalzoffset" )		SET_FLOAT( fEffects[EFFECT_TAN_DIGITAL_Z_OFFSET] )
	}
	else if( sBit.find("zigzag") != sBit.npos)
	{
	    if( sBit == "zigzag" )				SET_FLOAT( fEffects[EFFECT_ZIGZAG] )
	    else if( sBit == "zigzagperiod" )			SET_FLOAT( fEffects[EFFECT_ZIGZAG_PERIOD] )
	    else if( sBit == "zigzagoffset" )			SET_FLOAT( fEffects[EFFECT_ZIGZAG_OFFSET] )
	    else if( sBit == "zigzagz" )			SET_FLOAT( fEffects[EFFECT_ZIGZAG_Z] )
	    else if( sBit == "zigzagzperiod" )			SET_FLOAT( fEffects[EFFECT_ZIGZAG_Z_PERIOD] )
	    else if( sBit == "zigzagzoffset" )			SET_FLOAT( fEffects[EFFECT_ZIGZAG_Z_OFFSET] )
	}
	else if( sBit.find("sawtooth") != sBit.npos)
	{
	    if( sBit == "sawtooth" )				SET_FLOAT( fEffects[EFFECT_SAWTOOTH] )
	    else if( sBit == "sawtoothperiod" )			SET_FLOAT( fEffects[EFFECT_SAWTOOTH_PERIOD] )
	    else if( sBit == "sawtoothz" )			SET_FLOAT( fEffects[EFFECT_SAWTOOTH_Z] )
	    else if( sBit == "sawtoothzperiod" )		SET_FLOAT( fEffects[EFFECT_SAWTOOTH_Z_PERIOD] )
	}
	else if( sBit.find("square") != sBit.npos)
	{
	    if( sBit == "square" )				SET_FLOAT( fEffects[EFFECT_SQUARE] )
	    else if( sBit == "squareoffset" )			SET_FLOAT( fEffects[EFFECT_SQUARE_OFFSET] )
	    else if( sBit == "squareperiod" )			SET_FLOAT( fEffects[EFFECT_SQUARE_PERIOD] )
	    else if( sBit == "squarez" )			SET_FLOAT( fEffects[EFFECT_SQUARE_Z] )
	    else if( sBit == "squarezoffset" )			SET_FLOAT( fEffects[EFFECT_SQUARE_Z_OFFSET] )
	    else if( sBit == "squarezperiod" )			SET_FLOAT( fEffects[EFFECT_SQUARE_Z_PERIOD] )
	}
	else if( sBit.find("parabola") != sBit.npos)
	{
	    if( sBit == "parabolax" )				SET_FLOAT( fEffects[EFFECT_PARABOLA_X] )
	    else if( sBit == "parabolay" )			SET_FLOAT( fEffects[EFFECT_PARABOLA_Y] )
	    else if( sBit == "parabolaz" )			SET_FLOAT( fEffects[EFFECT_PARABOLA_Z] )
	}
	else if( sBit.find("attenuate") != sBit.npos)
	{
	    if( sBit == "attenuatex" )				SET_FLOAT( fEffects[EFFECT_ATTENUATE_X] )
	    else if( sBit == "attenuatey" )			SET_FLOAT( fEffects[EFFECT_ATTENUATE_Y] )
	    else if( sBit == "attenuatez" )			SET_FLOAT( fEffects[EFFECT_ATTENUATE_Z] )
	}
	else if( sBit == "xmode" )				SET_FLOAT( fEffects[EFFECT_XMODE] )
	else if( sBit == "twirl" )				SET_FLOAT( fEffects[EFFECT_TWIRL] )
	else if( sBit == "roll" )				SET_FLOAT( fEffects[EFFECT_ROLL] )
	else if( sBit == "hidden" )				SET_FLOAT( fAppearances[APPEARANCE_HIDDEN] )
	else if( sBit == "hiddenoffset" )			SET_FLOAT( fAppearances[APPEARANCE_HIDDEN_OFFSET] )
	else if( sBit == "sudden" )				SET_FLOAT( fAppearances[APPEARANCE_SUDDEN] )
	else if( sBit == "suddenoffset" )			SET_FLOAT( fAppearances[APPEARANCE_SUDDEN_OFFSET] )
	else if ( sBit.find("stealth") != sBit.npos)
	{
	    if( sBit == "stealth" )				SET_FLOAT( fAppearances[APPEARANCE_STEALTH] )
	    else if( sBit == "stealthtype" )			m_bStealthType = on;
	    else if( sBit == "stealthpastreceptors" )		m_bStealthPastReceptors = on;
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "stealth%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fStealth[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit == "blink" )				SET_FLOAT( fAppearances[APPEARANCE_BLINK] )
	else if( sBit == "randomvanish" )			SET_FLOAT( fAppearances[APPEARANCE_RANDOMVANISH] )
	else if( sBit == "turn" && !on )			ZERO( m_bTurns ); /* "no turn" */
	else if( sBit == "mirror" )				m_bTurns[TURN_MIRROR] = on;
	else if( sBit == "backwards" )			m_bTurns[TURN_BACKWARDS] = on;
	else if( sBit == "left" )				m_bTurns[TURN_LEFT] = on;
	else if( sBit == "right" )				m_bTurns[TURN_RIGHT] = on;
	else if( sBit == "shuffle" )				m_bTurns[TURN_SHUFFLE] = on;
	else if( sBit == "softshuffle" )				m_bTurns[TURN_SOFT_SHUFFLE] = on;
	else if( sBit == "supershuffle" )			m_bTurns[TURN_SUPER_SHUFFLE] = on;
	else if( sBit == "little" )				m_bTransforms[TRANSFORM_LITTLE] = on;
	else if( sBit == "wide" )				m_bTransforms[TRANSFORM_WIDE] = on;
	else if( sBit == "big" )				m_bTransforms[TRANSFORM_BIG] = on;
	else if( sBit == "quick" )				m_bTransforms[TRANSFORM_QUICK] = on;
	else if( sBit == "bmrize" )				m_bTransforms[TRANSFORM_BMRIZE] = on;
	else if( sBit == "skippy" )				m_bTransforms[TRANSFORM_SKIPPY] = on;
	else if( sBit == "mines" )				m_bTransforms[TRANSFORM_MINES] = on;
	else if( sBit == "attackmines" )			m_bTransforms[TRANSFORM_ATTACKMINES] = on;
	else if( sBit == "echo" )				m_bTransforms[TRANSFORM_ECHO] = on;
	else if( sBit == "stomp" )				m_bTransforms[TRANSFORM_STOMP] = on;
	else if( sBit == "planted" )				m_bTransforms[TRANSFORM_PLANTED] = on;
	else if( sBit == "floored" )				m_bTransforms[TRANSFORM_FLOORED] = on;
	else if( sBit == "twister" )				m_bTransforms[TRANSFORM_TWISTER] = on;
	else if( sBit == "holdrolls" )				m_bTransforms[TRANSFORM_HOLDROLLS] = on;
	else if( sBit == "nojumps" )				m_bTransforms[TRANSFORM_NOJUMPS] = on;
	else if( sBit == "nohands" )				m_bTransforms[TRANSFORM_NOHANDS] = on;
	else if( sBit == "noquads" )				m_bTransforms[TRANSFORM_NOQUADS] = on;
	else if ( sBit.find("reverse") != sBit.npos)
	{
	    if( sBit == "reverse" )				SET_FLOAT( fScrolls[SCROLL_REVERSE] )
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "reverse%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fReverse[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit == "split" )				SET_FLOAT( fScrolls[SCROLL_SPLIT] )
	else if( sBit == "alternate" )				SET_FLOAT( fScrolls[SCROLL_ALTERNATE] )
	else if( sBit == "cross" )				SET_FLOAT( fScrolls[SCROLL_CROSS] )
	else if( sBit == "centered" )				SET_FLOAT( fScrolls[SCROLL_CENTERED] )
	else if( sBit == "noholds" )				m_bTransforms[TRANSFORM_NOHOLDS] = on;
	else if( sBit == "norolls" )				m_bTransforms[TRANSFORM_NOROLLS] = on;
	else if( sBit == "nomines" )				m_bTransforms[TRANSFORM_NOMINES] = on;
	else if( sBit == "nostretch" )				m_bTransforms[TRANSFORM_NOSTRETCH] = on;
	else if( sBit == "nolifts" )				m_bTransforms[TRANSFORM_NOLIFTS] = on;
	else if( sBit == "nofakes" )				m_bTransforms[TRANSFORM_NOFAKES] = on;
	else if( sBit.find("dark") != sBit.npos )
	{
	    if( sBit == "dark" )				SET_FLOAT( fDark )
	    else
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "dark%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fDarks[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit == "blind" )				SET_FLOAT( fBlind )
	else if( sBit == "cover" )				SET_FLOAT( fCover )
	else if( sBit == "randomattacks" )			SET_FLOAT( fRandAttack )
	else if( sBit == "noattacks" )				SET_FLOAT( fNoAttack )
	else if( sBit == "playerautoplay" )			SET_FLOAT( fPlayerAutoPlay )
	else if( sBit == "passmark" )				SET_FLOAT( fPassmark )
	else if( sBit == "overhead" )				{ m_fSkew = 0;		m_fPerspectiveTilt = 0;		m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
	else if( sBit == "incoming" )				{ m_fSkew = level;	m_fPerspectiveTilt = -level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
	else if( sBit == "space" )				{ m_fSkew = level;	m_fPerspectiveTilt = +level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
	else if( sBit == "hallway" )				{ m_fSkew = 0;		m_fPerspectiveTilt = -level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
	else if( sBit == "distant" )				{ m_fSkew = 0;		m_fPerspectiveTilt = +level;	m_SpeedfSkew = m_SpeedfPerspectiveTilt = speed; }
	else if( NOTESKIN && NOTESKIN->DoesNoteSkinExist(sBit) )
	{
		m_sNoteSkin = sBit;
	}
	else if( sBit == "skew" ) SET_FLOAT( fSkew )
	else if( sBit == "tilt" ) SET_FLOAT( fPerspectiveTilt )
	else if( sBit == "noteskin" && !on ) /* "no noteskin" */
	{
		m_sNoteSkin = CommonMetrics::DEFAULT_NOTESKIN_NAME;
	}
	else if( sBit == "randomspeed" ) 			SET_FLOAT( fRandomSpeed )
	else if( sBit == "failarcade" ||
		 sBit == "failimmediate" )			m_FailType = FailType_Immediate;
	else if( sBit == "failendofsong" ||
		 sBit == "failimmediatecontinue" )		m_FailType = FailType_ImmediateContinue;
	else if( sBit == "failatend" )				m_FailType = FailType_EndOfSong;
	else if( sBit == "failoff" )				m_FailType = FailType_Off;
	else if( sBit == "faildefault" )
	{
		PlayerOptions po;
		GAMESTATE->GetDefaultPlayerOptions( po );
		m_FailType = po.m_FailType;
	}
	else if( sBit == "muteonerror" )			m_bMuteOnError = on;
	else if( sBit == "random" )				ChooseRandomModifiers();

	else if( sBit.find("move") != sBit.npos)
	{
	    if (sBit.find("x") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "movex%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesX[i] )
			break;
		    }
		}
	    }
	    else if (sBit.find("y") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "movey%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesY[i] )
			break;
		    }
		}
	    }
	    else if (sBit.find("z") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = ssprintf( "movez%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesZ[i] )
			break;
		    }
		}
	    }
	}
	else if( sBit == "zbuffer" )				m_bZBuffer = on;
	else if( sBit == "cosecant" )				m_bCosecant = on;
	// deprecated mods/left in for compatibility
	else if( sBit == "converge" )				SET_FLOAT( fScrolls[SCROLL_CENTERED] )
	// end of the list
	else
	{
		// Maybe the original string is a noteskin name with a space. -Kyz
		RString name= sOneMod;
		name.MakeLower();
		if(NOTESKIN && NOTESKIN->DoesNoteSkinExist(name))
		{
			m_sNoteSkin = name;
		}
		return false;
	}

	return true;
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
	case -1:		m_fPerspectiveTilt =  0;	break;
	case  0:		m_fPerspectiveTilt = +1;	break;
	case +1: default:	m_fPerspectiveTilt = -1;	break;
	}
}

void PlayerOptions::ChooseRandomModifiers()
{
	if( RandomFloat(0,1)<RANDOM_SPEED_CHANCE )
		m_fScrollSpeed = 1.5f;
	if( RandomFloat(0,1)<RANDOM_REVERSE_CHANCE )
		m_fScrolls[SCROLL_REVERSE] = 1;
	if( RandomFloat(0,1)<RANDOM_DARK_CHANCE )
		m_fDark = 1;
	float f;
	f = RandomFloat(0,1);
	if( f<RANDOM_ACCEL_CHANCE )
		m_fAccels[RandomInt(NUM_ACCELS)] = 1;
	else if( f<RANDOM_EFFECT_CHANCE )
		m_fEffects[RandomInt(NUM_EFFECTS)] = 1;
	f = RandomFloat(0,1);
	if( f<RANDOM_HIDDEN_CHANCE )
		m_fAppearances[APPEARANCE_HIDDEN] = 1;
	else if( f<RANDOM_SUDDEN_CHANCE )
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
	ASSERT(m_pn == PLAYER_1 || m_pn == PLAYER_2);
	ASSERT(GAMESTATE->GetCurrentStyle(m_pn) != nullptr);
	int iNumCols = GAMESTATE->GetCurrentStyle(m_pn)->m_iColsPerPlayer;

	f += m_fScrolls[SCROLL_REVERSE];
	f += m_fReverse[iCol];

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
	COMPARE(m_LifeType);
	COMPARE(m_DrainType);
	COMPARE(m_ModTimerType);
	COMPARE(m_fModTimerMult);
	COMPARE(m_fModTimerOffset);
	COMPARE(m_fDrawSize);
	COMPARE(m_fDrawSizeBack);
	COMPARE(m_BatteryLives);
	COMPARE(m_fTimeSpacing);
	COMPARE(m_fScrollSpeed);
	COMPARE(m_fScrollBPM);
	COMPARE(m_fMaxScrollBPM);
	COMPARE(m_fRandomSpeed);
	COMPARE(m_FailType);
	COMPARE(m_MinTNSToHideNotes);
	COMPARE(m_bMuteOnError);
	COMPARE(m_bStealthType);
	COMPARE(m_bStealthPastReceptors);
	COMPARE(m_bDizzyHolds);
	COMPARE(m_bZBuffer);
	COMPARE(m_bCosecant);
	COMPARE(m_fDark);
	COMPARE(m_fBlind);
	COMPARE(m_fCover);
	COMPARE(m_fRandAttack);
	COMPARE(m_fNoAttack);
	COMPARE(m_fPlayerAutoPlay);
	COMPARE(m_fPerspectiveTilt);
	COMPARE(m_fSkew);
	// The noteskin name needs to be compared case-insensitively because the
	// manager forces lowercase, but some obscure part of PlayerOptions
	// uppercases the first letter.  The previous code that used != probably
	// relied on RString::operator!= misbehaving. -Kyz
	if(strcasecmp(m_sNoteSkin, other.m_sNoteSkin) != 0)
	{
		return false;
	}
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
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fMovesX[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fMovesY[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fMovesZ[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fConfusionX[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fConfusionY[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fConfusionZ[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fDarks[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fStealth[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fTiny[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fBumpy[i]);
	for( int i = 0; i < 16; ++i )
		COMPARE(m_fReverse[i]);
#undef COMPARE
	return true;
}


PlayerOptions& PlayerOptions::operator=(PlayerOptions const& other)
{
	// Do not copy m_pn from the other, it must be preserved as what PlayerState set it to.
#define CPY(x) x= other.x;
#define CPY_SPEED(x) m_ ## x = other.m_ ## x; m_Speed ## x = other.m_Speed ## x;
	CPY(m_LifeType);
	CPY(m_DrainType);
	CPY(m_ModTimerType);
	CPY_SPEED(fModTimerMult);
	CPY_SPEED(fModTimerOffset);
	CPY_SPEED(fDrawSize);
	CPY_SPEED(fDrawSizeBack);
	CPY(m_BatteryLives);
	CPY_SPEED(fTimeSpacing);
	CPY_SPEED(fScrollSpeed);
	CPY_SPEED(fScrollBPM);
	CPY_SPEED(fMaxScrollBPM);
	CPY_SPEED(fRandomSpeed);
	CPY(m_FailType);
	CPY(m_MinTNSToHideNotes);
	CPY(m_bMuteOnError);
	CPY(m_bStealthType);
	CPY(m_bStealthPastReceptors);
	CPY(m_bDizzyHolds);
	CPY(m_bZBuffer);
	CPY(m_bCosecant);
	CPY_SPEED(fDark);
	CPY_SPEED(fBlind);
	CPY_SPEED(fCover);
	CPY_SPEED(fRandAttack);
	CPY_SPEED(fNoAttack);
	CPY_SPEED(fPlayerAutoPlay);
	CPY_SPEED(fPerspectiveTilt);
	CPY_SPEED(fSkew);
	if(!other.m_sNoteSkin.empty() &&
		NOTESKIN->DoesNoteSkinExist(other.m_sNoteSkin))
	{
		CPY(m_sNoteSkin);
	}
	for( int i = 0; i < PlayerOptions::NUM_ACCELS; ++i )
	{
		CPY_SPEED(fAccels[i]);
	}
	for( int i = 0; i < PlayerOptions::NUM_EFFECTS; ++i )
	{
		CPY_SPEED(fEffects[i]);
	}
	for( int i = 0; i < PlayerOptions::NUM_APPEARANCES; ++i )
	{
		CPY_SPEED(fAppearances[i]);
	}
	for( int i = 0; i < PlayerOptions::NUM_SCROLLS; ++i )
	{
		CPY_SPEED(fScrolls[i]);
	}
	for( int i = 0; i < PlayerOptions::NUM_TURNS; ++i )
	{
		CPY(m_bTurns[i]);
	}
	for( int i = 0; i < PlayerOptions::NUM_TRANSFORMS; ++i )
	{
		CPY(m_bTransforms[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fMovesX[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fMovesY[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fMovesZ[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fConfusionX[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fConfusionY[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fConfusionZ[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fDarks[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fStealth[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fTiny[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fBumpy[i]);
	}
	for( int i = 0; i < 16; ++i )
	{
		CPY_SPEED(fReverse[i]);
	}
#undef CPY
#undef CPY_SPEED
	return *this;
}


bool PlayerOptions::IsEasierForSongAndSteps( Song* pSong, Steps* pSteps, PlayerNumber pn ) const
{
	if( m_fTimeSpacing && pSteps->HasSignificantTimingChanges() )
		return true;
	const RadarValues &rv = pSteps->GetRadarValues( pn );
	if( m_bTransforms[TRANSFORM_NOHOLDS] && rv[RadarCategory_Holds]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOROLLS] && rv[RadarCategory_Rolls]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOMINES] && rv[RadarCategory_Mines]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOHANDS] && rv[RadarCategory_Hands]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOQUADS] && rv[RadarCategory_Hands]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOJUMPS] && rv[RadarCategory_Jumps]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOLIFTS] && rv[RadarCategory_Lifts]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOFAKES] && rv[RadarCategory_Fakes]>0 )
		return true;
	if( m_bTransforms[TRANSFORM_NOSTRETCH] )
		return true;

	// Inserted holds can be really easy on some songs, and scores will be
	// highly hold-weighted, and very little tap score weighted.
	if( m_bTransforms[TRANSFORM_LITTLE] )	return true;
	if( m_bTransforms[TRANSFORM_PLANTED] )	return true;
	if( m_bTransforms[TRANSFORM_FLOORED] )	return true;
	if( m_bTransforms[TRANSFORM_TWISTER] )	return true;

	// This makes songs with sparse notes easier.
	if( m_bTransforms[TRANSFORM_ECHO] )	return true;

	// Removing attacks is easier in general.
	if ((m_fNoAttack && pSteps->HasAttacks()) || m_fRandAttack)
		return true;

	if( m_fCover )	return true;

	// M-mods make songs with indefinite BPMs easier because
	// they ensure that the song has a scrollable speed.
	if( m_fMaxScrollBPM != 0 )
	{
		// BPM display is obfuscated
//		if( pSong->m_DisplayBPMType == DISPLAY_BPM_RANDOM )
//			return true;

		DisplayBpms bpms;
		if( GAMESTATE->IsCourseMode() )
		{
			Trail *pTrail = GAMESTATE->m_pCurCourse->GetTrail( GAMESTATE->GetCurrentStyle(m_pn)->m_StepsType );
			pTrail->GetDisplayBpms( bpms );
		}
		else
		{
			GAMESTATE->m_pCurSong->GetDisplayBpms( bpms );
		}
		pSong->GetDisplayBpms( bpms );

		// maximum BPM is obfuscated, so M-mods will set a playable speed.
		if( bpms.GetMax() <= 0 )
			return true;
	}
	if( m_fPlayerAutoPlay )	return true;
	return false;
}

bool PlayerOptions::IsEasierForCourseAndTrail( Course* pCourse, Trail* pTrail ) const
{
	ASSERT( pCourse != nullptr );
	ASSERT( pTrail != nullptr );

	return std::any_of(pTrail->m_vEntries.begin(), pTrail->m_vEntries.end(), [&](TrailEntry const &e) {
		return e.pSong && IsEasierForSongAndSteps(e.pSong, e.pSteps, PLAYER_1);
	});
}

void PlayerOptions::GetLocalizedMods( vector<RString> &AddTo ) const
{
	vector<RString> vMods;
	GetMods( vMods );
	for (RString const &sOneMod : vMods)
	{
		ASSERT( !sOneMod.empty() );

		vector<RString> asTokens;
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
		asTokens.back() = CommonMetrics::LocalizeOptionItem( asTokens.back(), true );

		RString sLocalizedMod = join( " ", asTokens );
		AddTo.push_back( sLocalizedMod );
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

RString PlayerOptions::GetSavedPrefsString() const
{
	PlayerOptions po_prefs;
#define SAVE(x) po_prefs.x = this->x;
	SAVE( m_fTimeSpacing );
	SAVE( m_fScrollSpeed );
	SAVE( m_fScrollBPM );
	SAVE( m_fMaxScrollBPM );
	SAVE( m_fScrolls[SCROLL_REVERSE] );
	SAVE( m_fPerspectiveTilt );
	SAVE( m_bTransforms[TRANSFORM_NOHOLDS] );
	SAVE( m_bTransforms[TRANSFORM_NOROLLS] );
	SAVE( m_bTransforms[TRANSFORM_NOMINES] );
	SAVE( m_bTransforms[TRANSFORM_NOJUMPS] );
	SAVE( m_bTransforms[TRANSFORM_NOHANDS] );
	SAVE( m_bTransforms[TRANSFORM_NOQUADS] );
	SAVE( m_bTransforms[TRANSFORM_NOSTRETCH] );
	SAVE( m_bTransforms[TRANSFORM_NOLIFTS] );
	SAVE( m_bTransforms[TRANSFORM_NOFAKES] );
	SAVE( m_bMuteOnError );
	SAVE( m_sNoteSkin );
#undef SAVE
	return po_prefs.GetString();
}

void PlayerOptions::ResetPrefs( ResetPrefsType type )
{
	PlayerOptions defaults;
#define CPY(x) this->x = defaults.x;
	switch( type )
	{
	DEFAULT_FAIL( type );
	case saved_prefs:
		CPY( m_fTimeSpacing );
		CPY( m_fScrollSpeed );
		CPY( m_fScrollBPM );
		CPY( m_fMaxScrollBPM );
		break;
	case saved_prefs_invalid_for_course:
		break;
	}
	CPY(m_LifeType);
	CPY(m_DrainType);
	CPY(m_BatteryLives);
	CPY(m_ModTimerType);
	CPY(m_fModTimerMult);
	CPY(m_fModTimerOffset);
	CPY(m_fDrawSize);
	CPY(m_fDrawSizeBack);
	CPY(m_bStealthType);
	CPY(m_bStealthPastReceptors);
	CPY(m_bDizzyHolds);
	CPY(m_bZBuffer);
	CPY(m_bCosecant);
	CPY(m_MinTNSToHideNotes);

	CPY( m_fPerspectiveTilt );
	CPY( m_bTransforms[TRANSFORM_NOHOLDS] );
	CPY( m_bTransforms[TRANSFORM_NOROLLS] );
	CPY( m_bTransforms[TRANSFORM_NOMINES] );
	CPY( m_bTransforms[TRANSFORM_NOJUMPS] );
	CPY( m_bTransforms[TRANSFORM_NOHANDS] );
	CPY( m_bTransforms[TRANSFORM_NOQUADS] );
	CPY( m_bTransforms[TRANSFORM_NOSTRETCH] );
	CPY( m_bTransforms[TRANSFORM_NOLIFTS] );
	CPY( m_bTransforms[TRANSFORM_NOFAKES] );
	// Don't clear this.
	// CPY( m_sNoteSkin );
#undef CPY
}

// lua start
#include "LuaBinding.h"
#include "OptionsBinding.h"

/** @brief Allow Lua to have access to PlayerOptions. */
class LunaPlayerOptions: public Luna<PlayerOptions>
{
public:
	static int IsEasierForSongAndSteps( T *p, lua_State *L )
	{
		Song* pSong = Luna<Song>::check(L,1);
		Steps* pSteps = Luna<Steps>::check(L,2);
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 3);
		lua_pushboolean(L, p->IsEasierForSongAndSteps(pSong, pSteps, pn) );
		return 1;
	}
	static int IsEasierForCourseAndTrail( T *p, lua_State *L )
	{
		// course, trail
		Course* pCourse = Luna<Course>::check(L,1);
		Trail* pTrail = Luna<Trail>::check(L,2);
		lua_pushboolean(L, p->IsEasierForCourseAndTrail(pCourse, pTrail) );
		return 1;
	}

	// Direct control functions, for themes that can handle it.

	ENUM_INTERFACE(LifeSetting, LifeType, LifeType);
	ENUM_INTERFACE(DrainSetting, DrainType, DrainType);
	ENUM_INTERFACE(ModTimerSetting, ModTimerType, ModTimerType)
	INT_INTERFACE(BatteryLives, BatteryLives);
	FLOAT_INTERFACE(ModTimerMult, ModTimerMult, true);
	FLOAT_INTERFACE(ModTimerOffset, ModTimerOffset, true);
	FLOAT_INTERFACE(DrawSize, DrawSize, true);
	FLOAT_INTERFACE(DrawSizeBack, DrawSizeBack, true);
	FLOAT_INTERFACE(TimeSpacing, TimeSpacing, true);
	FLOAT_INTERFACE(MaxScrollBPM, MaxScrollBPM, true);
	FLOAT_INTERFACE(ScrollSpeed, ScrollSpeed, true);
	FLOAT_INTERFACE(ScrollBPM, ScrollBPM, true);
	FLOAT_INTERFACE(Boost, Accels[PlayerOptions::ACCEL_BOOST], true);
	FLOAT_INTERFACE(Brake, Accels[PlayerOptions::ACCEL_BRAKE], true);
	FLOAT_INTERFACE(Wave, Accels[PlayerOptions::ACCEL_WAVE], true);
	FLOAT_INTERFACE(WavePeriod, Accels[PlayerOptions::ACCEL_WAVE_PERIOD], true);
	FLOAT_INTERFACE(Expand, Accels[PlayerOptions::ACCEL_EXPAND], true);
	FLOAT_INTERFACE(ExpandPeriod, Accels[PlayerOptions::ACCEL_EXPAND_PERIOD], true);
	FLOAT_INTERFACE(TanExpand, Accels[PlayerOptions::ACCEL_TAN_EXPAND], true);
	FLOAT_INTERFACE(TanExpandPeriod, Accels[PlayerOptions::ACCEL_TAN_EXPAND_PERIOD], true);
	FLOAT_INTERFACE(Boomerang, Accels[PlayerOptions::ACCEL_BOOMERANG], true);
	FLOAT_INTERFACE(Drunk, Effects[PlayerOptions::EFFECT_DRUNK], true);
	FLOAT_INTERFACE(DrunkSpeed, Effects[PlayerOptions::EFFECT_DRUNK_SPEED], true);
	FLOAT_INTERFACE(DrunkOffset, Effects[PlayerOptions::EFFECT_DRUNK_OFFSET], true);
	FLOAT_INTERFACE(DrunkPeriod, Effects[PlayerOptions::EFFECT_DRUNK_PERIOD], true);
	FLOAT_INTERFACE(TanDrunk, Effects[PlayerOptions::EFFECT_TAN_DRUNK], true);
	FLOAT_INTERFACE(TanDrunkSpeed, Effects[PlayerOptions::EFFECT_TAN_DRUNK_SPEED], true);
	FLOAT_INTERFACE(TanDrunkOffset, Effects[PlayerOptions::EFFECT_TAN_DRUNK_OFFSET], true);
	FLOAT_INTERFACE(TanDrunkPeriod, Effects[PlayerOptions::EFFECT_TAN_DRUNK_PERIOD], true);
	FLOAT_INTERFACE(DrunkZ, Effects[PlayerOptions::EFFECT_DRUNK_Z], true);
	FLOAT_INTERFACE(DrunkZSpeed, Effects[PlayerOptions::EFFECT_DRUNK_Z_SPEED], true);
	FLOAT_INTERFACE(DrunkZOffset, Effects[PlayerOptions::EFFECT_DRUNK_Z_OFFSET], true);
	FLOAT_INTERFACE(DrunkZPeriod, Effects[PlayerOptions::EFFECT_DRUNK_Z_PERIOD], true);
	FLOAT_INTERFACE(TanDrunkZ, Effects[PlayerOptions::EFFECT_TAN_DRUNK_Z], true);
	FLOAT_INTERFACE(TanDrunkZSpeed, Effects[PlayerOptions::EFFECT_TAN_DRUNK_Z_SPEED], true);
	FLOAT_INTERFACE(TanDrunkZOffset, Effects[PlayerOptions::EFFECT_TAN_DRUNK_Z_OFFSET], true);
	FLOAT_INTERFACE(TanDrunkZPeriod, Effects[PlayerOptions::EFFECT_TAN_DRUNK_Z_PERIOD], true);
	FLOAT_INTERFACE(Dizzy, Effects[PlayerOptions::EFFECT_DIZZY], true);
	FLOAT_INTERFACE(AttenuateX, Effects[PlayerOptions::EFFECT_ATTENUATE_X], true);
	FLOAT_INTERFACE(AttenuateY, Effects[PlayerOptions::EFFECT_ATTENUATE_Y], true);
	FLOAT_INTERFACE(AttenuateZ, Effects[PlayerOptions::EFFECT_ATTENUATE_Z], true);
	FLOAT_INTERFACE(ShrinkLinear, Effects[PlayerOptions::EFFECT_SHRINK_TO_LINEAR], true);
	FLOAT_INTERFACE(ShrinkMult, Effects[PlayerOptions::EFFECT_SHRINK_TO_MULT], true);
	FLOAT_INTERFACE(PulseInner, Effects[PlayerOptions::EFFECT_PULSE_INNER], true);
	FLOAT_INTERFACE(PulseOuter, Effects[PlayerOptions::EFFECT_PULSE_OUTER], true);
	FLOAT_INTERFACE(PulsePeriod, Effects[PlayerOptions::EFFECT_PULSE_PERIOD], true);
	FLOAT_INTERFACE(PulseOffset, Effects[PlayerOptions::EFFECT_PULSE_OFFSET], true);
	FLOAT_INTERFACE(Confusion, Effects[PlayerOptions::EFFECT_CONFUSION], true);
	FLOAT_INTERFACE(ConfusionOffset, Effects[PlayerOptions::EFFECT_CONFUSION_OFFSET], true);
	FLOAT_INTERFACE(ConfusionX, Effects[PlayerOptions::EFFECT_CONFUSION_X], true);
	FLOAT_INTERFACE(ConfusionXOffset, Effects[PlayerOptions::EFFECT_CONFUSION_X_OFFSET], true);
	FLOAT_INTERFACE(ConfusionY, Effects[PlayerOptions::EFFECT_CONFUSION_Y], true);
	FLOAT_INTERFACE(ConfusionYOffset, Effects[PlayerOptions::EFFECT_CONFUSION_Y_OFFSET], true);
	FLOAT_INTERFACE(Bounce, Effects[PlayerOptions::EFFECT_BOUNCE], true);
	FLOAT_INTERFACE(BouncePeriod, Effects[PlayerOptions::EFFECT_BOUNCE_PERIOD], true);
	FLOAT_INTERFACE(BounceOffset, Effects[PlayerOptions::EFFECT_BOUNCE_OFFSET], true);
	FLOAT_INTERFACE(BounceZ, Effects[PlayerOptions::EFFECT_BOUNCE_Z], true);
	FLOAT_INTERFACE(BounceZPeriod, Effects[PlayerOptions::EFFECT_BOUNCE_Z_PERIOD], true);
	FLOAT_INTERFACE(BounceZOffset, Effects[PlayerOptions::EFFECT_BOUNCE_Z_OFFSET], true);
	FLOAT_INTERFACE(Mini, Effects[PlayerOptions::EFFECT_MINI], true);
	FLOAT_INTERFACE(Tiny, Effects[PlayerOptions::EFFECT_TINY], true);
	FLOAT_INTERFACE(Flip, Effects[PlayerOptions::EFFECT_FLIP], true);
	FLOAT_INTERFACE(Invert, Effects[PlayerOptions::EFFECT_INVERT], true);
	FLOAT_INTERFACE(Tornado, Effects[PlayerOptions::EFFECT_TORNADO], true);
	FLOAT_INTERFACE(TornadoPeriod, Effects[PlayerOptions::EFFECT_TORNADO_PERIOD], true);
	FLOAT_INTERFACE(TornadoOffset, Effects[PlayerOptions::EFFECT_TORNADO_OFFSET], true);
	FLOAT_INTERFACE(TanTornado, Effects[PlayerOptions::EFFECT_TAN_TORNADO], true);
	FLOAT_INTERFACE(TanTornadoPeriod, Effects[PlayerOptions::EFFECT_TAN_TORNADO_PERIOD], true);
	FLOAT_INTERFACE(TanTornadoOffset, Effects[PlayerOptions::EFFECT_TAN_TORNADO_OFFSET], true);
	FLOAT_INTERFACE(TornadoZ, Effects[PlayerOptions::EFFECT_TORNADO_Z], true);
	FLOAT_INTERFACE(TornadoZPeriod, Effects[PlayerOptions::EFFECT_TORNADO_Z_PERIOD], true);
	FLOAT_INTERFACE(TornadoZOffset, Effects[PlayerOptions::EFFECT_TORNADO_Z_OFFSET], true);
	FLOAT_INTERFACE(TanTornadoZ, Effects[PlayerOptions::EFFECT_TAN_TORNADO_Z], true);
	FLOAT_INTERFACE(TanTornadoZPeriod, Effects[PlayerOptions::EFFECT_TAN_TORNADO_Z_PERIOD], true);
	FLOAT_INTERFACE(TanTornadoZOffset, Effects[PlayerOptions::EFFECT_TAN_TORNADO_Z_OFFSET], true);
	FLOAT_INTERFACE(Tipsy, Effects[PlayerOptions::EFFECT_TIPSY], true);
	FLOAT_INTERFACE(TipsySpeed, Effects[PlayerOptions::EFFECT_TIPSY_SPEED], true);
	FLOAT_INTERFACE(TipsyOffset, Effects[PlayerOptions::EFFECT_TIPSY_OFFSET], true);
	FLOAT_INTERFACE(TanTipsy, Effects[PlayerOptions::EFFECT_TAN_TIPSY], true);
	FLOAT_INTERFACE(TanTipsySpeed, Effects[PlayerOptions::EFFECT_TAN_TIPSY_SPEED], true);
	FLOAT_INTERFACE(TanTipsyOffset, Effects[PlayerOptions::EFFECT_TAN_TIPSY_OFFSET], true);
	FLOAT_INTERFACE(Bumpy, Effects[PlayerOptions::EFFECT_BUMPY], true);
	FLOAT_INTERFACE(BumpyOffset, Effects[PlayerOptions::EFFECT_BUMPY_OFFSET], true);
	FLOAT_INTERFACE(BumpyPeriod, Effects[PlayerOptions::EFFECT_BUMPY_PERIOD], true);
	FLOAT_INTERFACE(TanBumpy, Effects[PlayerOptions::EFFECT_TAN_BUMPY], true);
	FLOAT_INTERFACE(TanBumpyOffset, Effects[PlayerOptions::EFFECT_TAN_BUMPY_OFFSET], true);
	FLOAT_INTERFACE(TanBumpyPeriod, Effects[PlayerOptions::EFFECT_TAN_BUMPY_PERIOD], true);
	FLOAT_INTERFACE(BumpyX, Effects[PlayerOptions::EFFECT_BUMPY_X], true);
	FLOAT_INTERFACE(BumpyXOffset, Effects[PlayerOptions::EFFECT_BUMPY_X_OFFSET], true);
	FLOAT_INTERFACE(BumpyXPeriod, Effects[PlayerOptions::EFFECT_BUMPY_X_PERIOD], true);
	FLOAT_INTERFACE(TanBumpyX, Effects[PlayerOptions::EFFECT_TAN_BUMPY_X], true);
	FLOAT_INTERFACE(TanBumpyXOffset, Effects[PlayerOptions::EFFECT_TAN_BUMPY_X_OFFSET], true);
	FLOAT_INTERFACE(TanBumpyXPeriod, Effects[PlayerOptions::EFFECT_TAN_BUMPY_X_PERIOD], true);
	FLOAT_INTERFACE(Beat, Effects[PlayerOptions::EFFECT_BEAT], true);
	FLOAT_INTERFACE(BeatOffset, Effects[PlayerOptions::EFFECT_BEAT_OFFSET], true);
	FLOAT_INTERFACE(BeatPeriod, Effects[PlayerOptions::EFFECT_BEAT_PERIOD], true);
	FLOAT_INTERFACE(BeatMult, Effects[PlayerOptions::EFFECT_BEAT_MULT], true);
	FLOAT_INTERFACE(BeatY, Effects[PlayerOptions::EFFECT_BEAT_Y], true);
	FLOAT_INTERFACE(BeatYOffset, Effects[PlayerOptions::EFFECT_BEAT_Y_OFFSET], true);
	FLOAT_INTERFACE(BeatYPeriod, Effects[PlayerOptions::EFFECT_BEAT_Y_PERIOD], true);
	FLOAT_INTERFACE(BeatYMult, Effects[PlayerOptions::EFFECT_BEAT_Y_MULT], true);
	FLOAT_INTERFACE(BeatZ, Effects[PlayerOptions::EFFECT_BEAT_Z], true);
	FLOAT_INTERFACE(BeatZOffset, Effects[PlayerOptions::EFFECT_BEAT_Z_OFFSET], true);
	FLOAT_INTERFACE(BeatZPeriod, Effects[PlayerOptions::EFFECT_BEAT_Z_PERIOD], true);
	FLOAT_INTERFACE(BeatZMult, Effects[PlayerOptions::EFFECT_BEAT_Z_MULT], true);
	FLOAT_INTERFACE(Zigzag, Effects[PlayerOptions::EFFECT_ZIGZAG], true);
	FLOAT_INTERFACE(ZigzagPeriod, Effects[PlayerOptions::EFFECT_ZIGZAG_PERIOD], true);
	FLOAT_INTERFACE(ZigzagOffset, Effects[PlayerOptions::EFFECT_ZIGZAG_OFFSET], true);
	FLOAT_INTERFACE(ZigzagZ, Effects[PlayerOptions::EFFECT_ZIGZAG_Z], true);
	FLOAT_INTERFACE(ZigzagZPeriod, Effects[PlayerOptions::EFFECT_ZIGZAG_Z_PERIOD], true);
	FLOAT_INTERFACE(ZigzagZOffset, Effects[PlayerOptions::EFFECT_ZIGZAG_Z_OFFSET], true);
	FLOAT_INTERFACE(Sawtooth, Effects[PlayerOptions::EFFECT_SAWTOOTH], true);
	FLOAT_INTERFACE(SawtoothPeriod, Effects[PlayerOptions::EFFECT_SAWTOOTH_PERIOD], true);
	FLOAT_INTERFACE(SawtoothZ, Effects[PlayerOptions::EFFECT_SAWTOOTH_Z], true);
	FLOAT_INTERFACE(SawtoothZPeriod, Effects[PlayerOptions::EFFECT_SAWTOOTH_Z_PERIOD], true);
	FLOAT_INTERFACE(Square, Effects[PlayerOptions::EFFECT_SQUARE], true);
	FLOAT_INTERFACE(SquareOffset, Effects[PlayerOptions::EFFECT_SQUARE_OFFSET], true);
	FLOAT_INTERFACE(SquarePeriod, Effects[PlayerOptions::EFFECT_SQUARE_PERIOD], true);
	FLOAT_INTERFACE(SquareZ, Effects[PlayerOptions::EFFECT_SQUARE_Z], true);
	FLOAT_INTERFACE(SquareZOffset, Effects[PlayerOptions::EFFECT_SQUARE_Z_OFFSET], true);
	FLOAT_INTERFACE(SquareZPeriod, Effects[PlayerOptions::EFFECT_SQUARE_Z_PERIOD], true);
	FLOAT_INTERFACE(Digital, Effects[PlayerOptions::EFFECT_DIGITAL], true);
	FLOAT_INTERFACE(DigitalSteps, Effects[PlayerOptions::EFFECT_DIGITAL_STEPS], true);
	FLOAT_INTERFACE(DigitalPeriod, Effects[PlayerOptions::EFFECT_DIGITAL_PERIOD], true);
	FLOAT_INTERFACE(DigitalOffset, Effects[PlayerOptions::EFFECT_DIGITAL_OFFSET], true);
	FLOAT_INTERFACE(TanDigital, Effects[PlayerOptions::EFFECT_TAN_DIGITAL], true);
	FLOAT_INTERFACE(TanDigitalSteps, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_STEPS], true);
	FLOAT_INTERFACE(TanDigitalPeriod, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_PERIOD], true);
	FLOAT_INTERFACE(TanDigitalOffset, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_OFFSET], true);
	FLOAT_INTERFACE(DigitalZ, Effects[PlayerOptions::EFFECT_DIGITAL_Z], true);
	FLOAT_INTERFACE(DigitalZSteps, Effects[PlayerOptions::EFFECT_DIGITAL_Z_STEPS], true);
	FLOAT_INTERFACE(DigitalZPeriod, Effects[PlayerOptions::EFFECT_DIGITAL_Z_PERIOD], true);
	FLOAT_INTERFACE(DigitalZOffset, Effects[PlayerOptions::EFFECT_DIGITAL_Z_OFFSET], true);
	FLOAT_INTERFACE(TanDigitalZ, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_Z], true);
	FLOAT_INTERFACE(TanDigitalZSteps, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_STEPS], true);
	FLOAT_INTERFACE(TanDigitalZPeriod, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_PERIOD], true);
	FLOAT_INTERFACE(TanDigitalZOffset, Effects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_OFFSET], true);
	FLOAT_INTERFACE(ParabolaX, Effects[PlayerOptions::EFFECT_PARABOLA_X], true);
	FLOAT_INTERFACE(ParabolaY, Effects[PlayerOptions::EFFECT_PARABOLA_Y], true);
	FLOAT_INTERFACE(ParabolaZ, Effects[PlayerOptions::EFFECT_PARABOLA_Z], true);
	FLOAT_INTERFACE(Xmode, Effects[PlayerOptions::EFFECT_XMODE], true);
	FLOAT_INTERFACE(Twirl, Effects[PlayerOptions::EFFECT_TWIRL], true);
	FLOAT_INTERFACE(Roll, Effects[PlayerOptions::EFFECT_ROLL], true);
	FLOAT_INTERFACE(Hidden, Appearances[PlayerOptions::APPEARANCE_HIDDEN], true);
	FLOAT_INTERFACE(HiddenOffset, Appearances[PlayerOptions::APPEARANCE_HIDDEN_OFFSET], true);
	FLOAT_INTERFACE(Sudden, Appearances[PlayerOptions::APPEARANCE_SUDDEN], true);
	FLOAT_INTERFACE(SuddenOffset, Appearances[PlayerOptions::APPEARANCE_SUDDEN_OFFSET], true);
	FLOAT_INTERFACE(Stealth, Appearances[PlayerOptions::APPEARANCE_STEALTH], true);
	FLOAT_INTERFACE(Blink, Appearances[PlayerOptions::APPEARANCE_BLINK], true);
	FLOAT_INTERFACE(RandomVanish, Appearances[PlayerOptions::APPEARANCE_RANDOMVANISH], true);
	FLOAT_INTERFACE(Reverse, Scrolls[PlayerOptions::SCROLL_REVERSE], true);
	FLOAT_INTERFACE(Split, Scrolls[PlayerOptions::SCROLL_SPLIT], true);
	FLOAT_INTERFACE(Alternate, Scrolls[PlayerOptions::SCROLL_ALTERNATE], true);
	FLOAT_INTERFACE(Cross, Scrolls[PlayerOptions::SCROLL_CROSS], true);
	FLOAT_INTERFACE(Centered, Scrolls[PlayerOptions::SCROLL_CENTERED], true);
	FLOAT_INTERFACE(Dark, Dark, true);
	FLOAT_INTERFACE(Blind, Blind, true);
	FLOAT_INTERFACE(Cover, Cover, true);
	FLOAT_INTERFACE(RandAttack, RandAttack, true);
	FLOAT_INTERFACE(NoAttack, NoAttack, true);
	FLOAT_INTERFACE(PlayerAutoPlay, PlayerAutoPlay, true);
	FLOAT_INTERFACE(Skew, Skew, true);
	FLOAT_INTERFACE(Tilt, PerspectiveTilt, true);
	FLOAT_INTERFACE(Passmark, Passmark, true); // Passmark is not sanity checked to the [0, 1] range because LifeMeterBar::IsFailing is the only thing that uses it, and it's used in a <= test.  Any theme passing a value outside the [0, 1] range probably expects the result they get. -Kyz
	FLOAT_INTERFACE(RandomSpeed, RandomSpeed, true);

	MULTICOL_FLOAT_INTERFACE(MoveX, MovesX, true);
	MULTICOL_FLOAT_INTERFACE(MoveY, MovesY, true);
	MULTICOL_FLOAT_INTERFACE(MoveZ, MovesZ, true);
	MULTICOL_FLOAT_INTERFACE(ConfusionXOffset, ConfusionX, true);
	MULTICOL_FLOAT_INTERFACE(ConfusionYOffset, ConfusionY, true);
	MULTICOL_FLOAT_INTERFACE(ConfusionOffset, ConfusionZ, true);
	MULTICOL_FLOAT_INTERFACE(Dark, Darks, true);
	MULTICOL_FLOAT_INTERFACE(Stealth, Stealth, true);
	MULTICOL_FLOAT_INTERFACE(Tiny, Tiny, true);
	MULTICOL_FLOAT_INTERFACE(Bumpy, Bumpy, true);
	MULTICOL_FLOAT_INTERFACE(Reverse, Reverse, true);

	BOOL_INTERFACE(StealthType, StealthType);
	BOOL_INTERFACE(StealthPastReceptors, StealthPastReceptors);
	BOOL_INTERFACE(DizzyHolds, DizzyHolds);
	BOOL_INTERFACE(ZBuffer, ZBuffer);
	BOOL_INTERFACE(Cosecant, Cosecant);
	BOOL_INTERFACE(TurnNone, Turns[PlayerOptions::TURN_NONE]);
	BOOL_INTERFACE(Mirror, Turns[PlayerOptions::TURN_MIRROR]);
	BOOL_INTERFACE(Backwards, Turns[PlayerOptions::TURN_BACKWARDS]);
	BOOL_INTERFACE(Left, Turns[PlayerOptions::TURN_LEFT]);
	BOOL_INTERFACE(Right, Turns[PlayerOptions::TURN_RIGHT]);
	BOOL_INTERFACE(Shuffle, Turns[PlayerOptions::TURN_SHUFFLE]);
	BOOL_INTERFACE(SoftShuffle, Turns[PlayerOptions::TURN_SOFT_SHUFFLE]);
	BOOL_INTERFACE(SuperShuffle, Turns[PlayerOptions::TURN_SUPER_SHUFFLE]);
	BOOL_INTERFACE(NoHolds, Transforms[PlayerOptions::TRANSFORM_NOHOLDS]);
	BOOL_INTERFACE(NoRolls, Transforms[PlayerOptions::TRANSFORM_NOROLLS]);
	BOOL_INTERFACE(NoMines, Transforms[PlayerOptions::TRANSFORM_NOMINES]);
	BOOL_INTERFACE(Little, Transforms[PlayerOptions::TRANSFORM_LITTLE]);
	BOOL_INTERFACE(Wide, Transforms[PlayerOptions::TRANSFORM_WIDE]);
	BOOL_INTERFACE(Big, Transforms[PlayerOptions::TRANSFORM_BIG]);
	BOOL_INTERFACE(Quick, Transforms[PlayerOptions::TRANSFORM_QUICK]);
	BOOL_INTERFACE(BMRize, Transforms[PlayerOptions::TRANSFORM_BMRIZE]);
	BOOL_INTERFACE(Skippy, Transforms[PlayerOptions::TRANSFORM_SKIPPY]);
	BOOL_INTERFACE(Mines, Transforms[PlayerOptions::TRANSFORM_MINES]);
	BOOL_INTERFACE(AttackMines, Transforms[PlayerOptions::TRANSFORM_ATTACKMINES]);
	BOOL_INTERFACE(Echo, Transforms[PlayerOptions::TRANSFORM_ECHO]);
	BOOL_INTERFACE(Stomp, Transforms[PlayerOptions::TRANSFORM_STOMP]);
	BOOL_INTERFACE(Planted, Transforms[PlayerOptions::TRANSFORM_PLANTED]);
	BOOL_INTERFACE(Floored, Transforms[PlayerOptions::TRANSFORM_FLOORED]);
	BOOL_INTERFACE(Twister, Transforms[PlayerOptions::TRANSFORM_TWISTER]);
	BOOL_INTERFACE(HoldRolls, Transforms[PlayerOptions::TRANSFORM_HOLDROLLS]);
	BOOL_INTERFACE(NoJumps, Transforms[PlayerOptions::TRANSFORM_NOJUMPS]);
	BOOL_INTERFACE(NoHands, Transforms[PlayerOptions::TRANSFORM_NOHANDS]);
	BOOL_INTERFACE(NoLifts, Transforms[PlayerOptions::TRANSFORM_NOLIFTS]);
	BOOL_INTERFACE(NoFakes, Transforms[PlayerOptions::TRANSFORM_NOFAKES]);
	BOOL_INTERFACE(NoQuads, Transforms[PlayerOptions::TRANSFORM_NOQUADS]);
	BOOL_INTERFACE(NoStretch, Transforms[PlayerOptions::TRANSFORM_NOSTRETCH]);
	BOOL_INTERFACE(MuteOnError, MuteOnError);
	ENUM_INTERFACE(FailSetting, FailType, FailType);
	ENUM_INTERFACE(MinTNSToHideNotes, MinTNSToHideNotes, TapNoteScore);

	// NoteSkins
	static int NoteSkin(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if( p->m_sNoteSkin.empty()  )
		{
			lua_pushstring( L, CommonMetrics::DEFAULT_NOTESKIN_NAME.GetValue() );
		}
		else
		{
			lua_pushstring( L, p->m_sNoteSkin );
		}
		if(original_top >= 1 && lua_isstring(L, 1))
		{
			RString skin= SArg(1);
			if(NOTESKIN->DoesNoteSkinExist(skin))
			{
				p->m_sNoteSkin = skin;
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushnil(L);
			}
		}
		else
		{
			lua_pushnil(L);
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static void SetSpeedModApproaches(T* p, float speed)
	{
		p->m_SpeedfScrollBPM= speed;
		p->m_SpeedfScrollSpeed= speed;
		p->m_SpeedfMaxScrollBPM= speed;
		p->m_SpeedfTimeSpacing= speed;
	}

	// Speed Mods
	// Sanity checked functions for speed mods, for themes that want to use the
	// engine's enforcement of sane separation between speed mod types.
	static int CMod(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if(p->m_fTimeSpacing)
		{
			lua_pushnumber(L, p->m_fScrollBPM);
			lua_pushnumber(L, p->m_SpeedfScrollBPM);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(original_top >= 1 && lua_isnumber(L, 1))
		{
			float speed= FArg(1);
			if(!isfinite(speed) || speed <= 0.0f)
			{
				luaL_error(L, "CMod speed must be finite and greater than 0.");
			}
			p->m_fScrollBPM= speed;
			p->m_fTimeSpacing = 1;
			p->m_fScrollSpeed = 1;
			p->m_fMaxScrollBPM = 0;
		}
		if(original_top >= 2 && lua_isnumber(L, 2))
		{
			SetSpeedModApproaches(p, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static int XMod(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if(!p->m_fTimeSpacing)
		{
			lua_pushnumber(L, p->m_fScrollSpeed);
			lua_pushnumber(L, p->m_SpeedfScrollSpeed);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			p->m_fScrollSpeed = FArg(1);
			p->m_fTimeSpacing = 0;
			p->m_fScrollBPM= CMOD_DEFAULT;
			p->m_fMaxScrollBPM = 0;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetSpeedModApproaches(p, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static int MMod(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if(!p->m_fTimeSpacing && p->m_fMaxScrollBPM)
		{
			lua_pushnumber(L, p->m_fMaxScrollBPM);
			lua_pushnumber(L, p->m_SpeedfMaxScrollBPM);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			float speed= FArg(1);
			if(!isfinite(speed) || speed <= 0.0f)
			{
				luaL_error(L, "MMod speed must be finite and greater than 0.");
			}
			p->m_fScrollBPM= CMOD_DEFAULT;
			p->m_fTimeSpacing = 0;
			p->m_fScrollSpeed= 1;
			p->m_fMaxScrollBPM = speed;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetSpeedModApproaches(p, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static void SetPerspectiveApproach(T* p, lua_State* L, float speed)
	{
		p->m_SpeedfPerspectiveTilt= speed;
		p->m_SpeedfSkew= speed;
	}

	static int Overhead(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		lua_pushboolean(L, (p->m_fPerspectiveTilt == 0.0f && p->m_fSkew == 0.0f));
		if(lua_toboolean(L, 1))
		{
			p->m_fPerspectiveTilt= 0;
			p->m_fSkew= 0;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetPerspectiveApproach(p, L, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 1;
	}

	static int Incoming(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if((p->m_fSkew > 0.0f && p->m_fPerspectiveTilt < 0.0f) ||
			(p->m_fSkew < 0.0f && p->m_fPerspectiveTilt > 0.0f))
		{
			lua_pushnumber(L, p->m_fSkew);
			lua_pushnumber(L, p->m_SpeedfSkew);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			float value= FArg(1);
			p->m_fPerspectiveTilt= -value;
			p->m_fSkew= value;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetPerspectiveApproach(p, L, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static int Space(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if((p->m_fSkew > 0.0f && p->m_fPerspectiveTilt > 0.0f) ||
			(p->m_fSkew < 0.0f && p->m_fPerspectiveTilt < 0.0f))
		{
			lua_pushnumber(L, p->m_fSkew);
			lua_pushnumber(L, p->m_SpeedfSkew);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			float value= FArg(1);
			p->m_fPerspectiveTilt= value;
			p->m_fSkew= value;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetPerspectiveApproach(p, L, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static int Hallway(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if(p->m_fSkew == 0.0f && p->m_fPerspectiveTilt < 0.0f)
		{
			lua_pushnumber(L, -p->m_fPerspectiveTilt);
			lua_pushnumber(L, p->m_SpeedfPerspectiveTilt);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			p->m_fPerspectiveTilt= -FArg(1);
			p->m_fSkew= 0;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetPerspectiveApproach(p, L, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	static int Distant(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		if(p->m_fSkew == 0.0f && p->m_fPerspectiveTilt > 0.0f)
		{
			lua_pushnumber(L, p->m_fPerspectiveTilt);
			lua_pushnumber(L, p->m_SpeedfPerspectiveTilt);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			p->m_fPerspectiveTilt= FArg(1);
			p->m_fSkew= 0;
		}
		if(lua_isnumber(L, 2) && original_top >= 2)
		{
			SetPerspectiveApproach(p, L, FArgGTEZero(L, 2));
		}
		OPTIONAL_RETURN_SELF(original_top);
		return 2;
	}

	DEFINE_METHOD( UsingReverse, m_fScrolls[PlayerOptions::SCROLL_REVERSE] == 1.0f );

	static int GetReversePercentForColumn( T *p, lua_State *L )
	{
		const int colNum = IArg(1);
		const int numColumns = GAMESTATE->GetCurrentStyle(p->m_pn)->m_iColsPerPlayer;

		// We don't want to go outside the bounds.
		if(colNum < 0 || colNum > numColumns)
			lua_pushnil(L);
		else
			lua_pushnumber( L, p->GetReversePercentForColumn(colNum) );

		return 1;
	}

	static int GetStepAttacks( T *p, lua_State *L )
	{
		lua_pushnumber(L,
			(p->m_fNoAttack > 0 || p->m_fRandAttack > 0 ? false : true ));
		return 1;
	}

	static int FromString(T* p, lua_State* L)
	{
		p->FromString(SArg(1));
		COMMON_RETURN_SELF;
	}

	LunaPlayerOptions()
	{
		ADD_METHOD( IsEasierForSongAndSteps );
		ADD_METHOD( IsEasierForCourseAndTrail );

		ADD_METHOD(LifeSetting);
		ADD_METHOD(DrainSetting);
		ADD_METHOD(ModTimerSetting);
		ADD_METHOD(ModTimerMult);
		ADD_METHOD(ModTimerOffset);
		ADD_METHOD(DrawSize);
		ADD_METHOD(DrawSizeBack);
		ADD_METHOD(BatteryLives);
		ADD_METHOD(TimeSpacing);
		ADD_METHOD(MaxScrollBPM);
		ADD_METHOD(ScrollSpeed);
		ADD_METHOD(ScrollBPM);
		ADD_METHOD(Boost);
		ADD_METHOD(Brake);
		ADD_METHOD(Wave);
		ADD_METHOD(WavePeriod);
		ADD_METHOD(Expand);
		ADD_METHOD(ExpandPeriod);
		ADD_METHOD(TanExpand);
		ADD_METHOD(TanExpandPeriod);
		ADD_METHOD(Boomerang);
		ADD_METHOD(Drunk);
		ADD_METHOD(DrunkSpeed);
		ADD_METHOD(DrunkOffset);
		ADD_METHOD(DrunkPeriod);
		ADD_METHOD(TanDrunk);
		ADD_METHOD(TanDrunkSpeed);
		ADD_METHOD(TanDrunkOffset);
		ADD_METHOD(TanDrunkPeriod);
		ADD_METHOD(DrunkZ);
		ADD_METHOD(DrunkZSpeed);
		ADD_METHOD(DrunkZOffset);
		ADD_METHOD(DrunkZPeriod);
		ADD_METHOD(TanDrunkZ);
		ADD_METHOD(TanDrunkZSpeed);
		ADD_METHOD(TanDrunkZOffset);
		ADD_METHOD(TanDrunkZPeriod);
		ADD_METHOD(Dizzy);
		ADD_METHOD(ShrinkLinear);
		ADD_METHOD(ShrinkMult);
		ADD_METHOD(PulseInner);
		ADD_METHOD(PulseOuter);
		ADD_METHOD(PulseOffset);
		ADD_METHOD(PulsePeriod);
		ADD_METHOD(AttenuateX);
		ADD_METHOD(AttenuateY);
		ADD_METHOD(AttenuateZ);
		ADD_METHOD(Confusion);
		ADD_METHOD(ConfusionOffset);
		ADD_METHOD(ConfusionX);
		ADD_METHOD(ConfusionXOffset);
		ADD_METHOD(ConfusionY);
		ADD_METHOD(ConfusionYOffset);
		ADD_METHOD(Bounce);
		ADD_METHOD(BouncePeriod);
		ADD_METHOD(BounceOffset);
		ADD_METHOD(BounceZ);
		ADD_METHOD(BounceZPeriod);
		ADD_METHOD(BounceZOffset);
		ADD_METHOD(Mini);
		ADD_METHOD(Tiny);
		ADD_METHOD(Flip);
		ADD_METHOD(Invert);
		ADD_METHOD(Tornado);
		ADD_METHOD(TornadoPeriod);
		ADD_METHOD(TornadoOffset);
		ADD_METHOD(TanTornado);
		ADD_METHOD(TanTornadoPeriod);
		ADD_METHOD(TanTornadoOffset);
		ADD_METHOD(TornadoZ);
		ADD_METHOD(TornadoZPeriod);
		ADD_METHOD(TornadoZOffset);
		ADD_METHOD(TanTornadoZ);
		ADD_METHOD(TanTornadoZPeriod);
		ADD_METHOD(TanTornadoZOffset);
		ADD_METHOD(Tipsy);
		ADD_METHOD(TipsySpeed);
		ADD_METHOD(TipsyOffset);
		ADD_METHOD(TanTipsy);
		ADD_METHOD(TanTipsySpeed);
		ADD_METHOD(TanTipsyOffset);
		ADD_METHOD(Bumpy);
		ADD_METHOD(BumpyOffset);
		ADD_METHOD(BumpyPeriod);
		ADD_METHOD(TanBumpy);
		ADD_METHOD(TanBumpyOffset);
		ADD_METHOD(TanBumpyPeriod);
		ADD_METHOD(BumpyX);
		ADD_METHOD(BumpyXOffset);
		ADD_METHOD(BumpyXPeriod);
		ADD_METHOD(TanBumpyX);
		ADD_METHOD(TanBumpyXOffset);
		ADD_METHOD(TanBumpyXPeriod);
		ADD_METHOD(Beat);
		ADD_METHOD(BeatOffset);
		ADD_METHOD(BeatPeriod);
		ADD_METHOD(BeatMult);
		ADD_METHOD(BeatY);
		ADD_METHOD(BeatYOffset);
		ADD_METHOD(BeatYPeriod);
		ADD_METHOD(BeatYMult);
		ADD_METHOD(BeatZ);
		ADD_METHOD(BeatZOffset);
		ADD_METHOD(BeatZPeriod);
		ADD_METHOD(BeatZMult);
		ADD_METHOD(Zigzag);
		ADD_METHOD(ZigzagPeriod);
		ADD_METHOD(ZigzagOffset);
		ADD_METHOD(ZigzagZ);
		ADD_METHOD(ZigzagZPeriod);
		ADD_METHOD(ZigzagZOffset);
		ADD_METHOD(Sawtooth);
		ADD_METHOD(SawtoothPeriod);
		ADD_METHOD(SawtoothZ);
		ADD_METHOD(SawtoothZPeriod);
		ADD_METHOD(Square);
		ADD_METHOD(SquareOffset);
		ADD_METHOD(SquarePeriod);
		ADD_METHOD(SquareZ);
		ADD_METHOD(SquareZOffset);
		ADD_METHOD(SquareZPeriod);
		ADD_METHOD(Digital);
		ADD_METHOD(DigitalSteps);
		ADD_METHOD(DigitalPeriod);
		ADD_METHOD(DigitalOffset);
		ADD_METHOD(TanDigital);
		ADD_METHOD(TanDigitalSteps);
		ADD_METHOD(TanDigitalPeriod);
		ADD_METHOD(TanDigitalOffset);
		ADD_METHOD(DigitalZ);
		ADD_METHOD(DigitalZSteps);
		ADD_METHOD(DigitalZPeriod);
		ADD_METHOD(DigitalZOffset);
		ADD_METHOD(TanDigitalZ);
		ADD_METHOD(TanDigitalZSteps);
		ADD_METHOD(TanDigitalZPeriod);
		ADD_METHOD(TanDigitalZOffset);
		ADD_METHOD(ParabolaX);
		ADD_METHOD(ParabolaY);
		ADD_METHOD(ParabolaZ);
		ADD_METHOD(Xmode);
		ADD_METHOD(Twirl);
		ADD_METHOD(Roll);
		ADD_METHOD(Hidden);
		ADD_METHOD(HiddenOffset);
		ADD_METHOD(Sudden);
		ADD_METHOD(SuddenOffset);
		ADD_METHOD(Stealth);
		ADD_METHOD(Blink);
		ADD_METHOD(RandomVanish);
		ADD_METHOD(Reverse);
		ADD_METHOD(Split);
		ADD_METHOD(Alternate);
		ADD_METHOD(Cross);
		ADD_METHOD(Centered);
		ADD_METHOD(Dark);
		ADD_METHOD(Blind);
		ADD_METHOD(Cover);
		ADD_METHOD(StealthType);
		ADD_METHOD(StealthPastReceptors);
		ADD_METHOD(DizzyHolds);
		ADD_METHOD(ZBuffer);
		ADD_METHOD(Cosecant);
		ADD_METHOD(RandAttack);
		ADD_METHOD(NoAttack);
		ADD_METHOD(PlayerAutoPlay);
		ADD_METHOD(Tilt);
		ADD_METHOD(Skew);
		ADD_METHOD(Passmark);
		ADD_METHOD(RandomSpeed);
		ADD_METHOD(TurnNone);
		ADD_METHOD(Mirror);
		ADD_METHOD(Backwards);
		ADD_METHOD(Left);
		ADD_METHOD(Right);
		ADD_METHOD(Shuffle);
		ADD_METHOD(SoftShuffle);
		ADD_METHOD(SuperShuffle);
		ADD_METHOD(NoHolds);
		ADD_METHOD(NoRolls);
		ADD_METHOD(NoMines);
		ADD_METHOD(Little);
		ADD_METHOD(Wide);
		ADD_METHOD(Big);
		ADD_METHOD(Quick);
		ADD_METHOD(BMRize);
		ADD_METHOD(Skippy);
		ADD_METHOD(Mines);
		ADD_METHOD(AttackMines);
		ADD_METHOD(Echo);
		ADD_METHOD(Stomp);
		ADD_METHOD(Planted);
		ADD_METHOD(Floored);
		ADD_METHOD(Twister);
		ADD_METHOD(HoldRolls);
		ADD_METHOD(NoJumps);
		ADD_METHOD(NoHands);
		ADD_METHOD(NoLifts);
		ADD_METHOD(NoFakes);
		ADD_METHOD(NoQuads);
		ADD_METHOD(NoStretch);
		ADD_METHOD(MuteOnError);

		ADD_MULTICOL_METHOD(MoveX);
		ADD_MULTICOL_METHOD(MoveY);
		ADD_MULTICOL_METHOD(MoveZ);
		ADD_MULTICOL_METHOD(ConfusionOffset);
		ADD_MULTICOL_METHOD(ConfusionXOffset);
		ADD_MULTICOL_METHOD(ConfusionYOffset);
		ADD_MULTICOL_METHOD(Dark);
		ADD_MULTICOL_METHOD(Stealth);
		ADD_MULTICOL_METHOD(Tiny);
		ADD_MULTICOL_METHOD(Bumpy);
		ADD_MULTICOL_METHOD(Reverse);


		ADD_METHOD(NoteSkin);
		ADD_METHOD(FailSetting);
		ADD_METHOD(MinTNSToHideNotes);

		// Speed
		ADD_METHOD( CMod );
		ADD_METHOD( XMod );
		ADD_METHOD( MMod );

		ADD_METHOD(Overhead);
		ADD_METHOD(Incoming);
		ADD_METHOD(Space);
		ADD_METHOD(Hallway);
		ADD_METHOD(Distant);

		ADD_METHOD( UsingReverse );
		ADD_METHOD( GetReversePercentForColumn );
		ADD_METHOD( GetStepAttacks );

		ADD_METHOD(FromString);
	}
};

LUA_REGISTER_CLASS( PlayerOptions )
// lua end

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
