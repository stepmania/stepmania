#include "global.h"

#include "ActorUtil.h"
#include "BackgroundUtil.h"
#include "EnumHelper.h"
#include "Game.h"
#include "GameManager.h"
#include "GameState.h"
#include "LuaBinding.h"
#include "NoteField.h"
#include "NoteSkinManager.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "RageMath.h"
#include "RageUtil.h"
#include "ScreenDimensions.h"
#include "Song.h"
#include "Sprite.h"
#include "Steps.h"
#include "Style.h"
#include "ThemeManager.h"

using std::max;
using std::string;
using std::unordered_set;
using std::vector;

static const double note_size= 64.0;
static const double z_bias_per_thing= .01;
static const double lift_fade_dist_recip= 1.0 / 64.0;
static const int field_layer_column_index= -1;
static const int beat_bars_column_index= -2;
static const int holds_child_index= -1;
static const int lifts_child_index= -2;
static const int taps_child_index= -3;
static const int beat_bars_child_index= -4;
static const int selection_child_index= -5;
static const int draw_order_types= 4;
static const int non_board_draw_order= 0;
static const int beat_bars_draw_order= 0;
static const int selection_draw_order= 0;
static const int holds_draw_order= 200;
static const int lifts_draw_order= 200;
static const int taps_draw_order= 300;

static ThemeMetric<Rage::Color> AREA_HIGHLIGHT_COLOR("NoteField", "AreaHighlightColor");

static const char* FieldLayerFadeTypeNames[]= {
	"Receptor",
	"Note",
	"Explosion",
	"None",
};
XToString(FieldLayerFadeType);
LuaXType(FieldLayerFadeType);

static const char* FieldLayerTransformTypeNames[]= {
	"Full",
	"PosOnly",
	"None",
};
XToString(FieldLayerTransformType);
LuaXType(FieldLayerTransformType);

REGISTER_ACTOR_CLASS(NoteFieldColumn);

#define HOLD_COUNTS_AS_ACTIVE(tn) (tn.HoldResult.bActive && tn.HoldResult.fLife > 0.0f)

static void apply_render_info_to_layer(Actor* layer,
	FieldLayerRenderInfo& info, Rage::transform const& trans,
	double receptor_alpha, double receptor_glow,
	double explosion_alpha, double explosion_glow, double beat, double second,
	ModifiableValue& note_alpha, ModifiableValue& note_glow)
{
	switch(info.fade_type)
	{
		case FLFT_Receptor:
			layer->SetDiffuseAlpha(receptor_alpha);
			layer->SetGlowAlpha(receptor_glow);
			break;
		case FLFT_Explosion:
			layer->SetDiffuseAlpha(explosion_alpha);
			layer->SetGlowAlpha(explosion_glow);
			break;
		case FLFT_Note:
			{
				mod_val_inputs input(beat, second);
				double alpha= note_alpha.evaluate(input);
				double glow= note_glow.evaluate(input);
				layer->SetDiffuseAlpha(alpha);
				layer->SetGlowAlpha(glow);
			}
			break;
		default:
			break;
	}
	switch(info.transform_type)
	{
		case FLTT_Full:
			layer->set_transform(trans);
			break;
		case FLTT_PosOnly:
			layer->set_transform_pos(trans);
			break;
		default:
			break;
	}
}

bool operator<(NoteField::field_draw_entry const& rhs, NoteField::field_draw_entry const& lhs)
{
	return rhs.draw_order < lhs.draw_order;
}

NoteFieldColumn::NoteFieldColumn()
	:m_show_unjudgable_notes(true),
	 m_speed_segments_enabled(true), m_scroll_segments_enabled(true),
	 m_add_y_offset_to_position(true), m_holds_skewed_by_mods(true),
	 m_twirl_holds(true), m_use_moddable_hold_normal(false),
	 m_time_offset(&m_mod_manager, 0.0),
	 m_quantization_multiplier(&m_mod_manager, 1.0),
	 m_quantization_offset(&m_mod_manager, 0.0),
	 m_speed_mod(&m_mod_manager, 0.0),
	 m_lift_pretrail_length(&m_mod_manager, 0.25),
	 m_reverse_offset_pixels(&m_mod_manager, 240.0 - note_size),
	 m_reverse_scale(&m_mod_manager, 1.0),
	 m_center_percent(&m_mod_manager, 0.0),
	 m_note_mod(&m_mod_manager), m_column_mod(&m_mod_manager),
	 m_hold_normal_mod(&m_mod_manager, 0.0),
	 m_note_alpha(&m_mod_manager, 1.0), m_note_glow(&m_mod_manager, 0.0),
	 m_receptor_alpha(&m_mod_manager, 1.0), m_receptor_glow(&m_mod_manager, 0.0),
	 m_explosion_alpha(&m_mod_manager, 1.0), m_explosion_glow(&m_mod_manager, 0.0),
	 m_selection_start(-1.0), m_selection_end(-1.0),
	 m_curr_beat(0.0f), m_curr_second(0.0), m_prev_curr_second(-1000.0),
	 m_pixels_visible_before_beat(128.0f),
	 m_pixels_visible_after_beat(1024.0f),
	 m_upcoming_time(2.0),
	 m_playerize_mode(NPM_Off),
	 m_newskin(nullptr), m_player_colors(nullptr), m_field(nullptr),
	 m_defective_mods(nullptr), m_in_defective_mode(false),
	 m_note_data(nullptr),
	 m_timing_data(nullptr),
	 m_gameplay_zoom(1.0),
	 reverse_scale_sign(1.0),
	 pressed(false)
{
	DeleteChildrenWhenDone(true);
	m_area_highlight.SetEffectGlowShift(2, Rage::Color(0.f, 0.f, 0.f, 0.f),
		Rage::Color(.75f, .75f, .75f, 1.0f));
}

NoteFieldColumn::~NoteFieldColumn()
{}

void NoteFieldColumn::AddChild(Actor* act)
{
	// The actors have to be wrapped inside of frames so that mod transforms
	// can be applied without stomping the rotation the noteskin supplies.
	ActorFrame* frame= new ActorFrame;
	frame->WrapAroundChild(act);
	ActorFrame::AddChild(frame);
	m_layer_render_info.push_back({FLFT_None, FLTT_Full});
	act->PlayCommand("On");
	m_field->add_draw_entry(static_cast<int>(m_column), GetNumChildren()-1, act->GetDrawOrder());
	if(act->HasCommand("WidthSet"))
	{
		Message width_msg("WidthSet");
		lua_State* L= LUA->Get();
		PushSelf(L);
		width_msg.SetParamFromStack(L, "column");
		LUA->Release(L);
		width_msg.SetParam("column_id", get_mod_col());
		width_msg.SetParam("width", m_newskin->get_width());
		width_msg.SetParam("padding", m_newskin->get_padding());
		act->HandleMessage(width_msg);
	}
}

void NoteFieldColumn::RemoveChild(Actor* act)
{
	size_t index= FindChildID(act);
	if(index < m_SubActors.size())
	{
		m_field->remove_draw_entry(m_column, index);
		m_layer_render_info.erase(m_layer_render_info.begin() + index);
	}
	ActorFrame::RemoveChild(act);
}

void NoteFieldColumn::ChildChangedDrawOrder(Actor* child)
{
	size_t index= FindChildID(child);
	if(index < m_SubActors.size())
	{
		m_field->change_draw_entry(static_cast<int>(m_column),
			static_cast<int>(index), child->GetDrawOrder());
	}
	ActorFrame::ChildChangedDrawOrder(child);
}

void NoteFieldColumn::add_children_from_layers(size_t column,
	vector<NoteSkinLayer>& layers)
{
	for(size_t i= 0; i < layers.size(); ++i)
	{
		AddChild(layers[i].m_actors[column]);
	}
}

void NoteFieldColumn::set_note_data(size_t column, const NoteData* note_data,
	const TimingData* timing_data)
{
	m_note_data= note_data;
	m_timing_data= timing_data;
	note_row_closest_to_current_time= -1;
	for(auto&& moddable : {&m_time_offset, &m_quantization_multiplier,
				&m_quantization_offset, &m_speed_mod, &m_lift_pretrail_length,
				&m_reverse_offset_pixels, &m_reverse_scale,
				&m_center_percent, &m_note_alpha, &m_note_glow, &m_receptor_alpha,
				&m_receptor_glow, &m_explosion_alpha, &m_explosion_glow})
	{
		moddable->set_timing(timing_data);
		moddable->set_column(column);
	}
	for(auto&& moddable : {&m_note_mod, &m_column_mod})
	{
		moddable->set_timing(timing_data);
		moddable->set_column(column);
	}
	for(auto&& moddable : {&m_hold_normal_mod})
	{
		moddable->set_timing(timing_data);
		moddable->set_column(column);
	}
}

void NoteFieldColumn::set_column_info(NoteField* field, size_t column,
	NoteSkinColumn* newskin, ArrowDefects* defects,
	NoteSkinData& skin_data, std::vector<Rage::Color>* player_colors,
	const NoteData* note_data, const TimingData* timing_data, double x)
{
	m_field= field;
	m_column= column;
	m_newskin= newskin;
	m_defective_mods= defects;
	m_newskin->set_timing_source(&m_timing_source);
	set_note_data(column, note_data, timing_data);
	m_column_mod.pos_mod.x_mod.add_simple_mod("base_value", "number", x);
	m_use_game_music_beat= true;
	m_player_colors= player_colors;

	m_mod_manager.column= column;

	std::vector<Actor*> layers;
	ActorUtil::MakeActorSet(THEME->GetPathG("NoteColumn", "layers", true), layers);
	for(auto&& act : layers)
	{
		AddChild(act);
	}
	add_children_from_layers(column, skin_data.m_layers);
}

void NoteFieldColumn::take_over_mods(NoteFieldColumn& other)
{
#define CPY(name) name= other.name;
	CPY(m_use_game_music_beat);
	CPY(m_show_unjudgable_notes);
	CPY(m_speed_segments_enabled);
	CPY(m_scroll_segments_enabled);
	CPY(m_add_y_offset_to_position);
	CPY(m_holds_skewed_by_mods);
	CPY(m_twirl_holds);
	CPY(m_use_moddable_hold_normal);
#define CPY_MODS(name) name.take_over_mods(other.name);
	CPY_MODS(m_time_offset);
	CPY_MODS(m_quantization_multiplier);
	CPY_MODS(m_quantization_offset);
	CPY_MODS(m_speed_mod);
	CPY_MODS(m_lift_pretrail_length);
	CPY_MODS(m_reverse_offset_pixels);
	CPY_MODS(m_reverse_scale);
	CPY_MODS(m_center_percent);
	CPY_MODS(m_note_mod);
	CPY_MODS(m_column_mod);
	CPY_MODS(m_hold_normal_mod);
	CPY_MODS(m_note_alpha);
	CPY_MODS(m_note_glow);
	CPY_MODS(m_receptor_alpha);
	CPY_MODS(m_receptor_glow);
	CPY_MODS(m_explosion_alpha);
	CPY_MODS(m_explosion_glow);
#undef CPY_MODS
#undef CPY
}

void NoteFieldColumn::set_defective_mode(bool mode)
{
	if(!m_defective_mods->safe())
	{
		m_in_defective_mode= false;
		return;
	}
	m_in_defective_mode= mode;
	if(m_in_defective_mode)
	{
		SetXY(0.0, 0.0);
		SetZ(0.0);
		SetZoom(1.0);
		SetRotationX(0.0);
		SetRotationY(0.0);
		SetRotationZ(0.0);
	}
}

bool NoteFieldColumn::get_defective_mode()
{
	return m_in_defective_mode;
}

void NoteFieldColumn::set_speed(float time_spacing,
	float max_scroll_bpm, float scroll_speed, float scroll_bpm, float read_bpm,
	float music_rate)
{
	if(time_spacing == 0.f)
	{
		if(max_scroll_bpm != 0.f)
		{
			m_speed_mod.add_simple_mod("speed", "dist_beat",
				max_scroll_bpm / read_bpm / music_rate);
		}
		else
		{
			m_speed_mod.add_simple_mod("speed", "dist_beat",
				scroll_speed);
		}
	}
	else
	{
		m_speed_mod.add_simple_mod("speed", "dist_second",
			scroll_bpm / 60.f / music_rate);
	}
}

double NoteFieldColumn::get_beat_from_second(double second)
{
	return m_timing_data->GetBeatFromElapsedTime(static_cast<float>(second));
}

double NoteFieldColumn::get_second_from_beat(double beat)
{
	return m_timing_data->GetElapsedTimeFromBeat(static_cast<float>(beat));
}

void NoteFieldColumn::set_displayed_time(double beat, double second)
{
	m_timing_source.beat_delta= beat - m_curr_beat;
	m_timing_source.second_delta= second - m_curr_second;
	m_timing_source.curr_second= second;
	m_newskin->update_taps();
	m_curr_beat= beat;
	m_curr_second= second;
	m_mod_manager.update(beat, second);
	build_render_lists();
}

void NoteFieldColumn::update_displayed_time(double beat, double second)
{
	if(m_use_game_music_beat)
	{
		if(!m_time_offset.empty())
		{
			mod_val_inputs input(beat, second);
			double offset= m_time_offset.evaluate(input);
			second+= offset;
			beat= m_timing_data->GetBeatFromElapsedTime(static_cast<float>(second));
		}
		set_displayed_time(beat, second);
	}
}

void NoteFieldColumn::set_displayed_beat(double beat)
{
	set_displayed_time(beat, m_timing_data->GetElapsedTimeFromBeat(static_cast<float>(beat)));
}

void NoteFieldColumn::set_displayed_second(double second)
{
	set_displayed_time(m_timing_data->GetBeatFromElapsedTime(static_cast<float>(second)), second);
}

double NoteFieldColumn::calc_y_offset(mod_val_inputs& input)
{
	if(!m_in_defective_mode)
	{
		input.music_beat= m_curr_displayed_beat;
		double original_beat= input.eval_beat;
		if(m_scroll_segments_enabled)
		{
			if(original_beat == m_curr_beat)
			{
				input.change_eval_beat(m_curr_displayed_beat);
			}
			else
			{
				input.change_eval_beat(m_timing_data->GetDisplayedBeat(static_cast<float>(input.eval_beat)));
			}
		}
		double ret= note_size * m_speed_mod.evaluate(input);
		if(m_speed_segments_enabled)
		{
			ret*= m_timing_data->GetDisplayedSpeedPercent(static_cast<float>(m_curr_beat), static_cast<float>(m_curr_second));
		}
		input.music_beat= m_curr_beat;
		input.change_eval_beat(original_beat);
		return ret;
	}
	else
	{
		return m_defective_mods->get_y_offset(input.eval_beat, input.eval_second, m_column);
	}
	return 0.0;
}

double NoteFieldColumn::calc_y_offset(double beat, double second)
{
	mod_val_inputs input(beat, second, m_curr_beat, m_curr_second);
	return calc_y_offset(input);
}

void NoteFieldColumn::calc_transform(mod_val_inputs& input,
	Rage::transform& trans)
{
	if(!m_in_defective_mode)
	{
		m_note_mod.evaluate(input, trans);
	}
	else
	{
		m_defective_mods->get_transform(input.eval_beat, input.y_offset,
			apply_reverse_shift(input.y_offset), m_column, trans);
	}
}

void NoteFieldColumn::calc_transform_with_glow_alpha(mod_val_inputs& input,
	Rage::transform& trans)
{
	if(!m_in_defective_mode)
	{
		m_note_mod.evaluate(input, trans);
		trans.alpha= m_note_alpha.evaluate(input);
		trans.glow= m_note_glow.evaluate(input);
	}
	else
	{
		m_defective_mods->get_transform_with_glow_alpha(input.eval_beat,
			input.y_offset, apply_reverse_shift(input.y_offset), m_column, trans);
	}
}

void NoteFieldColumn::calc_pos_only(mod_val_inputs& input, Rage::Vector3& out)
{
	if(!m_in_defective_mode)
	{
		m_note_mod.pos_mod.evaluate(input, out);
	}
	else
	{
		// TODO:  Care.  calc_pos_only is only used for positioning the selection
		// area highlight. -Kyz
	}
}

void NoteFieldColumn::hold_render_transform(mod_val_inputs& input,
	Rage::transform& trans, bool do_rot)
{
	if(!m_in_defective_mode)
	{
		m_note_mod.hold_render_eval(input, trans, do_rot);
		trans.alpha= m_note_alpha.evaluate(input);
		trans.glow= m_note_glow.evaluate(input);
	}
	else
	{
		m_defective_mods->hold_render_transform(input.y_offset, m_column, trans);
	}
}

void NoteFieldColumn::calc_reverse_shift()
{
	double reverse_offset= 0.0;
	double center_percent= 0.0;
	if(!m_in_defective_mode)
	{
		mod_val_inputs input(m_curr_beat, m_curr_second);
		reverse_offset= m_reverse_offset_pixels.evaluate(input) * m_gameplay_zoom;
		center_percent= m_center_percent.evaluate(input);
		reverse_scale= m_reverse_scale.evaluate(input);
	}
	else
	{
		reverse_offset= m_defective_mods->get_reverse_offset();
		center_percent= m_defective_mods->get_center_percent();
		reverse_scale= m_defective_mods->get_reverse_scale(m_column);
	}
	reverse_shift= Rage::scale(reverse_scale, 1.0, -1.0, -reverse_offset, reverse_offset);
	reverse_shift= Rage::scale(center_percent, 0.0, 1.0, reverse_shift, 0.0);
	double old_scale_sign= reverse_scale_sign;
	if(reverse_scale < 0.0)
	{
		m_status.in_reverse= true;
		reverse_scale_sign= -1.0;
	}
	else
	{
		m_status.in_reverse= false;
		reverse_scale_sign= 1.0;
	}
	if(old_scale_sign != reverse_scale_sign)
	{
		Message revmsg("ReverseChanged");
		revmsg.SetParam("sign", reverse_scale_sign);
		pass_message_to_heads(revmsg);
		if(m_column == 0 && m_field != nullptr)
		{
			m_field->HandleMessage(revmsg);
		}
	}
	static double const min_visible_scale= 0.1;
	double visible_scale= fabs(reverse_scale);
	if(visible_scale < min_visible_scale)
	{
		visible_scale= min_visible_scale;
	}
	first_y_offset_visible= -m_pixels_visible_before_beat / visible_scale;
	last_y_offset_visible= m_pixels_visible_after_beat / visible_scale;
}

double NoteFieldColumn::apply_reverse_shift(double y_offset)
{
	return (y_offset * reverse_scale) + reverse_shift;
}

void NoteFieldColumn::apply_column_mods_to_actor(Actor* act)
{
	mod_val_inputs input(m_curr_beat, m_curr_second);
	Rage::transform trans;
	m_column_mod.evaluate(input, trans);
	act->set_transform(trans);
}

void NoteFieldColumn::apply_note_mods_to_actor(Actor* act, double beat,
	double second, double y_offset, bool use_alpha, bool use_glow)
{
	mod_val_inputs mod_input(beat, second, m_curr_beat, m_curr_beat, y_offset);
	if(use_alpha)
	{
		act->SetDiffuseAlpha(static_cast<float>(m_note_alpha.evaluate(mod_input)));
	}
	if(use_glow)
	{
		act->SetGlow(Rage::Color(1, 1, 1, static_cast<float>(m_note_glow.evaluate(mod_input))));
	}
	Rage::transform trans;
	calc_transform(mod_input, trans);
	if(m_add_y_offset_to_position)
	{
		trans.pos.y+= static_cast<float>(apply_reverse_shift(y_offset));
	}
	act->set_transform(trans);
}

float NoteFieldColumn::get_selection_glow()
{
	if(m_field != nullptr)
	{
		return m_field->selection_glow;
	}
	return 0.f;
}

void NoteFieldColumn::UpdateInternal(float delta)
{
	calc_reverse_shift();
	if(m_selection_start != -1.0)
	{
		m_area_highlight.Update(delta);
	}
	ActorFrame::UpdateInternal(delta);
}

void NoteFieldColumn::set_gameplay_zoom(double zoom)
{
	m_gameplay_zoom= 1.0 / zoom;
}

Rage::Color NoteFieldColumn::get_player_color(size_t pn)
{
	if(m_player_colors->empty())
	{
		return Rage::Color(0, 0, 0, 0);
	}
	return (*m_player_colors)[pn % m_player_colors->size()];
}

struct strip_buffer
{
	enum { size= 512 };
	Rage::SpriteVertex* buf;
	Rage::SpriteVertex* v;
	// Hold rendering requires two passes, the normal pass and the glow pass.
	// Recalculating all the vert positions for the second pass would be
	// expensive, so the glow color for each vert is stored in glow_buf.
	Rage::VColor* glow_buf;
	Rage::VColor* glow_v;
	strip_buffer()
	{
		buf= (Rage::SpriteVertex*) malloc(size * sizeof(Rage::SpriteVertex));
		glow_buf= (Rage::VColor*) malloc(size * sizeof(Rage::VColor));
		init();
	}
	~strip_buffer()
	{
		free(buf);
	}
	void init()
	{
		v= buf;
		glow_v= glow_buf;
	}
	void rollback()
	{
		// The buffer is full and has just been drawn, and more verts need to be
		// added to draw.  Move the last three verts to the beginning of the
		// buffer so that the vert calculating loop doesn't have to redo the work
		// for them.
		if(used() > 2)
		{
			buf[0]= v[-2];
			buf[1]= v[-1];
			v= buf + 2;
			glow_buf[0]= glow_v[-2];
			glow_buf[1]= glow_v[-1];
			glow_v= glow_buf + 2;
		}
	}
	void draw()
	{
		DISPLAY->DrawQuadStrip(buf, v-buf);
	}
	void swap_glow()
	{
		int verts_used= v - buf;
		for(int i= 0; i < verts_used; ++i)
		{
			Rage::VColor temp= buf[i].c;
			buf[i].c= glow_buf[i];
			glow_buf[i]= temp;
		}
	}
	int used() const { return v - buf; }
	int avail() const { return size - used(); }
	void add_vert(Rage::Vector3 const& pos, Rage::Color const& color, Rage::Color const& glow, Rage::Vector2 const& texcoord)
	{
		v->p= pos;  v->c= color;  v->t= texcoord;
		v+= 1;
		(*glow_v)= glow;
		glow_v+= 1;
	}
};

enum hold_tex_phase
{
	HTP_Top,
	HTP_Body,
	HTP_Bottom,
	HTP_Done
};

struct hold_texture_handler
{
	// Things that would be const if the calculations didn't force them to be
	// non-const.
	double tex_top;
	double tex_bottom;
	double tex_rect_h;
	double tex_body_height;
	double tex_top_end;
	double tex_body_end;
	double tex_per_y;
	double start_y;
	double body_start_y;
	double body_end_y;
	double end_y;
	// Things that will be changed/updated each time.
	double prev_bodies_left;
	int prev_phase;
	bool started_bottom;
	hold_texture_handler(double const note_size, double const head_y,
		double const tail_y, double const tex_t, double const tex_b,
		QuantizedHoldRenderData const& data)
	{
		double pix_h_recip= 1.0 / (data.part_lengths.head_pixs +
			data.part_lengths.body_pixs + data.part_lengths.body_pixs +
			data.part_lengths.tail_pixs);
		double head_pct= data.part_lengths.head_pixs * pix_h_recip;
		double body_pct= data.part_lengths.body_pixs * pix_h_recip;
		double tail_pct= data.part_lengths.tail_pixs * pix_h_recip;
		tex_top= tex_t;
		tex_bottom= tex_b;
		tex_rect_h= tex_bottom - tex_top;
		tex_per_y= tex_rect_h * pix_h_recip;
		double tex_top_height= tex_rect_h * head_pct;
		tex_body_height= tex_rect_h * body_pct;
		double tex_bottom_height= tex_rect_h * tail_pct;
		tex_top_end= tex_top + tex_top_height;
		tex_body_end= tex_bottom - tex_bottom_height;
		start_y= head_y + (note_size * data.part_lengths.start_note_offset);
		body_start_y= head_y;
		end_y= tail_y + (note_size * data.part_lengths.end_note_offset);
		body_end_y= end_y - data.part_lengths.tail_pixs;
		// constants go above this line.
		prev_bodies_left= 1.0;
		prev_phase= HTP_Top;
		started_bottom= false;
	}
	// curr_y will be modified on the transition to HTP_Bottom to make sure the
	// entire bottom is drawn.
	// The hold is drawn in several phases.  Each phase must be drawn in full,
	// so when transitioning from one phase to the next, two texture coords are
	// calculated, one with the previous phase and one with the current.  This
	// compresses the seam between phases to zero, making it invisible.
	// The texture coords are bottom aligned so that the end of the last body
	// lines up with the start of the bottom cap.
	int calc_tex_y(double& curr_y, vector<double>& ret_texc)
	{
		int phase= HTP_Top;
		if(curr_y >= end_y)
		{
			curr_y= end_y;
			ret_texc.push_back(tex_bottom);
			phase= HTP_Done;
		}
		else if(curr_y >= body_end_y)
		{
			if(started_bottom)
			{
				phase= HTP_Bottom;
			}
			else
			{
				curr_y= body_end_y;
				phase= HTP_Bottom;
				started_bottom= true;
			}
		}
		else if(curr_y >= body_start_y)
		{
			phase= HTP_Body;
		}
		if(phase != HTP_Done)
		{
			if(phase != prev_phase)
			{
				internal_calc_tex_y(prev_phase, curr_y, ret_texc);
			}
			internal_calc_tex_y(phase, curr_y, ret_texc);
			prev_phase= phase;
		}
		return phase;
	}
private:
	void internal_calc_tex_y(int phase, double& curr_y, vector<double>& ret_texc)
	{
		switch(phase)
		{
			case HTP_Top:
				ret_texc.push_back(tex_top + ((curr_y - start_y) * tex_per_y));
				break;
			case HTP_Body:
				// In the body phase, the first half of the body section of the
				// texture is repeated over the length of the hold.
				{
					double const tex_distance= (body_end_y - curr_y) * tex_per_y;
					// bodies_left decreases as more of the hold is drawn.
					double bodies_left= tex_distance / tex_body_height;
					double const floor_left= floor(bodies_left);
					bodies_left= (bodies_left - floor_left) + 1.0;
					double curr_tex_y= tex_body_end - (bodies_left * tex_body_height);
					if(bodies_left > prev_bodies_left)
					{
						ret_texc.push_back(curr_tex_y + tex_body_height);
					}
					ret_texc.push_back(curr_tex_y);
					prev_bodies_left= bodies_left;
				}
				break;
			case HTP_Bottom:
				ret_texc.push_back(((curr_y-body_end_y) * tex_per_y) + tex_body_end);
				break;
		}
	}
};

struct hold_time_lerper
{
	double start_y_off;
	double y_off_len;
	double start_time;
	double time_len;
	hold_time_lerper(double sy, double yl, double st, double tl)
		:start_y_off(sy), y_off_len(yl), start_time(st), time_len(tl)
	{}
	double lerp(double y_off)
	{
		return (((y_off - start_y_off) * time_len) / y_off_len) + start_time;
	}
};

static void add_vert_strip(float const tex_y, strip_buffer& verts_to_draw,
	Rage::Vector3 const& left,
	Rage::Vector3 const& right, Rage::Color const& color, Rage::Color const& glow_color,
	float const tex_left, float const tex_right)
{
	verts_to_draw.add_vert(left, color, glow_color, Rage::Vector2(tex_left, tex_y));
	verts_to_draw.add_vert(right, color, glow_color, Rage::Vector2(tex_right, tex_y));
}

struct hold_vert_step_state
{
	double y;
	double beat;
	double second;
	Rage::transform trans;
	vector<double> tex_coords;
	bool calc(NoteFieldColumn& col, double curr_y, double end_y,
		hold_time_lerper& beat_lerp, hold_time_lerper& second_lerp,
		hold_texture_handler& tex_handler,
		int& phase, bool is_lift, NoteFieldColumn::render_note& renderable)
	{
		tex_coords.clear();
		bool last_vert_set= false;
		y= curr_y;
		if(curr_y >= end_y)
		{
			// Different from the end check in hold_texture_handler because this
			// clips the hold off at the end of the notefield.  That clips the hold
			// at the end of the hold.
			y= end_y;
			last_vert_set= true;
		}
		// It's important to call the tex_handler before the lerpers because the
		// tex_handler changes y in certain conditions.
		phase= tex_handler.calc_tex_y(y, tex_coords);
		if(phase == HTP_Done)
		{
			last_vert_set= true;
		}
		beat= beat_lerp.lerp(y);
		second= second_lerp.lerp(y);
		renderable.input.change_eval_time(beat, second);
		renderable.input.y_offset= y;
		col.hold_render_transform(renderable.input, trans, col.m_twirl_holds);
		if(beat <= col.m_selection_end && beat >= col.m_selection_start)
		{
			trans.glow= col.get_selection_glow();
		}
		if(is_lift)
		{
			double along= (y - tex_handler.start_y) * lift_fade_dist_recip;
			if(along < 1.0)
			{
				trans.alpha*= along;
				trans.glow*= along;
			}
		}
		return last_vert_set;
	}
};

void NoteFieldColumn::draw_hold(QuantizedHoldRenderData& data,
	render_note& note, double head_beat, double head_second,
	double tail_beat, double tail_second, bool is_lift)
{
	double const original_beat= note.input.eval_beat;
	double const original_second= note.input.eval_second;
	// pos_z_vec will be used later to orient the hold.  Read below. -Kyz
	static const Rage::Vector3 pos_z_vec(0.0f, 0.0f, 1.0f);
	static const Rage::Vector3 neg_y_vec(0.0f, -1.0f, 0.0f);
	static strip_buffer verts_to_draw;
	verts_to_draw.init();
	static const double y_step= 4.0;
	double tex_top= data.rect->top;
	double tex_bottom= data.rect->bottom;
	double tex_left= data.rect->left;
	double tex_right= data.rect->right;
	switch(data.flip)
	{
		case TCFM_X:
			std::swap(tex_left, tex_right);
			break;
		case TCFM_XY:
			std::swap(tex_left, tex_right);
			std::swap(tex_top, tex_bottom);
			break;
		case TCFM_Y:
			std::swap(tex_top, tex_bottom);
			break;
		default:
			break;
	}
	double head_y_offset= note.y_offset;
	double tail_y_offset= note.tail_y_offset;
	if(tail_y_offset < head_y_offset)
	{
		// The speed mod is negative.
		std::swap(head_y_offset, tail_y_offset);
	}
	double y_off_len= tail_y_offset - head_y_offset;
	hold_time_lerper beat_lerper(head_y_offset, y_off_len, head_beat, tail_beat - head_beat);
	hold_time_lerper second_lerper(head_y_offset, y_off_len, head_second, tail_second - head_second);
	hold_texture_handler tex_handler(note_size, head_y_offset, tail_y_offset, tex_top, tex_bottom, data);
	double const body_start_render_y= apply_reverse_shift(tex_handler.body_start_y);
	double const body_end_render_y= apply_reverse_shift(tex_handler.body_end_y);
	float const color_scale= Rage::scale(note.note_iter->second.HoldResult.fLife, 0.f, 1.f, m_newskin->get_hold_gray_percent(), 1.f);
	DISPLAY->ClearAllTextures();
	DISPLAY->SetZTestMode(ZTEST_WRITE_ON_PASS);
	DISPLAY->SetZWrite(true);
	DISPLAY->SetTextureFiltering(TextureUnit_1, data.texture_filtering);
	DISPLAY->SetTextureFiltering(TextureUnit_2, data.texture_filtering);
	if(data.mask != nullptr)
	{
		Rage::Color player_color= get_player_color(note.note_iter->second.pn);
		DISPLAY->set_color_key_shader(player_color, data.mask->GetTexHandle());
	}
	bool last_vert_set= false;
	bool next_last_vert_set= false;
	// Set a start and end y so that the hold can be clipped to the start and
	// end of the field.
	double start_y= max(tex_handler.start_y, first_y_offset_visible);
	double end_y= std::min(tex_handler.end_y, last_y_offset_visible);
	// next_step exists so that the forward vector of a hold can be calculated
	// and used to make the hold turn and maintain constant width, instead of
	// being skewed.  Toggle the holds_skewed_by_mods flag with lua to see the
	// difference.
	int phase= HTP_Top;
	int next_phase= HTP_Top;
	hold_vert_step_state next_step; // The OS of the future.
	next_step.calc(*this, start_y, end_y, beat_lerper, second_lerper,
		tex_handler, next_phase, is_lift, note);
	bool need_glow_pass= false;
	for(double curr_y= start_y; !last_vert_set; curr_y+= y_step)
	{
		hold_vert_step_state curr_step= next_step;
		if(curr_step.trans.glow > .01)
		{
			need_glow_pass= true;
		}
		phase= next_phase;
		last_vert_set= next_last_vert_set;
		next_last_vert_set= next_step.calc(*this, curr_y + y_step, end_y,
			beat_lerper, second_lerper, tex_handler,
			phase, is_lift, note);
		Rage::Vector3 render_forward(0.0, 1.0, 0.0);
		if(!m_holds_skewed_by_mods)
		{
			render_forward.x= next_step.trans.pos.x - curr_step.trans.pos.x;
			if(m_add_y_offset_to_position)
			{
				render_forward.y= (next_step.y + next_step.trans.pos.y) -
					(curr_step.y + curr_step.trans.pos.y);
			}
			else
			{
				render_forward.y= next_step.trans.pos.y - curr_step.trans.pos.y;
			}
			render_forward.z= next_step.trans.pos.z - curr_step.trans.pos.z;
			render_forward= render_forward.GetNormalized();
		}
		Rage::Vector3 render_left;
		if(m_use_moddable_hold_normal)
		{
			Rage::Vector3 normal;
			m_hold_normal_mod.evaluate(note.input, normal);
			render_left= Rage::CrossProduct(normal, render_forward);
		}
		else
		{
			if(std::abs(render_forward.z) > 0.9f) // 0.9 arbitrariliy picked.
			{
				render_left= Rage::CrossProduct(neg_y_vec, render_forward);
			}
			else
			{
				render_left= Rage::CrossProduct(pos_z_vec, render_forward);
			}
		}
		if(m_twirl_holds && curr_step.trans.rot.y != 0.0)
		{
			RageAARotate(&render_left, &render_forward, -curr_step.trans.rot.y);
		}
		render_left*= (.5 * m_newskin->get_width()) * curr_step.trans.zoom.x;
		// Hold caps need to not be squished by the reverse_scale.
		double render_y= curr_y;
		if(m_add_y_offset_to_position)
		{
			switch(phase)
			{
				case HTP_Top:
					render_y= curr_step.trans.pos.y + body_start_render_y -
						((tex_handler.body_start_y - curr_y) * reverse_scale_sign);
					break;
				case HTP_Bottom:
				case HTP_Done:
					render_y= curr_step.trans.pos.y + body_end_render_y +
						((curr_y - tex_handler.body_end_y) * reverse_scale_sign);
					break;
				case HTP_Body:
				default:
					render_y= apply_reverse_shift(curr_y) + curr_step.trans.pos.y;
					break;
			}
		}
		else
		{
			render_y= curr_step.trans.pos.y;
		}
		const Rage::Vector3 left_vert(
			render_left.x + curr_step.trans.pos.x, render_y + render_left.y,
			render_left.z + curr_step.trans.pos.z);
		const Rage::Vector3 right_vert(
			-render_left.x + curr_step.trans.pos.x, render_y -render_left.y,
			-render_left.z + curr_step.trans.pos.z);
		const Rage::Color color(color_scale, color_scale, color_scale, curr_step.trans.alpha);
		const Rage::Color glow_color(1.0, 1.0, 1.0, curr_step.trans.glow);
#define add_vert_strip_args verts_to_draw, left_vert, right_vert, color, glow_color, tex_left, tex_right
		for(size_t i= 0; i < curr_step.tex_coords.size(); ++i)
		{
			add_vert_strip(curr_step.tex_coords[i], add_vert_strip_args);
		}
#undef add_vert_strip_args
		if(verts_to_draw.avail() < 6 || last_vert_set)
		{
			DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Modulate);
			DISPLAY->SetCullMode(CULL_NONE);
			DISPLAY->SetTextureWrapping(TextureUnit_1, false);
			for(size_t t= 0; t < data.parts.size(); ++t)
			{
				DISPLAY->SetTexture(TextureUnit_1, data.parts[t]->GetTexHandle());
				DISPLAY->SetBlendMode(t == 0 ? BLEND_NORMAL : BLEND_ADD);
				verts_to_draw.draw();
			}
			if(need_glow_pass)
			{
				verts_to_draw.swap_glow();
				DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Glow);
				for(size_t t= 0; t < data.parts.size(); ++t)
				{
					DISPLAY->SetTexture(TextureUnit_1, data.parts[t]->GetTexHandle());
					DISPLAY->SetBlendMode(t == 0 ? BLEND_NORMAL : BLEND_ADD);
					verts_to_draw.draw();
				}
			}
			if(!last_vert_set)
			{
				// Intentionally swap the glow back after calling rollback so that
				// only the set of verts that will remain are swapped.
				verts_to_draw.rollback();
				if(need_glow_pass)
				{
					verts_to_draw.swap_glow();
				}
				need_glow_pass= false;
			}
		}
	}
	note.input.change_eval_time(original_beat, original_second);
}

bool NoteFieldColumn::EarlyAbortDraw() const
{
	return m_newskin == nullptr || m_note_data == nullptr || m_timing_data == nullptr;
}

void NoteFieldColumn::imitate_did_note(TapNote const& tap)
{
	if(m_use_game_music_beat)
	{
		return;
	}
	double judged_time= tap.occurs_at_second + tap.result.fTapNoteOffset;
	bool prev_diff_over_zero= (m_prev_curr_second - judged_time > 0);
	bool curr_diff_over_zero= (m_curr_second - judged_time > 0);
	if(prev_diff_over_zero != curr_diff_over_zero)
	{
		// Pass false for the bright arg because it is not stored when the note
		// is hit.
		if(tap.type == TapNoteType_HoldHead)
		{
			did_hold_note_internal(tap.HoldResult.hns, false);
		}
		else
		{
			did_tap_note_internal(tap.result.tns, false);
		}
	}
}

void NoteFieldColumn::update_upcoming(double beat, double second)
{
	double const beat_dist= beat - m_curr_beat;
	double const sec_dist= second - m_curr_second;
	if(sec_dist > 0 && sec_dist < m_status.upcoming_second_dist)
	{
		m_status.found_upcoming= true;
		m_status.upcoming_beat_dist= beat_dist;
		m_status.upcoming_second_dist= sec_dist;
	}
}

void NoteFieldColumn::update_active_hold(TapNote const& tap)
{
	if(tap.subType != TapNoteSubType_Invalid && tap.HoldResult.bActive &&
		tap.occurs_at_second <= m_curr_second && tap.end_second >= m_curr_second)
	{
		m_status.active_hold= &tap;
	}
}

void NoteFieldColumn::get_hold_draw_time(TapNote const& tap, double const hold_beat, double& beat, double& second)
{
	double const last_held_beat= tap.HoldResult.GetLastHeldBeat();
	double const last_held_second= tap.HoldResult.last_held_second;
	if(last_held_second > tap.occurs_at_second && m_use_game_music_beat)
	{
		if(fabs(last_held_second - m_curr_second) < .01)
		{
			beat= m_curr_beat;
			second= m_curr_second;
			return;
		}
		// TODO: Figure out whether this does the wrong thing for holds that are
		// released during a stop.
		beat= last_held_beat;
		second= last_held_second;
		return;
	}
	beat= hold_beat;
	second= tap.occurs_at_second;
}

void init_render_note_for_lift_at_further_time(NoteFieldColumn::render_note& that,
	NoteFieldColumn* column, double pretrail_beat, double pretrail_second)
{
	double beat_from_second= column->get_beat_from_second(pretrail_second);
	double second_from_beat= column->get_second_from_beat(pretrail_beat);
	mod_val_inputs input= that.input;
	input.change_eval_time(beat_from_second, pretrail_second);
	double second_y_offset= column->calc_y_offset(input);
	input.change_eval_time(pretrail_beat, second_from_beat);
	double beat_y_offset= column->calc_y_offset(input);
	if(second_y_offset < beat_y_offset)
	{
		that.tail_y_offset= second_y_offset;
		that.tail_beat= beat_from_second;
		that.tail_second= pretrail_second;
	}
	else
	{
		that.tail_y_offset= beat_y_offset;
		that.tail_beat= pretrail_beat;
		that.tail_second= second_from_beat;
	}
}

NoteFieldColumn::render_note::render_note(NoteFieldColumn* column,
	NoteData::TrackMap::const_iterator column_begin,
	NoteData::TrackMap::const_iterator column_end,
	NoteData::TrackMap::const_iterator iter)
{
	note_iter= column_end;
	double beat= NoteRowToBeat(iter->first);
	input.init(beat, iter->second.occurs_at_second, column->get_curr_beat(),
		column->get_curr_second(), &(iter->second));
	switch(iter->second.type)
	{
		case TapNoteType_HoldHead:
			{
				double hold_draw_beat;
				double hold_draw_second;
				column->get_hold_draw_time(iter->second, beat, hold_draw_beat, hold_draw_second);
				input.change_eval_time(hold_draw_beat, hold_draw_second);
				y_offset= column->calc_y_offset(input);
				input.change_eval_time(beat + NoteRowToBeat(iter->second.iDuration), iter->second.end_second);
				tail_y_offset= column->calc_y_offset(input);
				int head_visible= column->y_offset_visible(y_offset);
				int tail_visible= column->y_offset_visible(tail_y_offset);
				// y_offset_visible returns 3 possible things:
				// -1 (before first), 0 (in range), 1 (after last)
				// If the head is off the top, and the tail is off the bottom, then
				// head_visible is -1 and tail_visible is 1, but the hold needs to be
				// drawn.
				// value table time. I means invisible, D means draw.
				//     -1 | 0 | 1
				// -1 | I | D | D
				//  0 | D | D | D
				//  1 | D | D | I
				// Thus, this weird condition.
				if(head_visible + tail_visible != 2 && head_visible + tail_visible != -2)
				{
					note_iter= iter;
				}
				input.change_eval_time(beat, iter->second.occurs_at_second);
			}
			break;
		case TapNoteType_Lift:
			{
				y_offset= column->calc_y_offset(input);
				double pretrail_len= column->m_lift_pretrail_length.evaluate(input);
				// To deal with the case where a lift is right after a stop, calculate
				// whether using pretrail_len as seconds or as beats puts the tail
				// further away.
				double pretrail_second= iter->second.occurs_at_second - pretrail_len;
				double pretrail_beat= beat - pretrail_len;
				if(note_iter == column_begin)
				{
					init_render_note_for_lift_at_further_time(*this, column, pretrail_beat,
						pretrail_second);
				}
				else
				{
					auto prev_note= iter;
					--prev_note;
					double prev_beat= 0.0;
					double prev_second= 0.0;
					if(prev_note->second.type == TapNoteType_HoldHead)
					{
						prev_beat= NoteRowToBeat(prev_note->first + prev_note->second.iDuration);
						prev_second= prev_note->second.end_second;
					}
					else
					{
						prev_beat= NoteRowToBeat(prev_note->first);
						prev_second= prev_note->second.occurs_at_second;
					}
					if(prev_beat > pretrail_beat || prev_second > pretrail_second)
					{
						input.change_eval_time(pretrail_beat, pretrail_second);
						tail_y_offset= column->calc_y_offset(input);
						input.change_eval_time(beat, iter->second.occurs_at_second);
						tail_beat= pretrail_beat;
						tail_second= pretrail_second;
					}
					else
					{
						init_render_note_for_lift_at_further_time(*this, column,
							pretrail_beat, pretrail_second);
					}
				}
				int head_visible= column->y_offset_visible(tail_y_offset);
				int tail_visible= column->y_offset_visible(y_offset);
				if(head_visible + tail_visible != 2 && head_visible + tail_visible != -2)
				{
					note_iter= iter;
				}
			}
			break;
		default:
			{
				y_offset= column->calc_y_offset(input);
				if(column->y_offset_visible(y_offset) == 0)
				{
					note_iter= iter;
				}
			}
			break;
	}
}

void NoteFieldColumn::add_renderable_to_lists(render_note& renderable)
{
	int tap_row= renderable.note_iter->first;
	double tap_beat= NoteRowToBeat(tap_row);
	const TapNote& tn= renderable.note_iter->second;
	switch(tn.type)
	{
		case TapNoteType_Empty:
			return;
		case TapNoteType_Tap:
		case TapNoteType_Mine:
		case TapNoteType_Attack:
		case TapNoteType_AutoKeysound:
		case TapNoteType_Fake:
			if((!tn.result.bHidden || !m_use_game_music_beat) &&
				(m_show_unjudgable_notes || m_timing_data->IsJudgableAtBeat(tap_beat)))
			{
				render_taps.push_front(renderable);
				imitate_did_note(tn);
				update_upcoming(tap_beat, tn.occurs_at_second);
				update_active_hold(tn);
			}
			break;
		case TapNoteType_HoldHead:
			if((tn.HoldResult.hns != HNS_Held || !m_use_game_music_beat) &&
				(m_show_unjudgable_notes || m_timing_data->IsJudgableAtBeat(tap_beat)))
			{
				// Hold heads are added to the tap list to take care of rendering
				// heads and tails in the same phase as taps.
				render_taps.push_front(renderable);
				render_holds.push_front(renderable);
				imitate_did_note(tn);
				update_upcoming(tap_beat, tn.occurs_at_second);
				update_active_hold(tn);
			}
			break;
		case TapNoteType_Lift:
			if((!tn.result.bHidden || !m_use_game_music_beat) &&
				(m_show_unjudgable_notes || m_timing_data->IsJudgableAtBeat(tap_beat)))
			{
				render_taps.push_front(renderable);
				render_lifts.push_front(renderable);
				imitate_did_note(tn);
				update_upcoming(tap_beat, tn.occurs_at_second);
				update_active_hold(tn);
			}
			break;
		default:
			break;
	}
}

void NoteFieldColumn::build_render_lists()
{
	m_curr_displayed_beat= m_curr_beat;
	if(m_scroll_segments_enabled)
	{
		m_curr_displayed_beat= m_timing_data->GetDisplayedBeat(m_curr_beat);
	}
	if(!m_in_defective_mode)
	{
		mod_val_inputs input(m_curr_beat, m_curr_second);
		Rage::transform trans;
		m_column_mod.evaluate(input, trans);
		set_transform(trans);
		calc_transform(input, head_transform);
		if(m_add_y_offset_to_position)
		{
			head_transform.pos.y+= apply_reverse_shift(head_y_offset());
		}
		receptor_alpha= m_receptor_alpha.evaluate(input);
		receptor_glow= m_receptor_glow.evaluate(input);
		explosion_alpha= m_explosion_alpha.evaluate(input);
		explosion_glow= m_explosion_glow.evaluate(input);
	}
	else
	{
		double head_yoff= head_y_offset();
		m_defective_mods->get_transform(m_curr_beat, head_yoff,
			apply_reverse_shift(head_yoff), m_column, head_transform);
		receptor_alpha= m_defective_mods->get_receptor_alpha();
		receptor_glow= 0.0;
		explosion_alpha= 1.0;
		explosion_glow= 0.0;
	}

	// Clearing and rebuilding the list of taps to render every frame is
	// unavoidable because the notes move every frame, which changes the y
	// offset, which changes what is visible.  So even if the list wasn't
	// cleared, it would still have to be traversed to recalculate the y
	// offsets every frame.
	render_holds.clear();
	render_lifts.clear();
	render_taps.clear();
	m_status.upcoming_beat_dist= 1000.0;
	m_status.upcoming_second_dist= 1000.0;
	m_status.prev_active_hold= m_status.active_hold;
	m_status.active_hold= nullptr;
	m_status.found_upcoming= false;
	if(m_newskin->get_anim_uses_beats())
	{
		m_status.anim_percent= m_curr_beat;
	}
	else
	{
		m_status.anim_percent= m_curr_second;
	}
	m_status.anim_percent= fmod(m_status.anim_percent * m_newskin->get_anim_mult(), 1.0);
	if(m_status.anim_percent < 0.0)
	{
		m_status.anim_percent+= 1.0;
	}

	auto column_end= m_note_data->end(m_column);
	auto column_begin= m_note_data->begin(m_column);
	// Don't do any note processing if there are no notes in the column. -Kyz
	if(column_begin != column_end)
	{
		// The note row closest to the current time is saved between frames to
		// make finding the place to start rendering faster. -Kyz
		if(note_row_closest_to_current_time == -1)
		{
			note_row_closest_to_current_time= BeatToNoteRow(m_curr_beat);
		}
		NoteData::TrackMap::const_iterator note_closest_to_current_time=
			m_note_data->lower_bound(m_column, note_row_closest_to_current_time);
		if(note_closest_to_current_time == column_end)
		{
			auto almost_end= column_end;
			--almost_end;
			if(fabs(m_curr_beat - NoteRowToBeat(column_begin->first)) >
				fabs(NoteRowToBeat(almost_end->first) - m_curr_beat))
			{
				note_closest_to_current_time= almost_end;
			}
			else
			{
				note_closest_to_current_time= column_begin;
			}
		}
		if(m_curr_beat - NoteRowToBeat(note_closest_to_current_time->first) > 0)
		{
			// Seek forward until the next note is after the current time.
			auto next_note= note_closest_to_current_time;
			++next_note;
			while(next_note != column_end &&
				(m_curr_beat - NoteRowToBeat(next_note->first) > 0))
			{
				note_closest_to_current_time= next_note;
				++next_note;
			}
		}
		else
		{
			// Seek backward until the next note is before the current time.
			auto next_note= note_closest_to_current_time;
			while(next_note != column_begin &&
				(m_curr_beat - NoteRowToBeat(next_note->first) < 0))
			{
				--next_note;
				note_closest_to_current_time= next_note;
			}
		}
		note_row_closest_to_current_time= note_closest_to_current_time->first;
		// There should be no notes between note_closest_to_current_time and the
		// current time. -Kyz

		// Work backwards from the current time until the first non visible note
		// is reached. -Kyz
		for(auto curr_note= note_closest_to_current_time; ; --curr_note)
		{
			render_note renderable(this, column_begin, column_end, curr_note);
			if(renderable.note_iter != column_end)
			{
				add_renderable_to_lists(renderable);
			}
			else
			{
				break;
			}
			if(curr_note == column_begin)
			{
				break;
			}
		}
		// Work forwards until we hit time_to_end_visible_notes consecutive
		// seconds of non-visible notes.  The consecutive requirement is to
		// handle things like boomerang, where a range of notes are visible, then
		// some notes are not, then another range is visible. -Kyz
		{
			// Make sure note_closest_to_current_time is not handled twice. -Kyz
			auto curr_note= note_closest_to_current_time;
			++curr_note;
			double const time_to_end_visible_notes= 10.0;
			double first_non_visible_time= -1000.0;
			bool found_end_of_visible_notes= false;
			for(; curr_note != column_end && !found_end_of_visible_notes; ++curr_note)
			{
				TapNote const& tn= curr_note->second;
				render_note renderable(this, column_begin, column_end, curr_note);
				if(renderable.note_iter != column_end)
				{
					add_renderable_to_lists(renderable);
					first_non_visible_time= -1000.0;
				}
				else
				{
					if(first_non_visible_time > 0.0)
					{
						if(tn.occurs_at_second > m_curr_second && tn.occurs_at_second - first_non_visible_time > time_to_end_visible_notes)
						{
							found_end_of_visible_notes= true;
						}
					}
					else
					{
						first_non_visible_time= tn.occurs_at_second;
					}
				}
			}
		}
		if(!m_status.found_upcoming)
		{
			// By logic, note_closest_to_current_time occurs before current time,
			// and the next note after it occurs after current time.  But, there
			// might be empty tapnote entries, so we still need a loop. -Kyz
			auto curr_note= note_closest_to_current_time;
			++curr_note;
			double const max_try_second= m_curr_second + m_upcoming_time;
			for(; curr_note != column_end && !m_status.found_upcoming; ++curr_note)
			{
				int tap_row= curr_note->first;
				double tap_beat= NoteRowToBeat(tap_row);
				const TapNote& tn= curr_note->second;
				if(tn.type != TapNoteType_Empty)
				{
					if(tn.occurs_at_second > max_try_second)
					{
						m_status.found_upcoming= true;
					}
					else
					{
						update_upcoming(tap_beat, tn.occurs_at_second);
					}
				}
			}
		}
		m_prev_curr_second= m_curr_second;
	}
	{
		double beat= m_curr_beat - floor(m_curr_beat);
		Message msg("BeatUpdate");
		msg.SetParam("beat", beat);
		msg.SetParam("beat_distance", m_status.upcoming_beat_dist);
		msg.SetParam("second_distance", m_status.upcoming_second_dist);
		if(pressed != was_pressed)
		{
			if(pressed)
			{
				msg.SetParam("pressed", true);
			}
			else
			{
				msg.SetParam("lifted", true);
			}
		}
		pass_message_to_heads(msg);
	}
	// The hold status should be updated if there is a currently active hold
	// or if there was one last frame.
	if(m_status.active_hold != nullptr || m_status.prev_active_hold != nullptr)
	{
		bool curr_is_null= m_status.active_hold == nullptr;
		bool prev_is_null= m_status.prev_active_hold == nullptr;
		TapNote const* pass= curr_is_null ? m_status.prev_active_hold : m_status.active_hold;
		set_hold_status(pass, prev_is_null, curr_is_null);
	}
}

void NoteFieldColumn::draw_child(int child)
{
#define RETURN_IF_EMPTY(empty_check) if(empty_check) { return; } break;
	switch(child)
	{
		case holds_child_index:
			RETURN_IF_EMPTY(render_holds.empty());
		case lifts_child_index:
			RETURN_IF_EMPTY(render_lifts.empty());
		case taps_child_index:
			RETURN_IF_EMPTY(render_taps.empty());
		case selection_child_index:
			RETURN_IF_EMPTY(m_field == nullptr || !m_field->m_in_edit_mode);
		default:
			RETURN_IF_EMPTY(child < 0 || static_cast<size_t>(child) >= m_SubActors.size());
	}
#undef RETURN_IF_EMPTY
	curr_render_child= child;
	Draw();
}

void NoteFieldColumn::draw_holds_internal()
{
	bool const reverse= reverse_scale_sign < 0.0;
	// Each hold body needs to be drawn with a different z bias so that they
	// don't z fight when they overlap.
	for(auto&& holdit : render_holds)
	{
		m_field->update_z_bias();
		// The hold loop does not need to call update_upcoming or
		// update_active_hold beccause the tap loop handles them when drawing
		// heads.
		TapNote const& tn= holdit.note_iter->second;
		double const hold_beat= NoteRowToBeat(holdit.note_iter->first);
		double const quantization= quantization_for_time(holdit.input);
		bool active= HOLD_COUNTS_AS_ACTIVE(tn);
		QuantizedHoldRenderData data;
		m_newskin->get_hold_render_data(tn.subType, m_playerize_mode, tn.pn,
			active, reverse, quantization, m_status.anim_percent, data);
		double hold_draw_beat;
		double hold_draw_second;
		get_hold_draw_time(tn, hold_beat, hold_draw_beat, hold_draw_second);
		double passed_amount= hold_draw_beat - hold_beat;
		if(!data.parts.empty())
		{
			draw_hold(data, holdit, hold_draw_beat, hold_draw_second,
				hold_draw_beat + NoteRowToBeat(tn.iDuration) - passed_amount,
				tn.end_second, false);
		}
	}
}

void NoteFieldColumn::draw_lifts_internal()
{
	bool const reverse= reverse_scale_sign < 0.0;
	for(auto&& liftit : render_lifts)
	{
		m_field->update_z_bias();
		TapNote const& tn= liftit.note_iter->second;
		double const lift_beat= NoteRowToBeat(liftit.note_iter->first);
		double const lift_second= tn.occurs_at_second;
		double const quantization= quantization_for_time(liftit.input);
		QuantizedHoldRenderData data;
		m_newskin->get_hold_render_data(TapNoteSubType_Hold, m_playerize_mode,
			tn.pn, false, reverse, quantization, m_status.anim_percent, data);
		if(!data.parts.empty())
		{
			// The tail of a lift comes before the note, so pass the tail time in
			// the head fields for draw_hold.
			draw_hold(data, liftit, liftit.tail_beat, liftit.tail_second,
				lift_beat, lift_second, true);
		}
	}
}

typedef Actor* (NoteSkinColumn::* get_norm_actor_fun)(size_t, double, double, bool, bool);
typedef Actor* (NoteSkinColumn::* get_play_actor_fun)(size_t, size_t, double, bool, bool);

struct tap_draw_info
{
	double draw_beat;
	double draw_second;
	double y_offset;
	Actor* act;
	tap_draw_info()
		:draw_beat(0.0), draw_second(0.0), y_offset(0.0), act(nullptr)
	{}
};

void set_tap_actor_info(tap_draw_info& draw_info, NoteFieldColumn& col,
	NoteSkinColumn* newskin, get_norm_actor_fun get_normal,
	get_play_actor_fun get_playerized, size_t part,
	size_t pn, double draw_beat, double draw_second, double yoff,
	double anim_percent, bool active,
	bool reverse, NoteFieldColumn::render_note& note)
{
	draw_info.draw_beat= draw_beat;
	draw_info.y_offset= yoff;
	if(col.y_offset_visible(yoff) == 0)
	{
		if(col.get_playerize_mode() != NPM_Quanta)
		{
			double const quantization= col.quantization_for_time(note.input);
			draw_info.act= (newskin->*get_normal)(part, quantization, anim_percent,
				active, reverse);
		}
		else
		{
			draw_info.act= (newskin->*get_playerized)(part, pn, anim_percent,
				active, reverse);
		}
		draw_info.draw_second= draw_second;
	}
}

void NoteFieldColumn::draw_taps_internal()
{
	for(auto&& tapit : render_taps)
	{
		TapNote const& tn= tapit.note_iter->second;
		double const tap_beat= NoteRowToBeat(tapit.note_iter->first);
		double const tap_second= tn.occurs_at_second;
		NoteSkinTapPart part= NSTP_Tap;
		NoteSkinTapOptionalPart head_part= NoteSkinTapOptionalPart_Invalid;
		NoteSkinTapOptionalPart tail_part= NoteSkinTapOptionalPart_Invalid;
		double head_beat;
		double tail_beat;
		double head_second;
		bool active= false;
		switch(tn.type)
		{
			case TapNoteType_Mine:
				part= NSTP_Mine;
				head_beat= tap_beat;
				break;
			case TapNoteType_Lift:
				part= NSTP_Lift;
				head_beat= tap_beat;
				break;
			case TapNoteType_HoldHead:
				part= NoteSkinTapPart_Invalid;
				get_hold_draw_time(tn, tap_beat, head_beat, head_second);
				active= HOLD_COUNTS_AS_ACTIVE(tn);
				tail_beat= tap_beat + NoteRowToBeat(tn.iDuration);
				switch(tn.subType)
				{
					case TapNoteSubType_Hold:
						head_part= NSTOP_HoldHead;
						tail_part= NSTOP_HoldTail;
						break;
					case TapNoteSubType_Roll:
						head_part= NSTOP_RollHead;
						tail_part= NSTOP_RollTail;
						break;
						/* TODO: Implement checkpoint holds as a subtype.  This code
							 makes the noteskin support it, but support is needed in other
							 areas.
					case TapNoteSubType_Checkpoint:
						head_part= NSTOP_CheckpointHead;
						tail_part= NSTOP_CheckpointTail;
						break;
						*/
					default:
						break;
				}
				break;
			default:
				part= NSTP_Tap;
				if(m_newskin->get_use_hold_head())
				{
					switch(tn.highest_subtype_on_row)
					{
						case TapNoteSubType_Hold:
							part= NoteSkinTapPart_Invalid;
							head_part= NSTOP_HoldHead;
							break;
						case TapNoteSubType_Roll:
							part= NoteSkinTapPart_Invalid;
							head_part= NSTOP_RollHead;
							break;
						default:
							break;
					}
				}
				head_beat= tap_beat;
				break;
		}
		vector<tap_draw_info> acts(2);
		if(part != NoteSkinTapPart_Invalid)
		{
			set_tap_actor_info(acts[0], *this, m_newskin,
				&NoteSkinColumn::get_tap_actor, &NoteSkinColumn::get_player_tap, part,
				tn.pn, tap_beat, tap_second, tapit.y_offset,
				m_status.anim_percent, active, m_status.in_reverse, tapit);
		}
		else
		{
			// Handle the case where it's an obscenity instead of an actual hold.
			if(tail_part == NoteSkinTapOptionalPart_Invalid)
			{
				set_tap_actor_info(acts[0], *this, m_newskin,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, head_part, tn.pn,
					tap_beat, tap_second, tapit.y_offset,
					m_status.anim_percent, active, m_status.in_reverse, tapit);
			}
			else
			{
				// Put tails on the list first because they need to be under the heads.
				set_tap_actor_info(acts[0], *this, m_newskin,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, tail_part, tn.pn,
					tail_beat, tn.end_second, tapit.tail_y_offset,
					m_status.anim_percent, active, m_status.in_reverse, tapit);
				set_tap_actor_info(acts[1], *this, m_newskin,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, head_part, tn.pn,
					tap_beat, tap_second, tapit.y_offset,
					m_status.anim_percent, active, m_status.in_reverse, tapit);
			}
		}
		for(auto&& act : acts)
		{
			// Tails are optional, get_optional_actor returns nullptr if the
			// noteskin doesn't have them.
			if(act.act != nullptr)
			{
				Rage::transform trans;
				tapit.input.change_eval_time(act.draw_beat, act.draw_second);
				tapit.input.y_offset= act.y_offset;
				calc_transform_with_glow_alpha(tapit.input, trans);
				if(!m_in_defective_mode)
				{
					if(m_add_y_offset_to_position)
					{
						trans.pos.y+= apply_reverse_shift(act.y_offset);
					}
				}
				act.act->set_transform_with_glow_alpha(trans);
				if(tap_beat <= m_selection_end && tap_beat >= m_selection_start)
				{
					act.act->SetGlow(Rage::Color(1, 1, 1, get_selection_glow()));
				}
				// Need update the z bias when drawing taps to handle 3D notes.
				act.act->recursive_set_z_bias(m_field->update_z_bias());
				if(m_playerize_mode == NPM_Mask)
				{
					act.act->recursive_set_mask_color(get_player_color(tn.pn));
				}
				act.act->Draw();
			}
		}
		tapit.input.change_eval_time(tap_beat, tap_second);
		tapit.input.y_offset= tapit.y_offset;
	}
}

void NoteFieldColumn::draw_selection_internal()
{
	if(m_selection_start == -1.0)
	{
		return;
	}
	double start_second= m_timing_data->GetElapsedTimeFromBeat(m_selection_start);
	mod_val_inputs start_input(m_selection_start, start_second, m_curr_beat, m_curr_second);
	start_input.y_offset= calc_y_offset(start_input);
	Rage::Vector3 start_pos;
	calc_pos_only(start_input, start_pos);
	if(m_selection_end == -1.0)
	{
		if(m_add_y_offset_to_position)
		{
			start_pos.y+= apply_reverse_shift(start_input.y_offset);
		}
		m_area_highlight.set_pos(start_pos);
		m_area_highlight.SetWidth(m_newskin->get_width());
		m_area_highlight.SetHeight(note_size);
		m_area_highlight.SetDiffuse(Rage::Color(0.5f, 0.5f, 0.5f, 0.5f));
	}
	else
	{
		double end_second= m_timing_data->GetElapsedTimeFromBeat(m_selection_end);
		mod_val_inputs end_input(m_selection_end, end_second, m_curr_beat, m_curr_second);
		end_input.y_offset= calc_y_offset(end_input);
		Rage::Vector3 end_pos;
		calc_pos_only(end_input, end_pos);
		if(m_add_y_offset_to_position)
		{
			start_pos.y+= apply_reverse_shift(start_input.y_offset);
			end_pos.y+= apply_reverse_shift(end_input.y_offset);
		}
		double height= end_pos.y - start_pos.y;
		start_pos.x= (start_pos.x + end_pos.x) * .5;
		start_pos.y= (start_pos.y + end_pos.y) * .5;
		start_pos.z= (start_pos.z + end_pos.z) * .5;
		m_area_highlight.set_pos(start_pos);
		m_area_highlight.SetWidth(m_newskin->get_width());
		m_area_highlight.SetHeight(height);
		m_area_highlight.SetDiffuse(AREA_HIGHLIGHT_COLOR);
	}
	m_area_highlight.Draw();
}

static Message create_did_message(bool bright)
{
	Message msg("ColumnJudgment");
	msg.SetParam("bright", bright);
	return msg;
}

void NoteFieldColumn::pass_message_to_heads(Message& msg)
{
	HandleMessage(msg);
}

void NoteFieldColumn::did_tap_note_internal(TapNoteScore tns, bool bright)
{
	Message msg(create_did_message(bright));
	msg.SetParam("tap_note_score", tns);
	pass_message_to_heads(msg);
}

void NoteFieldColumn::did_hold_note_internal(HoldNoteScore hns, bool bright)
{
	Message msg(create_did_message(bright));
	msg.SetParam("hold_note_score", hns);
	pass_message_to_heads(msg);
}

void NoteFieldColumn::did_tap_note(TapNoteScore tns, bool bright)
{
	if(m_use_game_music_beat)
	{
		did_tap_note_internal(tns, bright);
	}
}

void NoteFieldColumn::did_hold_note(HoldNoteScore hns, bool bright)
{
	if(m_use_game_music_beat)
	{
		did_hold_note_internal(hns, bright);
	}
}

void NoteFieldColumn::set_hold_status(TapNote const* tap, bool start, bool end)
{
	Message msg("Hold");
	if(tap != nullptr)
	{
		msg.SetParam("type", tap->subType);
		msg.SetParam("life", tap->HoldResult.fLife);
		msg.SetParam("start", start);
		msg.SetParam("finished", end);
	}
	pass_message_to_heads(msg);
}

void NoteFieldColumn::set_pressed(bool on)
{
	was_pressed= pressed;
	pressed= on;
}

void NoteFieldColumn::DrawPrimitives()
{
	if(!m_being_drawn_by_proxy)
	{
		draw_child_internal();
	}
	else
	{
		std::vector<NoteField::field_draw_entry> draw_entries;
		draw_entries.reserve(m_SubActors.size() + 3);
#define ADD_IF_HAVE_SOME(some) if(!render_##some.empty()) { \
		draw_entries.push_back({-1, some##_child_index, some##_draw_order}); }
		ADD_IF_HAVE_SOME(holds);
		ADD_IF_HAVE_SOME(lifts);
		ADD_IF_HAVE_SOME(taps);
#undef ADD_IF_HAVE_SOME
		for(int sub= 0; sub < static_cast<int>(m_SubActors.size()); ++sub)
		{
			draw_entries.push_back({-1, sub, m_SubActors[sub]->GetDrawOrder()});
		}
		std::sort(draw_entries.begin(), draw_entries.end());
		for(auto&& entry : draw_entries)
		{
			curr_render_child= entry.child;
			draw_child_internal();
		}
	}
}

void NoteFieldColumn::draw_child_internal()
{
	switch(curr_render_child)
	{
		case holds_child_index:
			draw_holds_internal();
			break;
		case lifts_child_index:
			draw_lifts_internal();
			break;
		case taps_child_index:
			draw_taps_internal();
			break;
		case selection_child_index:
			draw_selection_internal();
			break;
		default:
			if(curr_render_child >= 0 && static_cast<size_t>(curr_render_child) < m_SubActors.size())
			{
				apply_render_info_to_layer(m_SubActors[curr_render_child],
					m_layer_render_info[curr_render_child], head_transform,
					receptor_alpha, receptor_glow, explosion_alpha, explosion_glow,
					m_curr_beat, m_curr_second, m_note_alpha, m_note_glow);
				m_SubActors[curr_render_child]->Draw();
			}
			break;
	}
}

void NoteFieldColumn::position_actor_at_column_head(Actor* act,
	FieldLayerRenderInfo& info)
{
	apply_render_info_to_layer(act, info, head_transform, receptor_alpha,
		receptor_glow, explosion_alpha, explosion_glow,
		m_curr_beat, m_curr_second, m_note_alpha, m_note_glow);
}

void NoteFieldColumn::set_playerize_mode(NotePlayerizeMode mode)
{
	if(mode == NPM_Mask)
	{
		if(!m_newskin->supports_masking())
		{
			mode= NPM_Quanta;
		}
	}
	m_playerize_mode= mode;
}

void NoteFieldColumn::set_layer_fade_type(Actor* child, FieldLayerFadeType type)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		m_layer_render_info[index].fade_type= type;
	}
}

FieldLayerFadeType NoteFieldColumn::get_layer_fade_type(Actor* child)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		return m_layer_render_info[index].fade_type;
	}
	return FieldLayerFadeType_Invalid;
}

void NoteFieldColumn::set_layer_transform_type(Actor* child, FieldLayerTransformType type)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		m_layer_render_info[index].transform_type= type;
	}
}

FieldLayerTransformType NoteFieldColumn::get_layer_transform_type(Actor* child)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		return m_layer_render_info[index].transform_type;
	}
	return FieldLayerTransformType_Invalid;
}


REGISTER_ACTOR_CLASS(NoteField);

static const char* FieldVanishTypeNames[] = {
	"RelativeToParent",
	"RelativeToSelf",
	"RelativeToOrigin"
};
XToString(FieldVanishType);
LuaXType(FieldVanishType);

static ThemeMetric<Rage::Color> BPM_COLOR ("NoteField", "BPMColor");
static ThemeMetric<Rage::Color> STOP_COLOR ("NoteField", "StopColor");
static ThemeMetric<Rage::Color> DELAY_COLOR ("NoteField", "DelayColor");
static ThemeMetric<Rage::Color> WARP_COLOR ("NoteField", "WarpColor");
static ThemeMetric<Rage::Color> TIME_SIG_COLOR ("NoteField", "TimeSignatureColor");
static ThemeMetric<Rage::Color> TICKCOUNT_COLOR ("NoteField", "TickcountColor");
static ThemeMetric<Rage::Color> COMBO_COLOR ("NoteField", "ComboColor");
static ThemeMetric<Rage::Color> LABEL_COLOR ("NoteField", "LabelColor");
static ThemeMetric<Rage::Color> SPEED_COLOR ("NoteField", "SpeedColor");
static ThemeMetric<Rage::Color> SCROLL_COLOR ("NoteField", "ScrollColor");
static ThemeMetric<Rage::Color> FAKE_COLOR ("NoteField", "FakeColor");

static ThemeMetric<Rage::Color> BGL1_COLOR("NoteField", "BackgroundLayer1Color");
static ThemeMetric<Rage::Color> BGL2_COLOR("NoteField", "BackgroundLayer2Color");
static ThemeMetric<Rage::Color> FGL_COLOR("NoteField", "ForegroundLayerColor");

static ThemeMetric<bool> BPM_IS_LEFT_SIDE ("NoteField", "BPMIsLeftSide");
static ThemeMetric<bool> STOP_IS_LEFT_SIDE ("NoteField", "StopIsLeftSide");
static ThemeMetric<bool> DELAY_IS_LEFT_SIDE ("NoteField", "DelayIsLeftSide");
static ThemeMetric<bool> WARP_IS_LEFT_SIDE ("NoteField", "WarpIsLeftSide");
static ThemeMetric<bool> TIME_SIG_IS_LEFT_SIDE ("NoteField", "TimeSignatureIsLeftSide");
static ThemeMetric<bool> TICKCOUNT_IS_LEFT_SIDE ("NoteField", "TickcountIsLeftSide");
static ThemeMetric<bool> COMBO_IS_LEFT_SIDE ("NoteField", "ComboIsLeftSide");
static ThemeMetric<bool> LABEL_IS_LEFT_SIDE ("NoteField", "LabelIsLeftSide");
static ThemeMetric<bool> SPEED_IS_LEFT_SIDE ("NoteField", "SpeedIsLeftSide");
static ThemeMetric<bool> SCROLL_IS_LEFT_SIDE ("NoteField", "ScrollIsLeftSide");
static ThemeMetric<bool> FAKE_IS_LEFT_SIDE ("NoteField", "FakeIsLeftSide");
static ThemeMetric<float> BPM_OFFSETX ("NoteField", "BPMOffsetX");
static ThemeMetric<float> STOP_OFFSETX ("NoteField", "StopOffsetX");
static ThemeMetric<float> DELAY_OFFSETX ("NoteField", "DelayOffsetX");
static ThemeMetric<float> WARP_OFFSETX ("NoteField", "WarpOffsetX");
static ThemeMetric<float> TIME_SIG_OFFSETX ("NoteField", "TimeSignatureOffsetX");
static ThemeMetric<float> TICKCOUNT_OFFSETX ("NoteField", "TickcountOffsetX");
static ThemeMetric<float> COMBO_OFFSETX ("NoteField", "ComboOffsetX");
static ThemeMetric<float> LABEL_OFFSETX ("NoteField", "LabelOffsetX");
static ThemeMetric<float> SPEED_OFFSETX ("NoteField", "SpeedOffsetX");
static ThemeMetric<float> SCROLL_OFFSETX ("NoteField", "ScrollOffsetX");
static ThemeMetric<float> FAKE_OFFSETX ("NoteField", "FakeOffsetX");

NoteField::NoteField()
	:m_trans_mod(&m_mod_manager),
	 m_receptor_alpha(&m_mod_manager, 1.0), m_receptor_glow(&m_mod_manager, 0.0),
	 m_explosion_alpha(&m_mod_manager, 1.0), m_explosion_glow(&m_mod_manager, 0.0),
	 m_fov_mod(&m_mod_manager, 45.0),
	 m_vanish_x_mod(&m_mod_manager, 0.0), m_vanish_y_mod(&m_mod_manager, 0.0),
	 m_vanish_type(FVT_RelativeToParent), m_being_drawn_by_player(false),
	 m_draw_beat_bars(false), m_in_edit_mode(false), m_oitg_zoom_mode(false),
	 m_visible_bg_change_layer(BACKGROUND_LAYER_1),
	 m_pn(NUM_PLAYERS), m_in_defective_mode(false),
	 m_own_note_data(false), m_note_data(nullptr), m_timing_data(nullptr),
	 m_steps_type(StepsType_Invalid), m_gameplay_zoom(1.0),
	 defective_render_y(0.0), original_y(0.0)
{
	DeleteChildrenWhenDone(true);
	std::vector<Actor*> layers;
	ActorUtil::MakeActorSet(THEME->GetPathG("NoteField", "layers", true), layers);
	for(auto&& act : layers)
	{
		AddChild(act);
	}
	m_beat_bars.Load(THEME->GetPathG("NoteField","bars"));
	m_field_text.LoadFromFont(THEME->GetPathF("NoteField","MeasureNumber"));
	add_draw_entry(beat_bars_column_index, beat_bars_child_index, beat_bars_draw_order);
	MESSAGEMAN->Subscribe(this, "defective_field");
}

NoteField::~NoteField()
{
	MESSAGEMAN->Unsubscribe(this, "defective_field");
	if(m_own_note_data && m_note_data != nullptr)
	{
		Rage::safe_delete(m_note_data);
	}
}

void NoteField::HandleMessage(Message const& msg)
{
	if(msg.GetName() == "defective_field")
	{
		PlayerNumber pn;
		msg.GetParam("pn", pn);
		if(m_pn == pn)
		{
			bool mode= msg.GetParam("mode", mode);
			set_defective_mode(mode);
		}
	}
	else
	{
		ActorFrame::HandleMessage(msg);
	}
}

void NoteField::AddChild(Actor* act)
{
	// The actors have to be wrapped inside of frames so that mod alpha/glow
	// can be applied without stomping what the layer actor does.
	ActorFrame* frame= new ActorFrame;
	frame->WrapAroundChild(act);
	ActorFrame::AddChild(frame);
	m_layer_render_info.push_back({FLFT_None, FLTT_None});
	add_draw_entry(field_layer_column_index, GetNumChildren()-1, act->GetDrawOrder());
}

void NoteField::RemoveChild(Actor* act)
{
	size_t index= FindChildID(act);
	if(index < m_SubActors.size())
	{
		remove_draw_entry(field_layer_column_index, index);
		m_layer_render_info.erase(m_layer_render_info.begin() + index);
	}
	ActorFrame::RemoveChild(act);
}

void NoteField::ChildChangedDrawOrder(Actor* child)
{
	size_t index= FindChildID(child);
	if(index < m_SubActors.size())
	{
		change_draw_entry(field_layer_column_index, static_cast<int>(index),
			child->GetDrawOrder());
	}
	ActorFrame::ChildChangedDrawOrder(child);
}

void NoteField::UpdateInternal(float delta)
{
	for(auto&& col : m_columns)
	{
		col.Update(delta);
	}
	if(m_in_edit_mode && get_selection_end() != -1.0)
	{
		selection_glow= Rage::scale(Rage::FastCos(
				RageTimer::GetTimeSinceStartFast()*2), -1.f, 1.f, 0.1f, 0.3f);
	}
	ActorFrame::UpdateInternal(delta);
}

bool NoteField::EarlyAbortDraw() const
{
	return m_note_data == nullptr || m_timing_data == nullptr ||
		m_columns.empty() || !m_newskin.loaded_successfully() ||
		ActorFrame::EarlyAbortDraw();
}

void NoteField::PreDraw()
{
	if(m_in_defective_mode)
	{
		original_y= GetY();
		SetY(original_y + defective_render_y);
	}
	ActorFrame::PreDraw();
}

void NoteField::PostDraw()
{
	ActorFrame::PostDraw();
	if(m_in_defective_mode)
	{
		SetY(original_y);
	}
}

void NoteField::draw_board()
{
	m_first_undrawn_entry= 0;
	m_curr_draw_limit= non_board_draw_order;
	Draw();
}

void NoteField::draw_up_to_draw_order(int order)
{
	m_curr_draw_limit= order;
	if(m_first_undrawn_entry < m_draw_entries.size())
	{
		Draw();
	}
}

void NoteField::DrawPrimitives()
{
	if(!m_being_drawn_by_player || m_being_drawn_by_proxy)
	{
		m_first_undrawn_entry= 0;
		m_curr_draw_limit= max_draw_order;
	}
	// Things in columns are drawn in different steps so that the things in a
	// lower step in one column cannot go over the things in a higher step in
	// another column.  For example, things below notes (like receptors) cannot
	// go over notes in another column.  Holds are separated from taps for the
	// same reason.
	curr_z_bias= 1.0;
	while(m_first_undrawn_entry < m_draw_entries.size() &&
		m_draw_entries[m_first_undrawn_entry].draw_order < m_curr_draw_limit)
	{
		draw_entry(m_draw_entries[m_first_undrawn_entry]);
		++m_first_undrawn_entry;
	}
}

void NoteField::position_actor_at_column_head(Actor* act,
	FieldLayerRenderInfo& info, size_t col)
{
	if(col >= m_columns.size())
	{
		return;
	}
	m_columns[col].position_actor_at_column_head(act, info);
}

double NoteField::get_receptor_y()
{
	if(m_columns.empty())
	{
		return 0.0;
	}
	return m_columns[0].apply_reverse_shift(m_columns[0].head_y_offset());
}

void NoteField::push_columns_to_lua(lua_State* L)
{
	lua_createtable(L, m_columns.size(), 0);
	for(size_t c= 0; c < m_columns.size(); ++c)
	{
		m_columns[c].PushSelf(L);
		lua_rawseti(L, -2, c+1);
	}
}

void NoteField::set_player_color(size_t pn, Rage::Color const& color)
{
	if(pn >= m_player_colors.size())
	{
		m_player_colors.resize(pn+1);
	}
	m_player_colors[pn]= color;
}

void NoteField::set_gameplay_zoom(double zoom)
{
	m_gameplay_zoom= zoom;
	for(auto&& col : m_columns)
	{
		col.set_gameplay_zoom(zoom);
	}
}

void NoteField::clear_steps()
{
	if(m_own_note_data)
	{
		m_note_data->ClearAll();
	}
	m_note_data= nullptr;
	m_timing_data= nullptr;
	m_columns.clear();
}

void NoteField::set_skin(std::string const& skin_name, LuaReference& skin_params)
{
	NoteSkinLoader const* loader= NOTESKIN->get_loader_for_skin(skin_name);
	if(loader == nullptr)
	{
		LuaHelpers::ReportScriptErrorFmt("Could not find loader for newskin '%s'.", skin_name.c_str());
		return;
	}
	if(m_note_data != nullptr)
	{
		reload_columns(loader, skin_params);
	}
	else
	{
		m_skin_walker= *loader;
		m_skin_parameters= skin_params;
	}
}

std::string const& NoteField::get_skin()
{
	return m_skin_walker.get_name();
}

void NoteField::set_steps(Steps* data)
{
	if(data == nullptr)
	{
		clear_steps();
		return;
	}
	NoteData* note_data= new NoteData;
	data->GetNoteData(*note_data);
	set_note_data(note_data, data->GetTimingData(), data->m_StepsType);
	m_own_note_data= true;
}

void NoteField::set_note_data(NoteData* note_data, TimingData* timing, StepsType stype)
{
	m_note_data= note_data;
	note_data->SetOccuranceTimeForAllTaps(timing);
	m_own_note_data= false;
	m_timing_data= timing;
	m_defective_mods.set_timing(m_timing_data);
	m_defective_mods.set_num_pads(GAMEMAN->get_num_pads_for_stepstype(stype));
	m_trans_mod.set_timing(m_timing_data);
	m_trans_mod.set_column(0);
	for(auto&& moddable : {&m_receptor_alpha, &m_receptor_glow,
				&m_explosion_alpha, &m_explosion_glow,
				&m_fov_mod, &m_vanish_x_mod, &m_vanish_y_mod})
	{
		moddable->set_timing(m_timing_data);
		moddable->set_column(0);
	}
	if(stype != m_steps_type)
	{
		m_steps_type= stype;
		if(NOTESKIN->skin_supports_stepstype(m_skin_walker.get_name(), stype))
		{
			reload_columns(&m_skin_walker, m_skin_parameters);
		}
	}
	else
	{
		for(size_t i= 0; i < m_columns.size(); ++i)
		{
			m_columns[i].set_note_data(i, m_note_data, m_timing_data);
		}
	}
}

void NoteField::add_draw_entry(int column, int child, int draw_order)
{
	auto insert_pos= m_draw_entries.begin();
	for(; insert_pos != m_draw_entries.end(); ++insert_pos)
	{
		if(insert_pos->draw_order > draw_order)
		{
			break;
		}
	}
	m_draw_entries.insert(insert_pos, {column, child, draw_order});
}

void NoteField::remove_draw_entry(int column, int child)
{
	for(auto entry= m_draw_entries.begin(); entry != m_draw_entries.end(); ++entry)
	{
		if(entry->column == column && entry->child == child)
		{
			m_draw_entries.erase(entry);
			return;
		}
	}
}

void NoteField::change_draw_entry(int column, int child, int new_draw_order)
{
	size_t found_index= 0;
	for(; found_index < m_draw_entries.size(); ++found_index)
	{
		if(m_draw_entries[found_index].column == column &&
			m_draw_entries[found_index].child == child)
		{
			break;
		}
	}
	if(found_index >= m_draw_entries.size())
	{
		return;
	}
	m_draw_entries[found_index].draw_order= new_draw_order;
	if(new_draw_order > m_draw_entries[found_index].draw_order)
	{
		size_t next_index= found_index + 1;
		while(next_index < m_draw_entries.size() &&
			m_draw_entries[next_index].draw_order < new_draw_order)
		{
			std::swap(m_draw_entries[next_index], m_draw_entries[found_index]);
			++found_index;
			++next_index;
		}
		return;
	}
	else
	{
		if(found_index == 0)
		{
			return;
		}
		size_t next_index= found_index - 1;
		while(m_draw_entries[next_index].draw_order > new_draw_order)
		{
			std::swap(m_draw_entries[next_index], m_draw_entries[found_index]);
			if(next_index == 0)
			{
				return;
			}
			--found_index;
			--next_index;
		}
		return;
	}
}

void NoteField::clear_column_draw_entries()
{
	if(m_draw_entries.empty())
	{
		return;
	}
	vector<field_draw_entry> keep;
	for(auto&& entry : m_draw_entries)
	{
		if(entry.column < 0)
		{
			keep.push_back(entry);
		}
	}
	m_draw_entries.swap(keep);
}

void NoteField::draw_entry(field_draw_entry& entry)
{
	switch(entry.column)
	{
		case field_layer_column_index:
			if(entry.child >= 0 && static_cast<size_t>(entry.child) < m_SubActors.size())
			{
				apply_render_info_to_layer(m_SubActors[entry.child],
					m_layer_render_info[entry.child], m_columns[0].get_head_trans(),
					evaluated_receptor_alpha, evaluated_receptor_glow,
					evaluated_explosion_alpha, evaluated_explosion_glow,
					m_curr_beat, m_curr_second, m_receptor_alpha, m_receptor_glow);
				m_SubActors[entry.child]->Draw();
			}
			break;
		case beat_bars_column_index:
			if(m_draw_beat_bars)
			{
				draw_beat_bars_internal();
			}
			break;
		default:
			if(entry.column >= 0 && static_cast<size_t>(entry.column) < m_columns.size())
			{
				m_columns[entry.column].draw_child(entry.child);
			}
			break;
	}
}

void NoteField::draw_field_text(mod_val_inputs& input,
	double x_offset, float side_sign, float horiz_align,
	Rage::Color const& color, Rage::Color const& glow)
{
	Rage::transform trans;
	m_columns[0].calc_transform(input, trans);
	if(!m_in_defective_mode)
	{
		if(m_columns[0].m_add_y_offset_to_position)
		{
			trans.pos.y+= m_columns[0].apply_reverse_shift(input.y_offset);
		}
	}
	else
	{
		trans.pos.x-= m_defective_mods.get_column_x(0);
	}
	trans.pos.x+= ((m_field_width * .5) + x_offset) * side_sign;
	m_field_text.set_transform_pos(trans);
	m_field_text.set_counter_rotation(this);
	m_field_text.SetDiffuse(color);
	m_field_text.SetGlow(glow);
	m_field_text.SetHorizAlign(horiz_align);
	m_field_text.Draw();
}

void NoteField::draw_beat_bar(mod_val_inputs& input, int state, float alpha)
{
	if(m_columns[0].y_offset_visible(input.y_offset) != 0 || alpha <= 0.f)
	{
		return;
	}
	float const bar_width= m_beat_bars.GetUnzoomedWidth();
	Rage::transform trans;
	m_columns[0].calc_transform(input, trans);
	if(!m_in_defective_mode)
	{
		if(m_columns[0].m_add_y_offset_to_position)
		{
			trans.pos.y+= m_columns[0].apply_reverse_shift(input.y_offset);
		}
	}
	else
	{
		trans.pos.x-= m_defective_mods.get_column_x(0);
	}
	m_beat_bars.set_transform_pos(trans);
	m_beat_bars.SetDiffuse(Rage::Color(1,1,1,alpha));
	m_beat_bars.SetState(state);
	m_beat_bars.SetCustomTextureRect(
		Rage::RectF(0, Rage::scale(state + 0.f,0.f,4.f,0.f,1.f),
			m_field_width / bar_width, Rage::scale(state+1.f,0.f,4.f,0.f,1.f)));
	m_beat_bars.SetZoomX(m_field_width/bar_width);
	m_beat_bars.Draw();
}

void NoteField::draw_beat_bars_internal()
{
	if(m_columns.empty())
	{
		return;
	}
	float const beat_step_size= 1.f;
	float const start_beat= max(0., floor(m_curr_beat));
	float first_beat= start_beat;
	float last_beat= start_beat;
	Rage::Color measure_number_color(1,1,1,1);
	Rage::Color measure_number_glow(1,1,1,0);
	bool needs_second= m_in_defective_mode ||
		m_columns[0].m_speed_mod.needs_second() ||
		m_columns[0].m_note_mod.needs_second();
	bool found_begin= false;
	for(float step= 0.f; !found_begin; --step)
	{
		found_begin= draw_beat_bars_step(start_beat, step, measure_number_color, measure_number_glow, needs_second);
		if(!found_begin)
		{
			first_beat= start_beat + (step * beat_step_size);
		}
	}
	bool found_end= false;
	for(float step= 0.f; !found_end; ++step)
	{
		found_end= draw_beat_bars_step(start_beat, step, measure_number_color, measure_number_glow, needs_second);
		if(!found_end)
		{
			last_beat= start_beat + (step * beat_step_size);
		}
	}

	const Rage::Color text_glow= Rage::Color(1,1,1,Rage::FastCos(RageTimer::GetTimeSinceStartFast()*2)/2+0.5f);
	float horiz_align= align_right;
	float side_sign= 1;
	const vector<TimingSegment*>* segs[NUM_TimingSegmentType];
	FOREACH_TimingSegmentType(tst)
	{
		segs[tst] = &(m_timing_data->GetTimingSegments(tst));
	}

#define draw_all_segments(str_exp, name, caps_name) \
	horiz_align= caps_name##_IS_LEFT_SIDE ? align_right : align_left; \
	side_sign= caps_name##_IS_LEFT_SIDE ? -1 : 1; \
	for(size_t seg_index= 0; seg_index < segs[SEGMENT_##caps_name]->size(); ++seg_index) \
	{ \
		const name##Segment* seg= To##name((*segs[SEGMENT_##caps_name])[seg_index]); \
		double const beat= seg->GetBeat(); \
		if(beat < first_beat || beat > last_beat) {continue;} \
		double const second= needs_second ? m_timing_data->GetElapsedTimeFromBeat(beat) : 0.; \
		mod_val_inputs input(beat, second, m_curr_beat, m_curr_second); \
		input.y_offset= m_columns[0].calc_y_offset(input); \
		if(m_columns[0].y_offset_visible(input.y_offset) != 0) {continue;} \
		m_field_text.SetText(str_exp); \
		draw_field_text(input, caps_name##_OFFSETX, side_sign, \
			horiz_align, caps_name##_COLOR, text_glow); \
	}

	draw_all_segments(FloatToString(seg->GetRatio()), Scroll, SCROLL);
	draw_all_segments(FloatToString(seg->GetBPM()), BPM, BPM);
	draw_all_segments(FloatToString(seg->GetPause()), Stop, STOP);
	draw_all_segments(FloatToString(seg->GetPause()), Delay, DELAY);
	draw_all_segments(FloatToString(seg->GetLength()), Warp, WARP);
	draw_all_segments(fmt::sprintf("%d\n--\n%d", seg->GetNum(), seg->GetDen()),
		TimeSignature, TIME_SIG);
	draw_all_segments(fmt::sprintf("%d", seg->GetTicks()), Tickcount, TICKCOUNT);
	draw_all_segments(
		fmt::sprintf("%d/%d", seg->GetCombo(), seg->GetMissCombo()), Combo, COMBO);
	draw_all_segments(seg->GetLabel(), Label, LABEL);
	draw_all_segments(fmt::sprintf("%s\n%s\n%s",
			FloatToString(seg->GetRatio()).c_str(),
			(seg->GetUnit() == 1 ? "S" : "B"),
			FloatToString(seg->GetDelay()).c_str()), Speed, SPEED);
	draw_all_segments(FloatToString(seg->GetLength()), Fake, FAKE);
#undef draw_all_segments

	Song* curr_song= GAMESTATE->m_pCurSong;
	if(curr_song != nullptr)
	{
		BackgroundLayer const fg_layer_number= BACKGROUND_LAYER_Invalid;
		switch(m_visible_bg_change_layer)
		{
			case fg_layer_number:
				draw_bg_change_list(needs_second, first_beat, last_beat, curr_song->GetForegroundChanges(), FGL_COLOR, text_glow);
				break;
			case BACKGROUND_LAYER_1:
				draw_bg_change_list(needs_second, first_beat, last_beat, curr_song->GetBackgroundChanges(m_visible_bg_change_layer), BGL1_COLOR, text_glow);
				break;
			case BACKGROUND_LAYER_2:
				draw_bg_change_list(needs_second, first_beat, last_beat, curr_song->GetBackgroundChanges(m_visible_bg_change_layer), BGL2_COLOR, text_glow);
				break;
			default:
				break;
		}
	}
}

bool NoteField::draw_beat_bars_step(float const start_beat, float const step, Rage::Color const& measure_number_color, Rage::Color const& measure_number_glow, bool needs_second)
{
	std::vector<float> const sub_beats= {.25, .5, .75};
	std::vector<int> const sub_states= {2, 3, 2};
	float const beat_step_size= 1.f;
	static int non_visible_count= 0;
	static int num_this_dir= 0;
	bool cant_draw= false;
	double const beat= start_beat + (step * beat_step_size);
	if(beat < 0.f)
	{
		return true;
	}
	double const second= needs_second ?
		m_timing_data->GetElapsedTimeFromBeat(beat) : 0.;
	mod_val_inputs input(beat, second, m_curr_beat, m_curr_second);
	input.y_offset= m_columns[0].calc_y_offset(input);
	double const main_y_offset= input.y_offset;
	if(step == 0.f)
	{
		non_visible_count= 0;
		num_this_dir= 0;
	}
	++num_this_dir;
	if(num_this_dir > 128)
	{
		cant_draw= true;
	}
	int state= 1;
	int quantized_beat= static_cast<int>(beat);
	if(quantized_beat % 4 == 0)
	{
		state= 0;
		m_field_text.SetText(fmt::sprintf("%d", quantized_beat / 4));
		draw_field_text(input, 0., -1., align_right,
			measure_number_color, measure_number_glow);
	}
	draw_beat_bar(input, state, 1.f);
	if(m_columns[0].y_offset_visible(main_y_offset) != 0)
	{
		++non_visible_count;
		if(non_visible_count > 4)
		{
			cant_draw= true;
		}
	}
	else
	{
		non_visible_count= 0;
	}
	float first_and_third_sub_alpha= 1.f;
	for(size_t sub= 0; sub < sub_beats.size(); ++sub)
	{
		double sub_beat= beat + sub_beats[sub];
		double sub_second= needs_second ?
			m_timing_data->GetElapsedTimeFromBeat(sub_beat) : 0.;
		input.change_eval_time(sub_beat, sub_second);
		input.y_offset= m_columns[0].calc_y_offset(input);
		double offset_diff= fabs(input.y_offset - main_y_offset);
		float alpha= ((offset_diff / note_size) - .5) * 2.;
		switch(sub)
		{
			case 0:
				first_and_third_sub_alpha= alpha;
				break;
			case 2:
				alpha= first_and_third_sub_alpha;
				break;
			default:
				break;
		}
		draw_beat_bar(input, sub_states[sub], alpha);
	}
	return cant_draw;
}

void NoteField::draw_bg_change_list(bool needs_second,
	float const first_beat, float const last_beat,
	vector<BackgroundChange>& changes, Rage::Color const& color,
	Rage::Color const& text_glow)
{
	if(changes.empty())
	{
		return;
	}
	for(auto&& change : changes)
	{
		if(change.m_fStartBeat > last_beat)
		{
			break;
		}
		if(change.m_fStartBeat >= first_beat)
		{
			double const second= needs_second ? m_timing_data->GetElapsedTimeFromBeat(change.m_fStartBeat) : 0.;
			mod_val_inputs input(change.m_fStartBeat, second, m_curr_beat, m_curr_second);
			input.y_offset= m_columns[0].calc_y_offset(input);
			if(m_columns[0].y_offset_visible(input.y_offset) != 0) {continue;}
			m_field_text.SetText(change.GetTextDescription());
			draw_field_text(input, 0, 1, align_left, color, text_glow);
		}
	}
}

double NoteField::update_z_bias()
{
	curr_z_bias+= z_bias_per_thing;
	DISPLAY->SetZBias(curr_z_bias);
	return curr_z_bias;
}

void NoteField::reload_columns(NoteSkinLoader const* new_loader, LuaReference& new_params)
{
	NoteSkinLoader new_skin_walker= *new_loader;
	if(!new_skin_walker.supports_needed_buttons(m_steps_type))
	{
		LuaHelpers::ReportScriptError("The noteskin does not support the required buttons.");
		return;
	}
	// Load the noteskin into a temporary to protect against errors.
	NoteSkinData new_skin;
	string insanity;
	if(!new_skin_walker.load_into_data(m_steps_type, new_params, new_skin, insanity))
	{
		LuaHelpers::ReportScriptError("Error loading noteskin: " + insanity);
		return;
	}
	// Load successful, copy it into members.
	m_skin_walker.swap(new_skin_walker);
	m_newskin.swap(new_skin);
	m_skin_parameters= new_params;

	m_player_colors= m_newskin.m_player_colors;
	m_field_width= 0.0;
	double leftmost= 0.0;
	double rightmost= 0.0;
	double auto_place_width= 0.0;
	for(size_t i= 0; i < m_newskin.num_columns(); ++i)
	{
		double width= m_newskin.get_column(i)->get_width();
		double padding= m_newskin.get_column(i)->get_padding();
		if(m_newskin.get_column(i)->get_use_custom_x())
		{
			double custom_x= m_newskin.get_column(i)->get_custom_x();
			double hwp= (width + padding) * .5;
			leftmost= std::min(leftmost, custom_x - hwp);
			rightmost= std::max(rightmost, custom_x + hwp);
		}
		else
		{
			auto_place_width+= width;
			auto_place_width+= padding;
		}
	}
	double custom_width= rightmost - leftmost;
	m_field_width= std::max(custom_width, auto_place_width);

	clear_column_draw_entries();
	double curr_x= (auto_place_width * -.5);
	std::vector<NoteFieldColumn> old_columns;
	m_columns.swap(old_columns);
	m_columns.resize(m_note_data->GetNumTracks());
	// The column needs all of this info.
	Message pn_msg("PlayerStateSet");
	pn_msg.SetParam("PlayerNumber", m_pn);
	Lua* L= LUA->Get();
	lua_createtable(L, m_newskin.num_columns(), 0);
	vector<float> column_x;
	column_x.reserve(m_columns.size());
	for(size_t i= 0; i < m_columns.size(); ++i)
	{
		NoteSkinColumn* col= m_newskin.get_column(i);
		// curr_x is at the left edge of the column at the beginning of the loop.
		// To put the column in the center, we add half the width of the current
		// column, place the column, then add the other half of the width.  This
		// allows columns to have different widths.
		double col_x= curr_x;
		double width= col->get_width();
		double padding= col->get_padding();
		double wid_pad= width + padding;
		if(col->get_use_custom_x())
		{
			col_x= col->get_custom_x();
		}
		else
		{
			col_x= curr_x + wid_pad * .5;
		}
		lua_createtable(L, 0, 2);
		lua_pushnumber(L, width);
		lua_setfield(L, -2, "width");
		lua_pushnumber(L, padding);
		lua_setfield(L, -2, "padding");
		lua_pushnumber(L, col_x);
		lua_setfield(L, -2, "x");
		lua_rawseti(L, -2, i+1);

		m_columns[i].set_column_info(this, i, col, &m_defective_mods, m_newskin,
			&m_player_colors, m_note_data, m_timing_data, col_x);
		column_x.push_back(col_x);
		if(i < old_columns.size())
		{
			m_columns[i].take_over_mods(old_columns[i]);
		}
		if(!col->get_use_custom_x())
		{
			curr_x+= wid_pad;
		}
		add_draw_entry(static_cast<int>(i), holds_child_index, holds_draw_order);
		add_draw_entry(static_cast<int>(i), lifts_child_index, lifts_draw_order);
		add_draw_entry(static_cast<int>(i), taps_child_index, taps_draw_order);
		add_draw_entry(static_cast<int>(i), selection_child_index, selection_draw_order);
		m_columns[i].HandleMessage(pn_msg);
		if(m_in_defective_mode)
		{
			m_columns[i].set_defective_mode(m_in_defective_mode);
		}
	}
	Message width_msg("WidthSet");
	width_msg.SetParamFromStack(L, "columns");
	width_msg.SetParam("width", get_field_width());
	PushSelf(L);
	width_msg.SetParamFromStack(L, "field");
	LUA->Release(L);
	m_defective_mods.set_column_pos(column_x);
	// Handle the width message after the columns have been created so that the
	// board can fetch the columns.
	HandleMessage(width_msg);
}

void NoteField::set_player_number(PlayerNumber pn)
{
	m_pn= pn;
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", pn);
	HandleMessage(msg);
}

void NoteField::set_player_options(PlayerOptions* options)
{
	m_defective_mods.set_player_options(options);
}

void NoteField::set_defective_mode(bool mode)
{
	if(!m_defective_mods.safe())
	{
		m_in_defective_mode= false;
		return;
	}
	m_in_defective_mode= mode;
	if(m_in_defective_mode)
	{
		SetXY(0.0, m_defective_mods.get_field_y());
		SetZ(0.0);
		SetZoom(1.0);
		SetRotationX(0.0);
		SetRotationY(0.0);
		SetRotationZ(0.0);
		for(auto&& col : m_columns)
		{
			col.set_defective_mode(true);
		}
	}
	else
	{
		for(auto&& col : m_columns)
		{
			col.set_defective_mode(false);
		}
	}
}

bool NoteField::get_defective_mode()
{
	return m_in_defective_mode;
}

void NoteField::disable_defective_mode()
{
	set_defective_mode(false);
	m_defective_mods.set_player_options(nullptr);
}

void NoteField::set_speed(float time_spacing, float max_scroll_bpm,
	float scroll_speed, float scroll_bpm, float read_bpm, float music_rate)
{
	for(auto&& col : m_columns)
	{
		col.set_speed(time_spacing, max_scroll_bpm, scroll_speed,
			scroll_bpm, read_bpm, music_rate);
	}
}

void NoteField::disable_speed_scroll_segments()
{
	for(auto&& col : m_columns)
	{
		col.m_speed_segments_enabled= false;
		col.m_scroll_segments_enabled= false;
	}
}

void NoteField::turn_on_edit_mode()
{
	m_draw_beat_bars= true;
	m_in_edit_mode= true;
}

double NoteField::get_selection_start()
{
	for(auto&& col : m_columns)
	{
		if(col.m_selection_start != -1.0)
		{
			return col.m_selection_start;
		}
	}
	return -1.0;
}

double NoteField::get_selection_end()
{
	for(auto&& col : m_columns)
	{
		if(col.m_selection_end != -1.0)
		{
			return col.m_selection_end;
		}
	}
	return -1.0;
}

void NoteField::set_selection_start(double value)
{
	for(auto&& col : m_columns)
	{
		col.m_selection_start= value;
	}
}

void NoteField::set_selection_end(double value)
{
	for(auto&& col : m_columns)
	{
		col.m_selection_end= value;
	}
}

void NoteField::set_layer_fade_type(Actor* child, FieldLayerFadeType type)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		m_layer_render_info[index].fade_type= type;
	}
}

FieldLayerFadeType NoteField::get_layer_fade_type(Actor* child)
{
	size_t index= FindIDBySubChild(child);
	if(index < m_layer_render_info.size())
	{
		return m_layer_render_info[index].fade_type;
	}
	return FieldLayerFadeType_Invalid;
}

void NoteField::update_displayed_time(double beat, double second)
{
	m_curr_beat= beat;
	m_curr_second= second;
	m_mod_manager.update(beat, second);
	if(m_in_defective_mode)
	{
		m_defective_mods.update(m_curr_beat, m_curr_second);
	}
	for(auto&& col : m_columns)
	{
		col.update_displayed_time(beat, second);
	}
	// Evaluate mods only when the time changes to minimize the number of
	// times mods are evaluated. -Kyz
	if(!m_in_defective_mode)
	{
		mod_val_inputs input(m_curr_beat, m_curr_second);
		Rage::transform trans;
		m_trans_mod.evaluate(input, trans);
		trans.zoom.x*= m_gameplay_zoom;
		trans.zoom.y*= m_gameplay_zoom;
		trans.zoom.z*= m_gameplay_zoom;
		set_transform(trans);
		SetFOV(m_fov_mod.evaluate(input));
		double vanish_x= m_vanish_x_mod.evaluate(input);
		double vanish_y= m_vanish_y_mod.evaluate(input);
		Actor* parent= GetParent();
		switch(m_vanish_type)
		{
			case FVT_RelativeToParent:
				vanish_x+= parent->GetX();
				vanish_y+= parent->GetY();
			case FVT_RelativeToSelf:
				vanish_x+= GetX();
				vanish_y+= GetY();
				break;
			default:
				break;
		}
		SetVanishPoint(vanish_x, vanish_y);
		evaluated_receptor_alpha= m_receptor_alpha.evaluate(input);
		evaluated_receptor_glow= m_receptor_glow.evaluate(input);
		evaluated_explosion_alpha= m_explosion_alpha.evaluate(input);
		evaluated_explosion_glow= m_explosion_glow.evaluate(input);
	}
	else
	{
		SetFOV(45.0);
		double vanish_x= Rage::scale(m_defective_mods.get_skew(), 0.1f, 1.0f, GetParent()->GetX(), SCREEN_CENTER_X);
		double vanish_y= SCREEN_CENTER_Y;
		SetVanishPoint(vanish_x, vanish_y);
		float reverse_mult= m_defective_mods.get_reverse_scale(0) < 0.0f ? -1.0f : 1.0f;
		float tilt= m_defective_mods.get_tilt();
		float tilt_degrees= Rage::scale(tilt, -1.0f, 1.0f, 30.0f, -30.0f) * reverse_mult;
		float zoom= 1.f - (m_defective_mods.get_mini() * .5f);
		if(tilt > 0.0f)
		{
			zoom*= Rage::scale(tilt, 0.0f, 1.0f, 1.0f, .9f);
			defective_render_y= Rage::scale(tilt, 0.0f, 1.0f, 0.0f, -45.0f) * reverse_mult;
		}
		else
		{
			zoom*= Rage::scale(tilt, 0.0f, -1.0f, 1.0f, .9f);
			defective_render_y= Rage::scale(tilt, 0.0f, -1.0f, 0.0f, -20.0f) * reverse_mult;
		}
		if(m_oitg_zoom_mode)
		{
			SetZoomX(zoom);
			SetZoomY(zoom);
		}
		else
		{
			SetZoom(zoom);
		}
		SetRotationX(tilt_degrees);
	}
}

double NoteField::get_beat_from_second(double second)
{
	return m_timing_data->GetBeatFromElapsedTime(second);
}

double NoteField::get_second_from_beat(double beat)
{
	return m_timing_data->GetElapsedTimeFromBeat(beat);
}

void NoteField::set_displayed_beat(double beat)
{
	update_displayed_time(beat, m_timing_data->GetElapsedTimeFromBeat(beat));
}

void NoteField::set_displayed_second(double second)
{
	update_displayed_time(m_timing_data->GetBeatFromElapsedTime(second), second);
}

void NoteField::did_tap_note(size_t column, TapNoteScore tns, bool bright)
{
	if(column >= m_columns.size()) { return; }
	m_columns[column].did_tap_note(tns, bright);
}

void NoteField::did_hold_note(size_t column, HoldNoteScore hns, bool bright)
{
	if(column >= m_columns.size()) { return; }
	m_columns[column].did_hold_note(hns, bright);
}

void NoteField::set_pressed(size_t column, bool on)
{
	if(column >= m_columns.size()) { return; }
	m_columns[column].set_pressed(on);
}


// lua start
#define GET_MEMBER(member) \
static int get_##member(T* p, lua_State* L) \
{ \
	p->m_##member.PushSelf(L); \
	return 1; \
}
#define GET_TRANS_DIM(trans, part, dim) \
static int get_##trans##_##part##_##dim(T* p, lua_State* L) \
{ \
	p->m_##trans##_mod.part##_mod.dim##_mod.PushSelf(L); \
	return 1; \
}
#define GET_VEC_DIM(vec, dim) \
static int get_##vec##_##dim(T* p, lua_State* L) \
{ \
	p->m_##vec##_mod.dim##_mod.PushSelf(L); \
	return 1; \
}
#define GET_VEC(vec) \
GET_VEC_DIM(vec, x); \
GET_VEC_DIM(vec, y); \
GET_VEC_DIM(vec, z);
#define GET_TRANS_PART(trans, part) \
GET_TRANS_DIM(trans, part, x); \
GET_TRANS_DIM(trans, part, y); \
GET_TRANS_DIM(trans, part, z);
#define GET_TRANS(trans) \
GET_TRANS_PART(trans, pos); \
GET_TRANS_PART(trans, rot); \
GET_TRANS_PART(trans, zoom);
#define ADD_VEC(vec) \
ADD_METHOD(get_##vec##_x); \
ADD_METHOD(get_##vec##_y); \
ADD_METHOD(get_##vec##_z);
#define ADD_TRANS_PART(trans, part) \
ADD_METHOD(get_##trans##_##part##_##x); \
ADD_METHOD(get_##trans##_##part##_##y); \
ADD_METHOD(get_##trans##_##part##_##z);
#define ADD_TRANS(trans) \
ADD_TRANS_PART(trans, pos); \
ADD_TRANS_PART(trans, rot); \
ADD_TRANS_PART(trans, zoom);

#define SAFE_TIMING_CHECK(p, L, func_name) \
if(!p->timing_is_safe()) \
{ luaL_error(L, "Timing data is not set, " #func_name " is not safe."); }

struct LunaNoteFieldColumn : Luna<NoteFieldColumn>
{
	GET_MEMBER(time_offset);
	GET_MEMBER(quantization_multiplier);
	GET_MEMBER(quantization_offset);
	GET_MEMBER(speed_mod);
	GET_MEMBER(lift_pretrail_length);
	GET_MEMBER(reverse_offset_pixels);
	GET_MEMBER(reverse_scale);
	GET_MEMBER(center_percent);
	GET_TRANS(note);
	GET_TRANS(column);
	GET_VEC(hold_normal);
	GET_MEMBER(note_alpha);
	GET_MEMBER(note_glow);
	GET_MEMBER(receptor_alpha);
	GET_MEMBER(receptor_glow);
	GET_MEMBER(explosion_alpha);
	GET_MEMBER(explosion_glow);
	GET_SET_BOOL_METHOD(use_game_music_beat, m_use_game_music_beat);
	GET_SET_BOOL_METHOD(show_unjudgable_notes, m_show_unjudgable_notes);
	GET_SET_BOOL_METHOD(speed_segments_enabled, m_speed_segments_enabled);
	GET_SET_BOOL_METHOD(scroll_segments_enabled, m_scroll_segments_enabled);
	GET_SET_BOOL_METHOD(add_y_offset_to_position, m_add_y_offset_to_position);
	GET_SET_BOOL_METHOD(holds_skewed_by_mods, m_holds_skewed_by_mods);
	GET_SET_BOOL_METHOD(twirl_holds, m_twirl_holds);
	GET_SET_BOOL_METHOD(use_moddable_hold_normal, m_use_moddable_hold_normal);
	GETTER_SETTER_ENUM_METHOD(NotePlayerizeMode, playerize_mode);
	static int get_curr_beat(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_curr_beat());
		return 1;
	}
	static int set_curr_beat(T* p, lua_State* L)
	{
		SAFE_TIMING_CHECK(p, L, set_curr_beat);
		p->set_displayed_beat(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int get_curr_second(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_curr_second());
		return 1;
	}
	static int set_curr_second(T* p, lua_State* L)
	{
		SAFE_TIMING_CHECK(p, L, set_curr_second);
		p->set_displayed_second(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int set_pixels_visible_before(T* p, lua_State* L)
	{
		p->set_pixels_visible_before(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int set_pixels_visible_after(T* p, lua_State* L)
	{
		p->set_pixels_visible_after(FArg(1));
		COMMON_RETURN_SELF;
	}
	GETTER_SETTER_FLOAT_METHOD(upcoming_time);
	static int receptor_y_offset(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->head_y_offset());
		return 1;
	}
	static int get_reverse_shift(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_reverse_shift());
		return 1;
	}
	static int apply_column_mods_to_actor(T* p, lua_State* L)
	{
		Actor* act= Luna<Actor>::check(L, 1);
		p->apply_column_mods_to_actor(act);
		COMMON_RETURN_SELF;
	}
	static int apply_note_mods_to_actor(T* p, lua_State* L)
	{
		Actor* act= Luna<Actor>::check(L, 1);
		bool time_is_offset= lua_toboolean(L, 2) != 0;
		double beat= p->get_curr_beat();
		double second= p->get_curr_beat();
		double y_offset= 0;
		static const int bindex= 3;
		static const int sindex= 4;
		if(lua_isnumber(L, bindex))
		{
			beat= lua_tonumber(L, bindex);
			if(time_is_offset)
			{
				beat+= p->get_curr_beat();
			}
			if(!lua_isnumber(L, sindex))
			{
				SAFE_TIMING_CHECK(p, L, apply_note_mods_to_actor);
				second= p->get_second_from_beat(beat);
			}
		}
		if(lua_isnumber(L, sindex))
		{
			second= lua_tonumber(L, sindex);
			if(time_is_offset)
			{
				second+= p->get_curr_second();
			}
			if(!lua_isnumber(L, bindex))
			{
				SAFE_TIMING_CHECK(p, L, apply_note_mods_to_actor);
				beat= p->get_beat_from_second(second);
			}
		}
		if(lua_isnumber(L, 5))
		{
			y_offset= lua_tonumber(L, 5);
		}
		else
		{
			y_offset= p->calc_y_offset(beat, second);
		}
		bool use_alpha= lua_toboolean(L, 6) != 0;
		bool use_glow= lua_toboolean(L, 7) != 0;
		p->apply_note_mods_to_actor(act, beat, second, y_offset, use_alpha, use_glow);
		COMMON_RETURN_SELF;
	}
	static int get_layer_fade_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		Enum::Push(L, p->get_layer_fade_type(layer));
		return 1;
	}
	static int set_layer_fade_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		FieldLayerFadeType type= Enum::Check<FieldLayerFadeType>(L, 2);
		p->set_layer_fade_type(layer, type);
		COMMON_RETURN_SELF;
	}
	static int get_layer_transform_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		Enum::Push(L, p->get_layer_transform_type(layer));
		return 1;
	}
	static int set_layer_transform_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		FieldLayerTransformType type= Enum::Check<FieldLayerTransformType>(L, 2);
		p->set_layer_transform_type(layer, type);
		COMMON_RETURN_SELF;
	}
	LunaNoteFieldColumn()
	{
		ADD_METHOD(get_time_offset);
		ADD_METHOD(get_quantization_multiplier);
		ADD_METHOD(get_quantization_offset);
		ADD_METHOD(get_speed_mod);
		ADD_METHOD(get_reverse_offset_pixels);
		ADD_METHOD(get_reverse_scale);
		ADD_METHOD(get_center_percent);
		ADD_TRANS(note);
		ADD_TRANS(column);
		ADD_VEC(hold_normal);
		ADD_METHOD(get_note_alpha);
		ADD_METHOD(get_note_glow);
		ADD_METHOD(get_receptor_alpha);
		ADD_METHOD(get_receptor_glow);
		ADD_METHOD(get_explosion_alpha);
		ADD_METHOD(get_explosion_glow);
		ADD_GET_SET_METHODS(use_game_music_beat);
		ADD_GET_SET_METHODS(show_unjudgable_notes);
		ADD_GET_SET_METHODS(speed_segments_enabled);
		ADD_GET_SET_METHODS(scroll_segments_enabled);
		ADD_GET_SET_METHODS(add_y_offset_to_position);
		ADD_GET_SET_METHODS(holds_skewed_by_mods);
		ADD_GET_SET_METHODS(twirl_holds);
		ADD_GET_SET_METHODS(use_moddable_hold_normal);
		ADD_GET_SET_METHODS(playerize_mode);
		ADD_GET_SET_METHODS(curr_beat);
		ADD_GET_SET_METHODS(curr_second);
		ADD_METHOD(set_pixels_visible_before);
		ADD_METHOD(set_pixels_visible_after);
		ADD_GET_SET_METHODS(upcoming_time);
		ADD_METHOD(receptor_y_offset);
		ADD_METHOD(get_reverse_shift);
		ADD_METHOD(apply_column_mods_to_actor);
		ADD_METHOD(apply_note_mods_to_actor);
		ADD_METHOD(get_layer_fade_type);
		ADD_METHOD(get_layer_transform_type);
		ADD_METHOD(set_layer_fade_type);
		ADD_METHOD(set_layer_transform_type);
	}
};
LUA_REGISTER_DERIVED_CLASS(NoteFieldColumn, ActorFrame);

struct LunaNoteField : Luna<NoteField>
{
	static int set_skin(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		LuaReference skin_params;
		if(lua_type(L, 2) == LUA_TTABLE)
		{
			lua_pushvalue(L, 2);
			skin_params.SetFromStack(L);
		}
		p->set_skin(skin_name, skin_params);
		COMMON_RETURN_SELF;
	}
	static int get_skin(T* p, lua_State* L)
	{
		lua_pushstring(L, p->get_skin().c_str());
		return 1;
	}
	static int set_steps(T* p, lua_State* L)
	{
		// nullptr can be passed to tell the field to clear the steps.
		Steps* data= nullptr;
		if(!lua_isnil(L, 1))
		{
			data= Luna<Steps>::check(L, 1);
		}
		p->set_steps(data);
		COMMON_RETURN_SELF;
	}
	static int get_columns(T* p, lua_State* L)
	{
		p->push_columns_to_lua(L);
		return 1;
	}
	static int get_width(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_field_width());
		return 1;
	}
	static int get_curr_beat(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_curr_beat());
		return 1;
	}
	static int set_curr_beat(T* p, lua_State* L)
	{
		SAFE_TIMING_CHECK(p, L, set_curr_beat);
		p->set_displayed_beat(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int get_curr_second(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_curr_second());
		return 1;
	}
	static int set_curr_second(T* p, lua_State* L)
	{
		SAFE_TIMING_CHECK(p, L, set_curr_second);
		p->set_displayed_second(FArg(1));
		COMMON_RETURN_SELF;
	}
	GET_TRANS(trans);
	GET_MEMBER(receptor_alpha);
	GET_MEMBER(receptor_glow);
	GET_MEMBER(explosion_alpha);
	GET_MEMBER(explosion_glow);
	GET_MEMBER(fov_mod);
	GET_MEMBER(vanish_x_mod);
	GET_MEMBER(vanish_y_mod);
	GET_SET_ENUM_METHOD(vanish_type, FieldVanishType, m_vanish_type);
	static int set_player_color(T* p, lua_State* L)
	{
		int pn= IArg(1);
		if(pn < 1 || pn > 32)
		{
			luaL_error(L, "Invalid player number.");
		}
		Rage::Color temp;
		FromStack(temp, L, 2);
		p->set_player_color(pn, temp);
		COMMON_RETURN_SELF;
	}
	static int get_layer_fade_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		Enum::Push(L, p->get_layer_fade_type(layer));
		return 1;
	}
	static int set_layer_fade_type(T* p, lua_State* L)
	{
		Actor* layer= Luna<Actor>::check(L, 1);
		FieldLayerFadeType type= Enum::Check<FieldLayerFadeType>(L, 2);
		p->set_layer_fade_type(layer, type);
		COMMON_RETURN_SELF;
	}
	GETTER_SETTER_BOOL_METHOD(defective_mode);
	GET_SET_BOOL_METHOD(oitg_zoom_mode, m_oitg_zoom_mode);
	LunaNoteField()
	{
		ADD_GET_SET_METHODS(skin);
		ADD_METHOD(set_steps);
		ADD_METHOD(get_columns);
		ADD_METHOD(get_width);
		ADD_GET_SET_METHODS(curr_beat);
		ADD_GET_SET_METHODS(curr_second);
		ADD_TRANS(trans);
		ADD_METHOD(get_receptor_alpha);
		ADD_METHOD(get_receptor_glow);
		ADD_METHOD(get_explosion_alpha);
		ADD_METHOD(get_explosion_glow);
		ADD_METHOD(get_fov_mod);
		ADD_METHOD(get_vanish_x_mod);
		ADD_METHOD(get_vanish_y_mod);
		ADD_GET_SET_METHODS(vanish_type);
		ADD_METHOD(set_player_color);
		ADD_METHOD(get_layer_fade_type);
		ADD_METHOD(set_layer_fade_type);
		ADD_GET_SET_METHODS(defective_mode);
		ADD_GET_SET_METHODS(oitg_zoom_mode);
	}
};
LUA_REGISTER_DERIVED_CLASS(NoteField, ActorFrame);
