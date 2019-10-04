#include "global.h"
#include "ArrowEffects.h"
#include "Steps.h"
#include "GameConstantsAndTypes.h"
#include "GameManager.h"
#include "RageTimer.h"
#include "NoteDisplay.h"
#include "Song.h"
#include "RageMath.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "GameState.h"
#include "Style.h"
#include "ThemeMetric.h"
#include <float.h>

static char const dimension_names[4]= "XYZ";

static ThemeMetric<float>	ARROW_SPACING( "ArrowEffects", "ArrowSpacing" );
static ThemeMetric<bool>	QUANTIZE_ARROW_Y( "ArrowEffects", "QuantizeArrowYPosition");

/* For better or for worse, allow the themes to modify the various mod
 * effects for the different mods. In general, it is recommended to not
 * edit the default values and instead use percentage mods when changes
 * are wanted. Still, the option is available for those that want it.
 *
 * Is this a good idea? We'll find out. -aj & Wolfman2000 */
static ThemeMetric<float>	BLINK_MOD_FREQUENCY( "ArrowEffects", "BlinkModFrequency" );
static ThemeMetric<float>	BOOST_MOD_MIN_CLAMP( "ArrowEffects", "BoostModMinClamp" );
static ThemeMetric<float>	BOOST_MOD_MAX_CLAMP( "ArrowEffects", "BoostModMaxClamp" );
static ThemeMetric<float>	BRAKE_MOD_MIN_CLAMP( "ArrowEffects", "BrakeModMinClamp" );
static ThemeMetric<float>	BRAKE_MOD_MAX_CLAMP( "ArrowEffects", "BrakeModMaxClamp" );
static ThemeMetric<float>	WAVE_MOD_MAGNITUDE( "ArrowEffects", "WaveModMagnitude" );
static ThemeMetric<float>	WAVE_MOD_HEIGHT( "ArrowEffects", "WaveModHeight" );
static ThemeMetric<float>	BOOMERANG_PEAK_PERCENTAGE( "ArrowEffects", "BoomerangPeakPercentage" );
static ThemeMetric<float>	EXPAND_MULTIPLIER_FREQUENCY( "ArrowEffects", "ExpandMultiplierFrequency" );
static ThemeMetric<float>	EXPAND_MULTIPLIER_SCALE_FROM_LOW( "ArrowEffects", "ExpandMultiplierScaleFromLow" );
static ThemeMetric<float>	EXPAND_MULTIPLIER_SCALE_FROM_HIGH( "ArrowEffects", "ExpandMultiplierScaleFromHigh" );
static ThemeMetric<float>	EXPAND_MULTIPLIER_SCALE_TO_LOW( "ArrowEffects", "ExpandMultiplierScaleToLow" );
static ThemeMetric<float>	EXPAND_MULTIPLIER_SCALE_TO_HIGH( "ArrowEffects", "ExpandMultiplierScaleToHigh" );
static ThemeMetric<float>	EXPAND_SPEED_SCALE_FROM_LOW( "ArrowEffects", "ExpandSpeedScaleFromLow" );
static ThemeMetric<float>	EXPAND_SPEED_SCALE_FROM_HIGH( "ArrowEffects", "ExpandSpeedScaleFromHigh" );
static ThemeMetric<float>	EXPAND_SPEED_SCALE_TO_LOW( "ArrowEffects", "ExpandSpeedScaleToLow" );
static ThemeMetric<float>	TIPSY_TIMER_FREQUENCY( "ArrowEffects", "TipsyTimerFrequency" );
static ThemeMetric<float>	TIPSY_COLUMN_FREQUENCY( "ArrowEffects", "TipsyColumnFrequency" );
static ThemeMetric<float>	TIPSY_ARROW_MAGNITUDE( "ArrowEffects", "TipsyArrowMagnitude" );
static ThemeMetric<float>	TIPSY_OFFSET_TIMER_FREQUENCY( "ArrowEffects", "TipsyOffsetTimerFrequency" );
static ThemeMetric<float>	TIPSY_OFFSET_COLUMN_FREQUENCY( "ArrowEffects", "TipsyOffsetColumnFrequency" );
static ThemeMetric<float>	TIPSY_OFFSET_ARROW_MAGNITUDE( "ArrowEffects", "TipsyOffsetArrowMagnitude" );

static RString TPSTL_NAME(size_t i) { return ssprintf("Tornado%cPositionScaleToLow", dimension_names[i]); }
static ThemeMetric1D<float> TORNADO_POSITION_SCALE_TO_LOW("ArrowEffects", TPSTL_NAME, 3);
static RString TPSTH_NAME(size_t i) { return ssprintf("Tornado%cPositionScaleToHigh", dimension_names[i]); }
static ThemeMetric1D<float> TORNADO_POSITION_SCALE_TO_HIGH("ArrowEffects", TPSTH_NAME, 3);
static RString TOF_NAME(size_t i) { return ssprintf("Tornado%cOffsetFrequency", dimension_names[i]); }
static ThemeMetric1D<float> TORNADO_OFFSET_FREQUENCY("ArrowEffects", TOF_NAME, 3);
static RString TOSFL_NAME(size_t i) { return ssprintf("Tornado%cOffsetScaleFromLow", dimension_names[i]); }
static ThemeMetric1D<float> TORNADO_OFFSET_SCALE_FROM_LOW("ArrowEffects", TOSFL_NAME, 3);
static RString TOSFH_NAME(size_t i) { return ssprintf("Tornado%cOffsetScaleFromHigh", dimension_names[i]); }
static ThemeMetric1D<float> TORNADO_OFFSET_SCALE_FROM_HIGH("ArrowEffects", TOSFH_NAME, 3);

static ThemeMetric<float>	DRUNK_COLUMN_FREQUENCY( "ArrowEffects", "DrunkColumnFrequency" );
static ThemeMetric<float>	DRUNK_OFFSET_FREQUENCY( "ArrowEffects", "DrunkOffsetFrequency" );
static ThemeMetric<float>	DRUNK_ARROW_MAGNITUDE( "ArrowEffects", "DrunkArrowMagnitude" );

static ThemeMetric<float>	DRUNK_Z_COLUMN_FREQUENCY( "ArrowEffects", "DrunkZColumnFrequency" );
static ThemeMetric<float>	DRUNK_Z_OFFSET_FREQUENCY( "ArrowEffects", "DrunkZOffsetFrequency" );
static ThemeMetric<float>	DRUNK_Z_ARROW_MAGNITUDE( "ArrowEffects", "DrunkZArrowMagnitude" );

static ThemeMetric<float>	BEAT_OFFSET_HEIGHT( "ArrowEffects", "BeatOffsetHeight" );
static ThemeMetric<float>	BEAT_PI_HEIGHT( "ArrowEffects", "BeatPIHeight" );

static ThemeMetric<float>	BEAT_Y_OFFSET_HEIGHT( "ArrowEffects", "BeatYOffsetHeight" );
static ThemeMetric<float>	BEAT_Y_PI_HEIGHT( "ArrowEffects", "BeatYPIHeight" );
static ThemeMetric<float>	BEAT_Z_OFFSET_HEIGHT( "ArrowEffects", "BeatZOffsetHeight" );
static ThemeMetric<float>	BEAT_Z_PI_HEIGHT( "ArrowEffects", "BeatZPIHeight" );

static ThemeMetric<float>	TINY_PERCENT_BASE( "ArrowEffects", "TinyPercentBase" );
static ThemeMetric<float>	TINY_PERCENT_GATE( "ArrowEffects", "TinyPercentGate" );

static const PlayerOptions* curr_options= nullptr;

float ArrowGetPercentVisible(float fYPosWithoutReverse, int iCol, float fYOffset);

static float GetNoteFieldHeight()
{
	return SCREEN_HEIGHT + fabsf(curr_options->m_fPerspectiveTilt)*200;
}

float ArrowEffects::GetTime()
{
	float mult = 1.f + curr_options->m_fModTimerMult;
	float offset = curr_options->m_fModTimerOffset;
	ModTimerType modtimer = curr_options->m_ModTimerType;
	switch(modtimer)
	{
	    case ModTimerType_Default:
	    case ModTimerType_Game:
		return (RageTimer::GetTimeSinceStartFast()+offset)*mult;
	    case ModTimerType_Beat:
		return (GAMESTATE->m_Position.m_fSongBeatVisible+offset)*mult;
	    case ModTimerType_Song:
		return (GAMESTATE->m_Position.m_fMusicSeconds+offset)*mult;
	    default:
		return RageTimer::GetTimeSinceStartFast()+offset;
	}
}

namespace
{
	struct PerPlayerData
	{
		float m_MinTornado[3][MAX_COLS_PER_PLAYER];
		float m_MaxTornado[3][MAX_COLS_PER_PLAYER];
		float m_fInvertDistance[MAX_COLS_PER_PLAYER];
		float m_tipsy_result[MAX_COLS_PER_PLAYER];
		float m_tipsy_offset_result[MAX_COLS_PER_PLAYER];
		float m_tan_tipsy_result[MAX_COLS_PER_PLAYER];
		float m_tan_tipsy_offset_result[MAX_COLS_PER_PLAYER];
		float m_fBeatFactor[3];
		float m_fExpandSeconds;
		float m_fTanExpandSeconds;

		// m_prev_style is for checking whether ArrowEffects::Init needs to be
		// called.  Finding all the placed ArrowEffects is used and making sure
		// they all call Init after changing style is non-trivial and more likely
		// to cause bugs. -Kyz
		Style const* m_prev_style;
	};
	PerPlayerData g_EffectData[NUM_PLAYERS];
	int const dim_x= 0;
	int const dim_y= 1;
	int const dim_z= 2;

	float tornado_position_scale_to_low[3];
	float tornado_position_scale_to_high[3];
	float tornado_offset_frequency[3];
	float tornado_offset_scale_from_low[3];
	float tornado_offset_scale_from_high[3];
};

static float SelectTanType(float angle, bool is_cosec)
{
	if (is_cosec)
	    return RageFastCsc(angle);
	else
	    return RageFastTan(angle);
}

static float CalculateTornadoOffsetFromMagnitude(int dimension, int col_id,
	float magnitude, float effect_offset, float period,
	const Style::ColumnInfo* pCols, float field_zoom,
	PerPlayerData& data, float y_offset, bool is_tan)
{
	float const real_pixel_offset= pCols[col_id].fXOffset * field_zoom;
	float const position_between= SCALE(real_pixel_offset,
		data.m_MinTornado[dimension][col_id] * field_zoom,
		data.m_MaxTornado[dimension][col_id] * field_zoom,
		tornado_position_scale_to_low[dimension],
		tornado_position_scale_to_high[dimension]);
	float rads= acosf(position_between);
	float frequency= tornado_offset_frequency[dimension];
	rads+= (y_offset + effect_offset) * ((period * frequency) + frequency) / SCREEN_HEIGHT;
	float processed_rads = is_tan ? SelectTanType(rads, curr_options->m_bCosecant) : RageFastCos(rads); 
		
	float const adjusted_pixel_offset= SCALE(processed_rads,
		tornado_offset_scale_from_low[dimension],
		tornado_offset_scale_from_high[dimension],
		data.m_MinTornado[dimension][col_id] * field_zoom,
		data.m_MaxTornado[dimension][col_id] * field_zoom);
	return (adjusted_pixel_offset - real_pixel_offset) * magnitude;
}

static float CalculateDrunkAngle(float speed, int col, float offset, 
	float col_frequency, float y_offset, float period, float offset_frequency)
{
	float time = ArrowEffects::GetTime();
	return time * (1+speed) + col*( (offset*col_frequency) + col_frequency)
		+ y_offset * ( (period*offset_frequency) + offset_frequency) / SCREEN_HEIGHT;
}

static float CalculateBumpyAngle(float y_offset, float offset, float period)
{
	return (y_offset+(100.0f*offset))/((period*16.0f)+16.0f);
}

static float CalculateDigitalAngle(float y_offset, float offset, float period)
{
	return PI * (y_offset + (1.0f * offset ) ) / (ARROW_SIZE + (period * ARROW_SIZE) );
}

static void UpdateBeat(int dimension, PerPlayerData &data, const SongPosition &position, float beat_offset, float beat_mult)
{
	float fAccelTime = 0.2f, fTotalTime = 0.5f;
	float fBeat = ((position.m_fSongBeatVisible + fAccelTime + beat_offset) * (beat_mult+1));

	const bool bEvenBeat = ( int(fBeat) % 2 ) != 0;

	data.m_fBeatFactor[dimension] = 0;
	if( fBeat < 0 )
		return;

	// -100.2 -> -0.2 -> 0.2
	fBeat -= truncf( fBeat );
	fBeat += 1;
	fBeat -= truncf( fBeat );

	if( fBeat >= fTotalTime )
		return;

	if( fBeat < fAccelTime )
	{
		data.m_fBeatFactor[dimension] = SCALE( fBeat, 0.0f, fAccelTime, 0.0f, 1.0f);
		data.m_fBeatFactor[dimension] *= data.m_fBeatFactor[dimension];
	} else /* fBeat < fTotalTime */ {
		data.m_fBeatFactor[dimension] = SCALE( fBeat, fAccelTime, fTotalTime, 1.0f, 0.0f);
		data.m_fBeatFactor[dimension] = 1 - (1-data.m_fBeatFactor[dimension]) * (1-data.m_fBeatFactor[dimension]);
	}

	if( bEvenBeat )
		data.m_fBeatFactor[dimension] *= -1;
	data.m_fBeatFactor[dimension] *= 20.0f;
}

static void UpdateTipsy(float * tipsy_result, float * tipsy_offset_result, float offset, float speed, bool is_tan)
{
	const float time= ArrowEffects::GetTime();
	const float time_times_timer= time * ((speed * TIPSY_TIMER_FREQUENCY) + TIPSY_TIMER_FREQUENCY);
	const float arrow_times_mag= ARROW_SIZE * TIPSY_ARROW_MAGNITUDE;
	const float time_times_offset_timer= time *
		TIPSY_OFFSET_TIMER_FREQUENCY;
	const float arrow_times_offset_mag= ARROW_SIZE *
		TIPSY_OFFSET_ARROW_MAGNITUDE;
	for(int col= 0; col < MAX_COLS_PER_PLAYER; ++col)
	{
		if (is_tan)
		{
			tipsy_result[col]= SelectTanType(time_times_timer + (col * ((offset * 
				TIPSY_COLUMN_FREQUENCY) + TIPSY_COLUMN_FREQUENCY)), curr_options->m_bCosecant)
				* arrow_times_mag;
			tipsy_offset_result[col]= SelectTanType(time_times_offset_timer + (col * 
				TIPSY_OFFSET_COLUMN_FREQUENCY), curr_options->m_bCosecant)
				* arrow_times_offset_mag;
		}
		else
		{
			tipsy_result[col]= RageFastCos(time_times_timer + (col * ((offset * 
				TIPSY_COLUMN_FREQUENCY) + TIPSY_COLUMN_FREQUENCY))) * arrow_times_mag;
			tipsy_offset_result[col]= RageFastCos(time_times_offset_timer + (col * 
				TIPSY_OFFSET_COLUMN_FREQUENCY)) * arrow_times_offset_mag;
		}
	}
}


void ArrowEffects::Init(PlayerNumber pn)
{
	const Style* pStyle = GAMESTATE->GetCurrentStyle(pn);
	const Style::ColumnInfo* pCols = pStyle->m_ColumnInfo[pn];
	PerPlayerData &data = g_EffectData[pn];
	// Init tornado limits.
	// This used to run every frame, but it doesn't actually depend on anything
	// that changes every frame.  In openitg, it runs for every note. -Kyz

	// TRICKY: Tornado is very unplayable in doubles, so use a smaller
	// tornado width if there are many columns

	/* the wide_field check makes an assumption for dance mode.
	 * perhaps check if we are actually playing on singles without,
	 * say more than 6 columns. That would exclude IIDX, pop'n, and
	 * techno-8, all of which would be very hectic.
	 * certain non-singles modes (like halfdoubles 6cols)
	 * could possibly have tornado enabled.
	 * let's also take default resolution (640x480) into mind. -aj */
	bool wide_field= pStyle->m_iColsPerPlayer > 4;
	int max_player_col= pStyle->m_iColsPerPlayer-1;
	for(int dimension= 0; dimension < 3; ++dimension)
	{
		int width= 3;
		// wide_field only matters for x, which is dimension 0. -Kyz
		if(dimension == 0 && wide_field)
		{
			width= 2;
		}
		for(int col_id= 0; col_id <= max_player_col; ++col_id)
		{
			int start_col= col_id - width;
			int end_col= col_id + width;
			CLAMP(start_col, 0, max_player_col);
			CLAMP(end_col, 0, max_player_col);
			data.m_MinTornado[dimension][col_id]= FLT_MAX;
			data.m_MaxTornado[dimension][col_id]= FLT_MIN;
			for(int i= start_col; i <= end_col; ++i)
			{
				// Using the x offset when the dimension might be y or z feels so
				// wrong, but it provides min and max values when otherwise the
				// limits would just be zero, which would make it do nothing. -Kyz
				data.m_MinTornado[dimension][col_id] = min(pCols[i].fXOffset, data.m_MinTornado[dimension][col_id]);
				data.m_MaxTornado[dimension][col_id] = max(pCols[i].fXOffset, data.m_MaxTornado[dimension][col_id]);
			}
		}

		tornado_position_scale_to_low[dimension]= TORNADO_POSITION_SCALE_TO_LOW.GetValue(dimension);
		tornado_position_scale_to_high[dimension]= TORNADO_POSITION_SCALE_TO_HIGH.GetValue(dimension);
		tornado_offset_frequency[dimension]= TORNADO_OFFSET_FREQUENCY.GetValue(dimension);
		tornado_offset_scale_from_low[dimension]= TORNADO_OFFSET_SCALE_FROM_LOW.GetValue(dimension);
		tornado_offset_scale_from_high[dimension]= TORNADO_OFFSET_SCALE_FROM_HIGH.GetValue(dimension);
	}
}

void ArrowEffects::Update()
{
	static float fLastTime = 0;
	float fTime = RageTimer::GetTimeSinceStartFast();
	
	FOREACH_EnabledPlayer( pn )
	{
		const Style* pStyle = GAMESTATE->GetCurrentStyle(pn);
		const Style::ColumnInfo* pCols = pStyle->m_ColumnInfo[pn];
		const SongPosition &position = GAMESTATE->m_bIsUsingStepTiming
		? GAMESTATE->m_pPlayerState[pn]->m_Position : GAMESTATE->m_Position;
		const float field_zoom= GAMESTATE->m_pPlayerState[pn]->m_NotefieldZoom;
		const float* effects= GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_fEffects;
		const float* accels= GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetCurrent().m_fAccels;

		PerPlayerData &data = g_EffectData[pn];
		
		if(pStyle != data.m_prev_style)
		{
			Init(pn);
			data.m_prev_style= pStyle;
		}
		
		if( !position.m_bFreeze || !position.m_bDelay )
		{
			data.m_fExpandSeconds += fTime - fLastTime;
			data.m_fExpandSeconds = fmodf( data.m_fExpandSeconds, (PI*2)/(accels[PlayerOptions::ACCEL_EXPAND_PERIOD]+1) );
			data.m_fTanExpandSeconds += fTime - fLastTime;
			data.m_fTanExpandSeconds = fmodf( data.m_fTanExpandSeconds, (PI*2)/(accels[PlayerOptions::ACCEL_TAN_EXPAND_PERIOD]+1) );
		}

		// Update Invert
		for( int iColNum = 0; iColNum < MAX_COLS_PER_PLAYER; ++iColNum )
		{
			const int iNumCols = pStyle->m_iColsPerPlayer;
			const int iNumSides = (pStyle->m_StyleType==StyleType_OnePlayerTwoSides ||
					       pStyle->m_StyleType==StyleType_TwoPlayersSharedSides) ? 2 : 1;
			const int iNumColsPerSide = iNumCols / iNumSides;
			const int iSideIndex = iColNum / iNumColsPerSide;
			const int iColOnSide = iColNum % iNumColsPerSide;

			const int iColLeftOfMiddle = (iNumColsPerSide-1)/2;
			const int iColRightOfMiddle = (iNumColsPerSide+1)/2;

			int iFirstColOnSide = -1;
			int iLastColOnSide = -1;
			if( iColOnSide <= iColLeftOfMiddle )
			{
				iFirstColOnSide = 0;
				iLastColOnSide = iColLeftOfMiddle;
			}
			else if( iColOnSide >= iColRightOfMiddle )
			{
				iFirstColOnSide = iColRightOfMiddle;
				iLastColOnSide = iNumColsPerSide-1;
			}
			else
			{
				iFirstColOnSide = iColOnSide/2;
				iLastColOnSide = iColOnSide/2;
			}

			// mirror
			int iNewColOnSide;
			if( iFirstColOnSide == iLastColOnSide )
				iNewColOnSide = 0;
			else
				iNewColOnSide = SCALE( iColOnSide, iFirstColOnSide, iLastColOnSide, iLastColOnSide, iFirstColOnSide );
			const int iNewCol = iSideIndex*iNumColsPerSide + iNewColOnSide;

			const float fOldPixelOffset = pCols[iColNum].fXOffset;
			const float fNewPixelOffset = pCols[iNewCol].fXOffset;
			data.m_fInvertDistance[iColNum] = fNewPixelOffset - fOldPixelOffset;
		}

		// Update Tipsy
		if(effects[PlayerOptions::EFFECT_TIPSY] != 0)
		{
			UpdateTipsy(data.m_tipsy_result, data.m_tipsy_offset_result, 
				    effects[PlayerOptions::EFFECT_TIPSY_OFFSET], 
				    effects[PlayerOptions::EFFECT_TIPSY_SPEED], false);
		}
		else
		{
			for(int col= 0; col < MAX_COLS_PER_PLAYER; ++col)
			{
				data.m_tipsy_result[col]= 0;
			}
		}
		
		// Update TanTipsy
		if(effects[PlayerOptions::EFFECT_TAN_TIPSY] != 0)
		{
			UpdateTipsy(data.m_tan_tipsy_result, data.m_tan_tipsy_offset_result, 
				    effects[PlayerOptions::EFFECT_TAN_TIPSY_OFFSET], 
				    effects[PlayerOptions::EFFECT_TAN_TIPSY_SPEED], true);
		}
		else
		{
			for(int col= 0; col < MAX_COLS_PER_PLAYER; ++col)
			{
				data.m_tan_tipsy_result[col]= 0;
			}
		}

		// Update Beat
		UpdateBeat(dim_x, data, position, effects[PlayerOptions::EFFECT_BEAT_OFFSET], effects[PlayerOptions::EFFECT_BEAT_MULT]);

		// Update BeatY
		UpdateBeat(dim_y, data, position, effects[PlayerOptions::EFFECT_BEAT_Y_OFFSET], effects[PlayerOptions::EFFECT_BEAT_Y_MULT]);

		// Update BeatZ
		UpdateBeat(dim_z, data, position, effects[PlayerOptions::EFFECT_BEAT_Z_OFFSET], effects[PlayerOptions::EFFECT_BEAT_Z_MULT]);
	}
	fLastTime = fTime;
}

void ArrowEffects::SetCurrentOptions(const PlayerOptions* options)
{
	curr_options= options;
}

static float GetDisplayedBeat( const PlayerState* pPlayerState, float beat )
{
	// do a binary search here
	const vector<CacheDisplayedBeat> &data = pPlayerState->m_CacheDisplayedBeat;
	int max = data.size() - 1;
	int l = 0, r = max;
	while( l <= r )
	{
		int m = ( l + r ) / 2;
		if( ( m == 0 || data[m].beat <= beat ) && ( m == max || beat < data[m + 1].beat ) )
		{
			return data[m].displayedBeat + data[m].velocity * (beat - data[m].beat);
		}
		else if( data[m].beat <= beat )
		{
			l = m + 1;
		}
		else
		{
			r = m - 1;
		}
	}
	return beat;
}

/* For visibility testing: if bAbsolute is false, random modifiers must return
 * the minimum possible scroll speed. */
float ArrowEffects::GetYOffset( const PlayerState* pPlayerState, int iCol, float fNoteBeat, float &fPeakYOffsetOut, bool &bIsPastPeakOut, bool bAbsolute )
{
	// Default values that are returned if boomerang is off.
	fPeakYOffsetOut = FLT_MAX;
	bIsPastPeakOut = true;

	float fYOffset = 0;
	const SongPosition &position = pPlayerState->GetDisplayedPosition();
	
	float fSongBeat = position.m_fSongBeatVisible;
	
	Steps *pCurSteps = GAMESTATE->m_pCurSteps[pPlayerState->m_PlayerNumber];

	/* Usually, fTimeSpacing is 0 or 1, in which case we use entirely beat spacing or
	 * entirely time spacing (respectively). Occasionally, we tween between them. */
	if( curr_options->m_fTimeSpacing != 1.0f )
	{
		if( GAMESTATE->m_bInStepEditor ) {
			// Use constant spacing in step editor
			fYOffset = fNoteBeat - fSongBeat;
		} else {
			fYOffset = GetDisplayedBeat(pPlayerState, fNoteBeat) - GetDisplayedBeat(pPlayerState, fSongBeat);
			fYOffset *= pCurSteps->GetTimingData()->GetDisplayedSpeedPercent(
								     position.m_fSongBeatVisible,
								     position.m_fMusicSecondsVisible );
		}
		fYOffset *= 1 - curr_options->m_fTimeSpacing;
	}

	if( curr_options->m_fTimeSpacing != 0.0f )
	{
		float fSongSeconds = GAMESTATE->m_Position.m_fMusicSecondsVisible;
		float fNoteSeconds = pCurSteps->GetTimingData()->GetElapsedTimeFromBeat(fNoteBeat);
		float fSecondsUntilStep = fNoteSeconds - fSongSeconds;
		float fBPM = curr_options->m_fScrollBPM;
		float fBPS = fBPM/60.f / GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		float fYOffsetTimeSpacing = fSecondsUntilStep * fBPS;
		fYOffset += fYOffsetTimeSpacing * curr_options->m_fTimeSpacing;
	}

	// TODO: If we allow noteskins to have metricable row spacing
	// (per issue 24), edit this to reflect that. -aj
	fYOffset *= ARROW_SPACING;

	// Factor in scroll speed
	float fScrollSpeed = curr_options->m_fScrollSpeed;
	if(curr_options->m_fMaxScrollBPM != 0)
	{
		fScrollSpeed= curr_options->m_fMaxScrollBPM /
			(pPlayerState->m_fReadBPM * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate);
	}
	
	// don't mess with the arrows after they've crossed 0
	if( fYOffset < 0 )
	{
		return fYOffset * fScrollSpeed;
	}

	const float* fAccels = curr_options->m_fAccels;
	const float* fEffects = curr_options->m_fEffects;
	
	// TODO: Don't index by PlayerNumber.
	PerPlayerData &data = g_EffectData[pPlayerState->m_PlayerNumber];

	float fYAdjust = 0;	// fill this in depending on PlayerOptions

	if( fAccels[PlayerOptions::ACCEL_BOOST] != 0 )
	{
		float fEffectHeight = GetNoteFieldHeight();
		float fNewYOffset = fYOffset * 1.5f / ((fYOffset+fEffectHeight/1.2f)/fEffectHeight); 
		float fAccelYAdjust =	fAccels[PlayerOptions::ACCEL_BOOST] * (fNewYOffset - fYOffset);
		// TRICKY: Clamp this value, or else BOOST+BOOMERANG will draw a ton of arrows on the screen.
		CLAMP( fAccelYAdjust, BOOST_MOD_MIN_CLAMP, BOOST_MOD_MAX_CLAMP );
		fYAdjust += fAccelYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_BRAKE] != 0 )
	{
		float fEffectHeight = GetNoteFieldHeight();
		float fScale = SCALE( fYOffset, 0.f, fEffectHeight, 0, 1.f );
		float fNewYOffset = fYOffset * fScale; 
		float fBrakeYAdjust = fAccels[PlayerOptions::ACCEL_BRAKE] * (fNewYOffset - fYOffset);
		// TRICKY: Clamp this value the same way as BOOST so that in BOOST+BRAKE, BRAKE doesn't overpower BOOST
		CLAMP( fBrakeYAdjust, BRAKE_MOD_MIN_CLAMP, BRAKE_MOD_MAX_CLAMP );
		fYAdjust += fBrakeYAdjust;
	}
	if( fAccels[PlayerOptions::ACCEL_WAVE] != 0 )
		fYAdjust +=	fAccels[PlayerOptions::ACCEL_WAVE] * WAVE_MOD_MAGNITUDE *RageFastSin( fYOffset/((fAccels[PlayerOptions::ACCEL_WAVE_PERIOD]*WAVE_MOD_HEIGHT)+WAVE_MOD_HEIGHT) );
	
	if( fEffects[PlayerOptions::EFFECT_PARABOLA_Y] != 0 )
		fYAdjust += fEffects[PlayerOptions::EFFECT_PARABOLA_Y] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE);

	fYOffset += fYAdjust;

	// Factor in boomerang
	if( fAccels[PlayerOptions::ACCEL_BOOMERANG] != 0 )
	{
		float fPeakAtYOffset = SCREEN_HEIGHT * BOOMERANG_PEAK_PERCENTAGE;	// zero point of boomerang function
		fPeakYOffsetOut = (-1*fPeakAtYOffset*fPeakAtYOffset/SCREEN_HEIGHT) + 1.5f*fPeakAtYOffset;
		bIsPastPeakOut = fYOffset < fPeakAtYOffset;

		fYOffset = (-1*fYOffset*fYOffset/SCREEN_HEIGHT) + 1.5f*fYOffset;
	}

	if( curr_options->m_fRandomSpeed > 0 && !bAbsolute )
	{
		// Generate a deterministically "random" speed for each arrow.
		unsigned seed = GAMESTATE->m_iStageSeed + ( BeatToNoteRow( fNoteBeat ) << 8 ) + (iCol * 100);

		for( int i = 0; i < 3; ++i )
			seed = ((seed * 1664525u) + 1013904223u) & 0xFFFFFFFF;
		float fRandom = seed / 4294967296.0f;

		/* Random speed always increases speed: a random speed of 10 indicates
		 * [1,11]. This keeps it consistent with other mods: 0 means no effect. */
		fScrollSpeed *=
				SCALE( fRandom,
						0.0f, 1.0f,
						1.0f, curr_options->m_fRandomSpeed + 1.0f );
	}

	if( fAccels[PlayerOptions::ACCEL_EXPAND] != 0 )
	{
		float fExpandMultiplier = SCALE( RageFastCos(data.m_fExpandSeconds*EXPAND_MULTIPLIER_FREQUENCY*(fAccels[PlayerOptions::ACCEL_EXPAND_PERIOD]+1)),
						EXPAND_MULTIPLIER_SCALE_FROM_LOW, EXPAND_MULTIPLIER_SCALE_FROM_HIGH,
						EXPAND_MULTIPLIER_SCALE_TO_LOW, EXPAND_MULTIPLIER_SCALE_TO_HIGH );
		fScrollSpeed *=	SCALE( fAccels[PlayerOptions::ACCEL_EXPAND], 
				      EXPAND_SPEED_SCALE_FROM_LOW, EXPAND_SPEED_SCALE_FROM_HIGH,
				      EXPAND_SPEED_SCALE_TO_LOW, fExpandMultiplier );
	}

	if( fAccels[PlayerOptions::ACCEL_TAN_EXPAND] != 0 )
	{
		float fTanExpandMultiplier = SCALE( SelectTanType(data.m_fTanExpandSeconds*EXPAND_MULTIPLIER_FREQUENCY*(fAccels[PlayerOptions::ACCEL_TAN_EXPAND_PERIOD]+1), curr_options->m_bCosecant),
						EXPAND_MULTIPLIER_SCALE_FROM_LOW, EXPAND_MULTIPLIER_SCALE_FROM_HIGH,
						EXPAND_MULTIPLIER_SCALE_TO_LOW, EXPAND_MULTIPLIER_SCALE_TO_HIGH );
		fScrollSpeed *=	SCALE( fAccels[PlayerOptions::ACCEL_TAN_EXPAND], 
				      EXPAND_SPEED_SCALE_FROM_LOW, EXPAND_SPEED_SCALE_FROM_HIGH,
				      EXPAND_SPEED_SCALE_TO_LOW, fTanExpandMultiplier );
	}

	fYOffset *= fScrollSpeed;
	fPeakYOffsetOut *= fScrollSpeed;

	return fYOffset;
}

static void ArrowGetReverseShiftAndScale(int iCol, float fYReverseOffsetPixels, float &fShiftOut, float &fScaleOut)
{
	// XXX: Hack: we need to scale the reverse shift by the zoom.
	float fMiniPercent = curr_options->m_fEffects[PlayerOptions::EFFECT_MINI];
	float fZoom = 1 - fMiniPercent*0.5f;

	// don't divide by 0
	if( fabsf(fZoom) < 0.01 )
		fZoom = 0.01f;

	float fPercentReverse = curr_options->GetReversePercentForColumn(iCol);
	fShiftOut = SCALE( fPercentReverse, 0.f, 1.f, -fYReverseOffsetPixels/fZoom/2, fYReverseOffsetPixels/fZoom/2 );
	float fPercentCentered = curr_options->m_fScrolls[PlayerOptions::SCROLL_CENTERED];
	fShiftOut = SCALE( fPercentCentered, 0.f, 1.f, fShiftOut, 0.0f );

	fScaleOut = SCALE( fPercentReverse, 0.f, 1.f, 1.f, -1.f );
}

float ArrowEffects::GetYPos( const PlayerState* pPlayerState, int iCol, float fYOffset, float fYReverseOffsetPixels, bool WithReverse)
{
	float f = fYOffset;

	if( WithReverse )
	{
		float fShift, fScale;
		ArrowGetReverseShiftAndScale(iCol, fYReverseOffsetPixels, fShift, fScale);

		f *= fScale;
		f += fShift;
	}

	// TODO: Don't index by PlayerNumber.
	const Style* pStyle = GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber);
	const Style::ColumnInfo* pCols = pStyle->m_ColumnInfo[pPlayerState->m_PlayerNumber];
	const float* fEffects = curr_options->m_fEffects;
	
	// Doing the math with a precalculated result of 0 should be faster than
	// checking whether tipsy is on. -Kyz
	// TODO: Don't index by PlayerNumber.
	PerPlayerData& data= g_EffectData[curr_options->m_pn];
	f+= fEffects[PlayerOptions::EFFECT_TIPSY] * data.m_tipsy_result[iCol];
	f+= fEffects[PlayerOptions::EFFECT_TAN_TIPSY] * data.m_tan_tipsy_result[iCol];
		
	if( fEffects[PlayerOptions::EFFECT_ATTENUATE_Y] != 0 )
	{
		const float fXOffset = pCols[iCol].fXOffset;
		f += fEffects[PlayerOptions::EFFECT_ATTENUATE_Y] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE) * (fXOffset/ARROW_SIZE);
	}
	

	if( fEffects[PlayerOptions::EFFECT_BEAT_Y] != 0 )
	{
		const float fShift = data.m_fBeatFactor[dim_y]*RageFastSin( fYOffset / ((fEffects[PlayerOptions::EFFECT_BEAT_Y_PERIOD]*BEAT_Y_OFFSET_HEIGHT)+BEAT_Y_OFFSET_HEIGHT) + PI/BEAT_Y_PI_HEIGHT );
		f += fEffects[PlayerOptions::EFFECT_BEAT_Y] * fShift;
	}

	// In beware's DDR Extreme-focused fork of StepMania 3.9, this value is
	// floored, making arrows show on integer Y coordinates. Supposedly it makes
	// the arrows look better, but testing needs to be done.
	// todo: make this a noteskin metric instead of a theme metric? -aj
	return QUANTIZE_ARROW_Y ? floor(f) : f;
}

float ArrowEffects::GetYOffsetFromYPos(int iCol, float YPos, float fYReverseOffsetPixels)
{
	float f = YPos;

	const float* fEffects = curr_options->m_fEffects;
	// Doing the math with a precalculated result of 0 should be faster than
	// checking whether tipsy is on. -Kyz
	// TODO: Don't index by PlayerNumber.
	PerPlayerData& data= g_EffectData[curr_options->m_pn];
	f+= fEffects[PlayerOptions::EFFECT_TIPSY] * data.m_tipsy_offset_result[iCol];
	f+= fEffects[PlayerOptions::EFFECT_TAN_TIPSY] * data.m_tan_tipsy_offset_result[iCol];
	
	f+= fEffects[PlayerOptions::EFFECT_PARABOLA_Y] * (YPos/ARROW_SIZE) * (YPos/ARROW_SIZE);

	float fShift, fScale;
	ArrowGetReverseShiftAndScale(iCol, fYReverseOffsetPixels, fShift, fScale);

	f -= fShift;
	if( fScale )
		f /= fScale;

	return f;
}

float ArrowEffects::GetXPos( const PlayerState* pPlayerState, int iColNum, float fYOffset ) 
{
	float fPixelOffsetFromCenter = 0; // fill this in below

	const Style* pStyle = GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber);
	const float* fEffects = curr_options->m_fEffects;

	// TODO: Don't index by PlayerNumber.
	const Style::ColumnInfo* pCols = pStyle->m_ColumnInfo[pPlayerState->m_PlayerNumber];
	PerPlayerData &data = g_EffectData[pPlayerState->m_PlayerNumber];

	if( fEffects[PlayerOptions::EFFECT_TORNADO] != 0 )
	{
		fPixelOffsetFromCenter += CalculateTornadoOffsetFromMagnitude(dim_x,
			iColNum, fEffects[PlayerOptions::EFFECT_TORNADO],
			fEffects[PlayerOptions::EFFECT_TORNADO_OFFSET],
			fEffects[PlayerOptions::EFFECT_TORNADO_PERIOD],
			pCols, pPlayerState->m_NotefieldZoom, data, fYOffset, false);
	}

	if( fEffects[PlayerOptions::EFFECT_TAN_TORNADO] != 0 )
	{
		fPixelOffsetFromCenter += CalculateTornadoOffsetFromMagnitude(dim_x,
			iColNum, fEffects[PlayerOptions::EFFECT_TAN_TORNADO],
			fEffects[PlayerOptions::EFFECT_TAN_TORNADO_OFFSET],
			fEffects[PlayerOptions::EFFECT_TAN_TORNADO_PERIOD],
			pCols, pPlayerState->m_NotefieldZoom, data, fYOffset, true);
	}
	
	if( fEffects[PlayerOptions::EFFECT_BUMPY_X] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_BUMPY_X] * 
			40*RageFastSin( CalculateBumpyAngle(fYOffset,
			fEffects[PlayerOptions::EFFECT_BUMPY_X_OFFSET],
			fEffects[PlayerOptions::EFFECT_BUMPY_X_PERIOD]) );
	
	if( fEffects[PlayerOptions::EFFECT_TAN_BUMPY_X] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_TAN_BUMPY_X] * 
			40*SelectTanType( CalculateBumpyAngle(fYOffset,
			fEffects[PlayerOptions::EFFECT_TAN_BUMPY_X_OFFSET],
			fEffects[PlayerOptions::EFFECT_TAN_BUMPY_X_PERIOD]), curr_options->m_bCosecant );

	if( fEffects[PlayerOptions::EFFECT_DRUNK] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_DRUNK] * 
			( RageFastCos( CalculateDrunkAngle(fEffects[PlayerOptions::EFFECT_DRUNK_SPEED], iColNum, 
					fEffects[PlayerOptions::EFFECT_DRUNK_OFFSET], DRUNK_COLUMN_FREQUENCY,
					fYOffset, fEffects[PlayerOptions::EFFECT_DRUNK_PERIOD],
					DRUNK_OFFSET_FREQUENCY) ) * ARROW_SIZE*DRUNK_ARROW_MAGNITUDE );

	if( fEffects[PlayerOptions::EFFECT_TAN_DRUNK] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_TAN_DRUNK] * 
			( SelectTanType( CalculateDrunkAngle(fEffects[PlayerOptions::EFFECT_TAN_DRUNK_SPEED],
					iColNum, fEffects[PlayerOptions::EFFECT_TAN_DRUNK_OFFSET],
					DRUNK_COLUMN_FREQUENCY, fYOffset,
					fEffects[PlayerOptions::EFFECT_TAN_DRUNK_PERIOD], DRUNK_OFFSET_FREQUENCY)
					, curr_options->m_bCosecant) * ARROW_SIZE*DRUNK_ARROW_MAGNITUDE );

	if( fEffects[PlayerOptions::EFFECT_FLIP] != 0 )
	{
		const int iFirstCol = 0;
		const int iLastCol = pStyle->m_iColsPerPlayer-1;
		const int iNewCol = SCALE( iColNum, iFirstCol, iLastCol, iLastCol, iFirstCol );
		const float fOldPixelOffset = pCols[iColNum].fXOffset * pPlayerState->m_NotefieldZoom;
		const float fNewPixelOffset = pCols[iNewCol].fXOffset * pPlayerState->m_NotefieldZoom;
		const float fDistance = fNewPixelOffset - fOldPixelOffset;
		fPixelOffsetFromCenter += fDistance * fEffects[PlayerOptions::EFFECT_FLIP];
	}
	if( fEffects[PlayerOptions::EFFECT_INVERT] != 0 )
		fPixelOffsetFromCenter += data.m_fInvertDistance[iColNum] * fEffects[PlayerOptions::EFFECT_INVERT];

	if( fEffects[PlayerOptions::EFFECT_BEAT] != 0 )
	{
		const float fShift = data.m_fBeatFactor[dim_x]*RageFastSin( fYOffset / ((fEffects[PlayerOptions::EFFECT_BEAT_PERIOD]*BEAT_OFFSET_HEIGHT)+BEAT_OFFSET_HEIGHT) + PI/BEAT_PI_HEIGHT );
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_BEAT] * fShift;
	}
	
	if( fEffects[PlayerOptions::EFFECT_ZIGZAG] != 0 )
	{
		float fResult = RageTriangle( (PI * (1/(fEffects[PlayerOptions::EFFECT_ZIGZAG_PERIOD]+1)) * 
		((fYOffset+(100.0f*(fEffects[PlayerOptions::EFFECT_ZIGZAG_OFFSET])))/ARROW_SIZE) ) );
	    
		fPixelOffsetFromCenter += (fEffects[PlayerOptions::EFFECT_ZIGZAG]*ARROW_SIZE/2) * fResult;
	}
	
	if( fEffects[PlayerOptions::EFFECT_SAWTOOTH] != 0 )
		fPixelOffsetFromCenter += (fEffects[PlayerOptions::EFFECT_SAWTOOTH]*ARROW_SIZE) * 
			((0.5f / (fEffects[PlayerOptions::EFFECT_SAWTOOTH_PERIOD]+1) * fYOffset) / ARROW_SIZE - 
			floor((0.5f / (fEffects[PlayerOptions::EFFECT_SAWTOOTH_PERIOD]+1) * fYOffset) / ARROW_SIZE) );
	
	if( fEffects[PlayerOptions::EFFECT_PARABOLA_X] != 0 )
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_PARABOLA_X] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE);
	
	if( fEffects[PlayerOptions::EFFECT_ATTENUATE_X] != 0 )
	{
		const float fXOffset = pCols[iColNum].fXOffset;
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_ATTENUATE_X] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE) * (fXOffset/ARROW_SIZE);
	}

	if( fEffects[PlayerOptions::EFFECT_DIGITAL] != 0 )
		fPixelOffsetFromCenter += (fEffects[PlayerOptions::EFFECT_DIGITAL] * ARROW_SIZE * 0.5f) *
			round((fEffects[PlayerOptions::EFFECT_DIGITAL_STEPS]+1) * RageFastSin(
				CalculateDigitalAngle(fYOffset, 
				fEffects[PlayerOptions::EFFECT_DIGITAL_OFFSET], 
				fEffects[PlayerOptions::EFFECT_DIGITAL_PERIOD]) ) )/(fEffects[PlayerOptions::EFFECT_DIGITAL_STEPS]+1);

	if( fEffects[PlayerOptions::EFFECT_TAN_DIGITAL] != 0 )
		fPixelOffsetFromCenter += (fEffects[PlayerOptions::EFFECT_TAN_DIGITAL] * ARROW_SIZE * 0.5f) *
			round((fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_STEPS]+1) * SelectTanType(
				CalculateDigitalAngle(fYOffset, 
				fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_OFFSET], 
				fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_PERIOD]), curr_options->m_bCosecant ) )/(fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_STEPS]+1);
				
	
	if( fEffects[PlayerOptions::EFFECT_SQUARE] != 0 )
	{
		float fResult = RageSquare( (PI * (fYOffset+(1.0f*(fEffects[PlayerOptions::EFFECT_SQUARE_OFFSET]))) / 
			(ARROW_SIZE+(fEffects[PlayerOptions::EFFECT_SQUARE_PERIOD]*ARROW_SIZE))) );
		
		fPixelOffsetFromCenter += (fEffects[PlayerOptions::EFFECT_SQUARE] * ARROW_SIZE * 0.5f) * fResult;
	}

	if( fEffects[PlayerOptions::EFFECT_BOUNCE] != 0 )
	{
		float fBounceAmt = fabsf( RageFastSin( ( (fYOffset + (1.0f * (fEffects[PlayerOptions::EFFECT_BOUNCE_OFFSET]) ) ) / 
			( 60 + (fEffects[PlayerOptions::EFFECT_BOUNCE_PERIOD]*60) ) ) ) );
		
		fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_BOUNCE] * ARROW_SIZE * 0.5f * fBounceAmt;
	}

	if( fEffects[PlayerOptions::EFFECT_XMODE] != 0 )
	{
		// based off of code by v1toko for StepNXA, except it should work on
		// any gametype now.
		switch( pStyle->m_StyleType )
		{
			case StyleType_OnePlayerTwoSides:
			case StyleType_TwoPlayersSharedSides: // fall through?
				{
					// find the middle, and split based on iColNum
					// it's unknown if this will work for routine.
					const int iMiddleColumn = static_cast<int>(floor(pStyle->m_iColsPerPlayer/2.0f));
					if( iColNum > iMiddleColumn-1 )
						fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_XMODE]*-(fYOffset);
					else
						fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_XMODE]*fYOffset;
				}
				break;
			case StyleType_OnePlayerOneSide:
			case StyleType_TwoPlayersTwoSides: // fall through
				{
					// the code was the same for both of these cases in StepNXA.
					if( pPlayerState->m_PlayerNumber == PLAYER_2 )
						fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_XMODE]*-(fYOffset);
					else
						fPixelOffsetFromCenter += fEffects[PlayerOptions::EFFECT_XMODE]*fYOffset;
				}
				break;
			DEFAULT_FAIL(pStyle->m_StyleType);
		}
	}

	fPixelOffsetFromCenter += pCols[iColNum].fXOffset * pPlayerState->m_NotefieldZoom;

	if( fEffects[PlayerOptions::EFFECT_TINY] != 0 )
	{
		// Allow Tiny to pull tracks together, but not to push them apart.
		float fTinyPercent = fEffects[PlayerOptions::EFFECT_TINY];
		fTinyPercent = min( powf(TINY_PERCENT_BASE, fTinyPercent), (float)TINY_PERCENT_GATE );
		fPixelOffsetFromCenter *= fTinyPercent;
	}

	return fPixelOffsetFromCenter;
}

float ArrowEffects::GetRotationX(const PlayerState* pPlayerState, float fYOffset, bool bIsHoldCap, int iCol)
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_X] != 0 || fEffects[PlayerOptions::EFFECT_CONFUSION_X_OFFSET] != 0 ||
	curr_options->m_fConfusionX[iCol] != 0
	)
		fRotation += ReceptorGetRotationX( pPlayerState, iCol );
	if( fEffects[PlayerOptions::EFFECT_ROLL] != 0 && !bIsHoldCap )
	{
		fRotation += fEffects[PlayerOptions::EFFECT_ROLL] * fYOffset/2;
	}
	return fRotation;
}

float ArrowEffects::GetRotationY(const PlayerState* pPlayerState, float fYOffset, int iCol)
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_Y] != 0 || fEffects[PlayerOptions::EFFECT_CONFUSION_Y_OFFSET] != 0 ||
	curr_options->m_fConfusionY[iCol] != 0
	)
		fRotation += ReceptorGetRotationY( pPlayerState, iCol );
	if( fEffects[PlayerOptions::EFFECT_TWIRL] != 0 )
	{
		fRotation += fEffects[PlayerOptions::EFFECT_TWIRL] * fYOffset/2;
	}
	return fRotation;
}

float ArrowEffects::GetRotationZ( const PlayerState* pPlayerState, float fNoteBeat, bool bIsHoldHead, int iCol ) 
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;
	if( fEffects[PlayerOptions::EFFECT_CONFUSION] != 0 || fEffects[PlayerOptions::EFFECT_CONFUSION_OFFSET] != 0 ||
	curr_options->m_fConfusionZ[iCol] != 0
	)
		fRotation += ReceptorGetRotationZ( pPlayerState, iCol );

	// As usual, enable dizzy hold heads at your own risk. -Wolfman2000
	if( fEffects[PlayerOptions::EFFECT_DIZZY] != 0 && ( curr_options->m_bDizzyHolds || !bIsHoldHead ) )
	{
		const float fSongBeat = pPlayerState->m_Position.m_fSongBeatVisible;
		float fDizzyRotation = fNoteBeat - fSongBeat;
		fDizzyRotation *= fEffects[PlayerOptions::EFFECT_DIZZY];
		fDizzyRotation = fmodf( fDizzyRotation, 2*PI );
		fDizzyRotation *= 180/PI;
		fRotation += fDizzyRotation;
	}
	return fRotation;
}

float ArrowEffects::ReceptorGetRotationZ( const PlayerState* pPlayerState, int iCol ) 
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;

	if( curr_options->m_fConfusionZ[iCol] != 0 )
		fRotation += curr_options->m_fConfusionZ[iCol] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_OFFSET] != 0 )
		fRotation += fEffects[PlayerOptions::EFFECT_CONFUSION_OFFSET] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION] != 0 )
	{
		float fConfRotation = pPlayerState->m_Position.m_fSongBeatVisible;
		fConfRotation *= fEffects[PlayerOptions::EFFECT_CONFUSION];
		fConfRotation = fmodf( fConfRotation, 2*PI );
		fConfRotation *= -180/PI;
		fRotation += fConfRotation;
	}
	
	return fRotation;
}

float ArrowEffects::ReceptorGetRotationX( const PlayerState* pPlayerState, int iCol ) 
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;
	
	if( curr_options->m_fConfusionX[iCol] != 0 )
		fRotation += curr_options->m_fConfusionX[iCol] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_X_OFFSET] != 0 )
		fRotation += fEffects[PlayerOptions::EFFECT_CONFUSION_X_OFFSET] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_X] != 0 )
	{
		float fConfRotation = pPlayerState->m_Position.m_fSongBeatVisible;
		fConfRotation *= fEffects[PlayerOptions::EFFECT_CONFUSION_X];
		fConfRotation = fmodf( fConfRotation, 2*PI );
		fConfRotation *= -180/PI;
		fRotation += fConfRotation;
	}
	
	return fRotation;
}

float ArrowEffects::ReceptorGetRotationY( const PlayerState* pPlayerState, int iCol ) 
{
	const float* fEffects = curr_options->m_fEffects;
	float fRotation = 0;

	if( curr_options->m_fConfusionY[iCol] != 0 )
		fRotation += curr_options->m_fConfusionY[iCol] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_Y_OFFSET] != 0 )
		fRotation += fEffects[PlayerOptions::EFFECT_CONFUSION_Y_OFFSET] * 180.0f/PI;
	
	if( fEffects[PlayerOptions::EFFECT_CONFUSION_Y] != 0 )
	{
		float fConfRotation = pPlayerState->m_Position.m_fSongBeatVisible;
		fConfRotation *= fEffects[PlayerOptions::EFFECT_CONFUSION_Y];
		fConfRotation = fmodf( fConfRotation, 2*PI );
		fConfRotation *= -180/PI;
		fRotation += fConfRotation;
	}
	
	return fRotation;
}

float ArrowEffects::GetMoveX(int iCol)
{
	const float* fMoves = curr_options->m_fMovesX;
	float f = 0;
	if( fMoves[iCol] != 0 )
		f += ARROW_SIZE * fMoves[iCol];
	return f;
}

float ArrowEffects::GetMoveY(int iCol)
{
	const float* fMoves = curr_options->m_fMovesY;
	float f = 0;
	if( fMoves[iCol] != 0 )
		f += ARROW_SIZE * fMoves[iCol];
	return f;
}

float ArrowEffects::GetMoveZ(int iCol)
{
	const float* fMoves = curr_options->m_fMovesZ;
	float f = 0;
	if( fMoves[iCol] != 0 )
		f += ARROW_SIZE * fMoves[iCol];
	return f;
}

#define CENTER_LINE_Y 160	// from fYOffset == 0
#define FADE_DIST_Y 40

static float GetCenterLine()
{
	/* Another mini hack: if EFFECT_MINI is on, then our center line is at
	 * eg. 320, not 160. */
	const float fMiniPercent = curr_options->m_fEffects[PlayerOptions::EFFECT_MINI];
	const float fZoom = 1 - fMiniPercent*0.5f;
	return CENTER_LINE_Y / fZoom;
}

static float GetHiddenSudden()
{
	const float* fAppearances = curr_options->m_fAppearances;
	return fAppearances[PlayerOptions::APPEARANCE_HIDDEN] *
		fAppearances[PlayerOptions::APPEARANCE_SUDDEN];
}

//
//  -gray arrows-
// 
//  ...invisible...
//  -hidden end line-
//  -hidden start line-
//  ...visible...
//  -sudden end line-
//  -sudden start line-
//  ...invisible...
//
// TRICKY:  We fudge hidden and sudden to be farther apart if they're both on.
static float GetHiddenEndLine()
{
	return GetCenterLine() +
		FADE_DIST_Y * SCALE( GetHiddenSudden(), 0.f, 1.f, -1.0f, -1.25f ) +
		GetCenterLine() * curr_options->m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN_OFFSET];
}

static float GetHiddenStartLine()
{
	return GetCenterLine() +
		FADE_DIST_Y * SCALE( GetHiddenSudden(), 0.f, 1.f, +0.0f, -0.25f ) +
		GetCenterLine() * curr_options->m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN_OFFSET];
}

static float GetSuddenEndLine()
{
	return GetCenterLine() +
		FADE_DIST_Y * SCALE( GetHiddenSudden(), 0.f, 1.f, -0.0f, +0.25f ) +
		GetCenterLine() * curr_options->m_fAppearances[PlayerOptions::APPEARANCE_SUDDEN_OFFSET];
}

static float GetSuddenStartLine()
{
	return GetCenterLine() +
		FADE_DIST_Y * SCALE( GetHiddenSudden(), 0.f, 1.f, +1.0f, +1.25f ) +
		GetCenterLine() * curr_options->m_fAppearances[PlayerOptions::APPEARANCE_SUDDEN_OFFSET];
}

// used by ArrowGetAlpha and ArrowGetGlow below
float ArrowGetPercentVisible(float fYPosWithoutReverse, int iCol, float fYOffset)
{
	const float fDistFromCenterLine = fYPosWithoutReverse - GetCenterLine();

	float fYPos;
	if( curr_options->m_bStealthType )
		fYPos = fYOffset;
	else
		fYPos = fYPosWithoutReverse;
	
	
	if( fYPos < 0 && curr_options->m_bStealthPastReceptors == false)	// past Gray Arrows
		return 1;	// totally visible

	const float* fAppearances = curr_options->m_fAppearances;

	float fVisibleAdjust = 0;

	if( fAppearances[PlayerOptions::APPEARANCE_HIDDEN] != 0 )
	{
		float fHiddenVisibleAdjust = SCALE( fYPos, GetHiddenStartLine(), GetHiddenEndLine(), 0, -1 );
		CLAMP( fHiddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_HIDDEN] * fHiddenVisibleAdjust;
	}
	if( fAppearances[PlayerOptions::APPEARANCE_SUDDEN] != 0 )
	{
		float fSuddenVisibleAdjust = SCALE( fYPos, GetSuddenStartLine(), GetSuddenEndLine(), -1, 0 );
		CLAMP( fSuddenVisibleAdjust, -1, 0 );
		fVisibleAdjust += fAppearances[PlayerOptions::APPEARANCE_SUDDEN] * fSuddenVisibleAdjust;
	}

	if( fAppearances[PlayerOptions::APPEARANCE_STEALTH] != 0 )
		fVisibleAdjust -= fAppearances[PlayerOptions::APPEARANCE_STEALTH];
	if( curr_options->m_fStealth[iCol] != 0 ){
		fVisibleAdjust -= curr_options->m_fStealth[iCol];
	}
	if( fAppearances[PlayerOptions::APPEARANCE_BLINK] != 0 )
	{
		float f = RageFastSin(ArrowEffects::GetTime()*10);
		f = Quantize( f, BLINK_MOD_FREQUENCY );
		fVisibleAdjust += SCALE( f, 0, 1, -1, 0 );
	}
	if( fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH] != 0 )
	{
		const float fRealFadeDist = 80;
		fVisibleAdjust += SCALE( fabsf(fDistFromCenterLine), fRealFadeDist, 2*fRealFadeDist, -1, 0 )
			* fAppearances[PlayerOptions::APPEARANCE_RANDOMVANISH];
	}

	return clamp( 1+fVisibleAdjust, 0, 1 );
}

float ArrowEffects::GetAlpha( const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar)
{
	// Get the YPos without reverse (that is, factor in EFFECT_TIPSY).
	float fYPosWithoutReverse = ArrowEffects::GetYPos(pPlayerState, iCol, fYOffset, fYReverseOffsetPixels, false );

	float fPercentVisible = ArrowGetPercentVisible(fYPosWithoutReverse, iCol, fYOffset);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;


	float fFullAlphaY = fDrawDistanceBeforeTargetsPixels*(1-fFadeInPercentOfDrawFar);
	if( fYPosWithoutReverse > fFullAlphaY )
	{
		float f = SCALE( fYPosWithoutReverse, fFullAlphaY, fDrawDistanceBeforeTargetsPixels, 1.0f, 0.0f );
		return f;
	}
	return (fPercentVisible>0.5f) ? 1.0f : 0.0f;
}

float ArrowEffects::GetGlow( const PlayerState* pPlayerState, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar)
{
	// Get the YPos without reverse (that is, factor in EFFECT_TIPSY).
	float fYPosWithoutReverse = ArrowEffects::GetYPos(pPlayerState, iCol, fYOffset, fYReverseOffsetPixels, false );

	float fPercentVisible = ArrowGetPercentVisible(fYPosWithoutReverse, iCol, fYOffset);

	if( fPercentFadeToFail != -1 )
		fPercentVisible = 1 - fPercentFadeToFail;

	const float fDistFromHalf = fabsf( fPercentVisible - 0.5f );
	return SCALE( fDistFromHalf, 0, 0.5f, 1.3f, 0 );
}

float ArrowEffects::GetBrightness( const PlayerState* pPlayerState, float fNoteBeat )
{
	if( GAMESTATE->IsEditing() )
		return 1;

	float fSongBeat = pPlayerState->m_Position.m_fSongBeatVisible;
	float fBeatsUntilStep = fNoteBeat - fSongBeat;

	float fBrightness = SCALE( fBeatsUntilStep, 0, -1, 1.f, 0.f );
	CLAMP( fBrightness, 0, 1 );
	return fBrightness;
}


float ArrowEffects::GetZPos( const PlayerState* pPlayerState, int iCol, float fYOffset)
{
	float fZPos=0;
	const float* fEffects = curr_options->m_fEffects;
	const Style* pStyle = GAMESTATE->GetCurrentStyle(pPlayerState->m_PlayerNumber);
	
	// TODO: Don't index by PlayerNumber.
	const Style::ColumnInfo* pCols = pStyle->m_ColumnInfo[pPlayerState->m_PlayerNumber];
	PerPlayerData &data = g_EffectData[pPlayerState->m_PlayerNumber];

	if( fEffects[PlayerOptions::EFFECT_TORNADO_Z] != 0 )
	{
		fZPos += CalculateTornadoOffsetFromMagnitude(dim_z, iCol,
			fEffects[PlayerOptions::EFFECT_TORNADO_Z],
			fEffects[PlayerOptions::EFFECT_TORNADO_Z_OFFSET],
			fEffects[PlayerOptions::EFFECT_TORNADO_Z_PERIOD],
			pCols, pPlayerState->m_NotefieldZoom, data, fYOffset, false);
	}

	if( fEffects[PlayerOptions::EFFECT_TAN_TORNADO_Z] != 0 )
	{
		fZPos += CalculateTornadoOffsetFromMagnitude(dim_z, iCol,
			fEffects[PlayerOptions::EFFECT_TAN_TORNADO_Z],
			fEffects[PlayerOptions::EFFECT_TAN_TORNADO_Z_OFFSET],
			fEffects[PlayerOptions::EFFECT_TAN_TORNADO_Z_PERIOD],
			pCols, pPlayerState->m_NotefieldZoom, data, fYOffset, true);
	}
	
	if( fEffects[PlayerOptions::EFFECT_BUMPY] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_BUMPY] * 40*RageFastSin(
			CalculateBumpyAngle(fYOffset,
			fEffects[PlayerOptions::EFFECT_BUMPY_OFFSET],
			fEffects[PlayerOptions::EFFECT_BUMPY_PERIOD]) );

	if( curr_options->m_fBumpy[iCol] != 0 )
		fZPos += curr_options->m_fBumpy[iCol] * 40*RageFastSin(
			CalculateBumpyAngle(fYOffset,
			fEffects[PlayerOptions::EFFECT_BUMPY_OFFSET],
			fEffects[PlayerOptions::EFFECT_BUMPY_PERIOD]) );
	
	if( fEffects[PlayerOptions::EFFECT_TAN_BUMPY] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_TAN_BUMPY] * 40*SelectTanType(
			CalculateBumpyAngle(fYOffset,
			fEffects[PlayerOptions::EFFECT_TAN_BUMPY_OFFSET],
			fEffects[PlayerOptions::EFFECT_TAN_BUMPY_PERIOD]), curr_options->m_bCosecant );
		
	if( fEffects[PlayerOptions::EFFECT_ZIGZAG_Z] != 0 )
	{
		float fResult = RageTriangle( (PI * (1/(fEffects[PlayerOptions::EFFECT_ZIGZAG_Z_PERIOD]+1)) * 
			((fYOffset+(100.0f*(fEffects[PlayerOptions::EFFECT_ZIGZAG_Z_OFFSET])))/ARROW_SIZE) ) );
	    
		fZPos += (fEffects[PlayerOptions::EFFECT_ZIGZAG_Z]*ARROW_SIZE/2) * fResult;
	}
	
	if( fEffects[PlayerOptions::EFFECT_SAWTOOTH_Z] != 0 )
		fZPos += (fEffects[PlayerOptions::EFFECT_SAWTOOTH_Z]*ARROW_SIZE) * 
			((0.5f/(fEffects[PlayerOptions::EFFECT_SAWTOOTH_Z_PERIOD]+1)*fYOffset)/ARROW_SIZE - 
				floor((0.5f/(fEffects[PlayerOptions::EFFECT_SAWTOOTH_Z_PERIOD]+1)*fYOffset)/ARROW_SIZE));

	if( fEffects[PlayerOptions::EFFECT_PARABOLA_Z] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_PARABOLA_Z] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE);
	
	if( fEffects[PlayerOptions::EFFECT_ATTENUATE_Z] != 0 )
	{
		const float fXOffset = pCols[iCol].fXOffset;
		fZPos += fEffects[PlayerOptions::EFFECT_ATTENUATE_Z] * (fYOffset/ARROW_SIZE) * (fYOffset/ARROW_SIZE) * (fXOffset/ARROW_SIZE);
	}

	if( fEffects[PlayerOptions::EFFECT_DRUNK_Z] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_DRUNK_Z] * 
			( RageFastCos( CalculateDrunkAngle(fEffects[PlayerOptions::EFFECT_DRUNK_Z_SPEED], iCol, 
					fEffects[PlayerOptions::EFFECT_DRUNK_Z_OFFSET], DRUNK_Z_COLUMN_FREQUENCY,
					fYOffset, fEffects[PlayerOptions::EFFECT_DRUNK_Z_PERIOD],
					DRUNK_Z_OFFSET_FREQUENCY) ) * ARROW_SIZE*DRUNK_Z_ARROW_MAGNITUDE );

	if( fEffects[PlayerOptions::EFFECT_TAN_DRUNK_Z] != 0 )
		fZPos += fEffects[PlayerOptions::EFFECT_TAN_DRUNK_Z] * 
			( SelectTanType( CalculateDrunkAngle(fEffects[PlayerOptions::EFFECT_TAN_DRUNK_Z_SPEED],
					iCol, fEffects[PlayerOptions::EFFECT_TAN_DRUNK_Z_OFFSET],
					DRUNK_Z_COLUMN_FREQUENCY, fYOffset,
					fEffects[PlayerOptions::EFFECT_TAN_DRUNK_Z_PERIOD],
					DRUNK_Z_OFFSET_FREQUENCY)
					, curr_options->m_bCosecant) * ARROW_SIZE*DRUNK_Z_ARROW_MAGNITUDE );

	if( fEffects[PlayerOptions::EFFECT_BEAT_Z] != 0 )
	{
		const float fShift = data.m_fBeatFactor[dim_z]*RageFastSin( fYOffset / ((fEffects[PlayerOptions::EFFECT_BEAT_Z_PERIOD]*BEAT_Z_OFFSET_HEIGHT)+BEAT_Z_OFFSET_HEIGHT) + PI/BEAT_Z_PI_HEIGHT );
		fZPos += fEffects[PlayerOptions::EFFECT_BEAT_Z] * fShift;
	}
	
	if( fEffects[PlayerOptions::EFFECT_DIGITAL_Z] != 0 )
		fZPos += (fEffects[PlayerOptions::EFFECT_DIGITAL_Z] * ARROW_SIZE * 0.5f) *
			round((fEffects[PlayerOptions::EFFECT_DIGITAL_Z_STEPS]+1) * RageFastSin(
				CalculateDigitalAngle(fYOffset, 
				fEffects[PlayerOptions::EFFECT_DIGITAL_Z_OFFSET], 
				fEffects[PlayerOptions::EFFECT_DIGITAL_Z_PERIOD]) ) ) /(fEffects[PlayerOptions::EFFECT_DIGITAL_Z_STEPS]+1);
	
	if( fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z] != 0 )
		fZPos += (fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z] * ARROW_SIZE * 0.5f) *
			round((fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_STEPS]+1) * SelectTanType(
				CalculateDigitalAngle(fYOffset, 
				fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_OFFSET], 
				fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_PERIOD]), curr_options->m_bCosecant ) ) /(fEffects[PlayerOptions::EFFECT_TAN_DIGITAL_Z_STEPS]+1);
				
	
	if( fEffects[PlayerOptions::EFFECT_SQUARE_Z] != 0 )
	{
		float fResult = RageSquare( (PI * (fYOffset+(1.0f*(fEffects[PlayerOptions::EFFECT_SQUARE_Z_OFFSET]))) / 
			(ARROW_SIZE+(fEffects[PlayerOptions::EFFECT_SQUARE_Z_PERIOD]*ARROW_SIZE))) );
		fZPos += (fEffects[PlayerOptions::EFFECT_SQUARE_Z] * ARROW_SIZE * 0.5f) * fResult;
	}

	if( fEffects[PlayerOptions::EFFECT_BOUNCE_Z] != 0 )
	{
		float fBounceAmt = fabsf( RageFastSin( ( (fYOffset + (1.0f * (fEffects[PlayerOptions::EFFECT_BOUNCE_Z_OFFSET]) ) ) / 
			( 60 + (fEffects[PlayerOptions::EFFECT_BOUNCE_Z_PERIOD]*60) ) ) ) );
		
		fZPos += fEffects[PlayerOptions::EFFECT_BOUNCE_Z] * ARROW_SIZE * 0.5f * fBounceAmt;
	}

	return fZPos;
}

bool ArrowEffects::NeedZBuffer()
{
	const float* fEffects = curr_options->m_fEffects;
	// We also need to use the Z buffer if twirl is in play, because of
	// hold modulation. -vyhd (OpenITG r623)
	if( fEffects[PlayerOptions::EFFECT_BUMPY] != 0 ||
		fEffects[PlayerOptions::EFFECT_TWIRL] != 0 )
	{
		return true;
	}
	if( fEffects[PlayerOptions::EFFECT_BEAT_Z] != 0 ||
		fEffects[PlayerOptions::EFFECT_DIGITAL_Z] != 0 )
	{
		return true;
	}
	if( fEffects[PlayerOptions::EFFECT_ZIGZAG_Z] != 0 ||
		fEffects[PlayerOptions::EFFECT_SAWTOOTH_Z] != 0 )
	{
		return true;
	}
	if( fEffects[PlayerOptions::EFFECT_PARABOLA_Z] != 0 ||
		fEffects[PlayerOptions::EFFECT_SQUARE_Z] != 0 )
	{
		return true;
	}
	if( curr_options->m_bZBuffer ||
		fEffects[PlayerOptions::EFFECT_ATTENUATE_Z] != 0 )
	{
		return true;
	}
	if( fEffects[PlayerOptions::EFFECT_BOUNCE_Z] != 0 )
	{
		return true;
	}
	return false;
}

float ArrowEffects::GetZoom( const PlayerState* pPlayerState, float fYOffset, int iCol )
{
	float fZoom = 1.0f;
	// Design change:  Instead of having a flag in the style that toggles a
	// fixed zoom (0.6) that is only applied to the columns, ScreenGameplay now
	// calculates a zoom factor to apply to the notefield and puts it in the
	// PlayerState. -Kyz
	fZoom*= pPlayerState->m_NotefieldZoom;
	
	fZoom = GetZoomVariable( fYOffset, iCol, fZoom);

	float fTinyPercent = curr_options->m_fEffects[PlayerOptions::EFFECT_TINY];
	if( fTinyPercent != 0 )
	{
		fTinyPercent = powf( 0.5f, fTinyPercent );
		fZoom *= fTinyPercent;
	}
	if( curr_options->m_fTiny[iCol] != 0 )
	{
		fTinyPercent = powf( 0.5f, curr_options->m_fTiny[iCol] );
		fZoom *= fTinyPercent;
	}
	return fZoom;
}

float ArrowEffects::GetZoomVariable( float fYOffset, int iCol, float fCurZoom )
{
	float fZoom = fCurZoom;
	if( curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_INNER] != 0 || curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_OUTER] != 0 )
	{
		float sine = RageFastSin(((fYOffset+(100.0f*(curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_OFFSET])))/(0.4f*(ARROW_SIZE+(curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_PERIOD]*ARROW_SIZE)))));
		
		fZoom *= (sine*(curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_OUTER]*0.5f))+GetPulseInner();
	}
	if( curr_options->m_fEffects[PlayerOptions::EFFECT_SHRINK_TO_MULT] !=0 && fYOffset >= 0 )
		fZoom *= 1/(1+(fYOffset*(curr_options->m_fEffects[PlayerOptions::EFFECT_SHRINK_TO_MULT]/100.0f)));
		
	if( curr_options->m_fEffects[PlayerOptions::EFFECT_SHRINK_TO_LINEAR] !=0 && fYOffset >= 0 )
		fZoom += fYOffset*(0.5f*curr_options->m_fEffects[PlayerOptions::EFFECT_SHRINK_TO_LINEAR]/ARROW_SIZE);
	return fZoom;
}

float ArrowEffects::GetPulseInner()
{
	float fPulseInner = 1.0f;
	if( curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_INNER] != 0 || curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_OUTER] != 0 )
	{
		fPulseInner = ((curr_options->m_fEffects[PlayerOptions::EFFECT_PULSE_INNER]*0.5f)+1);
		if (fPulseInner == 0)
		{
			fPulseInner = 0.01f;
		}
	}
	return fPulseInner;
}

static ThemeMetric<float>	FRAME_WIDTH_EFFECTS_PIXELS_PER_SECOND( "ArrowEffects", "FrameWidthEffectsPixelsPerSecond" );
static ThemeMetric<float>	FRAME_WIDTH_EFFECTS_MIN_MULTIPLIER( "ArrowEffects", "FrameWidthEffectsMinMultiplier" );
static ThemeMetric<float>	FRAME_WIDTH_EFFECTS_MAX_MULTIPLIER( "ArrowEffects", "FrameWidthEffectsMaxMultiplier" );
static ThemeMetric<bool>	FRAME_WIDTH_LOCK_EFFECTS_TO_OVERLAPPING( "ArrowEffects", "FrameWidthLockEffectsToOverlapping" );
static ThemeMetric<float>	FRAME_WIDTH_LOCK_EFFECTS_TWEEN_PIXELS( "ArrowEffects", "FrameWidthLockEffectsTweenPixels" );

float ArrowEffects::GetFrameWidthScale( const PlayerState* pPlayerState, float fYOffset, float fOverlappedTime )
{
	float fFrameWidthMultiplier = 1.0f;

	float fPixelsPerSecond = FRAME_WIDTH_EFFECTS_PIXELS_PER_SECOND;
	float fSecond = fYOffset / fPixelsPerSecond;
	float fWidthEffect = pPlayerState->m_EffectHistory.GetSample( fSecond );
	if( fWidthEffect != 0 && FRAME_WIDTH_LOCK_EFFECTS_TO_OVERLAPPING )
	{
		// Don't display effect data that happened before this hold overlapped the top.
		float fFromEndOfOverlapped = fOverlappedTime - fSecond;
		float fTrailingPixels = FRAME_WIDTH_LOCK_EFFECTS_TWEEN_PIXELS;
		float fTrailingSeconds = fTrailingPixels / fPixelsPerSecond;
		float fScaleEffect = SCALE( fFromEndOfOverlapped, 0.0f, fTrailingSeconds, 0.0f, 1.0f );
		CLAMP( fScaleEffect, 0.0f, 1.0f );
		fWidthEffect *= fScaleEffect;
	}

	if( fWidthEffect > 0 )
		fFrameWidthMultiplier *= SCALE( fWidthEffect, 0.0f, 1.0f, 1.0f, FRAME_WIDTH_EFFECTS_MAX_MULTIPLIER );
	else if( fWidthEffect < 0 )
		fFrameWidthMultiplier *= SCALE( fWidthEffect, 0.0f, -1.0f, 1.0f, FRAME_WIDTH_EFFECTS_MIN_MULTIPLIER );

	return fFrameWidthMultiplier;
}

// To provide reasonable defaults to methods below.
ThemeMetric<float> FADE_BEFORE_TARGETS_PERCENT( "NoteField", "FadeBeforeTargetsPercent" );
ThemeMetric<float> DRAW_DISTANCE_BEFORE_TARGET_PIXELS( "Player", "DrawDistanceBeforeTargetsPixels" );
ThemeMetric<float> GRAY_ARROWS_Y_STANDARD( "Player", "ReceptorArrowsYStandard" );
ThemeMetric<float> GRAY_ARROWS_Y_REVERSE( "Player", "ReceptorArrowsYReverse" );
    
// lua start
#include "LuaBinding.h"

namespace
{
	/* Update() need to be exposed to use ArrowEffects off ScreenGameplay. It is harmless.	 */
	int Update( lua_State *L )	{ ArrowEffects::Update(); return 0; }

	// Provide a reasonable default value for fYReverseOffset
	float YReverseOffset( lua_State *L, int argnum )
	{
		float fYReverseOffsetPixels = GRAY_ARROWS_Y_REVERSE - GRAY_ARROWS_Y_STANDARD;
		if( lua_gettop(L) >= argnum && !lua_isnil(L, argnum) )
		{
			fYReverseOffsetPixels = FArg(argnum);
		}
		return fYReverseOffsetPixels;
	}
	
	// ( PlayerState ps, int iCol, float fNoteBeat )
	int GetYOffset( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		float fPeakYOffset;
		bool bIsPastPeak;

		lua_pushnumber( L, ArrowEffects::GetYOffset( ps, IArg(2)-1, FArg(3), fPeakYOffset, bIsPastPeak ) );
		lua_pushnumber( L, fPeakYOffset );
		lua_pushboolean( L, bIsPastPeak );
		return 3;
	}

	// ( PlayerState ps, int iCol, float fYOffset, float fYReverseOffsetPixels )
	int GetYPos( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		float fYReverseOffsetPixels = YReverseOffset( L, 4 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber(L, ArrowEffects::GetYPos(ps, IArg(2)-1, FArg(3), fYReverseOffsetPixels));
		return 1;
	}

	// ( PlayerState ps, int iCol, float fYPos, float fYReverseOffsetPixels )
	int GetYOffsetFromYPos( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		float fYReverseOffsetPixels = YReverseOffset( L, 4 );
		lua_pushnumber(L, ArrowEffects::GetYOffsetFromYPos(IArg(2)-1, FArg(3), fYReverseOffsetPixels));
		return 1;
	}

	// ( PlayerState ps, int iCol, float fYOffset )
	int GetXPos( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber( L, ArrowEffects::GetXPos( ps, IArg(2)-1, FArg(3) ) );
		return 1;
	}

	// ( PlayerState ps, int iCol, float fYOffset )
	int GetZPos( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber(L, ArrowEffects::GetZPos( ps, IArg(2)-1, FArg(3)));
		return 1;
	}

	// ( PlayerState ps, float fYOffset, int iCol )
	int GetRotationX( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		bool bIsHoldCap = false;
		lua_pushnumber(L, ArrowEffects::GetRotationX(ps,FArg(2), bIsHoldCap, IArg(3)-1));
		return 1;
	}

	// ( PlayerState ps, float fYOffset, int iCol )
	int GetRotationY( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber(L, ArrowEffects::GetRotationY(ps, FArg(2), IArg(3)-1));
		return 1;
	}

	// ( PlayerState ps, float fNoteBeat, bool bIsHoldHead, int iCol )
	int GetRotationZ( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		// Make bIsHoldHead optional.
		bool bIsHoldHead = false;
		if( lua_gettop(L) >= 3 && !lua_isnil(L, 3) )
		{
			bIsHoldHead = BArg(3);
		}
		lua_pushnumber( L, ArrowEffects::GetRotationZ( ps, FArg(2), bIsHoldHead, IArg(4)-1 ) );
		return 1;
	}

	// ( PlayerState ps, int iCol )
	int ReceptorGetRotationZ( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber( L, ArrowEffects::ReceptorGetRotationZ( ps, IArg(2)-1 ) );
		return 1;
	}

	//( PlayerState ps, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
	int GetAlpha( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		// Provide reasonable default values.
		float fPercentFadeToFail = -1;
		float fYReverseOffsetPixels = YReverseOffset( L, 5 );
		float fDrawDistanceBeforeTargetsPixels = DRAW_DISTANCE_BEFORE_TARGET_PIXELS;
		float fFadeInPercentOfDrawFar = FADE_BEFORE_TARGETS_PERCENT;
		if( lua_gettop(L) >= 4 && !lua_isnil(L, 4) )
		{
			fPercentFadeToFail = FArg(4);
		}
		if( lua_gettop(L) >= 6 && !lua_isnil(L, 6) )
		{
			fDrawDistanceBeforeTargetsPixels = FArg(6);
		}
		if( lua_gettop(L) >= 7 && !lua_isnil(L, 7) )
		{
			fFadeInPercentOfDrawFar = FArg(7);
		}
		lua_pushnumber(L, ArrowEffects::GetAlpha(ps, IArg(2)-1, FArg(3), fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar));
		return 1;
	}

	
	//( PlayerState ps, int iCol, float fYOffset, float fPercentFadeToFail, float fYReverseOffsetPixels, float fDrawDistanceBeforeTargetsPixels, float fFadeInPercentOfDrawFar )
	int GetGlow( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		// Provide reasonable default values.
		float fPercentFadeToFail = -1; // 
		float fYReverseOffsetPixels = YReverseOffset( L, 5 );
		float fDrawDistanceBeforeTargetsPixels = DRAW_DISTANCE_BEFORE_TARGET_PIXELS;
		float fFadeInPercentOfDrawFar = FADE_BEFORE_TARGETS_PERCENT;
		if( lua_gettop(L) >= 4 && !lua_isnil(L, 4) )
		{
			fPercentFadeToFail = FArg(4);
		}
		if( lua_gettop(L) >= 6 && !lua_isnil(L, 6) )
		{
			fDrawDistanceBeforeTargetsPixels = FArg(6);
		}
		if( lua_gettop(L) >= 7 && !lua_isnil(L, 7) )
		{
			fFadeInPercentOfDrawFar = FArg(7);
		}
		lua_pushnumber( L, ArrowEffects::GetGlow(ps, IArg(2)-1, FArg(3), fPercentFadeToFail, fYReverseOffsetPixels, fDrawDistanceBeforeTargetsPixels, fFadeInPercentOfDrawFar ) );
		return 1;
	}
	
	// ( PlayerState ps, float fNoteBeat )
	int GetBrightness( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber( L, ArrowEffects::GetBrightness( ps, FArg(2) ) );
		return 1;
	}

	// ( PlayerState ps )
	int NeedZBuffer( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushboolean(L, ArrowEffects::NeedZBuffer());
		return 1;
	}
	
	// ( PlayerState ps, float fYOffset, int iCol )
	int GetZoom( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());
		lua_pushnumber( L, ArrowEffects::GetZoom( ps, FArg(2), IArg(3)-1 ) );
		return 1;
	}
	
	// ( PlayerState ps, float fYOffset, fOverlappedTime )
	int GetFrameWidthScale( lua_State *L )
	{
		PlayerState *ps = Luna<PlayerState>::check( L, 1 );
		ArrowEffects::SetCurrentOptions(&ps->m_PlayerOptions.GetCurrent());

		// Make fOverlappedTime optional.
		float fOverlappedTime = 0;
		if( lua_gettop(L) >= 3 && !lua_isnil(L, 3) )
		{
			fOverlappedTime = FArg(3);
		}
		lua_pushnumber( L, ArrowEffects::GetFrameWidthScale( ps, FArg(2), fOverlappedTime ) );
		return 1;
	}

	const luaL_Reg ArrowEffectsTable[] =
	{
		LIST_METHOD( Update ),
		LIST_METHOD( GetYOffset ),
		LIST_METHOD( GetYPos ),
		LIST_METHOD( GetYOffsetFromYPos ),
		LIST_METHOD( GetXPos ),
		LIST_METHOD( GetZPos ),
		LIST_METHOD( GetRotationX ),
		LIST_METHOD( GetRotationY ),
		LIST_METHOD( GetRotationZ ),
		LIST_METHOD( ReceptorGetRotationZ ),
		LIST_METHOD( GetAlpha ),
		LIST_METHOD( GetGlow ),
		LIST_METHOD( GetBrightness ),
		LIST_METHOD( NeedZBuffer ),
		LIST_METHOD( GetZoom ),
		LIST_METHOD( GetFrameWidthScale ),
		{ nullptr, nullptr }
	};
}

LUA_REGISTER_NAMESPACE( ArrowEffects )

/*
 * (c) 2001-2004 Chris Danford
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
