#include "global.h"
#include "ArrowDefects.h"
#include "GameState.h"
#include "PlayerOptions.h"
#include "RageMath.hpp"
#include "RageTimer.h"
#include "ScreenDimensions.h"
#include "TimingData.h"

// Many comments copied from ArrowEffects to explain some mods.  Any comment
// in this file not signed by me was not written by me. -Kyz

// NOT using theme metrics for all these mod variables because this mod
// system is deprecated.  Anybody who wants tunable mods should use the new
// mod system provided in ModValue.cpp.  Additionally, anybody who was
// setting these metrics was screwing over gimmick authors more than the
// system inherently already does. -Kyz

static float const arrow_spacing= 64.0f;
static bool const hidden_sudden_past_receptor= true;
static float const blink_mod_frequency= 0.3333f;
static float const boost_mod_min_clamp= -400.0f;
static float const	boost_mod_max_clamp= 400.0f;
static float const	brake_mod_min_clamp= -400.0f;
static float const	brake_mod_max_clamp= 400.0f;
static float const	wave_mod_magnitude= 20.0f;
static float const	wave_mod_height= 38.0f;
static float const	boomerang_peak_percentage= .75f;
static float const	expand_multiplier_frequency= 3.0f;
static float const	expand_multiplier_scale_from_low= -1.0f;
static float const	expand_multiplier_scale_from_high= 1.0f;
static float const	expand_multiplier_scale_to_low= .75f;
static float const	expand_multiplier_scale_to_high= 1.75f;
static float const	expand_speed_scale_from_low= 0.0f;
static float const	expand_speed_scale_from_high= 1.0f;
static float const	expand_speed_scale_to_low= 1.0f;
static float const	tipsy_timer_frequency= 1.2f;
static float const	tipsy_column_frequency= 1.8f;
static float const	tipsy_arrow_magnitude= .4f;
static float const	tornado_position_scale_to_low= -1.0f;
static float const	tornado_position_scale_to_high= 1.0f;
static float const	tornado_offset_frequency= 6.0f;
static float const	tornado_offset_scale_from_low= -1.0f;
static float const	tornado_offset_scale_from_high= 1.0f;
static float const	drunk_column_frequency= .2f;
static float const	drunk_offset_frequency= 10.0f;
static float const	drunk_arrow_magnitude= .5f;
static float const	beat_offset_height= 15.0f;
static float const	beat_pi_height= 2.0f;
static float const	tiny_percent_base= .5f;
static float const	tiny_percent_gate= 1.0f;
static bool const	dizzy_hold_heads= false;

float const center_line_y= 160.f; // from y_offset == 0
float const fade_dist_y= 40.f;

ArrowDefects::ArrowDefects()
	:m_options(nullptr)
{
	m_receptor_pos_normal= THEME->GetMetricF("Player", "ReceptorArrowsYStandard");
	m_receptor_pos_reverse= THEME->GetMetricF("Player", "ReceptorArrowsYReverse");
	m_reverse_offset= (m_receptor_pos_reverse - m_receptor_pos_normal) * .5f;
}

float ArrowDefects::get_notefield_height()
{
	return SCREEN_HEIGHT + fabsf(m_options->m_fTilt)*200;
}

void ArrowDefects::set_player_options(PlayerOptions const* options)
{
	m_options= options;
	if(m_options != nullptr)
	{
		if(m_options->m_pn == PLAYER_1)
		{
			m_xmode_dir= 1.f;
		}
		else
		{
			m_xmode_dir= -1.f;
		}
	}
}

void ArrowDefects::set_column_pos(std::vector<float>& column_x)
{
	m_num_columns= column_x.size();
	for(auto&& member : {&m_min_tornado_x, &m_max_tornado_x, &m_invert_dist,
				&m_tipsy_result})
	{
		member->resize(m_num_columns);
	}
	m_column_x= column_x;

	// Update positions used by invert mod.
	// This is completely wrong for stepstypes that have a different number
	// of panels on each pad, but I don't care because those don't exist yet
	// and this is the old deprecated mod system. -Kyz
	int const num_cols_per_pad= m_num_columns / m_num_pads;
	int const col_left_of_middle= (num_cols_per_pad-1) / 2;
	int const col_right_of_middle= (num_cols_per_pad+1) / 2;
	for(size_t col= 0; col < m_num_columns; ++col)
	{
		int const side_index= col / num_cols_per_pad;
		int const col_on_side= col % num_cols_per_pad;
		int first_col_on_side= -1;
		int last_col_on_side= -1;
		if(col_on_side <= col_left_of_middle)
		{
			first_col_on_side= 0;
			last_col_on_side= col_left_of_middle;
		}
		else if(col_on_side >= col_right_of_middle)
		{
			first_col_on_side= col_right_of_middle;
			last_col_on_side= num_cols_per_pad - 1;
		}
		else
		{
			first_col_on_side= col_on_side / 2;
			last_col_on_side= col_on_side / 2;
		}
		// mirror
		int new_col_on_side;
		if(first_col_on_side == last_col_on_side)
		{
			new_col_on_side= 0;
		}
		else
		{
			new_col_on_side= Rage::scale(col_on_side, first_col_on_side,
				last_col_on_side, last_col_on_side, first_col_on_side);
		}
		int const new_col= (side_index * num_cols_per_pad) + new_col_on_side;
		float const old_pixel_offset= m_column_x[col];
		float const new_pixel_offset= m_column_x[new_col];
		m_invert_dist[col]= new_pixel_offset - old_pixel_offset;
	}
}

void ArrowDefects::set_timing(TimingData const* timing)
{
	m_timing_data= timing;
}

void ArrowDefects::set_num_pads(int num)
{
	m_num_pads= num;
}

void ArrowDefects::set_read_bpm(float read_bpm)
{
	m_read_bpm= read_bpm;
}

void ArrowDefects::update(float music_beat, float music_second)
{
	m_music_beat= music_beat;
	m_music_second= music_second;
	m_display_beat= m_timing_data->GetDisplayedBeat(m_music_beat);
	m_speed_percent= m_timing_data->GetDisplayedSpeedPercent(m_music_beat, m_music_second);

	float const* effects= m_options->m_fEffects;
	float const* accels= m_options->m_fAccels;
	if(accels[PlayerOptions::ACCEL_EXPAND] != 0.f)
	{
		TimingData::GetBeatArgs beat_data;
		beat_data.elapsed_time= m_music_second;
		m_timing_data->GetBeatAndBPSFromElapsedTime(beat_data);
		m_expand_seconds= fmodf(m_music_second - beat_data.total_stop_time, Rage::PI * 2.0);
	}
	if(effects[PlayerOptions::EFFECT_TORNADO] != 0.f)
	{
		for(size_t col= 0; col < m_num_columns; ++col)
		{
			// TRICKY: Tornado is very unplayable in doubles, so use a smaller
			// tornado width if there are many columns

			/* the below makes an assumption for dance mode.
			 * perhaps check if we are actually playing on singles without,
			 * say more than 6 columns. That would exclude IIDX, pop'n, and
			 * techno-8, all of which would be very hectic.
			 * certain non-singles modes (like halfdoubles 6cols)
			 * could possibly have tornado enabled.
			 * let's also take default resolution (640x480) into mind. -aj */
			bool wide_field= m_num_columns > 4;
			int tornado_width= wide_field ? 2 : 3;
			int start_col= col - tornado_width;
			int end_col= col + tornado_width;
			start_col= Rage::clamp(start_col, 0, static_cast<int>(m_num_columns));
			end_col= Rage::clamp(end_col, 0, static_cast<int>(m_num_columns));

			m_min_tornado_x[col]= std::numeric_limits<float>::max();
			m_max_tornado_x[col]= std::numeric_limits<float>::min();

			for(int i= start_col; i <= end_col; ++i)
			{
				m_min_tornado_x[col]= std::min(m_min_tornado_x[col], m_column_x[i]);
				m_max_tornado_x[col]= std::max(m_max_tornado_x[col], m_column_x[i]);
			}
		}
	}
	if(effects[PlayerOptions::EFFECT_TIPSY] != 0.f)
	{
		float const time_times_timer= m_music_second * tipsy_timer_frequency;
		float const arrow_times_mag= arrow_spacing * tipsy_arrow_magnitude;
		for(size_t col= 0; col < m_num_columns; ++col)
		{
			m_tipsy_result[col]= Rage::FastCos(time_times_timer +
				(col * tipsy_column_frequency)) * arrow_times_mag;
		}
	}
	else
	{
		for(size_t col= 0; col < m_num_columns; ++col)
		{
			m_tipsy_result[col]= 0;
		}
	}
	if(effects[PlayerOptions::EFFECT_BEAT] != 0.f)
	{
		float const accel_time= .2f;
		float const total_time= .5f;
		float beat= m_music_beat + accel_time;
		m_beat_factor= 0;
		if(beat >= 0.f)
		{
			// beat can't be negative because of the preceding condition. -Kyz
			// 100.2 -> 0.2
			beat-= std::trunc(beat);
			if(beat < total_time)
			{
				if(beat < accel_time)
				{
					m_beat_factor= Rage::scale(beat, 0.f, accel_time, 0.f, 1.f);
					m_beat_factor*= m_beat_factor;
				}
				else
				{
					m_beat_factor= Rage::scale(beat, accel_time, total_time, 1.f, 0.f);
					m_beat_factor= 1 - ((1-m_beat_factor) * (1-m_beat_factor));
				}
				if((int(beat) % 2) != 0)
				{
					m_beat_factor*= -1;
				}
				m_beat_factor*= 20.f;
			}
		}
	}
	else
	{
		m_beat_factor= 0;
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

	// Kyz note:  This irritates me because it's one more weird side effect.
	// Somebody wants the hidden line at a particular place, so they adjust its
	// offset, then they adjust the sudden offset, then they get screwed when
	// both are on at once.  There's also the odd behavior of turning hidden
	// above 100% moving the lines around. -Kyz
	float hidden_offset= m_options->m_fAppearances[
		PlayerOptions::APPEARANCE_HIDDEN_OFFSET];
	float sudden_offset= m_options->m_fAppearances[
		PlayerOptions::APPEARANCE_SUDDEN_OFFSET];
	float hidden_sudden= get_hidden_sudden();
	float center_line= get_center_line();
	m_hidden_end_line= center_line + (fade_dist_y * Rage::scale(hidden_sudden,
			0.f, 1.f, -1.f, -1.25f)) + (center_line * hidden_offset);
	m_hidden_start_line= center_line + (fade_dist_y*Rage::scale(hidden_sudden,
			0.f, 1.f, 0.f, -.25f)) + (center_line * hidden_offset);
	m_sudden_end_line= center_line + (fade_dist_y * Rage::scale(hidden_sudden,
			0.f, 1.f, 0.f, .25f)) + (center_line * sudden_offset);
	m_sudden_start_line= center_line + (fade_dist_y*Rage::scale(hidden_sudden,
			0.f, 1.f, 1.f, 1.25f)) + (center_line * sudden_offset);
}

float ArrowDefects::get_y_offset(float note_beat, float note_second, size_t col)
{
	float y_offset= 0.f;
	/* Usually, fTimeSpacing is 0 or 1, in which case we use entirely beat
	 * spacing or entirely time spacing (respectively). Occasionally, we tween
	 * between them. */
	if(m_options->m_fTimeSpacing != 1.f)
	{
		y_offset= m_timing_data->GetDisplayedBeat(note_beat) - m_display_beat;
		y_offset*= m_speed_percent;
		y_offset*= 1 - m_options->m_fTimeSpacing;
	}
	if(m_options->m_fTimeSpacing != 0.f)
	{
		float seconds_until_note= note_second - m_music_second;
		float bps= m_options->m_fScrollBPM / 60.f / GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;
		float time_space_offset= seconds_until_note * bps;
		y_offset+= time_space_offset * m_options->m_fTimeSpacing;
	}
	y_offset*= arrow_spacing;

	float scroll_speed= m_options->m_fScrollSpeed;
	if(m_options->m_fMaxScrollBPM != 0.f)
	{
		scroll_speed= m_options->m_fMaxScrollBPM / (m_read_bpm * GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate);
	}
	// don't mess with the arrows after they've crossed 0
	if(y_offset < 0.f)
	{
		return y_offset * scroll_speed;
	}
	float const* accels= m_options->m_fAccels;
	float y_adjust= 0.f;
	if(accels[PlayerOptions::ACCEL_BOOST] != 0.f)
	{
		float effect_height= get_notefield_height();
		float new_y_offset= y_offset * 1.5f / ((y_offset + effect_height / 1.2f)
			/ effect_height);
		float accel_y_adjust= accels[PlayerOptions::ACCEL_BOOST] *
			(new_y_offset - y_offset);
		// TRICKY: Clamp this value, or else BOOST+BOOMERANG will draw a ton of
		// arrows on the screen.
		accel_y_adjust= Rage::clamp(accel_y_adjust, boost_mod_min_clamp, boost_mod_max_clamp);
		y_adjust+= accel_y_adjust;
	}
	if(accels[PlayerOptions::ACCEL_BRAKE] != 0.f)
	{
		float effect_height= get_notefield_height();
		float scale= Rage::scale(y_offset, 0.f, effect_height, 0.f, 1.f);
		float new_y_offset= y_offset * scale;
		float brake_y_adjust= accels[PlayerOptions::ACCEL_BRAKE] *
			(new_y_offset - y_offset);
		// TRICKY: Clamp this value the same way as BOOST so that in BOOST+BRAKE,
		// BRAKE doesn't overpower BOOST
		brake_y_adjust= Rage::clamp(brake_y_adjust, brake_mod_min_clamp, brake_mod_max_clamp);
		y_adjust+= brake_y_adjust;
	}
	if(accels[PlayerOptions::ACCEL_WAVE] != 0)
	{
		y_adjust+= accels[PlayerOptions::ACCEL_WAVE] * wave_mod_magnitude *
			Rage::FastSin(y_offset / wave_mod_height);
	}
	y_offset+= y_adjust;
	if(accels[PlayerOptions::ACCEL_BOOMERANG] != 0.f)
	{
		y_offset= (-1*y_offset*y_offset/SCREEN_HEIGHT) + 1.5f*y_offset;
	}
	if(m_options->m_fRandomSpeed > 0.f)
	{
		// Generate a deterministically "random" speed for each arrow.
		uint32_t seed= GAMESTATE->m_iStageSeed + (BeatToNoteRow(note_beat) << 8)
			+ (col * 100);
		for(int i= 0; i < 3; ++i)
		{
			seed= ((seed * 1664525u) + 1013904223u) & 0xFFFFFFFF;
		}
		float random= seed / 4294967296.f;
		// Random speed always increases speed: a random speed of 10 indicates
		// [1,11]. This keeps it consistent with other mods: 0 means no effect.
		scroll_speed*= Rage::scale(random, 0.f, 1.f, 1.f, m_options->m_fRandomSpeed + 1.f);
	}
	if(accels[PlayerOptions::ACCEL_EXPAND] != 0.f)
	{
		float expand_multiplier= Rage::scale(
			Rage::FastCos(m_expand_seconds * expand_multiplier_frequency),
			expand_multiplier_scale_from_low, expand_multiplier_scale_from_high,
			expand_multiplier_scale_to_low, expand_multiplier_scale_to_high);
		scroll_speed*= Rage::scale(accels[PlayerOptions::ACCEL_EXPAND],
			expand_multiplier_scale_from_low, expand_multiplier_scale_from_high,
			expand_multiplier_scale_to_low, expand_multiplier);
	}
	y_offset*= scroll_speed;
	return y_offset;
}

float ArrowDefects::get_x_pos(size_t col, float y_offset)
{
	float pixel_offset_from_center= 0.f;
	float const* effects= m_options->m_fEffects;
	if(effects[PlayerOptions::EFFECT_TORNADO] != 0.f)
	{
		float const position_between= Rage::scale(m_column_x[col],
			m_min_tornado_x[col], m_max_tornado_x[col],
			tornado_position_scale_to_low, tornado_position_scale_to_high);
		float rads= std::acos(position_between);
		rads+= y_offset * tornado_offset_frequency / SCREEN_HEIGHT;
		float const adjusted_pixel_offset= Rage::scale(Rage::FastCos(rads),
			tornado_offset_scale_from_low, tornado_offset_scale_from_high,
			m_min_tornado_x[col], m_max_tornado_x[col]);
		pixel_offset_from_center+= (adjusted_pixel_offset - m_column_x[col]) *
			effects[PlayerOptions::EFFECT_TORNADO];
	}
	if(effects[PlayerOptions::EFFECT_DRUNK] != 0.f)
	{
		pixel_offset_from_center+= effects[PlayerOptions::EFFECT_DRUNK] *
			(Rage::FastCos(m_music_second + (col * drunk_column_frequency) +
				((y_offset * drunk_offset_frequency) / SCREEN_HEIGHT)) *
				arrow_spacing * drunk_arrow_magnitude);
	}
	if(effects[PlayerOptions::EFFECT_FLIP] != 0.f)
	{
		int const new_col= (m_num_columns - 1) - col;
		float const old_pixel_offset= m_column_x[col];
		float const new_pixel_offset= m_column_x[new_col];
		float const distance= new_pixel_offset - old_pixel_offset;
		pixel_offset_from_center+= distance * effects[PlayerOptions::EFFECT_FLIP];
	}
	if(effects[PlayerOptions::EFFECT_INVERT] != 0.f)
	{
		pixel_offset_from_center+= m_invert_dist[col] *
			effects[PlayerOptions::EFFECT_INVERT];
	}
	if(effects[PlayerOptions::EFFECT_BEAT] != 0.f)
	{
		float const shift= m_beat_factor * Rage::FastSin(y_offset /
			beat_offset_height + Rage::PI / beat_pi_height);
		pixel_offset_from_center+= effects[PlayerOptions::EFFECT_BEAT] * shift;
	}
	if(effects[PlayerOptions::EFFECT_XMODE] != 0.f)
	{
		// based off of code by v1toko for StepNXA, except it should work on
		// any gametype now.
		float offset_dir= 1.f;
		if(m_num_pads == 1)
		{
			// The old ArrowEffects system made P1's arrows come from the right and
			// P2's arrows from the left.  Since we don't know the player number,
			// the direction has to be set during init. -Kyz
			offset_dir= m_xmode_dir;
		}
		else
		{
			// find the middle, and split based on column
			// it's unknown if this will work for routine.
			const size_t middle_column= static_cast<size_t>(floor(m_num_columns/2.f));
			if(col > middle_column-1)
			{
				offset_dir= -1.f;
			}
			else
			{
				offset_dir= 1.f;
			}
		}
		pixel_offset_from_center+= effects[PlayerOptions::EFFECT_XMODE] *
			offset_dir * y_offset;
	}
	pixel_offset_from_center+= m_column_x[col];
	if(effects[PlayerOptions::EFFECT_TINY] != 0.f)
	{
		// Allow Tiny to pull tracks together, but not to push them apart.
		float tiny_percent= effects[PlayerOptions::EFFECT_TINY];
		tiny_percent= std::min(std::pow(tiny_percent_base, tiny_percent),
			tiny_percent_gate);
		pixel_offset_from_center*= tiny_percent;
	}
	return pixel_offset_from_center;
}

float ArrowDefects::get_y_pos(size_t col, float y_offset)
{
	float const* effects= m_options->m_fEffects;
	y_offset+= effects[PlayerOptions::EFFECT_TIPSY] * m_tipsy_result[col];
	// In beware's DDR Extreme-focused fork of StepMania 3.9, this value is
	// floored, making arrows show on integer Y coordinates. Supposedly it makes
	// the arrows look better, but testing needs to be done.
	// todo: make this a noteskin metric instead of a theme metric? -aj
	// That doesn't even make logical sense, because due to perspective and
	// theme size variations, an integer y offset from the mods doesn't mean
	// the note will be on an integer pixel position when it's rendered on the
	// screen.  So I'm not supporting quantizing the position. -Kyz
	return y_offset;
}

float ArrowDefects::get_z_pos(float y_offset)
{
	if(m_options->m_fEffects[PlayerOptions::EFFECT_BUMPY] != 0.f)
	{
		return m_options->m_fEffects[PlayerOptions::EFFECT_BUMPY] *
			40 * Rage::FastSin(y_offset / 16.f);
	}
	return 0.f;
}

float ArrowDefects::get_rotation_y(float y_offset)
{
	if(m_options->m_fEffects[PlayerOptions::EFFECT_TWIRL] != 0.f)
	{
		return Rage::DegreesToRadians(
			m_options->m_fEffects[PlayerOptions::EFFECT_TWIRL]*(y_offset*.5f));
	}
	return 0.f;
}

float ArrowDefects::get_zoom()
{
	float tiny_percent= m_options->m_fEffects[PlayerOptions::EFFECT_TINY];
	if(tiny_percent != 0.f)
	{
		tiny_percent= std::pow(.5f, tiny_percent);
		return tiny_percent;
	}
	return 1.f;
}

void ArrowDefects::get_glow_alpha(size_t col, float y_offset,
	Rage::transform& trans)
{
	float offset_with_tipsy= get_y_pos(col, y_offset);
	float percent_visible= get_percent_visible(offset_with_tipsy);
	float const dist_from_half= fabsf(percent_visible - .5f);
	// Logic that looked at fDrawDistanceBeforeTargetsPixels removed because
	// when I put a break point in, it never triggered. -Kyz
	trans.alpha= (percent_visible > .5f) ? 1.f : 0.f;
	trans.glow= Rage::scale(dist_from_half, 0.f, .5f, 1.3f, 0.f);
}

void ArrowDefects::get_transform(float note_beat, float y_offset,
	float shifted_offset, size_t col, Rage::transform& trans)
{
	float const* effects= m_options->m_fEffects;
	trans.pos.x= get_x_pos(col, y_offset);
	// get_y_pos is passed the reverse shifted y offset to avoid applying the
	// shift wrong. -Kyz
	trans.pos.y= get_y_pos(col, shifted_offset);
	trans.pos.z= get_z_pos(y_offset);
	trans.rot.x= 0.f;
	trans.rot.y= get_rotation_y(y_offset);
	trans.rot.z= 0.f;
	trans.zoom.x= trans.zoom.y= trans.zoom.z= get_zoom();
	if(effects[PlayerOptions::EFFECT_ROLL] != 0.f)
	{
		trans.rot.x= Rage::DegreesToRadians(
			effects[PlayerOptions::EFFECT_ROLL] * y_offset * .5f);
	}
	if(effects[PlayerOptions::EFFECT_CONFUSION] != 0.f)
	{
		trans.rot.z+= fmodf(m_music_beat *
			effects[PlayerOptions::EFFECT_CONFUSION], 2.0 * Rage::PI);
	}
	if(effects[PlayerOptions::EFFECT_DIZZY] != 0.f)
	{
		trans.rot.z+= fmodf((note_beat - m_music_beat) *
			effects[PlayerOptions::EFFECT_DIZZY], 2.0 * Rage::PI);
	}
}

void ArrowDefects::get_transform_with_glow_alpha(float note_beat,
	float y_offset, float shifted_offset, size_t col, Rage::transform& trans)
{
	get_transform(note_beat, y_offset, shifted_offset, col, trans);
	get_glow_alpha(col, y_offset, trans);
}

void ArrowDefects::hold_render_transform(float y_offset, size_t col,
	Rage::transform& trans)
{
	trans.pos.x= get_x_pos(col, y_offset);
	// get_y_pos is passed a y offset of 0 because the hold rendering logic
	// applies the reverse shift. -Kyz
	trans.pos.y= get_y_pos(col, 0.f);
	trans.pos.z= get_z_pos(y_offset);
	trans.rot.x= 0.f;
	trans.rot.y= get_rotation_y(y_offset);
	trans.rot.z= 0.f;
	trans.zoom.x= trans.zoom.y= trans.zoom.z= get_zoom();
	get_glow_alpha(col, y_offset, trans);
}

float ArrowDefects::get_center_line()
{
	// Another mini hack: if EFFECT_MINI is on, then our center line is at
	// eg. 320, not 160.
	float const zoom= 1 - (m_options->m_fEffects[PlayerOptions::EFFECT_MINI] * .5f);
	return center_line_y / zoom;
}

float ArrowDefects:: get_hidden_sudden()
{
	return m_options->m_fAppearances[PlayerOptions::APPEARANCE_HIDDEN] *
		m_options->m_fAppearances[PlayerOptions::APPEARANCE_SUDDEN];
}

float ArrowDefects::get_percent_visible(float y_offset)
{
	float dist_from_center_line= y_offset - get_center_line();
	if(y_offset < 0)
	{
		return 1.f;
	}
	float const* appearances= m_options->m_fAppearances;
	float visible_adjust= 0;
	if(appearances[PlayerOptions::APPEARANCE_HIDDEN] != 0.f)
	{
		float hidden_visible_adjust= Rage::scale(y_offset, m_hidden_start_line,
			m_hidden_end_line, 0.f, -1.f);
		hidden_visible_adjust= Rage::clamp(hidden_visible_adjust, -1.f, 0.f);
		visible_adjust+= appearances[PlayerOptions::APPEARANCE_HIDDEN] *
			hidden_visible_adjust;
	}
	if(appearances[PlayerOptions::APPEARANCE_SUDDEN] != 0.f)
	{
		float sudden_visible_adjust= Rage::scale(y_offset, m_sudden_start_line,
			m_sudden_end_line, -1.f, 0.f);
		sudden_visible_adjust= Rage::clamp(sudden_visible_adjust, -1.f, 0.f);
		visible_adjust+= appearances[PlayerOptions::APPEARANCE_SUDDEN] *
			sudden_visible_adjust;
	}
	visible_adjust-= appearances[PlayerOptions::APPEARANCE_STEALTH];
	if(appearances[PlayerOptions::APPEARANCE_BLINK] != 0.f)
	{
		float blink= Rage::FastSin(m_music_second * 10);
		blink= Quantize(blink, blink_mod_frequency);
		visible_adjust+= Rage::scale(blink, 0.f, 1.f, -1.f, 0.f);
	}
	if(appearances[PlayerOptions::APPEARANCE_RANDOMVANISH] != 0.f)
	{
		float const real_fade_dist= 80.f;
		visible_adjust+= Rage::scale(fabsf(dist_from_center_line),
			real_fade_dist, 2.f * real_fade_dist, -1.f, 0.f) *
			appearances[PlayerOptions::APPEARANCE_RANDOMVANISH];
	}
	return Rage::clamp(1.f + visible_adjust, 0.f, 1.f);
}

float ArrowDefects::get_field_y()
{
	return (m_receptor_pos_normal + m_receptor_pos_reverse) * .5f;
}

float ArrowDefects::get_receptor_alpha()
{
	return Rage::clamp(1.f - m_options->m_fDark, 0.f, 1.f);
}

float ArrowDefects::get_reverse_offset()
{
	float zoom= 1 - (get_mini() * .5f);
	if(fabsf(zoom) < .01)
	{
		zoom= .01;
	}
	return m_reverse_offset / zoom;
}

float ArrowDefects::get_center_percent()
{
	return m_options->m_fScrolls[PlayerOptions::SCROLL_CENTERED];
}

float ArrowDefects::get_reverse_scale(size_t col)
{
	return Rage::scale(m_options->GetReversePercentForColumn(col), 0.f, 1.f,
		1.f, -1.f);
}

float ArrowDefects::get_tilt()
{
	return m_options->m_fTilt;
}

float ArrowDefects::get_mini()
{
	return m_options->m_fEffects[PlayerOptions::EFFECT_MINI];
}

float ArrowDefects::get_skew()
{
	return m_options->m_fSkew;
}

float ArrowDefects::get_column_x(size_t col)
{
	return m_column_x[col];
}
