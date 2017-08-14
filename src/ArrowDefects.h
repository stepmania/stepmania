#ifndef ARROW_DEFECTS_H
#define ARROW_DEFECTS_H

#include "RageVector3.hpp"
#include "PlayerNumber.h"

class PlayerOptions;
class TimingData;
class Style;

struct ArrowDefects
{
	ArrowDefects();

	void Init();
	bool safe() { return m_options != nullptr; }
	void set_player_options(PlayerOptions const* options);
	void set_column_pos(std::vector<float>& column_x);
	void set_timing(TimingData const* timing);
	void set_num_pads(int num);
	void set_read_bpm(float read_bpm);
	void update(PlayerNumber pn, float music_beat, float music_second);

	float get_y_offset(float note_beat, float note_second, size_t col);
	void get_transform(float note_beat, float y_offset,
		float shifted_offset, size_t col, Rage::transform& trans);
	void get_transform_with_glow_alpha(float note_beat, float y_offset,
		float shifted_offset, size_t col, Rage::transform& trans);
	// hold_render_transform leaves out unused fields like x and z rotation.
	void hold_render_transform(float y_offset, size_t col,
		Rage::transform& trans);

	float get_field_y();
	float get_receptor_alpha(size_t col);
	float get_reverse_offset();
	float get_center_percent();
	float get_reverse_scale(size_t col);
	float get_tilt();
	float get_mini();
	float get_skew();
	float get_drawsize();
	float get_drawsizeback();

	float get_column_x(size_t col);

private:
	float get_time();
	float get_x_pos(size_t col, float y_offset);
	float get_y_pos(size_t col, float y_offset);
	float get_z_pos(size_t col, float y_offset);
	float get_move_x(size_t col);
	float get_move_y(size_t col);
	float get_move_z(size_t col);
	float get_rotation_y(float y_offset);
	float get_zoom(size_t col, float y_offset);
	float get_percent_visible(float y_pos_without_reverse, size_t col, float y_offset);
	void get_glow_alpha(size_t col, float y_offset, Rage::transform& trans);
	float calculate_tornado_offset_from_magnitude(int dimension, int col_id,
		float magnitude, float effect_offset, float period, float y_offset, bool is_tan);
	void update_beat(int dimension, float beat_offset, float beat_mult);
	void update_tipsy(float offset, float speed, bool is_tan);
	float select_tan_calc(float angle, bool is_cosec);
	float calculate_drunk_angle(float speed, int col, float offset, 
		float col_frequency, float y_offset, float period, float offset_frequency);
	float calculate_bumpy_angle(float y_offset, float offset, float period);
	float calculate_digital_angle(float y_offset, float offset, float period);
	
	float get_center_line();
	float get_hidden_sudden();
	float get_notefield_height();
	float get_pulse_inner();

	PlayerOptions const* m_options;
	TimingData const* m_timing_data; // For speed and scroll segments.
	float m_read_bpm;
	size_t m_num_columns;
	int m_num_pads;
	float m_xmode_dir;
	float m_receptor_pos_normal;
	float m_receptor_pos_reverse;
	float m_reverse_offset;

	std::vector<float> m_column_x;
	std::vector<float> m_min_tornado_x[3];
	std::vector<float> m_max_tornado_x[3];
	std::vector<float> m_invert_dist;
	std::vector<float> m_tipsy_result;
	std::vector<float> m_tan_tipsy_result;
	float m_beat_factor[3];
	float m_expand_seconds;
	float m_tan_expand_seconds;

	float m_hidden_end_line;
	float m_hidden_start_line;
	float m_sudden_end_line;
	float m_sudden_start_line;

	float m_music_beat;
	float m_music_second;
	float m_display_beat;
	float m_speed_percent;
	
	Style const* m_prev_style;
};

#endif
