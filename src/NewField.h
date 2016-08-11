#ifndef NEW_FIELD_H
#define NEW_FIELD_H

#include <string>
#include <unordered_set>
#include <vector>

#include "ActorFrame.h"
#include "ArrowDefects.h"
#include "AutoActor.h"
#include "BitmapText.h"
#include "ModValue.h"
#include "NewSkin.h"
#include "NoteData.h"
#include "Sprite.h"

class NoteData;
class Steps;
class TimingData;
struct NewField;

enum FieldLayerFadeType
{
	FLFT_Receptor,
	FLFT_Note,
	FLFT_Explosion,
	FLFT_None,
	NUM_FieldLayerFadeType,
	FieldLayerFadeType_Invalid
};
std::string const FieldLayerFadeTypeToString(FieldLayerFadeType fmt);
LuaDeclareType(FieldLayerFadeType);

enum FieldLayerTransformType
{
	FLTT_Full,
	FLTT_PosOnly,
	FLTT_None,
	NUM_FieldLayerTransformType,
	FieldLayerTransformType_Invalid
};
std::string const FieldLayerTransformTypeToString(FieldLayerTransformType fmt);
LuaDeclareType(FieldLayerTransformType);

struct FieldLayerRenderInfo
{
	FieldLayerFadeType fade_type;
	FieldLayerTransformType transform_type;
};

struct NewFieldColumn : ActorFrame
{
	NewFieldColumn();
	~NewFieldColumn();

	struct render_note
	{
		render_note(NewFieldColumn* column, NoteData::TrackMap::const_iterator
			column_begin, NoteData::TrackMap::const_iterator column_end,
			NoteData::TrackMap::const_iterator iter);
		double y_offset;
		double tail_y_offset;
		// tail_beat and tail_second are only used by lifts.
		double tail_beat;
		double tail_second;
		NoteData::TrackMap::const_iterator note_iter;
	};
	void add_children_from_layers(size_t column, std::vector<NewSkinLayer>& layers);
	void set_note_data(size_t column, const NoteData* note_data,
		const TimingData* timing_data);
	void set_column_info(NewField* field, size_t column, NewSkinColumn* newskin,
		ArrowDefects* defects,
		NewSkinData& skin_data, std::vector<Rage::Color>* player_colors,
		const NoteData* note_data, const TimingData* timing_data, double x);
	void take_over_mods(NewFieldColumn& old_column);
	void set_defective_mode(bool mode);
	bool get_defective_mode();
	void set_speed_old_way(float time_spacing, float max_scroll_bpm,
		float scroll_speed, float scroll_bpm, float read_bpm, float music_rate);
	size_t get_mod_col() { return m_column+1; }

	Rage::Color get_player_color(size_t pn);
	void get_hold_draw_time(TapNote const& tap, double const hold_beat,
		double& beat, double& second);
	void draw_hold(QuantizedHoldRenderData& data, render_note const& note,
		double head_beat, double head_second,
		double tail_beat, double tail_second, bool is_lift);
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
	bool timing_is_safe() { return m_timing_data != nullptr; }
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
	double calc_y_offset(double beat, double second, TapNote const* note);
	double head_y_offset()
	{
		return calc_y_offset(m_curr_beat, m_curr_second, nullptr);
	}
	double calc_lift_pretrail(double beat, double second, double yoffset);
	double get_reverse_shift()
	{
		return reverse_shift;
	}
	double quantization_for_time(mod_val_inputs& input)
	{
		double mult= m_quantization_multiplier.evaluate(input) * m_newskin->get_quantum_mult();
		double offset= m_quantization_offset.evaluate(input);
		return std::fmod((input.eval_beat * mult) + offset, 1.0);
	}
	void calc_transform(mod_val_inputs& input, Rage::transform& trans);
	void calc_transform_with_glow_alpha(mod_val_inputs& input,
		Rage::transform& trans);
	void hold_render_transform(mod_val_inputs& input, Rage::transform& trans,
		bool do_rot);
	void calc_reverse_shift();
	double apply_reverse_shift(double y_offset);
	void apply_column_mods_to_actor(Actor* act);
	void apply_note_mods_to_actor(Actor* act, double beat, double second,
		double y_offset, bool use_alpha, bool use_glow);

	void build_render_lists();
	void draw_child(int child);
	Rage::transform const& get_head_trans() { return head_transform; }

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
	void position_actor_at_column_head(Actor* act, FieldLayerRenderInfo& info);

	virtual void AddChild(Actor* act);
	virtual void RemoveChild(Actor* act);
	virtual void ChildChangedDrawOrder(Actor* child);

	virtual void PushSelf(lua_State *L);
	virtual NewFieldColumn* Copy() const;

	NotePlayerizeMode get_playerize_mode() { return m_playerize_mode; }
	void set_playerize_mode(NotePlayerizeMode mode);

	void set_layer_fade_type(Actor* child, FieldLayerFadeType type);
	FieldLayerFadeType get_layer_fade_type(Actor* child);
	void set_layer_transform_type(Actor* child, FieldLayerTransformType type);
	FieldLayerTransformType get_layer_transform_type(Actor* child);

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
		double anim_percent;
		TapNote const* active_hold;
		TapNote const* prev_active_hold;
		bool found_upcoming;
		bool in_reverse;
	};
	column_status m_status;

	ModManager m_mod_manager;
	// If you add another ModifiableValue member, be sure to add it to the
	// loop in set_column_info.  They need to have the timing data passed to
	// them so mods can have start and end times.
	// Also add new ModifiableValue members to the take_over_mods function.
	ModifiableValue m_time_offset;
	ModifiableValue m_quantization_multiplier;
	ModifiableValue m_quantization_offset;

	ModifiableValue m_speed_mod;
	ModifiableValue m_lift_pretrail_length;

	ModifiableValue m_reverse_offset_pixels;
	ModifiableValue m_reverse_scale;
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

	TimingSource m_timing_source;

private:
	void did_tap_note_internal(TapNoteScore tns, bool bright);
	void did_hold_note_internal(HoldNoteScore hns, bool bright);
	void draw_child_internal();

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
	NewField* m_field;
	std::vector<FieldLayerRenderInfo> m_layer_render_info;

	ArrowDefects* m_defective_mods;
	bool m_in_defective_mode;

	const NoteData* m_note_data;
	const TimingData* m_timing_data;
	// Data that needs to be stored for rendering below here.
	// Holds and taps are put into different lists because they have to be
	// rendered in different phases.  All hold bodies must be drawn first, then
	// all taps, so the taps appear on top of the hold bodies and are not
	// obscured.
	void draw_holds_internal();
	void draw_lifts_internal();
	void draw_taps_internal();
	NoteData::TrackMap::const_iterator first_note_visible_prev_frame;
	std::list<render_note> render_holds;
	std::list<render_note> render_lifts;
	std::list<render_note> render_taps;
	int curr_render_child;
	// Calculating the effects of reverse and center for every note is costly.
	// Only do it once per frame and store the result.
	double reverse_shift;
	double reverse_scale;
	double reverse_scale_sign;
	double first_y_offset_visible;
	double last_y_offset_visible;
	Rage::transform head_transform;
	double receptor_alpha;
	double receptor_glow;
	double explosion_alpha;
	double explosion_glow;
	bool pressed;
	bool was_pressed;
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
	virtual void PostDraw();
	void draw_board();
	void draw_up_to_draw_order(int order);
	virtual void DrawPrimitives();
	void position_actor_at_column_head(Actor* act, FieldLayerRenderInfo& info,
		size_t col);

	virtual void PushSelf(lua_State *L);
	virtual NewField* Copy() const;

	virtual void HandleMessage(Message const& msg);

	virtual void AddChild(Actor* act);
	virtual void RemoveChild(Actor* act);
	virtual void ChildChangedDrawOrder(Actor* child);

	void push_columns_to_lua(lua_State* L);
	double get_field_width() { return m_field_width; }
	size_t get_num_columns() { return m_columns.size(); }

	void set_player_color(size_t pn, Rage::Color const& color);

	void clear_steps();
	void set_skin(std::string const& skin_name, LuaReference& skin_params);
	void set_steps(Steps* data);
	void set_note_data(NoteData* note_data, TimingData* timing, StepsType stype);
	// set_player_number exists only so that the notefield layers can have
	// per-player configuration on gameplay.  Using it for any other purpose
	// is forbidden.
	void set_player_number(PlayerNumber pn);
	// set_player_options is for supporting the old ArrowEffects mods in
	// defective mode.
	void set_player_options(PlayerOptions* options);
	void set_read_bpm(float read_bpm) {m_defective_mods.set_read_bpm(read_bpm);}
	void set_defective_mode(bool mode);
	bool get_defective_mode();
	void set_speed_old_way(float time_spacing, float max_scroll_bpm,
		float scroll_speed, float scroll_bpm, float read_bpm, float music_rate);
	void turn_on_edit_text();

	void set_layer_fade_type(Actor* child, FieldLayerFadeType type);
	FieldLayerFadeType get_layer_fade_type(Actor* child);

	void update_displayed_time(double beat, double second);
	double get_curr_beat() { return m_curr_beat; }
	double get_curr_second() { return m_curr_second; }
	double get_beat_from_second(double second);
	double get_second_from_beat(double beat);
	void set_displayed_beat(double beat);
	void set_displayed_second(double second);
	bool timing_is_safe() { return m_timing_data != nullptr; }

	void did_tap_note(size_t column, TapNoteScore tns, bool bright);
	void did_hold_note(size_t column, HoldNoteScore hns, bool bright);
	void set_pressed(size_t column, bool on);

	ModManager m_mod_manager;
	ModifiableTransform m_trans_mod;
	ModifiableValue m_receptor_alpha;
	ModifiableValue m_receptor_glow;
	ModifiableValue m_explosion_alpha;
	ModifiableValue m_explosion_glow;
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
	bool m_being_drawn_by_player;
	bool m_draw_beat_bars;

	struct field_draw_entry
	{
		int column;
		int child;
		int draw_order;
	};
	void add_draw_entry(int column, int child, int draw_order);
	void remove_draw_entry(int column, int child);
	void change_draw_entry(int column, int child, int new_draw_order);
	void clear_column_draw_entries();
	double update_z_bias();

private:
	void draw_entry(field_draw_entry& entry);
	void draw_beat_bar(double beat, double second, double y_offset, int state,
		float alpha);
	void draw_field_text(double beat, double second, double y_offset,
		double x_offset, float side_sign, float horiz_align,
		Rage::Color const& color, Rage::Color const& glow);
	void draw_beat_bars_internal();
	void reload_columns(NewSkinLoader const* new_loader, LuaReference& new_params);
	double m_curr_beat;
	double m_curr_second;
	double m_field_width;

	// The player number has to be stored so that it can be passed to the
	// column layers when they're loaded.
	PlayerNumber m_pn;
	ArrowDefects m_defective_mods;
	bool m_in_defective_mode;

	bool m_own_note_data;
	NoteData* m_note_data;
	const TimingData* m_timing_data;
	StepsType m_steps_type;
	std::vector<NewFieldColumn> m_columns;
	NewSkinData m_newskin;
	NewSkinLoader m_skin_walker;
	LuaReference m_skin_parameters;
	std::vector<Rage::Color> m_player_colors;
	std::vector<FieldLayerRenderInfo> m_layer_render_info;

	Sprite m_beat_bars;
	BitmapText m_field_text;

	vector<field_draw_entry> m_draw_entries;
	size_t m_first_undrawn_entry;
	int m_curr_draw_limit;

	// Evaluation results of the mods need to be stored because Draw is called
	// twice: once for the board, once for everything else.  So the mods can't
	// be evaluated in Draw.  They are evaluated in update_displayed_time. -Kyz
	double evaluated_receptor_alpha;
	double evaluated_receptor_glow;
	double evaluated_explosion_alpha;
	double evaluated_explosion_glow;
	// The old distant and hallway mods would shift the notefield before
	// rendering, then move it back.  defective_render_y is used to store the
	// calculated shifted y.
	double defective_render_y;
	double original_y;

	double curr_z_bias;
};

#endif
