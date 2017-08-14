#include "global.h"
#include "PlayerOptions.h"
#include "RageMath.hpp"
#include "RageUtil.h"
#include "RageString.hpp"
#include "GameState.h"
#include "Song.h"
#include "Course.h"
#include "Steps.h"
#include "ThemeManager.h"
#include "Style.h"
#include "CommonMetrics.h"

using std::vector;

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
	m_fTilt = 0;		m_SpeedfTilt = 1.0f;
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
	m_bCosecant = false;
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

void PlayerOptions::Approach(PlayerOptions const& other, float delta)
{
	float rated_delta= delta * GAMESTATE->get_hasted_music_rate();
#define APPROACH( opt ) \
	fapproach(m_##opt, other.m_##opt, rated_delta * other.m_Speed ## opt);
#define DO_COPY( x ) \
	x = other.x;

	DO_COPY( m_LifeType );
	DO_COPY( m_DrainType );
	DO_COPY( m_BatteryLives );
	DO_COPY( m_ModTimerType );
	APPROACH( fModTimerMult );
	APPROACH( fModTimerOffset );
	APPROACH( fDrawSize );
	APPROACH( fDrawSizeBack );
	APPROACH( fTimeSpacing );
	APPROACH( fScrollSpeed );
	APPROACH( fMaxScrollBPM );
	fapproach(m_fScrollBPM, other.m_fScrollBPM, rated_delta * other.m_SpeedfScrollBPM*150);
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
	APPROACH( fTilt );
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
	DO_COPY( m_bCosecant );
	DO_COPY( m_FailType );
	DO_COPY( m_MinTNSToHideNotes );
#undef APPROACH
#undef DO_COPY
}

static void AddPart( vector<std::string> &AddTo, float level, std::string name )
{
	if( level == 0 )
	{
		return;
	}
	const std::string LevelStr = (level == 1)? std::string(""): fmt::sprintf( "%ld%% ", std::lrint(level*100) );

	AddTo.push_back( LevelStr + name );
}

std::string PlayerOptions::GetString() const
{
	vector<std::string> v;
	GetMods(v);
	return Rage::join( ", ", v );
}

void PlayerOptions::GetMods(vector<std::string> &AddTo) const
{
	//std::string sReturn;

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
				default: break;
			}
			break;
		case LifeType_Battery:
			AddTo.push_back(fmt::sprintf("%dLives", m_BatteryLives));
			break;
		case LifeType_Time:
			AddTo.push_back("LifeTime");
			break;
		default:
			FAIL_M(fmt::sprintf("Invalid LifeType: %i", m_LifeType));
	}

	if( !m_fTimeSpacing )
	{
		if( m_fMaxScrollBPM )
		{
			std::string s = fmt::sprintf( "m%.0f", m_fMaxScrollBPM );
			AddTo.push_back( s );
		}
		else if( m_bSetScrollSpeed || m_fScrollSpeed != 1 )
		{
			/* -> 1.00 */
			std::string s = fmt::sprintf( "%2.2f", m_fScrollSpeed );
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
		std::string s = fmt::sprintf( "C%.0f", m_fScrollBPM );
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
			FAIL_M(fmt::sprintf("Invalid ModTimerType: %i", m_ModTimerType));
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
	AddPart( AddTo, m_bCosecant,				"Cosecant");
	
	for( int i=0; i<16; i++)
	{
		std::string s = fmt::sprintf( "MoveX%d", i+1 );
		
		AddPart( AddTo, m_fMovesX[i],				s );
		s = fmt::sprintf( "MoveY%d", i+1 );
		AddPart( AddTo, m_fMovesY[i],				s );
		s = fmt::sprintf( "MoveZ%d", i+1 );
		AddPart( AddTo, m_fMovesZ[i],				s );
		s = fmt::sprintf( "ConfusionOffset%d", i+1);
		AddPart( AddTo, m_fConfusionX[i],				s );
		s = fmt::sprintf( "ConfusionYOffset%d", i+1 );
		AddPart( AddTo, m_fConfusionY[i],				s );
		s = fmt::sprintf( "ConfusionZOffset%d", i+1 );
		AddPart( AddTo, m_fConfusionZ[i],				s );
		s = fmt::sprintf( "Dark%d", i+1 );
		AddPart( AddTo, m_fDarks[i],				s );
		s = fmt::sprintf( "Stealth%d", i+1 );
		AddPart( AddTo, m_fStealth[i],				s );
		s = fmt::sprintf( "Tiny%d", i+1 );
		AddPart( AddTo, m_fTiny[i],				s );
		s = fmt::sprintf( "Bumpy%d", i+1 );
		AddPart( AddTo, m_fBumpy[i],				s );
		s = fmt::sprintf( "Reverse%d", i+1 );
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
		FAIL_M(fmt::sprintf("Invalid FailType: %i", m_FailType));
	}

	if( m_fSkew==0 && m_fTilt==0 )
	{
		AddTo.push_back( "Overhead" );
	}
	else if( m_fSkew == 0 )
	{
		if( m_fTilt > 0 )
			AddPart( AddTo, m_fTilt, "Distant" );
		else
			AddPart( AddTo, -m_fTilt, "Hallway" );
	}
	else if( fabsf(m_fSkew-m_fTilt) < 0.0001f )
	{
		AddPart( AddTo, m_fSkew, "Space" );
	}
	else if( fabsf(m_fSkew+m_fTilt) < 0.0001f )
	{
		AddPart( AddTo, m_fSkew, "Incoming" );
	}
	else
	{
		AddPart( AddTo, m_fSkew, "Skew" );
		AddPart( AddTo, m_fTilt, "Tilt" );
	}
}

/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void PlayerOptions::FromString( std::string const &sMultipleMods )
{
	m_changed_defective_mod= false;
	std::string sTemp = sMultipleMods;
	auto vs = Rage::split(sTemp, ",", Rage::EmptyEntries::skip);
	std::string sThrowAway;
	for (auto &s: vs)
	{
		if (!FromOneModString( s, sThrowAway ))
		{
			LOG->Trace( "Attempted to load a non-existing mod \'%s\' for the Player. Ignoring.", s.c_str() );
		}
	}
}

typedef void (*special_option_func_t)(PlayerOptions& options, float level, float speed);

#define SET_FLOAT(opt) {options.m_##opt= level; options.m_Speed##opt= speed;}

static void clearall(PlayerOptions& options, float, float)
{
	options.Init();
}

static void resetspeed(PlayerOptions& options, float level, float speed)
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

static void battery_lives(PlayerOptions& options, float level, float)
{
	// level is a percentage for every other option, so multiply by 100. -Kyz
	options.m_BatteryLives = static_cast<int>(level * 100.0f);
}

static void no_turn(PlayerOptions& options, float level, float)
{
	for(int i= 0; i < PlayerOptions::NUM_TURNS; ++i)
	{
		options.m_bTurns[i]= (level > .5f);
	}
}

static void default_fail(PlayerOptions& options, float, float)
{
	PlayerOptions po;
	GAMESTATE->GetDefaultPlayerOptions( po );
	options.m_FailType = po.m_FailType;
}

static void choose_random(PlayerOptions& options, float, float)
{
	options.ChooseRandomModifiers();
}

#undef SET_FLOAT

bool PlayerOptions::FromOneModString( std::string const &sOneMod, std::string &sErrorOut )
{
	std::string sBit = Rage::trim(Rage::make_lower(sOneMod));
	std::string sMod = "";

	/* "drunk"
	 * "no drunk"
	 * "150% drunk"
	 * "*2 100% drunk": approach at 2x normal speed */

	float level = 1;
	float speed = 1;
	auto asParts = Rage::split(sBit, " ", Rage::EmptyEntries::skip);

	for (auto const &s: asParts)
	{
		if( s == "no" )
		{
			level = 0;
		}
		else if( isdigit(s[0]) || s[0] == '-' )
		{
			/* If the last character is a *, they probably said "123*" when
			 * they meant "*123". */
			if( Rage::ends_with(s, "*") )
			{
				// XXX: We know what they want, is there any reason not to handle it?
				// Yes. We should be strict in handling the format. -Chris
				sErrorOut = fmt::sprintf("Invalid player options \"%s\"; did you mean '*%d'?", s.c_str(), StringToInt(s) );
				return false;
			}
			else
			{
				level = StringToFloat( s ) / 100.0f;
			}
		}
		else if( s[0]=='*' )
		{
			std::sscanf( s.c_str(), "*%f", &speed );
			if( !std::isfinite(speed) )
			{
				speed = 1.0f;
			}
		}
	}

	sBit = asParts.back();

#define SET_FLOAT( opt ) { m_ ## opt = level; m_Speed ## opt = speed; }
	const bool on = (level > 0.5f);

	static Regex mult("^([0-9]+(\\.[0-9]+)?)x$");
	vector<std::string> matches;
	if( mult.Compare(sBit, matches) )
	{
		StringConversion::FromString( matches[0], level );
		SET_FLOAT( fScrollSpeed )
		SET_FLOAT( fTimeSpacing )
		m_fTimeSpacing = 0;
		m_fMaxScrollBPM = 0;
		m_changed_defective_mod= true;
	}
	else if( sscanf( sBit.c_str(), "c%f", &level ) == 1 )
	{
		if( !std::isfinite(level) || level <= 0.0f )
			level = CMOD_DEFAULT;
		SET_FLOAT( fScrollBPM )
		SET_FLOAT( fTimeSpacing )
		m_fTimeSpacing = 1;
		m_fMaxScrollBPM = 0;
		m_changed_defective_mod= true;
	}
	// oITG's m-mods
	else if( sscanf( sBit.c_str(), "m%f", &level ) == 1 )
	{
		// OpenITG doesn't have this block:
		/*
		if( !std::isfinite(level) || level <= 0.0f )
			level = CMOD_DEFAULT;
		*/
		SET_FLOAT( fMaxScrollBPM )
		m_fTimeSpacing = 0;
		m_changed_defective_mod= true;
	}
	else if( sBit.find("move") != sBit.npos)
	{
	    if (sBit.find("x") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = fmt::sprintf( "movex%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesX[i] )
			m_changed_defective_mod= true;
			break;
		    }
		}
	    }
	    else if (sBit.find("y") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = fmt::sprintf( "movey%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesY[i] )
			m_changed_defective_mod= true;
			break;
		    }
		}
	    }
	    else if (sBit.find("z") != sBit.npos)
	    {
		for (int i=0; i<16; i++)
		{
		    sMod = fmt::sprintf( "movez%d", i+1 );
		    if( sBit == sMod)
		    {
			SET_FLOAT( fMovesZ[i] )
			m_changed_defective_mod= true;
			break;
		    }
		}
	    }
	}
	else
	{
		static std::unordered_map<std::string, special_option_func_t> special_options= {
			{"clearall", clearall},
			{"resetspeed", resetspeed},
			{"life", battery_lives},
			{"lives", battery_lives},
			{"turn", no_turn},
			{"faildefault", default_fail},
			{"random", choose_random},
		};
		static std::unordered_map<std::string, LifeType> life_types= {
			{"bar", LifeType_Bar},
			{"batter", LifeType_Battery},
			{"lifetime", LifeType_Time},
		};
		static std::unordered_map<std::string, DrainType> drain_types= {
			{"norecover", DrainType_NoRecover},
			{"suddendeath", DrainType_SuddenDeath},
			{"death", DrainType_SuddenDeath},
			{"normal-drain", DrainType_Normal},
		};
		static std::unordered_map<std::string, FailType> fail_types= {
			{"failarcade", FailType_Immediate},
			{"failimmediate", FailType_Immediate},
			{"failendofsong", FailType_ImmediateContinue},
			{"failimmediatecontinue", FailType_ImmediateContinue},
			{"failatend", FailType_EndOfSong},
			{"failoff", FailType_Off},
			{"failarcade", FailType_Immediate},
		};
		static std::unordered_map<std::string, ModTimerType> mod_timer_types= {
			{"modtimerdefault", ModTimerType_Default},
			{"modtimersong", ModTimerType_Song},
			{"modtimerbeat", ModTimerType_Beat},
			{"modtimergame", ModTimerType_Game},
		};
		static std::unordered_map<std::string, Accel> accel_options= {
			{"boost", ACCEL_BOOST},
			{"brake", ACCEL_BRAKE},
			{"land", ACCEL_BRAKE},
			{"wave", ACCEL_WAVE},
			{"waveperiod", ACCEL_WAVE_PERIOD},
			{"expand", ACCEL_EXPAND},
			{"expandperiod", ACCEL_EXPAND_PERIOD},
			{"tanexpand", ACCEL_TAN_EXPAND},
			{"tanexpandperiod", ACCEL_TAN_EXPAND_PERIOD},
			{"dwiwave", ACCEL_EXPAND},
			{"boomerang", ACCEL_BOOMERANG},
		};
		static std::unordered_map<std::string, Effect> effect_options= {
			{"drunk", EFFECT_DRUNK},
			{"drunkspeed", EFFECT_DRUNK_SPEED},
			{"drunkoffset", EFFECT_DRUNK_OFFSET},
			{"drunkperiod", EFFECT_DRUNK_PERIOD},
			{"tandrunk", EFFECT_TAN_DRUNK},
			{"tandrunkspeed", EFFECT_TAN_DRUNK_SPEED},
			{"tandrunkoffset", EFFECT_TAN_DRUNK_OFFSET},
			{"tandrunkperiod", EFFECT_TAN_DRUNK_PERIOD},
			{"drunkz", EFFECT_DRUNK_Z},
			{"drunkzspeed", EFFECT_DRUNK_Z_SPEED},
			{"drunkzoffset", EFFECT_DRUNK_Z_OFFSET},
			{"drunkzperiod", EFFECT_DRUNK_Z_PERIOD},
			{"tandrunkz", EFFECT_TAN_DRUNK_Z},
			{"tandrunkzspeed", EFFECT_TAN_DRUNK_Z_SPEED},
			{"tandrunkzoffset", EFFECT_TAN_DRUNK_Z_OFFSET},
			{"tandrunkzperiod", EFFECT_TAN_DRUNK_Z_PERIOD},
			{"shrinklinear", EFFECT_SHRINK_TO_LINEAR},
			{"shrinkmult", EFFECT_SHRINK_TO_MULT},
			{"pulseinner", EFFECT_PULSE_INNER},
			{"pulseouter", EFFECT_PULSE_OUTER},
			{"pulseoffset", EFFECT_PULSE_OFFSET},
			{"pulseperiod", EFFECT_PULSE_PERIOD},
			{"dizzy", EFFECT_DIZZY},
			{"confusion", EFFECT_CONFUSION},
			{"confusionoffset", EFFECT_CONFUSION_OFFSET},
			{"confusionx", EFFECT_CONFUSION_X},
			{"confusionxoffset", EFFECT_CONFUSION_X_OFFSET},
			{"confusiony", EFFECT_CONFUSION_Y},
			{"confusionyoffset", EFFECT_CONFUSION_Y_OFFSET},
			{"bounce", EFFECT_BOUNCE},
			{"bounceperiod", EFFECT_BOUNCE_PERIOD},
			{"bounceoffset", EFFECT_BOUNCE_OFFSET},
			{"bouncez", EFFECT_BOUNCE_Z},
			{"bouncezperiod", EFFECT_BOUNCE_Z_PERIOD},
			{"bouncezoffset", EFFECT_BOUNCE_Z_OFFSET},
			{"mini", EFFECT_MINI},
			{"tiny", EFFECT_TINY},
			{"flip", EFFECT_FLIP},
			{"invert", EFFECT_INVERT},
			{"tornado", EFFECT_TORNADO},
			{"tornadoperiod", EFFECT_TORNADO_PERIOD},
			{"tornadooffset", EFFECT_TORNADO_OFFSET},
			{"tantornado", EFFECT_TAN_TORNADO},
			{"tantornadoperiod", EFFECT_TAN_TORNADO_PERIOD},
			{"tantornadooffset", EFFECT_TAN_TORNADO_OFFSET},
			{"tornadoz", EFFECT_TORNADO_Z},
			{"tornadozperiod", EFFECT_TORNADO_Z_PERIOD},
			{"tornadozoffset", EFFECT_TORNADO_Z_OFFSET},
			{"tantornadoz", EFFECT_TAN_TORNADO_Z},
			{"tantornadozperiod", EFFECT_TAN_TORNADO_Z_PERIOD},
			{"tantornadozoffset", EFFECT_TAN_TORNADO_Z_OFFSET},
			{"tipsy", EFFECT_TIPSY},
			{"tipsyspeed", EFFECT_TIPSY_SPEED},
			{"tipsyoffset", EFFECT_TIPSY_OFFSET},
			{"tantipsy", EFFECT_TAN_TIPSY},
			{"tantipsyspeed", EFFECT_TAN_TIPSY_SPEED},
			{"tantipsyoffset", EFFECT_TAN_TIPSY_OFFSET},
			{"bumpy", EFFECT_BUMPY},
			{"bumpyoffset", EFFECT_BUMPY_OFFSET},
			{"bumpyperiod", EFFECT_BUMPY_PERIOD},
			{"tanbumpy", EFFECT_TAN_BUMPY},
			{"tanbumpyoffset", EFFECT_TAN_BUMPY_OFFSET},
			{"tanbumpyperiod", EFFECT_TAN_BUMPY_PERIOD},
			{"bumpyx", EFFECT_BUMPY_X},
			{"bumpyxoffset", EFFECT_BUMPY_X_OFFSET},
			{"bumpyxperiod", EFFECT_BUMPY_X_PERIOD},
			{"tanbumpyx", EFFECT_TAN_BUMPY_X},
			{"tanbumpyxoffset", EFFECT_TAN_BUMPY_X_OFFSET},
			{"tanbumpyxperiod", EFFECT_TAN_BUMPY_X_PERIOD},
			{"beat", EFFECT_BEAT},
			{"beatoffset", EFFECT_BEAT_OFFSET},
			{"beatperiod", EFFECT_BEAT_PERIOD},
			{"beatmult", EFFECT_BEAT_MULT},
			{"beaty", EFFECT_BEAT_Y},
			{"beatyoffset", EFFECT_BEAT_Y_OFFSET},
			{"beatyperiod", EFFECT_BEAT_Y_PERIOD},
			{"beatymult", EFFECT_BEAT_Y_MULT},
			{"beatz", EFFECT_BEAT_Z},
			{"beatzoffset", EFFECT_BEAT_Z_OFFSET},
			{"beatzperiod", EFFECT_BEAT_Z_PERIOD},
			{"beatzmult", EFFECT_BEAT_Z_MULT},
			{"digital", EFFECT_DIGITAL},
			{"digitalsteps", EFFECT_DIGITAL_STEPS},
			{"digitalperiod", EFFECT_DIGITAL_PERIOD},
			{"digitaloffset", EFFECT_DIGITAL_OFFSET},
			{"tandigital", EFFECT_TAN_DIGITAL},
			{"tandigitalsteps", EFFECT_TAN_DIGITAL_STEPS},
			{"tandigitalperiod", EFFECT_TAN_DIGITAL_PERIOD},
			{"tandigitaloffset", EFFECT_TAN_DIGITAL_OFFSET},
			{"digitalz", EFFECT_DIGITAL_Z},
			{"digitalzsteps", EFFECT_DIGITAL_Z_STEPS},
			{"digitalzperiod", EFFECT_DIGITAL_Z_PERIOD},
			{"digitalzoffset", EFFECT_DIGITAL_Z_OFFSET},
			{"tandigitalz", EFFECT_TAN_DIGITAL_Z},
			{"tandigitalzsteps", EFFECT_TAN_DIGITAL_Z_STEPS},
			{"tandigitalzperiod", EFFECT_TAN_DIGITAL_Z_PERIOD},
			{"tandigitalzoffset", EFFECT_TAN_DIGITAL_Z_OFFSET},
			{"zigzag", EFFECT_ZIGZAG},
			{"zigzagoffset", EFFECT_ZIGZAG_OFFSET},
			{"zigzagperiod", EFFECT_ZIGZAG_PERIOD},
			{"zigzagz", EFFECT_ZIGZAG_Z},
			{"zigzagzoffset", EFFECT_ZIGZAG_Z_OFFSET},
			{"zigzagzperiod", EFFECT_ZIGZAG_Z_PERIOD},
			{"sawtooth", EFFECT_SAWTOOTH},
			{"sawtoothperiod", EFFECT_SAWTOOTH_PERIOD},
			{"sawtoothz", EFFECT_SAWTOOTH_Z},
			{"sawtoothzperiod", EFFECT_SAWTOOTH_Z_PERIOD},
			{"square",EFFECT_SQUARE},
			{"squareoffset",EFFECT_SQUARE_OFFSET},
			{"squareperiod",EFFECT_SQUARE_PERIOD},
			{"squarez",EFFECT_SQUARE_Z},
			{"squarezoffset",EFFECT_SQUARE_Z_OFFSET},
			{"squarezperiod",EFFECT_SQUARE_Z_PERIOD},
			{"parabolax", EFFECT_PARABOLA_X},
			{"parabolay", EFFECT_PARABOLA_Y},
			{"parabolaz", EFFECT_PARABOLA_Z},
			{"attenuatex", EFFECT_ATTENUATE_X},
			{"attenuatey", EFFECT_ATTENUATE_Y},
			{"attenuatez", EFFECT_ATTENUATE_Z},
			{"xmode", EFFECT_XMODE},
			{"twirl", EFFECT_TWIRL},
			{"roll", EFFECT_ROLL},
		};
		static std::unordered_map<std::string, Appearance> appear_options= {
			{"hidden", APPEARANCE_HIDDEN},
			{"hiddenoffset", APPEARANCE_HIDDEN_OFFSET},
			{"sudden", APPEARANCE_SUDDEN},
			{"suddenoffset", APPEARANCE_SUDDEN_OFFSET},
			{"stealth", APPEARANCE_STEALTH},
			{"blink", APPEARANCE_BLINK},
			{"randomvanish", APPEARANCE_RANDOMVANISH},
		};
		static std::unordered_map<std::string, Scroll> scroll_options= {
			{"reverse", SCROLL_REVERSE},
			{"split", SCROLL_SPLIT},
			{"alternate", SCROLL_ALTERNATE},
			{"cross", SCROLL_CROSS},
			{"centered", SCROLL_CENTERED},
			{"converge", SCROLL_CENTERED},
		};
		static std::unordered_map<std::string, Turn> turn_options= {
			{"mirror", TURN_MIRROR},
			{"backwards", TURN_BACKWARDS},
			{"left", TURN_LEFT},
			{"right", TURN_RIGHT},
			{"shuffle", TURN_SHUFFLE},
			{"softshuffle", TURN_SOFT_SHUFFLE},
			{"supershuffle", TURN_SUPER_SHUFFLE},
		};
		static std::unordered_map<std::string, Transform> transform_options= {
			{"little", TRANSFORM_LITTLE},
			{"wide", TRANSFORM_WIDE},
			{"big", TRANSFORM_BIG},
			{"quick", TRANSFORM_QUICK},
			{"bmrize", TRANSFORM_BMRIZE},
			{"skippy", TRANSFORM_SKIPPY},
			{"mines", TRANSFORM_MINES},
			{"attackmines", TRANSFORM_ATTACKMINES},
			{"echo", TRANSFORM_ECHO},
			{"stomp", TRANSFORM_STOMP},
			{"planted", TRANSFORM_PLANTED},
			{"floored", TRANSFORM_FLOORED},
			{"twister", TRANSFORM_TWISTER},
			{"holdrolls", TRANSFORM_HOLDROLLS},
			{"nojumps", TRANSFORM_NOJUMPS},
			{"nohands", TRANSFORM_NOHANDS},
			{"noquads", TRANSFORM_NOQUADS},
			{"noholds", TRANSFORM_NOHOLDS},
			{"norolls", TRANSFORM_NOROLLS},
			{"nomines", TRANSFORM_NOMINES},
			{"nostretch", TRANSFORM_NOSTRETCH},
			{"nolifts", TRANSFORM_NOLIFTS},
			{"nofakes", TRANSFORM_NOFAKES},
		};
		static std::unordered_map<std::string, std::pair<float PlayerOptions::*, float PlayerOptions::*> > other_options= {
			{"dark", {&PlayerOptions::m_fDark, &PlayerOptions::m_SpeedfDark}},
			{"blind", {&PlayerOptions::m_fBlind, &PlayerOptions::m_SpeedfBlind}},
			{"cover", {&PlayerOptions::m_fCover, &PlayerOptions::m_SpeedfCover}},
			{"randomattacks", {&PlayerOptions::m_fRandAttack, &PlayerOptions::m_SpeedfRandAttack}},
			{"noattacks", {&PlayerOptions::m_fNoAttack, &PlayerOptions::m_SpeedfNoAttack}},
			{"playerautoplay", {&PlayerOptions::m_fPlayerAutoPlay, &PlayerOptions::m_SpeedfPlayerAutoPlay}},
			{"passmark", {&PlayerOptions::m_fPassmark, &PlayerOptions::m_SpeedfPassmark}},
			{"skew", {&PlayerOptions::m_fSkew, &PlayerOptions::m_SpeedfSkew}},
			{"tilt", {&PlayerOptions::m_fTilt, &PlayerOptions::m_SpeedfTilt}},
			{"randomspeed", {&PlayerOptions::m_fRandomSpeed, &PlayerOptions::m_SpeedfRandomSpeed}},
			{"modtimermult", {&PlayerOptions::m_fModTimerMult, &PlayerOptions::m_SpeedfModTimerMult}},
			{"modtimeroffset", {&PlayerOptions::m_fModTimerOffset, &PlayerOptions::m_SpeedfModTimerOffset}},
			{"drawsize", {&PlayerOptions::m_fDrawSize, &PlayerOptions::m_SpeedfDrawSize}},
			{"drawsizeback", {&PlayerOptions::m_fDrawSizeBack, &PlayerOptions::m_SpeedfDrawSizeBack}},
		};
		static std::unordered_map<std::string, std::pair<float, float> > perspective_options= {
			{"overhead", {0.f, 0.f}},
			{"incoming", {1.f, -1.f}},
			{"space", {1.f, 1.f}},
			{"hallway", {0.f, -1.f}},
			{"distant", {0.f, 1.f}},
		};
		static std::unordered_map<std::string, bool PlayerOptions::* > bool_options= {
			{"stealthtype", &PlayerOptions::m_bStealthType},
			{"stealthpastreceptors", &PlayerOptions::m_bStealthPastReceptors},
			{"cosecant", &PlayerOptions::m_bCosecant},
		};
#define FIND_ENTRY_COMMON_START(option_set) \
		{ auto entry= option_set.find(sBit); if(entry != option_set.end()) {
#define FIND_ENTRY_CLOSE }}
#define FIND_ENTRY_NO_SPEED(option_set, member) \
		FIND_ENTRY_COMMON_START(option_set) \
			member= entry->second; \
			return true; \
		FIND_ENTRY_CLOSE;
#define FIND_ENTRY_DEFECT_ARRAY(option_set, array_name) \
		FIND_ENTRY_COMMON_START(option_set) \
			m_##array_name[entry->second]= level; \
			m_Speed##array_name[entry->second]= speed; \
			m_changed_defective_mod= true; \
		FIND_ENTRY_CLOSE;
#define FIND_ENTRY_BOOL_ARRAY(option_set, array_name) \
		FIND_ENTRY_COMMON_START(option_set) \
			m_##array_name[entry->second]= on; \
		FIND_ENTRY_CLOSE;
		auto special_entry= special_options.find(sBit);
		if(special_entry != special_options.end())
		{
			special_entry->second(*this, level, speed);
			return true;
		}
		FIND_ENTRY_DEFECT_ARRAY(accel_options, fAccels);
		FIND_ENTRY_DEFECT_ARRAY(effect_options, fEffects);
		FIND_ENTRY_DEFECT_ARRAY(appear_options, fAppearances);
		FIND_ENTRY_DEFECT_ARRAY(scroll_options, fScrolls);
		FIND_ENTRY_COMMON_START(other_options)
			(this->*(entry->second.first)) = level;
			(this->*(entry->second.second)) = speed;
			m_changed_defective_mod= true;
		FIND_ENTRY_CLOSE;
		FIND_ENTRY_COMMON_START(bool_options)
			(this->*(entry->second)) = on;
			m_changed_defective_mod= true;
		FIND_ENTRY_CLOSE;
		FIND_ENTRY_COMMON_START(perspective_options)
			m_fSkew= level * entry->second.first;
			m_fTilt= level * entry->second.second;
			m_SpeedfSkew= m_SpeedfTilt= speed;
			m_changed_defective_mod= true;
		FIND_ENTRY_CLOSE;
		FIND_ENTRY_NO_SPEED(life_types, m_LifeType);
		FIND_ENTRY_NO_SPEED(drain_types, m_DrainType);
		FIND_ENTRY_NO_SPEED(fail_types, m_FailType);
		FIND_ENTRY_NO_SPEED(mod_timer_types, m_ModTimerType);
		FIND_ENTRY_BOOL_ARRAY(turn_options, bTurns);
		FIND_ENTRY_BOOL_ARRAY(transform_options, bTransforms);

#undef FIND_ENTRY_COMMON_START
#undef FIND_ENTRY_CLOSE
#undef FIND_ENTRY_NO_SPEED
#undef FIND_ENTRY_DEFECT_ARRAY
#undef FIND_ENTRY_BOOL_ARRAY

		if(sBit.find("dark") != sBit.npos)
		{
			for (int i=0; i<16; i++)
			{
				sMod = fmt::sprintf( "dark%d", i+1 );
				if( sBit == sMod)
				{
					SET_FLOAT( fDarks[i] )
					m_changed_defective_mod= true;
					break;
				}
			}
		}
		else if(sBit.find("reverse") != sBit.npos)
		{
			for (int i=0; i<16; i++)
			{
				sMod = fmt::sprintf( "reverse%d", i+1 );
				if( sBit == sMod)
				{
					SET_FLOAT( fReverse[i] )
					m_changed_defective_mod= true;
					break;
				}
			}
		}
		else if(sBit.find("tiny") != sBit.npos)
		{
			for (int i=0; i<16; i++)
			{
				sMod = fmt::sprintf( "tiny%d", i+1 );
				if( sBit == sMod)
				{
					SET_FLOAT( fTiny[i] )
					m_changed_defective_mod= true;
					break;
				}
			}
		}
		else if(sBit.find("bumpy") != sBit.npos)
		{
			for (int i=0; i<16; i++)
			{
				sMod = fmt::sprintf( "bumpy%d", i+1 );
				if( sBit == sMod)
				{
					SET_FLOAT( fBumpy[i] )
					m_changed_defective_mod= true;
					break;
				}
			}
		}
		else if(sBit.find("stealth") != sBit.npos)
		{
			for (int i=0; i<16; i++)
			{
				sMod = fmt::sprintf( "stealth%d", i+1 );
				if( sBit == sMod)
				{
					SET_FLOAT( fStealth[i] )
					m_changed_defective_mod= true;
					break;
				}
			}
		}
		else if( sBit.find("confusion") != sBit.npos)
		{
			if( sBit.find("x") != sBit.npos)
			{
				for (int i=0; i<16; i++)
				{
					sMod = fmt::sprintf( "confusionxoffset%d", i+1 );
					if( sBit == sMod)
					{
						SET_FLOAT( fConfusionX[i] )
						m_changed_defective_mod= true;
						break;
					}
				}
			}
			if( sBit.find("y") != sBit.npos)
			{
				for (int i=0; i<16; i++)
				{
					sMod = fmt::sprintf( "confusionyoffset%d", i+1 );
					if( sBit == sMod)
					{
						SET_FLOAT( fConfusionY[i] )
						m_changed_defective_mod= true;
						break;
					}
				}
			}
			else
			{
				for (int i=0; i<16; i++)
				{
					sMod = fmt::sprintf( "confusionoffset%d", i+1 );
					if( sBit == sMod)
					{
						SET_FLOAT( fConfusionZ[i] )
						m_changed_defective_mod= true;
						break;
					}
				}
			}
		}
		else if(sBit == "muteonerror")
		{
			m_bMuteOnError = on;
			return true;
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
	switch( (int)m_fTilt )
	{
	case -1:		m_fTilt =  0;	break;
	case  0:		m_fTilt = +1;	break;
	case +1: default:	m_fTilt = -1;	break;
	}
}

void PlayerOptions::ChooseRandomModifiers()
{
	m_changed_defective_mod= true;
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
		f = Rage::scale( f, 1.f, 2.f, 1.f, 0.f );
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
	COMPARE(m_bCosecant);
	COMPARE(m_fDark);
	COMPARE(m_fBlind);
	COMPARE(m_fCover);
	COMPARE(m_fRandAttack);
	COMPARE(m_fNoAttack);
	COMPARE(m_fPlayerAutoPlay);
	COMPARE(m_fTilt);
	COMPARE(m_fSkew);
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
	CPY(m_bCosecant);
	CPY_SPEED(fDark);
	CPY_SPEED(fBlind);
	CPY_SPEED(fCover);
	CPY_SPEED(fRandAttack);
	CPY_SPEED(fNoAttack);
	CPY_SPEED(fPlayerAutoPlay);
	CPY_SPEED(fTilt);
	CPY_SPEED(fSkew);
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
			GAMESTATE->get_curr_song()->GetDisplayBpms( bpms );
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

	auto isEasier = [this](TrailEntry const &e) {
		return e.pSong && this->IsEasierForSongAndSteps(e.pSong, e.pSteps, PLAYER_1);
	};

	return std::any_of(pTrail->m_vEntries.begin(), pTrail->m_vEntries.end(), isEasier);
}

void PlayerOptions::GetLocalizedMods( vector<std::string> &AddTo ) const
{
	vector<std::string> vMods;
	GetMods( vMods );
	for (auto const &sOneMod: vMods)
	{
		ASSERT( !sOneMod.empty() );

		auto asTokens = Rage::split(sOneMod, " ");

		if( asTokens.empty() )
		{
			continue;
		}
		// Strip the approach speed token, if any
		if( asTokens[0][0] == '*' )
		{
			asTokens.erase( asTokens.begin() );
		}
		asTokens.back() = Capitalize( asTokens.back() );

		/* Theme the mod name (the last string).  Allow this to not exist, since
		 * characters might use modifiers that don't exist in the theme. */
		asTokens.back() = CommonMetrics::LocalizeOptionItem( asTokens.back(), true );

		std::string sLocalizedMod = Rage::join( " ", asTokens );
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

std::string PlayerOptions::GetSavedPrefsString() const
{
	PlayerOptions po_prefs;
#define SAVE(x) po_prefs.x = this->x;
	SAVE( m_fTimeSpacing );
	SAVE( m_fScrollSpeed );
	SAVE( m_fScrollBPM );
	SAVE( m_fMaxScrollBPM );
	SAVE( m_fScrolls[SCROLL_REVERSE] );
	SAVE( m_fTilt );
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
	CPY(m_bCosecant);
	CPY(m_MinTNSToHideNotes);

	CPY( m_fTilt );
	CPY( m_bTransforms[TRANSFORM_NOHOLDS] );
	CPY( m_bTransforms[TRANSFORM_NOROLLS] );
	CPY( m_bTransforms[TRANSFORM_NOMINES] );
	CPY( m_bTransforms[TRANSFORM_NOJUMPS] );
	CPY( m_bTransforms[TRANSFORM_NOHANDS] );
	CPY( m_bTransforms[TRANSFORM_NOQUADS] );
	CPY( m_bTransforms[TRANSFORM_NOSTRETCH] );
	CPY( m_bTransforms[TRANSFORM_NOLIFTS] );
	CPY( m_bTransforms[TRANSFORM_NOFAKES] );
#undef CPY
}

std::string get_player_mod_string(PlayerNumber pn, bool hide_fail)
{
	LuaReference func;
	THEME->GetMetric("Common", "ModStringFunction", func);
	if(func.GetLuaType() != LUA_TFUNCTION)
	{
		LuaHelpers::ReportScriptError("Common::ModStringFunction metric must be a function.");
		return "";
	}
	std::string ret;
	Lua* L= LUA->Get();
	func.PushSelf(L);
	Enum::Push(L, pn);
	lua_pushboolean(L, hide_fail);
	std::string err= "Error running ModStringFunction:  ";
	if(LuaHelpers::RunScriptOnStack(L, err, 2, 1, true))
	{
		ret= lua_tostring(L, -1);
	}
	lua_settop(L, 0);
	LUA->Release(L);
	return ret;
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
	ENUM_INTERFACE(ModTimerSetting, ModTimerType, ModTimerType);
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
	FLOAT_INTERFACE(Tilt, Tilt, true);
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
			if(!std::isfinite(speed) || speed <= 0.0f)
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
			if(!std::isfinite(speed) || speed <= 0.0f)
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

	static void SetPerspectiveApproach(T* p, lua_State*, float speed)
	{
		p->m_SpeedfTilt= speed;
		p->m_SpeedfSkew= speed;
	}

	static int Overhead(T* p, lua_State* L)
	{
		int original_top= lua_gettop(L);
		lua_pushboolean(L, (p->m_fTilt == 0.0f && p->m_fSkew == 0.0f));
		if(lua_toboolean(L, 1))
		{
			p->m_fTilt= 0;
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
		if((p->m_fSkew > 0.0f && p->m_fTilt < 0.0f) ||
			(p->m_fSkew < 0.0f && p->m_fTilt > 0.0f))
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
			p->m_fTilt= -value;
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
		if((p->m_fSkew > 0.0f && p->m_fTilt > 0.0f) ||
			(p->m_fSkew < 0.0f && p->m_fTilt < 0.0f))
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
			p->m_fTilt= value;
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
		if(p->m_fSkew == 0.0f && p->m_fTilt < 0.0f)
		{
			lua_pushnumber(L, -p->m_fTilt);
			lua_pushnumber(L, p->m_SpeedfTilt);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			p->m_fTilt= -FArg(1);
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
		if(p->m_fSkew == 0.0f && p->m_fTilt > 0.0f)
		{
			lua_pushnumber(L, p->m_fTilt);
			lua_pushnumber(L, p->m_SpeedfTilt);
		}
		else
		{
			lua_pushnil(L);
			lua_pushnil(L);
		}
		if(lua_isnumber(L, 1) && original_top >= 1)
		{
			p->m_fTilt= FArg(1);
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
