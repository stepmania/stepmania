#ifndef NEW_SKIN_H
#define NEW_SKIN_H

#include <unordered_set>

#include "Actor.h"
#include "ActorFrame.h"
#include "AutoActor.h"
#include "NoteTypes.h"
#include "RageTexture.h"
#include "RageUtil.hpp"

// Receptors and explosions are full actors.  There are a fixed number of
// them, and that number is relatively small.  Their update functions will
// be called each frame.
// Taps are actors that occur at a single point in time.  One is made for
// each NoteSkinTapPart and NoteSkinTapOptionalPart, and that one is reused
// whenever a tap of that part is needed.
// Everything in Tap and Hold is considered quantizable.  They get a
// state map to control what part of their texture is used at a given
// quantization and beat.
// Holds are loaded by the tap loader, so there isn't a separate enum entry
// for holds.
// Holds must be stretched over a period, so they are not actors at all.
// Instead, they only have 6 textures: the two caps and the body, in active
// and inactive states.  These textures are then rendered to generated
// quads.

enum NoteSkinTapPart
{
	// These tap parts must be provided by the noteskin.  If they are absent,
	// it is an error.
	NSTP_Tap,
	NSTP_Mine,
	NSTP_Lift,
	NUM_NoteSkinTapPart,
	NoteSkinTapPart_Invalid
};
std::string const NoteSkinTapPartToString(NoteSkinTapPart nsp);
LuaDeclareType(NoteSkinTapPart);

enum NoteSkinTapOptionalPart
{
	// These tap parts are optional.  If none of them exist, nothing is used.
	// If HoldHead exists and RollHead does not, HoldHead is used when a
	// RollHead is needed.
	NSTOP_HoldHead,
	NSTOP_HoldTail,
	NSTOP_RollHead,
	NSTOP_RollTail,
	NSTOP_CheckpointHead,
	NSTOP_CheckpointTail,
	NUM_NoteSkinTapOptionalPart,
	NoteSkinTapOptionalPart_Invalid
};
std::string const NoteSkinTapOptionalPartToString(NoteSkinTapOptionalPart nsp);
LuaDeclareType(NoteSkinTapOptionalPart);

enum NoteSkinHoldPart
{
	NSHP_Top,
	NSHP_Body,
	NSHP_Bottom,
	NUM_NoteSkinHoldPart,
	NoteSkinHoldPart_Invalid
};
std::string const NoteSkinHoldPartToString(NoteSkinHoldPart nsp);
LuaDeclareType(NoteSkinHoldPart);

// There are three modes for playerizing notes for routine mode.
// NPM_Off is for not playerizing at all.
// NPM_Mask uses the color mask in the noteskin.
// NPM_Quanta uses the quanta in the noteskin as per-player notes.
enum NotePlayerizeMode
{
	NPM_Off,
	NPM_Mask,
	NPM_Quanta,
	NUM_NotePlayerizeMode,
	NotePlayerizeMode_Invalid
};
std::string const NotePlayerizeModeToString(NotePlayerizeMode npm);
LuaDeclareType(NotePlayerizeMode);

struct NoteSkinLoader;
struct TimingSource;

// The notefield may apply modifiers to change the quantization data in the
// note before it is passed to the noteskin.
struct note_quant_anim_data
{
	int parts_per_beat;
	int part_id;
	int row_id;
	float beat;
	bool player_mode;
	bool rainbow_mode;
};

struct Quantizer
{
	static const size_t max_quanta= 256;
	static const size_t max_states= 256;
	struct Quantum
	{
		int part_id;
		float trans_x;
		float trans_y;
		std::vector<size_t> states;
		size_t beat_to_state(double beat) const
		{
			return states[size_t(floor(beat * states.size())) % states.size()];
		}
	};
	struct Quanta
	{
		int parts_per_beat;
		std::vector<Quantum> quanta;
	};

	Quantum const& get_quantum(note_quant_anim_data& input);

	size_t get_quant_total();

	bool load_from_lua(lua_State* L, int index, std::string& insanity_diagnosis);
	// load_from_state_map is for backwards compatibility.
	bool load_from_state_map(lua_State* L, int index, std::string& insanity_diagnosis);

private:
	bool load_quanta(Quanta& dest, lua_State* L, int index);
	void finalize_load();

	std::vector<Quanta> m_quanta_set;
	Quanta m_unknown_quanta;
	size_t m_quant_total;
};

struct QuantizedTap
{
	void set_timing_source(TimingSource* source)
	{
		m_actor->SetTimingSource(source);
	}
	void update()
	{
		m_actor->Update(1.0f);
	}
	Actor* get_quantized(note_quant_anim_data& input, bool active)
	{
		Quantizer::Quantum const& quant= active ?
			m_quantizer.get_quantum(input) :
			m_inactive_quantizer.get_quantum(input);
		if(quant.states.empty())
		{
			m_actor->SetSecondsIntoAnimation(input.beat);
			m_actor->SetTextureTranslate(quant.trans_x, quant.trans_y);
		}
		else
		{
			m_actor->SetState(quant.beat_to_state(input.beat));
		}
		return &m_frame;
	}
	bool load_from_lua(lua_State* L, int index, std::string& insanity_diagnosis);
	// What happened to vivid?
private:
	Quantizer m_quantizer;
	Quantizer m_inactive_quantizer;
	AutoActor m_actor;
	ActorFrame m_frame;
};

enum TexCoordFlipMode
{
	TCFM_None,
	TCFM_X,
	TCFM_Y,
	TCFM_XY,
	NUM_TexCoordFlipMode,
	TexCoordFlipMode_Invalid
};
std::string const TexCoordFlipModeToString(TexCoordFlipMode tcfm);
LuaDeclareType(TexCoordFlipMode);

struct hold_part_lengths
{
	double start_note_offset;
	double end_note_offset;
	double head_pixs;
	double body_pixs;
	double tail_pixs;
};

struct QuantizedHoldRenderData
{
	QuantizedHoldRenderData() { clear(); }
	std::vector<RageTexture*> parts;
	RageTexture* mask;
	Rage::RectF const* rect;
	TexCoordFlipMode flip;
	hold_part_lengths part_lengths;
	bool texture_filtering;
	void clear()
	{
		parts.clear();
		mask= nullptr;
		rect= nullptr;
		texture_filtering= true;
	}
};

struct QuantizedHold
{
	static const size_t max_hold_layers= 32;
	Quantizer m_quantizer;
	std::vector<RageTexture*> m_parts;
	TexCoordFlipMode m_flip;
	bool m_texture_filtering;
	hold_part_lengths m_part_lengths;
	~QuantizedHold();
	void get_quantized(note_quant_anim_data& input, QuantizedHoldRenderData& ret)
	{
		Quantizer::Quantum const& quant= m_quantizer.get_quantum(input);
		const size_t state= quant.beat_to_state(input.beat);
		for(size_t i= 0; i < m_parts.size(); ++i)
		{
			ret.parts.push_back(m_parts[i]);
			if(ret.rect == nullptr)
			{
				ret.rect= m_parts[i]->GetTextureCoordRect(state);
			}
		}
		ret.flip= m_flip;
		ret.part_lengths= m_part_lengths;
		ret.texture_filtering= m_texture_filtering;
	}
	bool load_from_lua(lua_State* L, int index, NoteSkinLoader const* load_skin, std::string& insanity_diagnosis);
};

struct NoteSkinColumn
{
	void set_timing_source(TimingSource* source);
	void update_taps();
	Actor* get_tap_actor(size_t type, bool active, bool reverse,
		note_quant_anim_data& input);
	Actor* get_optional_actor(size_t type, bool active, bool reverse,
		note_quant_anim_data& input);
	void get_hold_render_data(TapNoteSubType sub_type,
		NotePlayerizeMode playerize_mode, bool active, bool reverse,
		note_quant_anim_data& input, QuantizedHoldRenderData& data);
	double get_width() { return m_width; }
	double get_padding() { return m_padding; }
	double get_custom_x() { return m_custom_x; }
	double get_anim_mult() { return m_anim_mult; }
	double get_quantum_mult() { return m_quantum_mult; }
	float get_hold_gray_percent() { return m_hold_gray_percent; }
	bool get_use_hold_head() { return m_use_hold_heads_for_taps_on_row; }
	bool get_anim_uses_beats() { return m_anim_uses_beats; }
	bool get_use_custom_x() { return m_use_custom_x; }
	bool supports_masking()
	{
		return !(m_hold_player_masks.empty() || m_hold_reverse_player_masks.empty());
	}
	bool load_holds_from_lua(lua_State* L, int index,
		std::vector<std::vector<QuantizedHold> >& holder,
		std::string const& holds_name,
		NoteSkinLoader const* load_skin, std::string& insanity_diagnosis);
	bool load_texs_from_lua(lua_State* L, int index,
		std::vector<RageTexture*>& dest,
		std::string const& texs_name,
		NoteSkinLoader const* load_skin, std::string& insanity_diagnosis);
	bool load_from_lua(lua_State* L, int index, NoteSkinLoader const* load_skin,
		std::string& insanity_diagnosis);
	void clear_optionals()
	{
		for(auto&& tap_set : {&m_optional_taps, &m_reverse_optional_taps})
		{
			for(auto&& tap : *tap_set)
			{
				if(tap != nullptr)
				{
					Rage::safe_delete(tap);
				}
			}
		}
	}
	NoteSkinColumn()
		:m_optional_taps(NUM_NoteSkinTapOptionalPart, nullptr)
	{}
	~NoteSkinColumn()
	{
		clear_optionals();
	}
private:
	// m_taps is indexed by NoteSkinTapPart.
	std::vector<QuantizedTap> m_taps;
	std::vector<QuantizedTap> m_reverse_taps;
	// m_optional_taps is indexed by NoteSkinTapOptionalPart.
	// If an entry is null, the skin doesn't use that part.
	std::vector<QuantizedTap*> m_optional_taps;
	std::vector<QuantizedTap*> m_reverse_optional_taps;
	// Dimensions of m_holds:
	// note subtype, active/inactive.
	std::vector<std::vector<QuantizedHold> > m_holds;
	std::vector<std::vector<QuantizedHold> > m_reverse_holds;
	// m_hold_player_masks is indexed by note subtype.
	std::vector<RageTexture*> m_hold_player_masks;
	std::vector<RageTexture*> m_hold_reverse_player_masks;
	double m_width;
	double m_padding;
	double m_custom_x;
	double m_hold_gray_percent;
	// m_anim_mult and m_quantization_mult are used to control how many beats
	// the animation and quantization are spread over.  The noteskin supplies a
	// number of beats, which is converted to its reciprocal.  The reciprocal
	// is used because multiplication is faster than division.
	double m_anim_mult;
	double m_quantum_mult;
	bool m_anim_uses_beats;
	bool m_use_hold_heads_for_taps_on_row;
	bool m_use_custom_x;
};

struct NoteSkinLayer
{
	bool load_from_lua(lua_State* L, int index, size_t columns, std::string& insanity_diagnosis);
	// The actors are public so that the NoteFieldColumns can go through and
	// take ownership of the actors after loading.
	std::vector<Actor*> m_actors;
};

struct NoteSkinData
{
	static const size_t max_columns= 256;
	NoteSkinData();
	void swap(NoteSkinData& other);
	NoteSkinColumn* get_column(size_t column)
	{
		if(column >= m_columns.size())
		{
			return nullptr;
		}
		return &m_columns[column];
	}
	size_t num_columns() { return m_columns.size(); }
	bool load_taps_from_lua(lua_State* L, int index, size_t columns,
		NoteSkinLoader const* load_skin, std::string& insanity_diagnosis);
	bool loaded_successfully() const { return m_loaded; }

	// The layers are public so that the NoteFieldColumns can go through and
	// take ownership of the actors after loading.
	std::vector<NoteSkinLayer> m_layers;
	std::vector<Rage::Color> m_player_colors;
private:
	std::vector<NoteSkinColumn> m_columns;
	LuaReference m_skin_parameters;
	bool m_loaded;
};

struct NoteSkinLoader
{
	static const size_t max_layers= 16;
	NoteSkinLoader()
		:m_supports_all_buttons(false)
	{}
	std::string const& get_name() const
	{
		return m_skin_name;
	}
	std::string const& get_fallback_name() const
	{
		return m_fallback_skin_name;
	}
	std::string const& get_load_path() const
	{
		return m_load_path;
	}
	void swap(NoteSkinLoader& other);
	bool load_from_file(std::string const& path);
	bool load_from_lua(lua_State* L, int index, std::string const& name,
		std::string const& path, std::string& insanity_diagnosis);
	bool supports_needed_buttons(StepsType stype) const;
	bool push_loader_function(lua_State* L, std::string const& loader);
	bool load_layer_set_into_data(lua_State* L, LuaReference& skin_params,
		int button_list_index, int stype_index,
		size_t columns, std::vector<std::string> const& loader_set,
		std::vector<NoteSkinLayer>& dest, std::string& insanity_diagnosis);
	bool load_into_data(StepsType stype, LuaReference& skin_params,
		NoteSkinData& dest, std::string& insanity_diagnosis);
	void sanitize_skin_parameters(lua_State* L, LuaReference& params);
	void push_skin_parameter_info(lua_State* L) const;
	void push_skin_parameter_defaults(lua_State* L) const;
private:
	void recursive_sanitize_skin_parameters(lua_State* L,
		std::unordered_set<void const*>& visited_tables, int curr_depth,
		int curr_param_set_info, int curr_param_set_defaults,
		int curr_param_set_dest);
	std::string m_skin_name;
	std::string m_fallback_skin_name;
	std::string m_load_path;
	std::string m_notes_loader;
	std::vector<std::string> m_layer_loaders;
	std::vector<Rage::Color> m_player_colors;
	std::unordered_set<std::string> m_supported_buttons;
	LuaReference m_skin_parameters;
	LuaReference m_skin_parameter_info;
	bool m_supports_all_buttons;
};

#endif
