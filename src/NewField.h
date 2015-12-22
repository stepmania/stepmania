#ifndef NEW_FIELD_H
#define NEW_FIELD_H

#include <string>
#include <unordered_set>
#include <vector>

#include "ActorFrame.h"
#include "AutoActor.h"
#include "ModValue.h"
#include "NewSkin.h"
#include "NoteData.h"

class NoteData;
class Steps;
class TimingData;

struct NewFieldColumn : ActorFrame
{
	NewFieldColumn();
	~NewFieldColumn();

	struct column_head
	{
		// The actors have to be wrapped inside of frames so that mod transforms
		// can be applied without stomping the rotation the noteskin supplies.
		ActorFrame frame;
		AutoActor actor;
		void load(Actor* act)
		{
			actor.Load(act);
			frame.AddChild(actor);
		}
	};
	struct render_note
	{
		render_note(NewFieldColumn* column, NoteData::TrackMap::const_iterator column_end, NoteData::TrackMap::const_iterator iter);
		double y_offset;
		double tail_y_offset;
		NoteData::TrackMap::const_iterator note_iter;
	};
	void add_heads_from_layers(size_t column, std::vector<column_head>& heads,
		std::vector<NewSkinLayer>& layers);
	void set_column_info(size_t column, NewSkinColumn* newskin,
		NewSkinData& skin_data, std::vector<Rage::Color>* player_colors,
		const NoteData* note_data, const TimingData* timing_data, double x);

	Rage::Color get_player_color(size_t pn);
	void get_hold_draw_time(TapNote const& tap, double const hold_beat, double& beat, double& second);
	void draw_hold(QuantizedHoldRenderData& data, render_note const& note,
		double head_beat, double head_second,
		double tail_beat, double tail_second);
	void set_displayed_time(double beat, double second);
	// update_displayed_time is called by the field when the Player draws in
	// gameplay.  set_displayed_beat is for lua to call when lua is controlling
	// the current beat.
	void update_displayed_time(double beat, double second);
	double get_curr_beat() { return m_curr_beat; }
	double get_curr_second() { return m_curr_second; }
	double get_beat_from_second(double second);
	double get_second_from_beat(double beat);
	void set_displayed_beat(double beat);
	void set_displayed_second(double second);
	void set_pixels_visible_before(double pix)
	{
		m_pixels_visible_before_beat= pix;
	}
	void set_pixels_visible_after(double pix)
	{
		m_pixels_visible_after_beat= pix;
	}
	double get_upcoming_time()
	{
		return m_upcoming_time;
	}
	void set_upcoming_time(double up)
	{
		m_upcoming_time= up;
	}
	int y_offset_visible(double off)
	{
		if(off < first_y_offset_visible) { return -1; }
		if(off > last_y_offset_visible) { return 1; }
		return 0;
	}
	double calc_y_offset(double beat, double second);
	double head_y_offset()
	{
		return calc_y_offset(m_curr_beat, m_curr_second);
	}
	double get_reverse_shift()
	{
		return reverse_shift;
	}
	double get_reverse_scale()
	{
		return reverse_scale;
	}
	double quantization_for_time(mod_val_inputs& input)
	{
		double mult= m_quantization_multiplier.evaluate(input);
		double offset= m_quantization_offset.evaluate(input);
		return std::fmod((input.eval_beat * mult) + offset, 1.0);
	}
	void calc_transform(mod_val_inputs& input, Rage::transform& trans);
	void hold_render_transform(mod_val_inputs& input, Rage::transform& trans, bool do_rot);
	void calc_reverse_shift();
	double apply_reverse_shift(double y_offset);
	void apply_column_mods_to_actor(Actor* act);
	void apply_note_mods_to_actor(Actor* act, double beat, double second,
		double y_offset, bool use_alpha, bool use_glow);

	enum render_step
	{
		RENDER_BELOW_NOTES,
		RENDER_HOLDS,
		RENDER_TAPS,
		RENDER_CHILDREN,
		RENDER_ABOVE_NOTES
	};
	void build_render_lists();
	void draw_things_in_step(render_step step);

	void pass_message_to_heads(Message& msg);
	void did_tap_note(TapNoteScore tns, bool bright);
	void did_hold_note(HoldNoteScore hns, bool bright);
	void set_hold_status(TapNote const* tap, bool start, bool end);
	void set_pressed(bool on);

	virtual void UpdateInternal(float delta);
	virtual bool EarlyAbortDraw() const;
	void imitate_did_note(TapNote const& tap);
	void update_upcoming(double beat, double second);
	void update_active_hold(TapNote const& tap);
	virtual void DrawPrimitives();

	virtual void PushSelf(lua_State *L);
	virtual NewFieldColumn* Copy() const;

	NotePlayerizeMode get_playerize_mode() { return m_playerize_mode; }
	void set_playerize_mode(NotePlayerizeMode mode);

	bool m_use_game_music_beat;
	bool m_show_unjudgable_notes;
	bool m_speed_segments_enabled;
	bool m_scroll_segments_enabled;
	bool m_add_y_offset_to_position;
	bool m_holds_skewed_by_mods;
	bool m_twirl_holds;
	bool m_use_moddable_hold_normal;

	struct column_status
	{
		column_status()
			:active_hold(nullptr), prev_active_hold(nullptr)
		{}
		double upcoming_beat_dist;
		double upcoming_second_dist;
		TapNote const* active_hold;
		TapNote const* prev_active_hold;
		bool found_upcoming;
	};
	column_status m_status;

	ModManager m_mod_manager;
	// If you add another ModifiableValue member, be sure to add it to the
	// loop in set_column_info.  They need to have the timing data passed to
	// them so mods can have start and end times.
	ModifiableValue m_time_offset;
	ModifiableValue m_quantization_multiplier;
	ModifiableValue m_quantization_offset;

	ModifiableValue m_speed_mod;

	ModifiableValue m_reverse_offset_pixels;
	ModifiableValue m_reverse_percent;
	ModifiableValue m_center_percent;

	ModifiableTransform m_note_mod;
	ModifiableTransform m_column_mod;
	ModifiableVector3 m_hold_normal_mod;

	ModifiableValue m_note_alpha;
	ModifiableValue m_note_glow;
	ModifiableValue m_receptor_alpha;
	ModifiableValue m_receptor_glow;
	ModifiableValue m_explosion_alpha;
	ModifiableValue m_explosion_glow;

private:
	void did_tap_note_internal(TapNoteScore tns, bool bright);
	void did_hold_note_internal(HoldNoteScore hns, bool bright);

	double m_curr_beat;
	double m_curr_displayed_beat;
	double m_curr_second;
	double m_prev_curr_second;
	double m_pixels_visible_before_beat;
	double m_pixels_visible_after_beat;
	double m_upcoming_time;
	size_t m_column;
	NotePlayerizeMode m_playerize_mode;
	NewSkinColumn* m_newskin;
	std::vector<Rage::Color>* m_player_colors;

	AutoActor m_beat_bars;

	std::vector<column_head> m_heads_below_notes;
	std::vector<column_head> m_heads_above_notes;

	const NoteData* m_note_data;
	const TimingData* m_timing_data;
	// Data that needs to be stored for rendering below here.
	// Holds and taps are put into different lists because they have to be
	// rendered in different phases.  All hold bodies must be drawn first, then
	// all taps, so the taps appear on top of the hold bodies and are not
	// obscured.
	void draw_heads_internal(std::vector<column_head>& heads, bool receptors);
	void draw_holds_internal();
	void draw_taps_internal();
	NoteData::TrackMap::const_iterator first_note_visible_prev_frame;
	std::vector<render_note> render_holds;
	std::vector<render_note> render_taps;
	render_step curr_render_step;
	// Calculating the effects of reverse and center for every note is costly.
	// Only do it once per frame and store the result.
	double reverse_shift;
	double reverse_scale;
	double reverse_scale_sign;
	double first_y_offset_visible;
	double last_y_offset_visible;
	bool pressed;
};

enum FieldVanishType
{
	FVT_RelativeToParent,
	FVT_RelativeToSelf,
	FVT_RelativeToOrigin,
	NUM_FieldVanishType,
	FieldVanishType_Invalid
};
std::string const FieldVanishTypeToString(FieldVanishType fmt);
LuaDeclareType(FieldVanishType);

struct NewField : ActorFrame
{
	NewField();
	~NewField();
	virtual void UpdateInternal(float delta);
	virtual bool EarlyAbortDraw() const;
	virtual void PreDraw();
	void draw_board();
	virtual void DrawPrimitives();

	virtual void PushSelf(lua_State *L);
	virtual NewField* Copy() const;

	void push_columns_to_lua(lua_State* L);
	double get_field_width() { return m_field_width; }

	void set_player_color(size_t pn, Rage::Color const& color);

	void clear_steps();
	void set_skin(std::string const& skin_name, LuaReference& skin_params);
	void set_steps(Steps* data);
	void set_note_data(NoteData* note_data, TimingData* timing, Style const* curr_style);
	// set_player_number exists only so that the notefield board can have
	// per-player configuration on gameplay.  Using it for any other purpose
	// is forbidden.
	void set_player_number(PlayerNumber pn);

	void update_displayed_time(double beat, double second);

	void did_tap_note(size_t column, TapNoteScore tns, bool bright);
	void did_hold_note(size_t column, HoldNoteScore hns, bool bright);
	void set_pressed(size_t column, bool on);

	ModManager m_mod_manager;
	ModifiableTransform m_trans_mod;
	ModifiableValue m_fov_mod;
	ModifiableValue m_vanish_x_mod;
	ModifiableValue m_vanish_y_mod;
	// To allow the Player actor the field is inside to be moved around without
	// causing skew problems, the field adds the vanish position to the actor
	// position.  m_vanish_type tells the field whether to look at its parent,
	// itself, or nothing when offsetting the vanish point.
	// The relative to self and relative to nothing choices exist for when a
	// field is displayed without a player actor.
	FieldVanishType m_vanish_type;

private:
	double m_curr_beat;
	double m_curr_second;
	double m_field_width;

	bool m_own_note_data;
	NoteData* m_note_data;
	const TimingData* m_timing_data;
	std::vector<NewFieldColumn> m_columns;
	NewSkinData m_newskin;
	NewSkinLoader m_skin_walker;
	LuaReference m_skin_parameters;
	std::vector<Rage::Color> m_player_colors;

	bool m_drawing_board;
	AutoActor m_board;
};

#endif
