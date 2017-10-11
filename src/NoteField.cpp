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
static const int draw_order_types= 4;
static const int non_board_draw_order= 0;
static const int beat_bars_draw_order= 0;
static const int selection_draw_order= 0;
static const int holds_draw_order= 200;
static const int lifts_draw_order= 200;
static const int taps_draw_order= 300;
// Field layers have the id of the noteskin they were loaded from, so they
// can be unloaded when the skin is removed.
static const size_t theme_noteskin_id= 1000;

static const std::string no_skin_name= "";

static ThemeMetric<Rage::Color> AREA_HIGHLIGHT_COLOR("NoteField", "AreaHighlightColor");

static const char* FieldLayerFadeTypeNames[]= {
	"Receptor",
	"Note",
	"Explosion",
	"None",
	"Upcoming",
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

FieldChild::FieldChild(Actor* act, FieldLayerFadeType ftype,
	FieldLayerTransformType ttype, size_t from_noteskin)
	:m_child(act), m_fade_type(ftype), m_transform_type(ttype),
	 m_from_noteskin(from_noteskin)
{
	WrapAroundChild(act);
	act->PlayCommand("On");
}

void FieldChild::apply_render_info(Rage::transform const& trans,
	double receptor_alpha, double receptor_glow,
	double explosion_alpha, double explosion_glow,
	double beat, double second,
	ModifiableValue& note_alpha, ModifiableValue& note_glow)
{
	switch(m_fade_type)
	{
		case FLFT_Receptor:
			SetDiffuseAlpha(receptor_alpha);
			SetGlowAlpha(receptor_glow);
			break;
		case FLFT_Explosion:
			SetDiffuseAlpha(explosion_alpha);
			SetGlowAlpha(explosion_glow);
			break;
		case FLFT_Note:
			{
				mod_val_inputs input(beat, second);
				double alpha= note_alpha.evaluate(input);
				double glow= note_glow.evaluate(input);
				SetDiffuseAlpha(alpha);
				SetGlowAlpha(glow);
			}
			break;
		default:
			break;
	}
	switch(m_transform_type)
	{
		case FLTT_Full:
			set_transform(trans);
			break;
		case FLTT_PosOnly:
			set_transform_pos(trans);
			break;
		default:
			break;
	}
}

bool operator<(field_draw_entry const& rhs, field_draw_entry const& lhs)
{
	return rhs.draw_order < lhs.draw_order;
}

NoteFieldColumn::NoteFieldColumn()
	:m_show_unjudgable_notes(true),
	 m_speed_segments_enabled(true), m_scroll_segments_enabled(true),
	 m_holds_skewed_by_mods(true),
	 m_twirl_holds(true), m_use_moddable_hold_normal(false),
	 m_time_offset(m_mod_manager, "time_offset", 0.0),
	 m_quantization_multiplier(m_mod_manager, "quantization_multiplier", 1.0),
	 m_quantization_offset(m_mod_manager, "quantization_offset", 0.0),
	 m_speed_mod(m_mod_manager, "speed", 0.0),
	 m_lift_pretrail_length(m_mod_manager, "lift_pretrail", 0.25),
	 m_num_upcoming(m_mod_manager, "num_upcoming", 0.0),
	 m_note_skin_id(m_mod_manager, "note_skin_id", 0.0),
	 m_layer_skin_id(m_mod_manager, "layer_skin_id", 0.0),
	 m_y_offset_vec_mod(m_mod_manager, "y_offset_vec", 0.0, 1.0, 0.0),
	 m_reverse_offset_pixels(m_mod_manager, "reverse_offset", 240.0 - note_size),
	 m_reverse_scale(m_mod_manager, "reverse", 1.0),
	 m_center_percent(m_mod_manager, "center", 0.0),
	 m_note_mod(m_mod_manager, "note"), m_column_mod(m_mod_manager, "column"),
	 m_hold_normal_mod(m_mod_manager, "hold_normal", 0.0),
	 m_note_alpha(m_mod_manager, "note_alpha", 1.0),
	 m_note_glow(m_mod_manager, "note_glow", 0.0),
	 m_receptor_alpha(m_mod_manager, "receptor_alpha", 1.0),
	 m_receptor_glow(m_mod_manager, "receptor_glow", 0.0),
	 m_explosion_alpha(m_mod_manager, "explosion_alpha", 1.0),
	 m_explosion_glow(m_mod_manager, "explosion_glow", 0.0),
	 m_selection_start(-1.0), m_selection_end(-1.0),
	 m_curr_beat(0.0f), m_curr_second(0.0), m_prev_curr_second(-1000.0),
	 m_pixels_visible_before_beat(128.0f),
	 m_pixels_visible_after_beat(1024.0f),
	 m_upcoming_time(2.0),
	 m_playerize_mode(NPM_Off),
	 m_player_colors(nullptr), m_field(nullptr),
	 m_pn(NUM_PLAYERS), m_defective_mods(nullptr), m_in_defective_mode(false),
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

void NoteFieldColumn::HandleMessage(Message const& msg)
{
	ActorFrame::HandleMessage(msg);
	for(auto&& lay : m_layers)
	{
		lay.m_child->HandleMessage(msg);
	}
}

void NoteFieldColumn::AddChild(Actor* act)
{
	AddChildInternal(act, theme_noteskin_id);
}

void NoteFieldColumn::AddChildInternal(Actor* act, size_t from_noteskin)
{
	// The actors have to be wrapped inside of frames so that modifiers
	// can be applied without stomping what the layer actor does.
	m_layers.emplace_back(act, FLFT_None, FLTT_Full, from_noteskin);
	FieldChild* new_child= &m_layers.back();
	m_field->add_draw_entry({new_child, static_cast<int>(m_column), act->GetDrawOrder(), fdem_layer});
	if(!m_noteskins.empty())
	{
		act->HandleMessage(create_width_message());
	}
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", m_pn);
	act->HandleMessage(msg);
	new_child->SetParent(this);
}

void NoteFieldColumn::RemoveChild(Actor* act)
{
	for(auto iter= m_layers.begin(); iter != m_layers.end(); ++iter)
	{
		if(iter->m_child == act)
		{
			m_field->remove_draw_entry(static_cast<int>(m_column), &*iter);
			m_layers.erase(iter);
			return;
		}
	}
}

void NoteFieldColumn::ChildChangedDrawOrder(Actor* child)
{
	for(auto iter= m_layers.begin(); iter != m_layers.end(); ++iter)
	{
		if(iter->m_child == child)
		{
			m_field->change_draw_entry(static_cast<int>(m_column), &*iter,
				child->GetDrawOrder());
		}
	}
}

Message NoteFieldColumn::create_width_message()
{
	Message width_msg("WidthSet");
	lua_State* L= LUA->Get();
	PushSelf(L);
	width_msg.SetParamFromStack(L, "column");
	LUA->Release(L);
	width_msg.SetParam("column_id", get_mod_col());
	width_msg.SetParam("width", m_noteskins[0]->get_width());
	width_msg.SetParam("padding", m_noteskins[0]->get_padding());
	return width_msg;
}

void NoteFieldColumn::set_parent_info(NoteField* field, size_t column,
		ArrowDefects* defects)
{
	m_field= field;
	m_column= column;
	m_defective_mods= defects;
	m_use_game_music_beat= true;
	m_mod_manager.column= column;
	std::vector<Actor*> layers;
	ActorUtil::MakeActorSet(THEME->GetPathG("NoteColumn", "layers", true), layers);
	for(auto&& act : layers)
	{
		AddChild(act);
	}
}

void NoteFieldColumn::set_note_data(const NoteData* note_data,
	const TimingData* timing_data)
{
	m_note_data= note_data;
	m_timing_data= timing_data;
	note_row_closest_to_current_time= -1;
	m_mod_manager.set_timing(timing_data);
	for(auto&& moddable : {&m_time_offset, &m_quantization_multiplier,
				&m_quantization_offset, &m_speed_mod, &m_lift_pretrail_length,
				&m_num_upcoming, &m_note_skin_id, &m_layer_skin_id,
				&m_reverse_offset_pixels, &m_reverse_scale,
				&m_center_percent, &m_note_alpha, &m_note_glow, &m_receptor_alpha,
				&m_receptor_glow, &m_explosion_alpha, &m_explosion_glow})
	{
		moddable->set_column(m_column);
	}
	for(auto&& moddable : {&m_note_mod, &m_column_mod})
	{
		moddable->set_column(m_column);
	}
	for(auto&& moddable : {&m_y_offset_vec_mod, &m_hold_normal_mod})
	{
		moddable->set_column(m_column);
	}
}

void NoteFieldColumn::replace_render_note_skin_entries(
	NoteSkinColumn* being_removed, NoteSkinColumn* replacement)
{
	for(auto&& render_list : {&render_holds, &render_lifts, &render_taps})
	{
		for(auto&& note : *render_list)
		{
			if(note.skin == being_removed)
			{
				note.skin= replacement;
			}
		}
	}
}

void NoteFieldColumn::add_layers_from_skin(NoteSkinData& data, size_t id)
{
	for(auto&& layer : data.m_layers)
	{
		AddChildInternal(layer.m_actors[m_column], id);
	}
}

void NoteFieldColumn::remove_layers_from_skin(size_t id, bool shift_others)
{
	if(!m_layers.empty())
	{
		auto iter= m_layers.begin();
		while(iter != m_layers.end())
		{
			if(iter->m_from_noteskin == id)
			{
				m_field->remove_draw_entry(static_cast<int>(m_column), &*iter);
				iter= m_layers.erase(iter);
			}
			else
			{
				if(iter->m_from_noteskin != theme_noteskin_id && shift_others && iter->m_from_noteskin > id)
				{
					--(iter->m_from_noteskin);
				}
				++iter;
			}
		}
	}
}

void NoteFieldColumn::apply_base_skin(std::vector<Rage::Color>* player_colors, double x)
{
	Rage::transform tmp= {{static_cast<float>(x), 0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 1.f, 0.f};
	m_column_mod.set_base_value(tmp);
	m_player_colors= player_colors;

	// Send width to already existing children.
	Message width_msg= create_width_message();
	pass_message_to_heads(width_msg);
}

void NoteFieldColumn::set_skin(NoteSkinData& data)
{
	NoteSkinColumn* being_removed= m_noteskins[0];
	remove_layers_from_skin(0, false);
	m_noteskins[0]= data.get_column(m_column);
	add_layers_from_skin(data, 0);
	NoteSkinColumn* replacement= m_noteskins[0];
	replace_render_note_skin_entries(being_removed, replacement);
}

void NoteFieldColumn::add_skin(NoteSkinData& data)
{
	size_t id= m_noteskins.size();
	m_noteskins.push_back(data.get_column(m_column));
	add_layers_from_skin(data, id);
}

void NoteFieldColumn::remove_skin(size_t id)
{
	NoteSkinColumn* being_removed= m_noteskins[id];
	remove_layers_from_skin(id, true);
	m_noteskins.erase(m_noteskins.begin() + id);
	NoteSkinColumn* replacement= m_noteskins[0];
	replace_render_note_skin_entries(being_removed, replacement);
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
	if(!m_noteskins.empty())
	{
		float upde= std::max(0.f, float(m_timing_source.second_delta));
		for(auto&& skin : m_noteskins)
		{
			skin->update_taps(upde);
		}
	}
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
	Rage::transform& trans, bool do_rot, bool do_y_zoom)
{
	if(!m_in_defective_mode)
	{
		m_note_mod.hold_render_eval(input, trans, do_rot, do_y_zoom);
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
	if(!m_in_defective_mode)
	{
		first_y_offset_visible= -m_pixels_visible_before_beat / visible_scale;
		last_y_offset_visible= m_pixels_visible_after_beat / visible_scale;
	}
	else
	{
		first_y_offset_visible= -(m_pixels_visible_before_beat * m_defective_mods->get_drawsizeback()) / visible_scale;
		last_y_offset_visible= (m_pixels_visible_after_beat * m_defective_mods->get_drawsize()) / visible_scale;
	}
}

double NoteFieldColumn::apply_reverse_shift(double y_offset)
{
	return (y_offset * reverse_scale) + reverse_shift;
}

void NoteFieldColumn::apply_yoffset_to_pos(mod_val_inputs& input,
	Rage::Vector3& pos)
{
	Rage::Vector3 yoff;
	m_y_offset_vec_mod.evaluate(input, yoff);
	for(int i= 0; i < 3; ++i)
	{
		pos[i]+= static_cast<float>(
			(reverse_scale * (yoff[i] * input.y_offset)) +
			(reverse_shift * yoff[i]));
	}
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
	apply_yoffset_to_pos(mod_input, trans.pos);
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
	for(auto&& layer : m_layers)
	{
		layer.Update(delta);
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
	bool need_glow_pass;
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
		need_glow_pass= false;
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
		need_glow_pass= false;
	}
	void internal_draw(std::vector<RageTexture*>& textures)
	{
		for(size_t t= 0; t < textures.size(); ++t)
		{
			DISPLAY->SetTexture(TextureUnit_1, textures[t]->GetTexHandle());
			DISPLAY->SetBlendMode(t == 0 ? BLEND_NORMAL : BLEND_ADD);
			DISPLAY->DrawQuadStrip(buf, v-buf);
		}
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
	void draw(std::vector<RageTexture*>& textures)
	{
		DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Modulate);
		DISPLAY->SetCullMode(CULL_NONE);
		DISPLAY->SetTextureWrapping(TextureUnit_1, false);
		internal_draw(textures);
		if(need_glow_pass)
		{
			swap_glow();
			DISPLAY->SetTextureMode(TextureUnit_1, TextureMode_Glow);
			internal_draw(textures);
		}
	}
	int used() const { return v - buf; }
	int avail() const { return size - used(); }
	void internal_add_vert(Rage::Vector3 const& pos, Rage::Color const& color, Rage::Color const& glow, Rage::Vector2 const& texcoord)
	{
		v->p= pos;  v->c= color;  v->t= texcoord;
		v+= 1;
		(*glow_v)= glow;
		glow_v+= 1;
	}
	void add_verts(float const tex_y, Rage::Vector3 const& left,
		Rage::Vector3 const& right, Rage::Color const& color,
		Rage::Color const& glow, float const tex_left, float const tex_right,
		std::vector<RageTexture*>& textures)
	{
		internal_add_vert(left, color, glow, Rage::Vector2(tex_left, tex_y));
		internal_add_vert(right, color, glow, Rage::Vector2(tex_right, tex_y));
		if(glow.a > .01)
		{
			need_glow_pass= true;
		}
		if(avail() < 4)
		{
			draw(textures);
			// Intentionally swap the glow back after calling rollback so that
			// only the set of verts that will remain are swapped.
			rollback();
			if(need_glow_pass)
			{
				swap_glow();
			}
			need_glow_pass= false;
		}
	}
};

struct hold_time_lerper
{
	double start_y_off;
	double y_off_len_recip;
	double start_beat;
	double beat_len;
	double start_second;
	double second_len;
	hold_time_lerper(double sy, double yl, double sb, double bl, double ss,
		double sl)
		:start_y_off(sy), y_off_len_recip(1.0/yl), start_beat(sb), beat_len(bl),
		 start_second(ss), second_len(sl)
	{}
	void lerp(double y_off, double& beat, double& second)
	{
		double y_dist= (y_off - start_y_off) * y_off_len_recip;
		beat= (y_dist * beat_len) + start_beat;
		second= (y_dist * second_len) + start_second;
	}
};

struct hold_vert_step_state
{
	double start_y;
	double y;
	double beat;
	double second;
	Rage::transform trans;
	NoteFieldColumn& col;
	hold_time_lerper& time_lerp;
	NoteFieldColumn::render_note& renderable;
	bool is_lift;
	bool uninitialized;
	hold_vert_step_state(NoteFieldColumn& c, hold_time_lerper& tl,
		NoteFieldColumn::render_note& note, double sy, bool lift)
		:start_y(sy), col(c), time_lerp(tl), renderable(note), is_lift(lift),
		 uninitialized(true)
	{}
	hold_vert_step_state& operator=(hold_vert_step_state const& rhs)
	{
		y= rhs.y;
		beat= rhs.beat;
		second= rhs.second;
		trans= rhs.trans;
		return *this;
	}

	void calc(double curr_y, bool do_y_zoom= false)
	{
		uninitialized= false;
		y= curr_y;
		time_lerp.lerp(y, beat, second);
		renderable.input.change_eval_time(beat, second);
		renderable.input.y_offset= y;
		col.hold_render_transform(renderable.input, trans, col.m_twirl_holds, do_y_zoom);
		col.apply_yoffset_to_pos(renderable.input, trans.pos);
		if(beat <= col.m_selection_end && beat >= col.m_selection_start)
		{
			trans.glow= col.get_selection_glow();
		}
		if(is_lift)
		{
			double along= (y - start_y) * lift_fade_dist_recip;
			if(along < 1.0)
			{
				trans.alpha*= along;
				trans.glow*= along;
			}
		}
	}
};

void NoteFieldColumn::calc_forward_and_left_for_hold(
	Rage::transform& curr_trans, Rage::transform& next_trans,
	Rage::Vector3& forward, Rage::Vector3& left,
	NoteFieldColumn::render_note& note)
{
	// pos_z_vec will be used later to orient the hold.  Read below. -Kyz
	static const Rage::Vector3 pos_z_vec(0.0f, 0.0f, 1.0f);
	static const Rage::Vector3 neg_y_vec(0.0f, -1.0f, 0.0f);
	forward.x= next_trans.pos.x - curr_trans.pos.x;
	forward.y= next_trans.pos.y - curr_trans.pos.y;
	forward.z= next_trans.pos.z - curr_trans.pos.z;
	forward= forward.GetNormalized();
	if(m_holds_skewed_by_mods)
	{
		if(forward.y > 0.f)
		{
			forward.x= 0.f;
			forward.y= 1.f;
			forward.z= 0.f;
		}
		else
		{
			forward.x= 0.f;
			forward.y= -1.f;
			forward.z= 0.f;
		}
	}
	if(m_use_moddable_hold_normal)
	{
		Rage::Vector3 normal;
		m_hold_normal_mod.evaluate(note.input, normal);
		left= Rage::CrossProduct(normal, forward);
	}
	else
	{
		if(std::abs(forward.z) > 0.9f) // 0.9 arbitrariliy picked.
		{
			left= Rage::CrossProduct(neg_y_vec, forward);
		}
		else
		{
			left= Rage::CrossProduct(pos_z_vec, forward);
		}
	}
	if(m_twirl_holds && curr_trans.rot.y != 0.0)
	{
		RageAARotate(&left, &forward, -curr_trans.rot.y);
	}
	left*= (.5 * note.skin->get_width()) * curr_trans.zoom.x;
}

static void calc_left_and_right_verts(
	Rage::Vector3& render_left, Rage::Vector3& curr_pos,
	Rage::Vector3& left_vert, Rage::Vector3& right_vert)
{
	left_vert.x= curr_pos.x + render_left.x;
	left_vert.y= curr_pos.y + render_left.y;
	left_vert.z= curr_pos.z + render_left.z;
	right_vert.x= curr_pos.x - render_left.x;
	right_vert.y= curr_pos.y - render_left.y;
	right_vert.z= curr_pos.z - render_left.z;
}

void NoteFieldColumn::draw_hold(QuantizedHoldRenderData& data,
	render_note& note, double head_beat, double head_second,
	double tail_beat, double tail_second, bool is_lift)
{
	double const original_beat= note.input.eval_beat;
	double const original_second= note.input.eval_second;
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
	double num_tex_pixels= data.part_lengths.topcap_pixels +
			data.part_lengths.body_pixels + data.part_lengths.bottomcap_pixels;
	if(data.part_lengths.needs_jumpback)
	{
		num_tex_pixels+= data.part_lengths.body_pixels;
	}
	double tex_per_y= (tex_bottom - tex_top) / num_tex_pixels;
	double head_y_offset= note.y_offset;
	double tail_y_offset= note.tail_y_offset;
	if(tail_y_offset < head_y_offset)
	{
		// The speed mod is negative.
		std::swap(head_y_offset, tail_y_offset);
	}
	double y_off_len= tail_y_offset - head_y_offset;
	hold_time_lerper time_lerper(head_y_offset, y_off_len, head_beat, tail_beat - head_beat, head_second, tail_second - head_second);
	float const color_scale= Rage::scale(note.note_iter->second.HoldResult.fLife, 0.f, 1.f, note.skin->get_hold_gray_percent(), 1.f);
	Rage::Color color(color_scale, color_scale, color_scale, 1.f);
	Rage::Color glow_color(1.f, 1.f, 1.f, 0.f);
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
	// These Vector3s will be reused by each phase.
	Rage::Vector3 forward, left, left_vert, right_vert;
	double start_y= max(head_y_offset, first_y_offset_visible);
	// next_step exists so that the forward vector of a hold can be calculated
	// and used to make the hold turn and maintain constant width, instead of
	// being skewed.  Toggle the holds_skewed_by_mods flag with lua to see the
	// difference.
	hold_vert_step_state curr_step(*this, time_lerper, note, start_y, is_lift);
	// The OS of the future.
	hold_vert_step_state next_step(*this, time_lerper, note, start_y, is_lift);
	if(head_y_offset > first_y_offset_visible &&
		data.part_lengths.pixels_before_note > 0.)
	{
		curr_step.calc(start_y, true);
		next_step.calc(start_y + y_step);
		// Draw the cap before the note.
		calc_forward_and_left_for_hold(curr_step.trans, next_step.trans,
			forward, left, note);
		// Reverse the direction of forward because it's positioning the verts
		// before the head.
		// Scale it by the y zoom so it matches the note.
		// Don't clip to first_y_offset_visible because the note isn't clipped to
		// that either.
		forward*= -1.f * data.part_lengths.pixels_before_note *
			curr_step.trans.zoom.y;
		color.a= curr_step.trans.alpha;
		glow_color.a= curr_step.trans.glow;
		{
			// Does not use calc_left_and_right_verts because forward needs
			// to be added in.
			Rage::Vector3 lv(
				forward.x + left.x + curr_step.trans.pos.x,
				forward.y + left.y + curr_step.trans.pos.y,
				forward.z + left.z + curr_step.trans.pos.z);
			Rage::Vector3 rv(
				forward.x - left.x + curr_step.trans.pos.x,
				forward.y - left.y + curr_step.trans.pos.y,
				forward.z - left.z + curr_step.trans.pos.z);
			verts_to_draw.add_verts(tex_top, lv, rv, color, glow_color, tex_left,
				tex_right, data.parts);
		}
		{
			double head_tex_y= tex_top + (tex_per_y * data.part_lengths.pixels_before_note);
			calc_left_and_right_verts(left,
				curr_step.trans.pos, left_vert, right_vert);
			verts_to_draw.add_verts(head_tex_y, left_vert, right_vert, color,
				glow_color, tex_left, tex_right, data.parts);
		}
	}
	double topcap_end_y= head_y_offset + (data.part_lengths.topcap_pixels -
		data.part_lengths.pixels_before_note);
	double body_end_y= tail_y_offset - (data.part_lengths.bottomcap_pixels -
		data.part_lengths.pixels_after_note);
	if(topcap_end_y > body_end_y)
	{
		topcap_end_y= body_end_y;
	}
#define SINGLE_STEP(calc_tex_y, next_y) \
		next_step.calc(next_y); \
		calc_forward_and_left_for_hold(curr_step.trans, next_step.trans, \
			forward, left, note); \
		calc_left_and_right_verts(left, curr_step.trans.pos, left_vert, \
			right_vert); \
		color.a= curr_step.trans.alpha; \
		glow_color.a= curr_step.trans.glow; \
		calc_tex_y; \
		verts_to_draw.add_verts(tex_y, left_vert, right_vert, color, \
			glow_color, tex_left, tex_right, data.parts); \
		curr_step= next_step;
#define STEPPING_LOOP(limit, calc_tex_y) \
	for(double curr_y= start_y; curr_y < limit && \
				curr_y <= last_y_offset_visible; curr_y+= y_step) \
	{ \
		SINGLE_STEP(calc_tex_y, curr_y + y_step); \
	} \
	start_y= limit;
	// Each phase will advance start_y to the start point of the next phase.

	if(start_y < body_end_y)
	{
		if(start_y < topcap_end_y)
		{
			curr_step.calc(start_y);
			// Topcap section after the note.
			double head_tex_y= tex_top + (tex_per_y * data.part_lengths.pixels_before_note);
			STEPPING_LOOP(topcap_end_y, float tex_y= (tex_per_y * (curr_y - start_y)) + head_tex_y);
		}
		// There might not be a body between the topcap and the bottomcap if
		// the topcap after the note and the bottomcap before the note are longer
		// than the hold height.
		if(start_y < body_end_y)
		{
			// Repeating body section.
			// The body is aligned to have a seam at the top (because the length is
			//   not multiple of body_pixels), and not have a seam at the bottom.
			// Thus, texture coords are calculated by distance from the bottom.
			// The first step has to be shorter than the others because
			// body_end_y - topcap_end_y is not a multiple of y_step.
			double body_len= body_end_y - topcap_end_y;
			// body_mid_tex_y is between the two body sections.
			// body_end_tex_y is where the bottomcap starts.
			double body_mid_tex_y= tex_bottom;
			double body_end_tex_y= tex_bottom;
			if(data.part_lengths.needs_jumpback)
			{
				body_mid_tex_y-= tex_per_y * (data.part_lengths.bottomcap_pixels +
					data.part_lengths.body_pixels);
				body_end_tex_y-= tex_per_y * data.part_lengths.bottomcap_pixels;
			}
			else
			{
				body_mid_tex_y-= tex_per_y * data.part_lengths.bottomcap_pixels;
				body_end_tex_y= body_mid_tex_y;
			}
			double first_step= fmod(body_end_y - topcap_end_y, y_step);
			float prev_tex_y= 0.f;
			if(curr_step.uninitialized)
			{
				curr_step.calc(start_y);
			}
			if(first_step > 0.001)
			{
				start_y+= first_step;
				SINGLE_STEP(float tex_y= body_mid_tex_y - (fmod(body_len, data.part_lengths.body_pixels) * tex_per_y), start_y);
				prev_tex_y= tex_y;
			}
			// Cover the jump back to beginning the first body section with
			// verts in the identical second body section.
#define BODY_CALC_TEX_Y \
			float tex_offset= (fmod(body_end_y - curr_y, data.part_lengths.body_pixels) * tex_per_y); \
			float tex_y= body_mid_tex_y - tex_offset; \
			if(data.part_lengths.needs_jumpback && tex_y < prev_tex_y) \
			{ \
				verts_to_draw.add_verts(body_end_tex_y - tex_offset, left_vert, \
					right_vert, color, glow_color, tex_left, tex_right, data.parts); \
			} \
			prev_tex_y= tex_y;

			STEPPING_LOOP(body_end_y, BODY_CALC_TEX_Y);
#undef BODY_CALC_TEX_Y
			if(data.part_lengths.needs_jumpback)
			{
				// The bottomcap starts from the end of the second body section, but
				// the last verts were from the end of the first body section.
				verts_to_draw.add_verts(body_end_tex_y - (tex_per_y * y_step), left_vert, right_vert, color,
					glow_color, tex_left, tex_right, data.parts);
			}
		}
	}
	if(body_end_y < tail_y_offset && start_y < tail_y_offset)
	{
		// Bottomcap before the note.
		if(curr_step.uninitialized)
		{
			curr_step.calc(start_y);
		}
		// body_end_tex_y is where the bottomcap starts.
		double body_end_tex_y= tex_bottom -
			(tex_per_y * data.part_lengths.bottomcap_pixels);
		STEPPING_LOOP(tail_y_offset, float tex_y= (tex_per_y * (curr_y - start_y)) + body_end_tex_y);
	}
	if(start_y < last_y_offset_visible &&
		tail_y_offset < last_y_offset_visible &&
		data.part_lengths.pixels_after_note > 0.)
	{
		// Bottomcap after the note.
		// Even if curr_step was last evaluated for start_y, the y zoom is needed
		// now.
		curr_step.calc(start_y, true);
		next_step.calc(start_y + y_step);
		calc_forward_and_left_for_hold(curr_step.trans, next_step.trans,
			forward, left, note);
		forward*= data.part_lengths.pixels_after_note *
			curr_step.trans.zoom.y;
		color.a= curr_step.trans.alpha;
		glow_color.a= curr_step.trans.glow;
		{
			double tail_tex_y= tex_bottom - (tex_per_y * data.part_lengths.pixels_after_note);
			calc_left_and_right_verts(left,
				curr_step.trans.pos, left_vert, right_vert);
			verts_to_draw.add_verts(tail_tex_y, left_vert, right_vert, color,
				glow_color, tex_left, tex_right, data.parts);
		}
		{
			// Does not use calc_left_and_right_verts because forward needs
			// to be added in.
			Rage::Vector3 lv(
				forward.x + left.x + curr_step.trans.pos.x,
				forward.y + left.y + curr_step.trans.pos.y,
				forward.z + left.z + curr_step.trans.pos.z);
			Rage::Vector3 rv(
				forward.x - left.x + curr_step.trans.pos.x,
				forward.y - left.y + curr_step.trans.pos.y,
				forward.z - left.z + curr_step.trans.pos.z);
			verts_to_draw.add_verts(tex_bottom, lv, rv, color, glow_color, tex_left,
				tex_right, data.parts);
		}
	}
#undef STEPPING_LOOP
#undef SINGLE_STEP
	if(verts_to_draw.used() > 0)
	{
		verts_to_draw.draw(data.parts);
	}
	// Return the original eval time because it was changed during hold
	// rendering.
	note.input.change_eval_time(original_beat, original_second);
}

bool NoteFieldColumn::EarlyAbortDraw() const
{
	return m_noteskins.empty() || m_note_data == nullptr || m_timing_data == nullptr;
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
	input.y_offset= y_offset;
}

void NoteFieldColumn::add_renderable_to_lists(render_note& renderable)
{
	int tap_row= renderable.note_iter->first;
	double tap_beat= NoteRowToBeat(tap_row);
	const TapNote& tn= renderable.note_iter->second;
	if(m_noteskins.size() == 1)
	{
		renderable.skin= m_noteskins[0];
	}
	else
	{
		size_t id= size_t(std::round(m_note_skin_id.evaluate(renderable.input)))
			% m_noteskins.size();
		renderable.skin= m_noteskins[id];
	}
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
				// The upcoming note code in draw_thing_internal depends on the
				// notes closest to the receptor being at the end of the list.
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
	// Clearing and rebuilding the list of taps to render every frame is
	// unavoidable because the notes move every frame, which changes the y
	// offset, which changes what is visible.  So even if the list wasn't
	// cleared, it would still have to be traversed to recalculate the y
	// offsets every frame.
	render_holds.clear();
	render_lifts.clear();
	render_taps.clear();
	if(m_noteskins.empty())
	{
		return;
	}
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
		apply_yoffset_to_pos(input, head_transform.pos);
		receptor_alpha= m_receptor_alpha.evaluate(input);
		receptor_glow= m_receptor_glow.evaluate(input);
		explosion_alpha= m_explosion_alpha.evaluate(input);
		explosion_glow= m_explosion_glow.evaluate(input);
		int temp_nr= std::round(m_num_upcoming.evaluate(input));
		if(temp_nr <= 0)
		{
			num_upcoming= 0;
			use_column_num_upcoming= false;
		}
		else
		{
			num_upcoming= static_cast<size_t>(temp_nr);
			use_column_num_upcoming= true;
		}
		layer_skin_id= size_t(std::round(m_layer_skin_id.evaluate(input)))
			% m_noteskins.size();
	}
	else
	{
		double head_yoff= head_y_offset();
		m_defective_mods->get_transform(m_curr_beat, head_yoff,
			apply_reverse_shift(head_yoff), m_column, head_transform);
		receptor_alpha= m_defective_mods->get_receptor_alpha(m_column);
		receptor_glow= 0.0;
		explosion_alpha= 1.0;
		explosion_glow= 0.0;
		num_upcoming= 0;
		layer_skin_id= 0;
	}

	m_status.upcoming_beat_dist= 1000.0;
	m_status.upcoming_second_dist= 1000.0;
	m_status.prev_active_hold= m_status.active_hold;
	m_status.active_hold= nullptr;
	m_status.found_upcoming= false;
	if(m_noteskins[0]->get_anim_uses_beats())
	{
		m_status.anim_percent= m_curr_beat;
	}
	else
	{
		m_status.anim_percent= m_curr_second;
	}
	m_status.anim_percent= fmod(m_status.anim_percent * m_noteskins[0]->get_anim_mult(), 1.0);
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

void NoteFieldColumn::draw_thing(field_draw_entry* entry)
{
#define RETURN_IF(empty_check) if(empty_check) { return; }
	switch(entry->meaning)
	{
		case fdem_holds:
			RETURN_IF(render_holds.empty());
			break;
		case fdem_lifts:
			RETURN_IF(render_lifts.empty());
			break;
		case fdem_taps:
			RETURN_IF(render_taps.empty());
			break;
		case fdem_selection:
			RETURN_IF(m_field == nullptr || !m_field->m_in_edit_mode);
			break;
		case fdem_layer:
			{
				FieldChild* child= static_cast<FieldChild*>(entry->child);
				RETURN_IF((child->m_from_noteskin != theme_noteskin_id &&
						child->m_from_noteskin != layer_skin_id) ||
					(child->m_fade_type == FLFT_Upcoming && (
						num_upcoming == 0 || render_taps.empty())));
			}
			break;
		default:
			break;
	}
#undef RETURN_IF
	curr_draw_entry= entry;
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
		double const quantization= quantization_for_time(holdit.input, holdit.skin);
		bool active= HOLD_COUNTS_AS_ACTIVE(tn);
		QuantizedHoldRenderData data;
		holdit.skin->get_hold_render_data(tn.subType, m_playerize_mode, tn.pn,
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
		double const quantization= quantization_for_time(liftit.input, liftit.skin);
		QuantizedHoldRenderData data;
		liftit.skin->get_hold_render_data(TapNoteSubType_Hold, m_playerize_mode,
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
	get_norm_actor_fun get_normal,
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
			double const quantization= col.quantization_for_time(note.input,
				note.skin);
			draw_info.act= (note.skin->*get_normal)(part, quantization,
				anim_percent, active, reverse);
		}
		else
		{
			draw_info.act= (note.skin->*get_playerized)(part, pn, anim_percent,
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
		double head_beat= tap_beat;
		double tail_beat;
		double head_second= tap_second;
		bool active= false;
		switch(tn.type)
		{
			case TapNoteType_Mine:
				part= NSTP_Mine;
				break;
			case TapNoteType_Lift:
				part= NSTP_Lift;
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
				if(tapit.skin->get_use_hold_head())
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
				break;
		}
		vector<tap_draw_info> acts(2);
		if(part != NoteSkinTapPart_Invalid)
		{
			set_tap_actor_info(acts[0], *this,
				&NoteSkinColumn::get_tap_actor, &NoteSkinColumn::get_player_tap, part,
				tn.pn, head_beat, head_second, tapit.y_offset,
				m_status.anim_percent, active, m_status.in_reverse, tapit);
		}
		else
		{
			// Handle the case where it's an obscenity instead of an actual hold.
			if(tail_part == NoteSkinTapOptionalPart_Invalid)
			{
				set_tap_actor_info(acts[0], *this,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, head_part, tn.pn,
					head_beat, head_second, tapit.y_offset,
					m_status.anim_percent, active, m_status.in_reverse, tapit);
			}
			else
			{
				// Put tails on the list first because they need to be under the heads.
				set_tap_actor_info(acts[0], *this,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, tail_part, tn.pn,
					tail_beat, tn.end_second, tapit.tail_y_offset,
					m_status.anim_percent, active, m_status.in_reverse, tapit);
				set_tap_actor_info(acts[1], *this,
					&NoteSkinColumn::get_optional_actor,
					&NoteSkinColumn::get_player_optional_tap, head_part, tn.pn,
					head_beat, head_second, tapit.y_offset,
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
					apply_yoffset_to_pos(tapit.input, trans.pos);
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
		apply_yoffset_to_pos(start_input, start_pos);
		m_area_highlight.set_pos(start_pos);
		m_area_highlight.SetWidth(m_noteskins[0]->get_width());
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
		apply_yoffset_to_pos(start_input, start_pos);
		apply_yoffset_to_pos(end_input, end_pos);
		double height= end_pos.y - start_pos.y;
		start_pos.x= (start_pos.x + end_pos.x) * .5;
		start_pos.y= (start_pos.y + end_pos.y) * .5;
		start_pos.z= (start_pos.z + end_pos.z) * .5;
		m_area_highlight.set_pos(start_pos);
		m_area_highlight.SetWidth(m_noteskins[0]->get_width());
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
	for(auto&& layer : m_layers)
	{
		layer.HandleMessage(msg);
	}
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

void NoteFieldColumn::add_upcoming_notes(
	std::vector<std::pair<double, size_t>>& upcoming_notes, size_t num_upcoming)
{
	if(use_column_num_upcoming) { return; }
	// Upcoming notes, shared by the whole field.
	if(!render_taps.empty())
	{
		for(auto tapit= render_taps.rbegin(); tapit != render_taps.rend(); ++tapit)
		{
			double usd= tapit->input.eval_second - m_curr_second;
			if(usd < 0.0) { continue; }
			// Linear search from the end to find the insertion point, because
			// inserting into a vector shifts all elements anyway.
			std::pair<double, size_t> entry= {usd, m_column};
			if(upcoming_notes.empty())
			{
				// upcoming_notes.size is 0.
				upcoming_notes.push_back(entry);
			}
			else
			{
				// upcoming_notes.size is at least 1 at this point.
				bool added= false;
				for(size_t upid= 0; upid < upcoming_notes.size(); ++upid)
				{
					if(upcoming_notes[upid].first > entry.first)
					{
						added= true;
						auto insert_at= upcoming_notes.begin() + upid;
						upcoming_notes.insert(insert_at, entry);
						break;
					}
				}
				if(!added)
				{
					if(upcoming_notes.size() < num_upcoming)
					{
						upcoming_notes.push_back(entry);
					}
					else
					{
						return;
					}
				}
			}
		}
	}
}

void NoteFieldColumn::set_num_upcoming(size_t count)
{
	num_upcoming= count;
}


void NoteFieldColumn::DrawPrimitives()
{
	if(!m_being_drawn_by_proxy)
	{
		draw_thing_internal();
	}
	else
	{
		size_t after_layers= m_layers.size();
		std::vector<field_draw_entry> draw_entries(after_layers + 3);
		size_t eid= 0;
		for(auto&& lay : m_layers)
		{
			draw_entries[eid].meaning= fdem_layer;
			draw_entries[eid].child= &lay;
			draw_entries[eid].draw_order= lay.m_child->GetDrawOrder();
			++eid;
		}
		draw_entries[after_layers+0].meaning= fdem_holds;
		draw_entries[after_layers+0].draw_order= holds_draw_order;
		draw_entries[after_layers+1].meaning= fdem_lifts;
		draw_entries[after_layers+1].draw_order= lifts_draw_order;
		draw_entries[after_layers+2].meaning= fdem_taps;
		draw_entries[after_layers+2].draw_order= taps_draw_order;

		std::sort(draw_entries.begin(), draw_entries.end());
		for(auto&& entry : draw_entries)
		{
			curr_draw_entry= &entry;
			draw_thing_internal();
		}
	}
}

void NoteFieldColumn::draw_thing_internal()
{
	switch(curr_draw_entry->meaning)
	{
		case fdem_holds:
			draw_holds_internal();
			break;
		case fdem_lifts:
			draw_lifts_internal();
			break;
		case fdem_taps:
			draw_taps_internal();
			break;
		case fdem_selection:
			draw_selection_internal();
			break;
		case fdem_layer:
			{
				FieldChild* child= static_cast<FieldChild*>(curr_draw_entry->child);
				if(child->m_fade_type != FLFT_Upcoming)
				{
					child->apply_render_info(
						head_transform, receptor_alpha, receptor_glow,
						explosion_alpha, explosion_glow,
						m_curr_beat, m_curr_second, m_note_alpha, m_note_glow);
					curr_draw_entry->child->Draw();
				}
				else
				{
					// Upcoming notes.
					size_t nid= 0;
					for(auto tapit= render_taps.rbegin(); nid < num_upcoming && tapit != render_taps.rend(); ++tapit)
					{
						double usd= tapit->input.eval_second - m_curr_second;
						if(usd < 0.0) { continue; }
						Rage::transform trans;
						// y_offset will be set back to original after this render.
						tapit->input.y_offset= 0.0;
						calc_transform(tapit->input, trans);
						apply_yoffset_to_pos(tapit->input, trans.pos);
						double ubd= tapit->input.eval_beat - m_curr_beat;
						Message msg("Upcoming");
						msg.SetParam("beat_distance", ubd);
						msg.SetParam("second_distance", usd);
						child->m_child->HandleMessage(msg);
						child->apply_render_info(
							trans, receptor_alpha, receptor_glow,
							explosion_alpha, explosion_glow,
							m_curr_beat, m_curr_second, m_note_alpha, m_note_glow);
						curr_draw_entry->child->Draw();
						// y_offset must be set back.
						tapit->input.y_offset= tapit->y_offset;
						++nid;
					}
				}
			}
			break;
		default:
			break;
	}
}

void NoteFieldColumn::set_playerize_mode(NotePlayerizeMode mode)
{
	if(mode == NPM_Mask)
	{
		if(!m_noteskins[0]->supports_masking())
		{
			mode= NPM_Quanta;
		}
	}
	m_playerize_mode= mode;
}

void NoteFieldColumn::set_layer_fade_type(Actor* child, FieldLayerFadeType type)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			entry.m_fade_type= type;
		}
	}
}

FieldLayerFadeType NoteFieldColumn::get_layer_fade_type(Actor* child)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			return entry.m_fade_type;
		}
	}
	return FieldLayerFadeType_Invalid;
}

void NoteFieldColumn::set_layer_transform_type(Actor* child, FieldLayerTransformType type)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			entry.m_transform_type= type;
		}
	}
}

FieldLayerTransformType NoteFieldColumn::get_layer_transform_type(Actor* child)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			return entry.m_transform_type;
		}
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
	:m_trans_mod(m_mod_manager, "transform"),
	 m_receptor_alpha(m_mod_manager, "receptor_alpha", 1.0),
	 m_receptor_glow(m_mod_manager, "receptor_glow", 0.0),
	 m_explosion_alpha(m_mod_manager, "explosion_alpha", 1.0),
	 m_explosion_glow(m_mod_manager, "explosion_glow", 0.0),
	 m_fov_mod(m_mod_manager, "fov", 0.0, 0.0, 45.0),
	 m_num_upcoming(m_mod_manager, "num_upcoming", 0.0),
	 m_layer_skin_id(m_mod_manager, "layer_skin_id", 0.0),
	 m_vanish_type(FVT_RelativeToParent), m_being_drawn_by_player(false),
	 m_in_edit_mode(false), m_oitg_zoom_mode(false),
	 m_visible_bg_change_layer(BACKGROUND_LAYER_1),
	 m_curr_beat(0.0), m_curr_second(0.0), m_field_width(0.0),
	 m_share_steps_parent(nullptr),
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
	MESSAGEMAN->Subscribe(this, "defective_field");
}

NoteField::~NoteField()
{
	if(!m_share_steps_children.empty())
	{
		for(auto&& child : m_share_steps_children)
		{
			child->share_steps_parent_being_destroyed();
		}
	}
	MESSAGEMAN->Unsubscribe(this, "defective_field");
	if(m_own_note_data)
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
			bool mode;
			msg.GetParam("mode", mode);
			set_defective_mode(mode);
		}
	}
	else
	{
		ActorFrame::HandleMessage(msg);
		for(auto&& lay : m_layers)
		{
			lay.m_child->HandleMessage(msg);
		}
	}
}

void NoteField::AddChild(Actor* act)
{
	AddChildInternal(act, theme_noteskin_id);
}

void NoteField::AddChildInternal(Actor* act, size_t from_noteskin)
{
	// The actors have to be wrapped inside of frames so that modifiers
	// can be applied without stomping what the layer actor does.
	m_layers.emplace_back(act, FLFT_None, FLTT_None, from_noteskin);
	FieldChild* new_child= &m_layers.back();
	add_draw_entry({new_child, field_layer_column_index, act->GetDrawOrder(), fdem_layer});
	if(!m_noteskins.empty())
	{
		act->HandleMessage(create_width_message());
	}
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", m_pn);
	act->HandleMessage(msg);
	new_child->SetParent(this);
}

void NoteField::RemoveChild(Actor* act)
{
	for(auto iter= m_layers.begin(); iter != m_layers.end(); ++iter)
	{
		if(iter->m_child == act)
		{
			remove_draw_entry(field_layer_column_index, &*iter);
			m_layers.erase(iter);
		}
	}
}

void NoteField::ChildChangedDrawOrder(Actor* child)
{
	for(auto iter= m_layers.begin(); iter != m_layers.end(); ++iter)
	{
		if(iter->m_child == child)
		{
			change_draw_entry(field_layer_column_index, &*iter,
				child->GetDrawOrder());
		}
	}
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
	for(auto&& layer : m_layers)
	{
		layer.Update(delta);
	}
	ActorFrame::UpdateInternal(delta);
}

bool NoteField::EarlyAbortDraw() const
{
	return m_note_data == nullptr || m_timing_data == nullptr ||
		m_columns.empty() || m_noteskins.empty() ||
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

double NoteField::get_receptor_y()
{
	if(m_columns.empty())
	{
		return 0.0;
	}
	return m_columns[0].apply_reverse_shift(m_columns[0].head_y_offset());
}

bool NoteField::is_in_reverse()
{
	if(m_columns.empty())
	{
		return false;
	}
	return m_columns[0].m_status.in_reverse;
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
		Rage::safe_delete(m_note_data);
	}
	m_note_data= nullptr;
	if(m_timing_data != nullptr)
	{
		m_timing_data->ReleaseLookup();
	}
	m_timing_data= nullptr;
	m_columns.clear();
	if(!m_share_steps_children.empty())
	{
		for(auto&& child : m_share_steps_children)
		{
			child->clear_steps();
		}
	}
	m_steps_type= StepsType_Invalid;
}

bool NoteField::fill_skin_entry(field_skin_entry* entry, std::string const& name,
	LuaReference& params)
{
	entry->loader= NOTESKIN->get_loader_for_skin(name);
	if(entry->loader == nullptr)
	{
		return false;
	}
	entry->name= name;
	entry->params= params;
	return true;
}

enum FSED
{
	FSED_success,
	FSED_stepstype_mismatch,
	FSED_loader_failed,
	FSED_column_count_mismatch
};

int NoteField::fill_skin_entry_data(field_skin_entry* entry)
{
	if(m_steps_type == StepsType_Invalid)
	{
		return FSED_stepstype_mismatch;
	}
	if(!entry->loader->supports_needed_buttons(m_steps_type))
	{
		return FSED_stepstype_mismatch;
	}
	string insanity;
	if(!entry->loader->load_into_data(m_steps_type, entry->params, entry->data, insanity))
	{
		LuaHelpers::ReportScriptError("Error loading noteskin: " + insanity);
		entry->data.clear();
		return FSED_loader_failed;
	}
	if(entry->data.num_columns() != static_cast<size_t>(m_note_data->GetNumTracks()))
	{
		LuaHelpers::ReportScriptErrorFmt("Error loading noteskin %s: Noteskin returned %zu columns, note data has %i columns", entry->loader->get_name().c_str(), entry->data.num_columns(), m_note_data->GetNumTracks());
		entry->data.clear();
		return FSED_column_count_mismatch;
	}
	return FSED_success;
}

field_skin_entry* NoteField::set_add_skin_common(std::string const& name,
	LuaReference& params, bool replace_base)
{
	field_skin_entry* temp= new field_skin_entry;
	if(!fill_skin_entry(temp, name, params))
	{
		// No loader, don't add it to m_unapplied_noteskins.
		delete temp;
		return nullptr;
	}
	temp->replaces_first_skin= replace_base;
	switch(fill_skin_entry_data(temp))
	{
		case FSED_stepstype_mismatch:
			m_unapplied_noteskins.push_back(temp);
			return nullptr;
			break;
		case FSED_loader_failed:
			delete temp;
			return nullptr;
			break;
		case FSED_column_count_mismatch:
			m_unapplied_noteskins.push_back(temp);
			return nullptr;
			break;
		default:
			break;
	}
	return temp;
}

void NoteField::set_base_skin(std::string const& name, LuaReference& params, int uid)
{
	field_skin_entry temp;
	if(!fill_skin_entry(&temp, name, params))
	{
		return;
	}
	m_base_noteskin= temp;
	m_base_noteskin.uid= uid;
}

void NoteField::set_skin(std::string const& name, LuaReference& params, int uid)
{
	field_skin_entry* temp= set_add_skin_common(name, params, true);
	if(temp == nullptr)
	{
		return;
	}
	temp->uid= uid;
	if(m_base_noteskin.name.empty())
	{
		// m_base_noteskin should not contain filled data because the stepstype
		// might be different when clear_to_base_skin applies it.  Also, the data
		// wouldn't be used anyway.
		m_base_noteskin.loader= temp->loader;
		m_base_noteskin.name= temp->name;
		m_base_noteskin.params= temp->params;
		m_base_noteskin.uid= temp->uid;
	}
	if(m_noteskins.empty())
	{
		m_noteskins.push_back(temp);
		add_layers_from_skin(m_noteskins[0]->data, 0);
		add_skin_to_columns(m_noteskins[0]->data);
	}
	else
	{
		for(auto&& col : m_columns)
		{
			col.set_skin(temp->data);
		}
		remove_layers_from_skin(0, false);
		delete m_noteskins[0];
		m_noteskins[0]= temp;
		add_layers_from_skin(m_noteskins[0]->data, 0);
	}
	apply_base_skin_to_columns();
}

std::string const& NoteField::get_skin()
{
	if(m_noteskins.empty())
	{
		return no_skin_name;
	}
	return m_noteskins[0]->name;
}

void NoteField::add_skin(std::string const& name, LuaReference& params, int uid)
{
	ASSERT_M(m_noteskins.size() + m_unapplied_noteskins.size() < theme_noteskin_id, "I want a 1000-word essay on why there are so many noteskins loaded.");
	field_skin_entry* temp= set_add_skin_common(name, params, false);
	if(temp == nullptr)
	{
		return;
	}
	temp->uid= uid;
	m_noteskins.push_back(temp);
	add_layers_from_skin(m_noteskins.back()->data, m_noteskins.size()-1);
	add_skin_to_columns(m_noteskins.back()->data);
	if(m_noteskins.size() == 1)
	{
		apply_base_skin_to_columns();
	}
}

void NoteField::apply_base_skin_to_columns()
{
	NoteSkinData& base_data= m_noteskins[0]->data;
	m_player_colors= base_data.m_player_colors;
	m_field_width= 0.0;
	double leftmost= 0.0;
	double rightmost= 0.0;
	double auto_place_width= 0.0;
	size_t max_column= size_t(m_note_data->GetNumTracks());
	for(size_t i= 0; i < max_column; ++i)
	{
		NoteSkinColumn* column= base_data.get_column(i);
		double width= column->get_width();
		double padding= column->get_padding();
		if(column->get_use_custom_x())
		{
			double custom_x= column->get_custom_x();
			double hwp= (width + padding) * .5;
			leftmost= std::min(leftmost, custom_x - hwp);
			rightmost= std::max(rightmost, custom_x + hwp);
		}
		else
		{
			auto_place_width+= width + padding;
		}
	}
	double custom_width= rightmost - leftmost;
	m_field_width= std::max(custom_width, auto_place_width);

	double left_col_x= 0.0;
	double right_col_x= 0.0;
	m_left_column_id= 0;
	m_right_column_id= 0;
	double curr_x= (auto_place_width * -.5);
	// The column needs all of this info.
	Lua* L= LUA->Get();
	lua_createtable(L, max_column, 0);
	int column_info_table= lua_gettop(L);
	vector<float> column_x;
	column_x.reserve(max_column);
	for(size_t i= 0; i < max_column; ++i)
	{
		NoteSkinColumn* col= base_data.get_column(i);
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
		if(col_x < left_col_x)
		{
			left_col_x= col_x;
			m_left_column_id= i;
		}
		if(col_x > right_col_x)
		{
			right_col_x= col_x;
			m_right_column_id= i;
		}
		lua_createtable(L, 0, 3);
		int this_col_info_table= lua_gettop(L);
		lua_pushnumber(L, width);
		lua_setfield(L, this_col_info_table, "width");
		lua_pushnumber(L, padding);
		lua_setfield(L, this_col_info_table, "padding");
		lua_pushnumber(L, col_x);
		lua_setfield(L, this_col_info_table, "x");
		lua_rawseti(L, column_info_table, i+1);

		m_columns[i].apply_base_skin(&m_player_colors, col_x);

		// The draw entries for a column can only be safely set when the noteskin
		// is set, but the note data might change to a type that is invalid for
		// the noteskin, causing the columns to be remade without a noteskin.
		// Adding the draw entries during reskinning ensures the noteskin is
		// valid.
		// add_draw_entry filters out duplicate entries, so this piece of code
		// doesn't need to.
		int id_as_i= int(i);
		add_draw_entry({nullptr, id_as_i, holds_draw_order, fdem_holds});
		add_draw_entry({nullptr, id_as_i, lifts_draw_order, fdem_lifts});
		add_draw_entry({nullptr, id_as_i, taps_draw_order, fdem_taps});
		add_draw_entry({nullptr, id_as_i, selection_draw_order, fdem_selection});

		column_x.push_back(col_x);
		if(!col->get_use_custom_x())
		{
			curr_x+= wid_pad;
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

void NoteField::delete_skin_entry(size_t id, field_skin_entry* entry)
{
	remove_layers_from_skin(id, true);
	remove_skin_from_columns(id);
	delete entry;
}

void NoteField::remove_skin(std::string const& name, int uid)
{
	if(m_noteskins.size() == 1)
	{
		return;
	}
	for(size_t id= 0; id < m_noteskins.size(); ++id)
	{
		field_skin_entry* entry= m_noteskins[id];
		if(entry->name == name && entry->uid == uid)
		{
			delete_skin_entry(id, entry);
			m_noteskins.erase(m_noteskins.begin() + id);
			if(id == 0 && !m_noteskins.empty())
			{
				apply_base_skin_to_columns();
			}
			for(auto&& col : m_columns)
			{
				col.build_render_lists();
			}
			return;
		}
	}
}

void NoteField::clear_to_base_skin()
{
	if(m_base_noteskin.name.empty())
	{
		return;
	}
	for(auto&& skin : m_unapplied_noteskins)
	{
		delete skin;
	}
	m_unapplied_noteskins.clear();
	if(!m_noteskins.empty())
	{
		for(size_t id= m_noteskins.size()-1; id > 0; --id)
		{
			delete_skin_entry(id, m_noteskins[id]);
		}
		m_noteskins.resize(1);
		if(m_noteskins[0]->name != m_base_noteskin.name)
		{
			delete_skin_entry(0, m_noteskins[0]);
			m_noteskins.clear();
			set_skin(m_base_noteskin.name, m_base_noteskin.params, m_base_noteskin.uid);
		}
	}
	else
	{
		set_skin(m_base_noteskin.name, m_base_noteskin.params, m_base_noteskin.uid);
	}
	for(auto&& col : m_columns)
	{
		col.build_render_lists();
	}
}

void NoteField::set_steps(Steps* data)
{
	if(data == nullptr)
	{
		clear_steps();
		return;
	}
	if(m_share_steps_parent != nullptr)
	{
		m_share_steps_parent->remove_share_steps_child(this);
	}
	NoteData* note_data= new NoteData;
	data->GetNoteData(*note_data);
	set_note_data(note_data, data->GetTimingData(), data->m_StepsType);
	m_own_note_data= true;
}

void NoteField::set_note_data(NoteData* note_data, TimingData const* timing, StepsType stype)
{
	timing->RequestLookup();
	if(m_timing_data != nullptr)
	{
		m_timing_data->ReleaseLookup();
	}
	if(m_own_note_data)
	{
		Rage::safe_delete(m_note_data);
	}
	m_note_data= note_data;
	// TODO: When multiple notefields share the note data, SetOccuranceTimeForAllTaps doesn't need to be called by every one of them.
	note_data->SetOccuranceTimeForAllTaps(timing);
	m_own_note_data= false;
	m_timing_data= timing;
	m_defective_mods.set_timing(m_timing_data);
	m_defective_mods.set_num_pads(GAMEMAN->get_num_pads_for_stepstype(stype));
	m_mod_manager.set_timing(m_timing_data);
	m_trans_mod.set_column(0);
	m_fov_mod.set_column(0);
	for(auto&& moddable : {&m_receptor_alpha, &m_receptor_glow,
				&m_explosion_alpha, &m_explosion_glow, &m_num_upcoming,
				&m_layer_skin_id})
	{
		moddable->set_column(0);
	}
	NotePlayerizeMode player_mode= NPM_Off;
	if(GAMEMAN->stepstype_is_multiplayer(stype))
	{
		player_mode= NPM_Quanta;
	}
	if(stype != m_steps_type)
	{
		m_steps_type= stype;
		recreate_columns();
		remove_all_noteskin_layers();
		std::vector<field_skin_entry*> old_noteskins= m_noteskins;
		m_noteskins.clear();
		// Find the base noteskin first, either from m_unapplied_noteskins, or
		// from old_noteskins.
		// A noteskin in m_unapplied_noteskins with replaces_first_skin set takes
		// priority for base because it was probably set with the intent of
		// taking effect with the stepstype change.
		// After the base is set, then old_noteskins are applied, then
		// m_unapplied_noteskins.  Any noteskins in either group that don't fit
		// the stepstype are discarded.
		size_t old_start= 0;
		for(size_t id= 0; id < m_unapplied_noteskins.size(); ++id)
		{
			if(m_unapplied_noteskins[id]->replaces_first_skin)
			{
				if(fill_skin_entry_data(m_unapplied_noteskins[id]) == FSED_success)
				{
					m_noteskins.push_back(m_unapplied_noteskins[id]);
					// It will be applied later, after m_noteskins is filled.
					old_start= 1;
					m_unapplied_noteskins.erase(m_unapplied_noteskins.begin()+id);
					break;
				}
			}
		}
		// If the base noteskin was set by m_unapplied_noteskins, skip the first
		// entry in old_noteskins, because it has been replaced.
		for(size_t id= old_start; id < old_noteskins.size(); ++id)
		{
			old_noteskins[id]->data.clear();
			if(fill_skin_entry_data(old_noteskins[id]) == FSED_success)
			{
				m_noteskins.push_back(old_noteskins[id]);
			}
			else
			{
				delete old_noteskins[id];
			}
		}
		old_noteskins.clear();
		for(size_t id= 0; id < m_unapplied_noteskins.size(); ++id)
		{
			if(fill_skin_entry_data(m_unapplied_noteskins[id]) == FSED_success)
			{
				m_noteskins.push_back(m_unapplied_noteskins[id]);
			}
			else
			{
				delete m_unapplied_noteskins[id];
			}
		}
		m_unapplied_noteskins.clear();
		// Now apply the base noteskin, then layers from all.
		if(!m_noteskins.empty())
		{
			for(size_t id= 0; id < m_noteskins.size(); ++id)
			{
				add_layers_from_skin(m_noteskins[id]->data, id);
				add_skin_to_columns(m_noteskins[id]->data);
			}
			apply_base_skin_to_columns();
		}
	}
	for(size_t i= 0; i < m_columns.size(); ++i)
	{
		m_columns[i].set_note_data(m_note_data, m_timing_data);
		m_columns[i].set_playerize_mode(player_mode);
	}
	if(!m_share_steps_children.empty())
	{
		for(auto&& child : m_share_steps_children)
		{
			child->set_note_data(note_data, timing, stype);
		}
	}
}

void NoteField::share_steps(NoteField* share_to)
{
	share_to->become_share_steps_child(this, m_note_data, m_timing_data, m_steps_type);
	m_share_steps_children.push_back(share_to);
}

void NoteField::become_share_steps_child(NoteField* parent,
	NoteData* note_data, TimingData const* timing, StepsType stype) // this is child
{
	if(m_share_steps_parent == parent)
	{
		return;
	}
	if(m_share_steps_parent != nullptr)
	{
		m_share_steps_parent->remove_share_steps_child(this);
	}
	m_share_steps_parent= parent;
	if(note_data)
	{
		set_note_data(note_data, timing, stype);
	}
	else
	{
		clear_steps();
	}
}

void NoteField::remove_share_steps_child(NoteField* child) // this is parent
{
	auto entry= m_share_steps_children.begin();
	for(; entry != m_share_steps_children.end(); ++entry)
	{
		if(*entry == child)
		{
			m_share_steps_children.erase(entry);
			return;
		}
	}
}

void NoteField::share_steps_parent_being_destroyed() // this is child
{
	clear_steps();
	m_share_steps_parent= nullptr;
}

void NoteField::add_draw_entry(field_draw_entry const& entry)
{
	auto insert_pos= m_draw_entries.begin();
	for(; insert_pos != m_draw_entries.end(); ++insert_pos)
	{
		// Ignore an attempt to add a duplicate entry.  NoteFieldColumn::reskin
		// needs this.
		if(insert_pos->draw_order == entry.draw_order &&
			insert_pos->column == entry.column &&
			insert_pos->meaning == entry.meaning &&
			insert_pos->child == entry.child)
		{
			return;
		}
		if(insert_pos->draw_order > entry.draw_order)
		{
			break;
		}
	}
	m_draw_entries.insert(insert_pos, entry);
}

void NoteField::remove_draw_entry(int column, Actor* child)
{
	auto entry= m_draw_entries.begin();
	while(entry != m_draw_entries.end())
	{
		if(entry->column == column && entry->child == child)
		{
			entry= m_draw_entries.erase(entry);
			return;
		}
		else
		{
			++entry;
		}
	}
}

void NoteField::change_draw_entry(int column, Actor* child, int new_draw_order)
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
			{
				FieldChild* child= static_cast<FieldChild*>(entry.child);
				if(child->m_from_noteskin != theme_noteskin_id &&
					child->m_from_noteskin != layer_skin_id)
				{
					return;
				}
				if(child->m_transform_type != FLTT_None && !avg_head_trans_is_fresh)
				{
					Rage::transform left= m_columns[m_left_column_id].get_head_trans();
					Rage::transform right= m_columns[m_right_column_id].get_head_trans();
					Rage::avg_vec3(left.pos, right.pos, avg_head_trans.pos);
					Rage::avg_vec3(left.rot, right.rot, avg_head_trans.rot);
					Rage::avg_vec3(left.zoom, right.zoom, avg_head_trans.zoom);
					avg_head_trans_is_fresh= true;
				}
				child->apply_render_info(
					avg_head_trans,
					evaluated_receptor_alpha, evaluated_receptor_glow,
					evaluated_explosion_alpha, evaluated_explosion_glow,
					m_curr_beat, m_curr_second, m_receptor_alpha, m_receptor_glow);
				child->Draw();
			}
			break;
		case beat_bars_column_index:
			draw_beat_bars_internal();
			break;
		default:
			if(entry.column >= 0 && static_cast<size_t>(entry.column) < m_columns.size())
			{
				m_columns[entry.column].draw_thing(&entry);
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
		m_columns[0].apply_yoffset_to_pos(input, trans.pos);
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
		m_columns[0].apply_yoffset_to_pos(input, trans.pos);
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

	Song* curr_song= GAMESTATE->get_curr_song();
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

void NoteField::recreate_columns()
{
	clear_column_draw_entries();
	// resize alone won't lower vector capacity back down, but swapping will.
	{
		std::vector<NoteFieldColumn> old_columns;
		m_columns.swap(old_columns);
		m_columns.resize(m_note_data->GetNumTracks());
	}
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", m_pn);
	for(size_t i= 0; i < m_columns.size(); ++i)
	{
		NoteFieldColumn& col= m_columns[i];
		col.set_parent_info(this, i, &m_defective_mods);
		col.set_player_number(m_pn);
		col.HandleMessage(msg);
		if(m_in_defective_mode)
		{
			col.set_defective_mode(m_in_defective_mode);
		}
	}
}

void NoteField::add_skin_to_columns(NoteSkinData& data)
{
	for(auto&& col : m_columns)
	{
		col.add_skin(data);
	}
}

void NoteField::remove_skin_from_columns(size_t id)
{
	for(auto&& col : m_columns)
	{
		col.remove_skin(id);
	}
}

void NoteField::add_layers_from_skin(NoteSkinData& data, size_t id)
{
	for(auto&& layer : data.m_field_layers)
	{
		AddChildInternal(layer, id);
	}
	data.m_children_owned_by_field_now= true;
}

void NoteField::remove_layers_from_skin(size_t id, bool shift_others)
{
	if(!m_layers.empty())
	{
		auto iter= m_layers.begin();
		while(iter != m_layers.end())
		{
			if(iter->m_from_noteskin == id)
			{
				remove_draw_entry(field_layer_column_index, &*iter);
				iter= m_layers.erase(iter);
			}
			else
			{
				if(iter->m_from_noteskin != theme_noteskin_id && shift_others && iter->m_from_noteskin > id)
				{
					--(iter->m_from_noteskin);
				}
				++iter;
			}
		}
	}
}

void NoteField::remove_all_noteskin_layers()
{
	if(!m_layers.empty())
	{
		auto iter= m_layers.begin();
		while(iter != m_layers.end())
		{
			if(iter->m_from_noteskin != theme_noteskin_id)
			{
				remove_draw_entry(field_layer_column_index, &*iter);
				iter= m_layers.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}

Message NoteField::create_width_message()
{
	Message width_msg("WidthSet");
	Lua* L= LUA->Get();
	lua_createtable(L, m_columns.size(), 0);
	int column_info_table= lua_gettop(L);
	for(size_t i= 0; i < m_columns.size(); ++i)
	{
		NoteSkinColumn* col= m_noteskins[0]->data.get_column(i);
		lua_createtable(L, 0, 3);
		int this_col_info_table= lua_gettop(L);
		lua_pushnumber(L, col->get_width());
		lua_setfield(L, this_col_info_table, "width");
		lua_pushnumber(L, col->get_padding());
		lua_setfield(L, this_col_info_table, "padding");
		lua_pushnumber(L, m_columns[i].m_column_mod.pos_mod.x_mod.get_base_value());
		lua_setfield(L, this_col_info_table, "x");
		lua_rawseti(L, column_info_table, i+1);
	}
	width_msg.SetParamFromStack(L, "columns");
	width_msg.SetParam("width", get_field_width());
	PushSelf(L);
	width_msg.SetParamFromStack(L, "field");
	LUA->Release(L);
	return width_msg;
}

void NoteField::set_player_number(PlayerNumber pn)
{
	m_pn= pn;
	Message msg("PlayerStateSet");
	msg.SetParam("PlayerNumber", pn);
	HandleMessage(msg);
	for(auto&& col : m_columns)
	{
		col.set_player_number(pn);
		col.HandleMessage(msg);
	}
}

PlayerNumber NoteField::get_player_number()
{
	return m_pn;
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

void NoteField::set_speed(float scroll_speed)
{
	// Instead of writing a C++ interface for setting the speed mod directly,
	// call the lua function for doing it.  Less work and ensures similar
	// behavior.
	lua_State* L= LUA->Get();
	// Push self to be able to get the set_speed_mod function.
	PushSelf(L);
	lua_getfield(L, lua_gettop(L), "set_speed_mod");
	// Push self to be able to pass self to the function.
	PushSelf(L);
	lua_pushboolean(L, false);
	lua_pushnumber(L, scroll_speed);
	lua_pushnil(L);
	std::string err;
	LuaHelpers::RunScriptOnStack(L, err, 4, 1);
	// The function returns self, but we don't care.
	lua_settop(L, 0);
	LUA->Release(L);
	// Disable speed and scroll segments because this is for edit mode.
	disable_speed_scroll_segments();
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
	m_in_edit_mode= true;
	add_draw_entry({nullptr, beat_bars_column_index, beat_bars_draw_order, fdem_beat_bars});
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
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			entry.m_fade_type= type;
		}
	}
}

FieldLayerFadeType NoteField::get_layer_fade_type(Actor* child)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			return entry.m_fade_type;
		}
	}
	return FieldLayerFadeType_Invalid;
}

void NoteField::set_layer_transform_type(Actor* child, FieldLayerTransformType type)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			entry.m_transform_type= type;
		}
	}
}

FieldLayerTransformType NoteField::get_layer_transform_type(Actor* child)
{
	for(auto&& entry : m_layers)
	{
		if(entry.m_child == child)
		{
			return entry.m_transform_type;
		}
	}
	return FieldLayerTransformType_Invalid;
}

void NoteField::update_displayed_time(double beat, double second)
{
	m_curr_beat= beat;
	m_curr_second= second;
	m_mod_manager.update(beat, second);
	avg_head_trans_is_fresh= false;
	if(m_in_defective_mode)
	{
		m_defective_mods.update(m_pn, m_curr_beat, m_curr_second);
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
		Rage::Vector3 fov_result;
		m_fov_mod.evaluate(input, fov_result);
		SetFOV(fov_result.z);
		float vanish_x= fov_result.x;
		float vanish_y= fov_result.y;
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
		layer_skin_id= size_t(std::round(m_layer_skin_id.evaluate(input)))
			% m_noteskins.size();
		int temp_nr= std::floor(m_num_upcoming.evaluate(input));
		if(temp_nr > 0)
		{
			size_t num_upcoming= static_cast<size_t>(temp_nr);
			// First element of pair is the time of the note.
			// Second element is the column id.
			std::vector<std::pair<double, size_t>> upcoming_notes;
			upcoming_notes.reserve(num_upcoming+1);
			for(size_t cid= 0; cid < m_columns.size(); ++cid)
			{
				m_columns[cid].add_upcoming_notes(upcoming_notes, num_upcoming);
			}
			if(upcoming_notes.size() > num_upcoming)
			{
				upcoming_notes.resize(num_upcoming);
			}
			std::vector<size_t> counts(m_columns.size(), 0);
			for(auto&& upentry : upcoming_notes)
			{
				++counts[upentry.second];
			}
			for(size_t cid= 0; cid < m_columns.size(); ++cid)
			{
				m_columns[cid].set_num_upcoming(counts[cid]);
			}
		}
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

void NoteField::assign_permanent_mods_to_columns(lua_State* L, int mod_set)
{
	lua_getfield(L, mod_set, "field");
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		m_mod_manager.add_permanent_mods(L, lua_gettop(L));
	}
	lua_pop(L, 1);
	for(size_t c= 0; c < m_columns.size(); ++c)
	{
		lua_rawgeti(L, mod_set, c+1);
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			m_columns[c].m_mod_manager.add_permanent_mods(L, lua_gettop(L));
		}
		lua_pop(L, 1);
	}
}

void NoteField::assign_timed_mods_to_columns(lua_State* L, int mod_set)
{
	lua_getfield(L, mod_set, "field");
	if(lua_type(L, -1) == LUA_TTABLE)
	{
		m_mod_manager.add_timed_mods(L, lua_gettop(L));
	}
	lua_pop(L, 1);
	for(size_t c= 0; c < m_columns.size(); ++c)
	{
		lua_rawgeti(L, mod_set, c+1);
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			m_columns[c].m_mod_manager.add_timed_mods(L, lua_gettop(L));
		}
		lua_pop(L, 1);
	}
}

void NoteField::clear_timed_mods()
{
	m_mod_manager.clear_timed_mods();
	for(auto&& col : m_columns)
	{
		col.m_mod_manager.clear_timed_mods();
	}
}

// lua start
#define SAFE_TIMING_CHECK(p, L, func_name) \
if(!p->timing_is_safe()) \
{ luaL_error(L, "Timing data is not set, " #func_name " is not safe."); }

struct LunaNoteFieldColumn : Luna<NoteFieldColumn>
{
	static int set_base_values(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Base values for set_base_values must be in a table.");
		}
		p->m_mod_manager.set_base_values(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_permanent_mods must be in a table.");
		}
		p->m_mod_manager.add_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_timed_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_timed_mods must be in a table.");
		}
		p->m_mod_manager.add_timed_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int remove_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for remove_permanent_mods must be in an unmarked secure envelope.");
		}
		p->m_mod_manager.remove_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int clear_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for clear_permanent_mods must be in a table.");
		}
		p->m_mod_manager.clear_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int get_mod_target_info(T* p, lua_State* L)
	{
		p->m_mod_manager.push_target_info(L);
		return 1;
	}
	GET_SET_BOOL_METHOD(use_game_music_beat, m_use_game_music_beat);
	GET_SET_BOOL_METHOD(show_unjudgable_notes, m_show_unjudgable_notes);
	GET_SET_BOOL_METHOD(speed_segments_enabled, m_speed_segments_enabled);
	GET_SET_BOOL_METHOD(scroll_segments_enabled, m_scroll_segments_enabled);
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
		ADD_METHOD(set_base_values);
		ADD_METHOD(set_permanent_mods);
		ADD_METHOD(set_timed_mods);
		ADD_METHOD(remove_permanent_mods);
		ADD_METHOD(clear_permanent_mods);
		ADD_METHOD(get_mod_target_info);
		ADD_GET_SET_METHODS(use_game_music_beat);
		ADD_GET_SET_METHODS(show_unjudgable_notes);
		ADD_GET_SET_METHODS(speed_segments_enabled);
		ADD_GET_SET_METHODS(scroll_segments_enabled);
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
		ADD_GET_SET_METHODS(layer_fade_type);
		ADD_GET_SET_METHODS(layer_transform_type);
	}
};
LUA_REGISTER_DERIVED_CLASS(NoteFieldColumn, ActorFrame);

struct LunaNoteField : Luna<NoteField>
{
	static int set_base_values(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Base values for set_base_values must be in a table.");
		}
		p->m_mod_manager.set_base_values(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_permanent_mods must be in a table.");
		}
		p->m_mod_manager.add_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_timed_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_timed_mods must be in a table.");
		}
		p->m_mod_manager.add_timed_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int remove_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for remove_permanent_mods must be in an unmarked secure envelope.");
		}
		p->m_mod_manager.remove_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int clear_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for clear_permanent_mods must be in a table.");
		}
		p->m_mod_manager.clear_permanent_mods(L, 1);
		COMMON_RETURN_SELF;
	}
	static int clear_timed_mods(T* p, lua_State* L)
	{
		p->clear_timed_mods();
		COMMON_RETURN_SELF;
	}
	static int get_mod_target_info(T* p, lua_State* L)
	{
		p->m_mod_manager.push_target_info(L);
		return 1;
	}
	static int set_per_column_permanent_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_per_column_permanent_mods must be in a table.");
		}
		p->assign_permanent_mods_to_columns(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_per_column_timed_mods(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Mods for set_per_column_permanent_mods must be in a table.");
		}
		p->assign_timed_mods_to_columns(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_skin(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		LuaReference skin_params;
		if(lua_type(L, 2) == LUA_TTABLE)
		{
			lua_pushvalue(L, 2);
			skin_params.SetFromStack(L);
		}
		int uid= 0;
		if(lua_type(L, 3) == LUA_TNUMBER)
		{
			uid= lua_tonumber(L, 3);
		}
		p->set_skin(skin_name, skin_params, uid);
		COMMON_RETURN_SELF;
	}
	static int add_skin(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		LuaReference skin_params;
		if(lua_type(L, 2) == LUA_TTABLE)
		{
			lua_pushvalue(L, 2);
			skin_params.SetFromStack(L);
		}
		int uid= 0;
		if(lua_type(L, 3) == LUA_TNUMBER)
		{
			uid= lua_tonumber(L, 3);
		}
		p->add_skin(skin_name, skin_params, uid);
		COMMON_RETURN_SELF;
	}
	static int remove_skin(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		int uid= 0;
		if(lua_type(L, 2) == LUA_TNUMBER)
		{
			uid= lua_tonumber(L, 2);
		}
		p->remove_skin(skin_name, uid);
		COMMON_RETURN_SELF;
	}
	static int clear_to_base_skin(T* p, lua_State* L)
	{
		p->clear_to_base_skin();
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
	static int share_steps(T* p, lua_State* L)
	{
		NoteField* share_to= Luna<NoteField>::check(L, 1);
		p->share_steps(share_to);
		COMMON_RETURN_SELF;
	}
	static int get_stepstype(T* p, lua_State* L)
	{
		Enum::Push(L, p->get_stepstype());
		return 1;
	}
	static int get_columns(T* p, lua_State* L)
	{
		p->push_columns_to_lua(L);
		return 1;
	}
	static int get_num_columns(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_num_columns());
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
	static int get_player_number(T* p, lua_State* L)
	{
		Enum::Push(L, p->get_player_number());
		return 1;
	}
	static int set_player_number(T* p, lua_State* L)
	{
		PlayerNumber pn= PlayerNumber_Invalid;
		if(!lua_isnil(L, 1))
		{
			pn= Enum::Check<PlayerNumber>(L, 1);
		}
		p->set_player_number(pn);
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
	GETTER_SETTER_BOOL_METHOD(defective_mode);
	GET_SET_BOOL_METHOD(oitg_zoom_mode, m_oitg_zoom_mode);
	LunaNoteField()
	{
		ADD_METHOD(set_base_values);
		ADD_METHOD(set_permanent_mods);
		ADD_METHOD(set_timed_mods);
		ADD_METHOD(remove_permanent_mods);
		ADD_METHOD(clear_permanent_mods);
		ADD_METHOD(clear_timed_mods);
		ADD_METHOD(get_mod_target_info);
		ADD_METHOD(set_per_column_permanent_mods);
		ADD_METHOD(set_per_column_timed_mods);
		ADD_GET_SET_METHODS(skin);
		ADD_METHOD(add_skin);
		ADD_METHOD(remove_skin);
		ADD_METHOD(clear_to_base_skin);
		ADD_METHOD(set_steps);
		ADD_METHOD(share_steps);
		ADD_METHOD(get_stepstype);
		ADD_METHOD(get_columns);
		ADD_METHOD(get_num_columns);
		ADD_METHOD(get_width);
		ADD_GET_SET_METHODS(curr_beat);
		ADD_GET_SET_METHODS(curr_second);
		ADD_GET_SET_METHODS(vanish_type);
		ADD_METHOD(set_player_color);
		ADD_GET_SET_METHODS(player_number);
		ADD_GET_SET_METHODS(layer_fade_type);
		ADD_GET_SET_METHODS(layer_transform_type);
		ADD_GET_SET_METHODS(defective_mode);
		ADD_GET_SET_METHODS(oitg_zoom_mode);
	}
};
LUA_REGISTER_DERIVED_CLASS(NoteField, ActorFrame);
