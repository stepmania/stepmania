#include "global.h"

#include "ActorUtil.h"
#include "NoteSkin.h"
#include "NoteSkinManager.h"
#include "RageFileManager.h"
#include "RageTextureManager.h"
#include "RageUtil.hpp"
#include "Model.h"
#include "Sprite.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"

using std::max;
using std::string;
using std::unordered_set;
using std::vector;

static const double default_column_width= 64.0;
static const double default_column_padding= 0.0;
static const double default_hold_gray_percent= 0.0;
static const double default_anim_time= 1.0;
static const double min_anim_time= 0.01;
static const double default_quantum_time= 1.0;
static const double min_quantum_time= 0.01;
static const double invalid_length= -1000.0;

static const char* NoteSkinTapPartNames[] = {
	"Tap",
	"Mine",
	"Lift",
};
XToString(NoteSkinTapPart);
LuaXType(NoteSkinTapPart);

static const char* NoteSkinTapOptionalPartNames[] = {
	"HoldHead",
	"HoldTail",
	"RollHead",
	"RollTail",
	"CheckpointHead",
	"CheckpointTail",
};
XToString(NoteSkinTapOptionalPart);
LuaXType(NoteSkinTapOptionalPart);

static const char* NoteSkinHoldPartNames[] = {
	"Top",
	"Body",
	"Bottom",
};
XToString(NoteSkinHoldPart);
LuaXType(NoteSkinHoldPart);

static const char* TexCoordFlipModeNames[] = {
	"None",
	"X",
	"Y",
	"XY",
};
XToString(TexCoordFlipMode);
LuaXType(TexCoordFlipMode);

static const char* NotePlayerizeModeNames[]= {
	"Off",
	"Mask",
	"Quanta"
};
XToString(NotePlayerizeMode);
LuaXType(NotePlayerizeMode);

static size_t get_table_len(lua_State* L, int index, size_t max_entries,
	string const& table_name, string& insanity_diagnosis)
{
	if(!lua_istable(L, index))
	{
		insanity_diagnosis= fmt::sprintf("%s is not a table.", table_name.c_str());
		return 0;
	}
	size_t ret= lua_objlen(L, index);
	if(ret == 0)
	{
		insanity_diagnosis= fmt::sprintf("The %s table is empty.",table_name.c_str());
		return 0;
	}
	if(ret > max_entries)
	{
		insanity_diagnosis= fmt::sprintf("The %s table has over %zu entries.",
			table_name.c_str(), max_entries);
	}
	return ret;
}

template<typename el_type>
static bool load_simple_table(lua_State* L, int index, size_t max_entries,
		vector<el_type>& dest, el_type offset, el_type max_value,
		string const& table_name, string& insanity_diagnosis)
{
	size_t tab_size= get_table_len(L, index, max_entries, table_name, insanity_diagnosis);
	if(tab_size == 0)
	{
		return false;
	}
	dest.resize(tab_size);
	for(size_t i= 0; i < tab_size; ++i)
	{
		lua_rawgeti(L, index, i+1);
		el_type value= static_cast<el_type>(lua_tonumber(L, -1) - offset);
		lua_pop(L, 1);
		if(value >= max_value)
		{
			insanity_diagnosis= fmt::sprintf("Entry %zu in the %s table is not valid.",
				i+1, table_name.c_str());
			return false;
		}
		dest[i]= value;
	}
	lua_pop(L, 1);
	return true;
}

static bool load_string_table(lua_State* L, int index, size_t max_entries,
	vector<string>& dest, string const& table_name, string& insanity_diagnosis)
{
	size_t tab_size= get_table_len(L, index, max_entries, table_name, insanity_diagnosis);
	if(tab_size == 0)
	{
		return false;
	}
	dest.resize(tab_size);
	for(size_t i= 0; i < tab_size; ++i)
	{
		lua_rawgeti(L, index, i+1);
		if(!lua_isstring(L, -1))
		{
			insanity_diagnosis= fmt::sprintf("Entry %zu in the %s table is not valid.",
				i+1, table_name.c_str());
			return false;
		}
		dest[i]= lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return true;
}

template<typename el_type, typename en_type>
static void load_enum_table(lua_State* L, int index, en_type begin, en_type end,
		vector<el_type>& dest, el_type offset, el_type max_value,
		el_type default_value)
{
	// To allow expansion later, a missing element is not an error.  Instead,
	// the default value is used.
	dest.resize(end - begin);
	for(auto&& el : dest)
	{
		el= default_value;
	}
	if(!lua_istable(L, index))
	{
		return;
	}
	for(int curr= begin; curr != end; ++curr)
	{
		Enum::Push(L, static_cast<en_type>(curr));
		lua_gettable(L, index);
		if(!lua_isnoneornil(L, -1))
		{
			el_type value= static_cast<el_type>(lua_tonumber(L, -1) - offset);
			if(value < max_value)
			{
				dest[curr-begin]= value;
			}
		}
		lua_pop(L, 1);
	}
}

RageTexture* load_noteskin_tex(std::string const& path, NoteSkinLoader const* load_skin)
{
	RageTexture* as_tex= nullptr;
	// Check to see if a texture is registered before trying to convert it to
	// a full path.  This allows someone to make an AFT and name the texture
	// of the AFT, then use that texture name in the part.
	RageTextureID as_id(path);
	if(TEXTUREMAN->IsTextureRegistered(as_id))
	{
		as_tex= TEXTUREMAN->LoadTexture(as_id);
	}
	else
	{
		std::string resolved= NOTESKIN->get_path(load_skin, path);
		if(resolved.empty())
		{
			as_tex= TEXTUREMAN->LoadTexture(TEXTUREMAN->GetDefaultTextureID());
		}
		else
		{
			as_tex= TEXTUREMAN->LoadTexture(resolved);
		}
	}
	return as_tex;
}

void unload_texture_list(vector<RageTexture*>& tex_list)
{
	for(auto&& tex : tex_list)
	{
		TEXTUREMAN->UnloadTexture(tex);
	}
}

bool QuantizedStateMap::load_from_lua(lua_State* L, int index, string& insanity_diagnosis)
{
	// Loading is atomic:  If a single error occurs during loading the data,
	// none of it is used.
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, "quanta");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No quanta found");
	}
	size_t num_quanta= get_table_len(L, -1, max_quanta, "quanta", insanity_diagnosis);
	if(num_quanta == 0)
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	int quanta_index= lua_gettop(L);
	m_quanta.resize(num_quanta);
	for(size_t i= 0; i < m_quanta.size(); ++i)
	{
		lua_rawgeti(L, quanta_index, i+1);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid quantum %zu.", i+1));
		}
		lua_getfield(L, -1, "per_beat");
		if(!lua_isnumber(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid per_beat value in quantum %zu.", i+1));
		}
		m_quanta[i].per_beat= lua_tointeger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, -1, "states");
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid states in quantum %zu.", i+1));
		}
		if(!load_simple_table(L, lua_gettop(L), max_states,
				m_quanta[i].states, static_cast<size_t>(1), max_states, "states",
				insanity_diagnosis))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid states in quantum %zu: %s", i+1, insanity_diagnosis.c_str()));
		}
		lua_pop(L, 1);
	}
	lua_getfield(L, index, "parts_per_beat");
	if(!lua_isnumber(L, -1))
	{
		RETURN_NOT_SANE("Invalid parts_per_beat.");
	}
	m_parts_per_beat= lua_tointeger(L, -1);
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool QuantizedTextureMap::load_from_lua(lua_State* L, int index, std::string& insanity_diagnosis)
{
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, "quanta");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No quanta found");
	}
	size_t num_quanta= get_table_len(L, -1, max_quanta, "quanta", insanity_diagnosis);
	if(num_quanta == 0)
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	int quanta_index= lua_gettop(L);
	m_quanta.resize(num_quanta);
	for(size_t i= 0; i < m_quanta.size(); ++i)
	{
		lua_rawgeti(L, quanta_index, i+1);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid quantum %zu.", i+1));
		}
		int this_quanta= lua_gettop(L);
		lua_getfield(L, this_quanta, "per_beat");
		if(!lua_isnumber(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Invalid per_beat value in quantum %zu.", i+1));
		}
		m_quanta[i].per_beat= lua_tointeger(L, -1);
		m_quanta[i].trans_x= get_optional_double(L, this_quanta, "trans_x", .0);
		m_quanta[i].trans_y= get_optional_double(L, this_quanta, "trans_y", .0);
		lua_pop(L, 1);
	}
	lua_getfield(L, index, "parts_per_beat");
	if(!lua_isnumber(L, -1))
	{
		RETURN_NOT_SANE("Invalid parts_per_beat.");
	}
	m_parts_per_beat= lua_tointeger(L, -1);
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool QuantizedTap::load_from_lua(lua_State* L, int index, string& insanity_diagnosis)
{
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	bool found_state_or_texture_map= false;
	lua_getfield(L, index, "state_map");
	if(lua_istable(L, -1))
	{
		found_state_or_texture_map= true;
		if(!m_state_map.load_from_lua(L, lua_gettop(L), insanity_diagnosis))
		{
			RETURN_NOT_SANE(insanity_diagnosis);
		}
		lua_getfield(L, index, "inactive_state_map");
		if(lua_istable(L, -1))
		{
			if(!m_inactive_map.load_from_lua(L, lua_gettop(L), insanity_diagnosis))
			{
				RETURN_NOT_SANE(insanity_diagnosis);
			}
		}
		else
		{
			m_inactive_map= m_state_map;
		}
	}
	lua_getfield(L, index, "texture_map");
	if(lua_istable(L, -1))
	{
		found_state_or_texture_map= true;
		if(!m_texture_map.load_from_lua(L, lua_gettop(L), insanity_diagnosis))
		{
			RETURN_NOT_SANE(insanity_diagnosis);
		}
		lua_getfield(L, index, "inactive_texture_map");
		if(lua_istable(L, -1))
		{
			if(!m_inactive_texture_map.load_from_lua(L, lua_gettop(L), insanity_diagnosis))
			{
				RETURN_NOT_SANE(insanity_diagnosis);
			}
		}
		else
		{
			m_inactive_texture_map= m_texture_map;
		}
	}
	if(!found_state_or_texture_map)
	{
		RETURN_NOT_SANE("Could not find state or texture map.");
	}
	lua_getfield(L, index, "actor");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("Actor not found.");
	}
	std::unique_ptr<XNode> node(XmlFileUtil::XNodeFromTable(L));
	if(node.get() == nullptr)
	{
		RETURN_NOT_SANE("Actor not valid.");
	}
	Actor* act= ActorUtil::LoadFromNode(node.get(), nullptr);
	if(act == nullptr)
	{
		RETURN_NOT_SANE("Error loading actor.");
	}
	m_actor.Load(act);
	m_states_used= std::max(m_state_map.get_states_used(), m_inactive_map.get_states_used());
	recursive_find_parts(act);
	m_frame.AddChild(m_actor);
	lua_getfield(L, index, "vivid");
	m_vivid= lua_toboolean(L, -1);
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

void QuantizedTap::recursive_find_parts(Actor* part)
{
	ActorFrame* as_frame= dynamic_cast<ActorFrame*>(part);
	if(as_frame)
	{
		std::vector<Actor*> sub_parts= as_frame->GetChildren();
		for(auto&& sub : sub_parts)
		{
			recursive_find_parts(sub);
		}
	}
	else
	{
		Sprite* as_sprite= dynamic_cast<Sprite*>(part);
		if(as_sprite)
		{
			if(as_sprite->GetNumStates() > m_states_used)
			{
				m_sprites.push_back(as_sprite);
			}
		}
		else
		{
			Model* as_model= dynamic_cast<Model*>(part);
			if(as_model)
			{
				m_models.push_back(part);
			}
		}
	}
}

QuantizedHold::~QuantizedHold()
{
	unload_texture_list(m_parts);
}

bool QuantizedHold::load_from_lua(lua_State* L, int index, NoteSkinLoader const* load_skin, string& insanity_diagnosis)
{
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, "state_map");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No state map found.");
	}
	if(!m_state_map.load_from_lua(L, lua_gettop(L), insanity_diagnosis))
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	lua_getfield(L, index, "textures");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No textures found.");
	}
	size_t num_tex= get_table_len(L, -1, max_hold_layers, "textures", insanity_diagnosis);
	if(num_tex == 0)
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	int texind= lua_gettop(L);
	m_parts.resize(num_tex);
	for(size_t part= 0; part < num_tex; ++part)
	{
		lua_rawgeti(L, texind, part+1);
		if(!lua_isstring(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Texture entry for layer %zu is not a string.",
					part+1));
		}
		string path= lua_tostring(L, -1);
		if(path.empty())
		{
			RETURN_NOT_SANE("Empty texture path is not valid.");
		}
		m_parts[part]= load_noteskin_tex(path, load_skin);
	}
	lua_getfield(L, index, "flip");
	m_flip= TCFM_None;
	if(lua_isstring(L, -1))
	{
		m_flip= Enum::Check<TexCoordFlipMode>(L, -1, true, true);
		if(m_flip >= NUM_TexCoordFlipMode)
		{
			LuaHelpers::ReportScriptErrorFmt("Invalid flip mode %s", lua_tostring(L, -1));
			m_flip= TCFM_None;
		}
	}
	lua_getfield(L, index, "length_data");
	if(lua_istable(L, -1))
	{
		int length_data_index= lua_gettop(L);
		m_part_lengths.topcap_pixels= get_optional_double(L, length_data_index, "topcap_pixels", invalid_length);
		if(m_part_lengths.topcap_pixels == invalid_length)
		{
			m_part_lengths.topcap_pixels= get_optional_double(L, length_data_index, "head_pixs", invalid_length);
			if(m_part_lengths.topcap_pixels == invalid_length)
			{
				m_part_lengths.topcap_pixels= 32.0;
			}
		}
		m_part_lengths.pixels_before_note= get_optional_double(L, length_data_index, "pixels_before_note", invalid_length);
		if(m_part_lengths.pixels_before_note == invalid_length)
		{
			double note_offset= get_optional_double(L, length_data_index, "start_note_offset", invalid_length);
			if(note_offset == invalid_length)
			{
				m_part_lengths.pixels_before_note= 32.0;
			}
			else
			{
				m_part_lengths.pixels_before_note= note_offset * -1.0 * 64.0;
			}
		}
		m_part_lengths.body_pixels= get_optional_double(L, length_data_index, "head_pixels", invalid_length);
		if(m_part_lengths.body_pixels == invalid_length)
		{
			m_part_lengths.body_pixels= get_optional_double(L, length_data_index, "body_pixs", 64.0);
		}
		m_part_lengths.pixels_after_note= get_optional_double(L, length_data_index, "pixels_after_note", invalid_length);
		if(m_part_lengths.pixels_after_note == invalid_length)
		{
			double note_offset= get_optional_double(L, length_data_index, "end_note_offset", invalid_length);
			if(note_offset == invalid_length)
			{
				m_part_lengths.pixels_after_note= 32.0;
			}
			else
			{
				m_part_lengths.pixels_after_note= note_offset * 64.0;
			}
		}
		m_part_lengths.bottomcap_pixels= get_optional_double(L, length_data_index, "bottomcap_pixels", invalid_length);
		if(m_part_lengths.bottomcap_pixels == invalid_length)
		{
			m_part_lengths.bottomcap_pixels= get_optional_double(L, length_data_index, "tail_pixs", invalid_length);
			if(m_part_lengths.bottomcap_pixels == invalid_length)
			{
				m_part_lengths.bottomcap_pixels= 32.0;
			}
		}
		m_part_lengths.needs_jumpback= get_optional_bool(L, length_data_index, "needs_jumpback", true);
	}
	else
	{
		m_part_lengths.topcap_pixels= 32.0;
		m_part_lengths.bottomcap_pixels= 32.0;
		m_part_lengths.pixels_before_note= 32.0;
		m_part_lengths.pixels_after_note= 32.0;
		m_part_lengths.body_pixels= 64.0;
		m_part_lengths.needs_jumpback= true;
	}
	lua_getfield(L, index, "vivid");
	m_vivid= lua_toboolean(L, -1);
	m_texture_filtering= !get_optional_bool(L, index, "disable_filtering", false);
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

void NoteSkinColumn::set_timing_source(TimingSource* source)
{
	for(auto&& tap_set : {&m_taps, &m_reverse_taps})
	{
		for(auto&& tap : *tap_set)
		{
			tap.set_timing_source(source);
		}
	}
	for(auto&& tap_set : {&m_optional_taps, &m_reverse_optional_taps})
	{
		for(auto&& tap : *tap_set)
		{
			if(tap != nullptr)
			{
				tap->set_timing_source(source);
			}
		}
	}
}

void NoteSkinColumn::update_taps(float delta)
{
	for(auto&& tap_set : {&m_taps, &m_reverse_taps})
	{
		for(auto&& tap : *tap_set)
		{
			tap.update(delta);
		}
	}
	for(auto&& tap_set : {&m_optional_taps, &m_reverse_optional_taps})
	{
		for(auto&& tap : *tap_set)
		{
			if(tap != nullptr)
			{
				tap->update(delta);
			}
		}
	}
}

// I didn't want to use a macro to make get_tap_actor and get_player_tap be
// the same, but then the reverse logic had to be added, which made them
// complex.  So GET_TAP_BODY is to make sure they don't drift apart.
#define GET_TAP_BODY(get_func, quant_param) \
	ASSERT_M(type < m_taps.size(), "Invalid NoteSkinTapPart type."); \
	if(reverse && !m_reverse_taps.empty()) \
	{ \
		m_reverse_taps[type].get_func(quant_param, beat, active); \
	} \
	return m_taps[type].get_func(quant_param, beat, active);

Actor* NoteSkinColumn::get_tap_actor(size_t type,
	double quantization, double beat, bool active, bool reverse)
{
	GET_TAP_BODY(get_quantized, quantization);
}

Actor* NoteSkinColumn::get_player_tap(size_t type, size_t pn, double beat,
	bool active, bool reverse)
{
	GET_TAP_BODY(get_playerized, pn);
}

#undef GET_TAP_BODY

// Heads fall back to taps.  Since NoteSkinTapOptionalPart alternates between
// Head and Tail, an easy way to check whether a head is being fetched is to
// use type % 2.
#define GET_OPTIONAL_TAP_BODY(get_tap_func, get_func, quant_param) \
	ASSERT_M(type < m_optional_taps.size(), "Invalid NoteSkinTapOptionalPart type."); \
	auto* use_taps= &m_optional_taps; \
	if(reverse && !m_reverse_optional_taps.empty()) \
	{ \
		use_taps= &m_reverse_optional_taps; \
	} \
	QuantizedTap* tap= (*use_taps)[type]; \
	if(tap == nullptr) \
	{ \
		tap= (*use_taps)[type % 2]; \
	} \
	if(tap == nullptr) \
	{ \
		if(type % 2 == 0) \
		{ \
			return get_tap_func(NSTP_Tap, quant_param, beat, active, reverse); \
		} \
		return nullptr; \
	} \
	return tap->get_func(quant_param, beat, active);

Actor* NoteSkinColumn::get_optional_actor(size_t type,
	double quantization, double beat, bool active, bool reverse)
{
	GET_OPTIONAL_TAP_BODY(get_tap_actor, get_quantized, quantization);
}

Actor* NoteSkinColumn::get_player_optional_tap(size_t type, size_t pn,
	double beat, bool active, bool reverse)
{
	GET_OPTIONAL_TAP_BODY(get_player_tap, get_playerized, pn);
}

#undef GET_OPTIONAL_TAP_BODY

void NoteSkinColumn::get_hold_render_data(TapNoteSubType sub_type,
	NotePlayerizeMode playerize_mode, size_t pn, bool active, bool reverse,
	double quantization, double beat, QuantizedHoldRenderData& data)
{
	if(sub_type >= NUM_TapNoteSubType)
	{
		data.clear();
		return;
	}
	auto& hold_set= reverse ? m_reverse_holds : m_holds;
	auto& mask_set= reverse ? m_hold_reverse_player_masks : m_hold_player_masks;
	if(playerize_mode != NPM_Quanta)
	{
		hold_set[sub_type][active].get_quantized(quantization, beat, data);
	}
	else
	{
		hold_set[sub_type][active].get_playerized(pn, beat, data);
	}
	if(playerize_mode == NPM_Mask && !mask_set.empty())
	{
		data.mask= mask_set[sub_type];
	}
}

bool NoteSkinColumn::load_holds_from_lua(lua_State* L, int index,
	std::vector<std::vector<QuantizedHold> >& holder,
	std::string const& holds_name,
	NoteSkinLoader const* load_skin, std::string& insanity_diagnosis)
{
	string sub_sanity;
	int original_top= lua_gettop(L);
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, holds_name.c_str());
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No " + holds_name + " given.");
	}
	int holds_index= lua_gettop(L);
	holder.resize(NUM_TapNoteSubType);
	for(size_t part= 0; part < NUM_TapNoteSubType; ++part)
	{
		Enum::Push(L, static_cast<TapNoteSubType>(part));
		lua_gettable(L, holds_index);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Hold subtype %s not returned.", TapNoteSubTypeToString(static_cast<TapNoteSubType>(part)).c_str()));
		}
		int actives_index= lua_gettop(L);
		static const size_t num_active_states= 2;
		holder[part].resize(num_active_states);
		for(size_t a= 0; a < num_active_states; ++a)
		{
			lua_rawgeti(L, actives_index, a+1);
			if(!lua_istable(L, -1))
			{
				RETURN_NOT_SANE(fmt::sprintf("Hold info not given for active state %zu of subtype %s.", a, TapNoteSubTypeToString(static_cast<TapNoteSubType>(part)).c_str()));
			}
			if(!holder[part][a].load_from_lua(L, lua_gettop(L), load_skin, sub_sanity))
			{
				RETURN_NOT_SANE(fmt::sprintf("Error loading active state %zu of subtype %s: %s", a, TapNoteSubTypeToString(static_cast<TapNoteSubType>(part)).c_str(), sub_sanity.c_str()));
			}
		}
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool NoteSkinColumn::load_texs_from_lua(lua_State* L, int index,
	std::vector<RageTexture*>& dest,
	std::string const& texs_name,
	NoteSkinLoader const* load_skin, std::string& insanity_diagnosis)
{
	string sub_sanity;
	int original_top= lua_gettop(L);
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, texs_name.c_str());
	if(!lua_istable(L, -1))
	{
		// load_texs_from_lua is only used for optional player mask textures.
		return true;
		//RETURN_NOT_SANE("No " + texs_name + " textures given.");
	}
	int texs_index= lua_gettop(L);
	dest.resize(NUM_TapNoteSubType);
	for(size_t part= 0; part < NUM_TapNoteSubType; ++part)
	{
		Enum::Push(L, static_cast<TapNoteSubType>(part));
		lua_gettable(L, texs_index);
		if(!lua_isstring(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Texture entry for layer %zu is not a string.",
					part+1));
		}
		string path= lua_tostring(L, -1);
		if(path.empty())
		{
			RETURN_NOT_SANE("Empty texture path is not valid.");
		}
		dest[part]= load_noteskin_tex(path, load_skin);
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool load_tap_set_from_lua(lua_State* L, int taps_index, vector<QuantizedTap>& tap_set, string& insanity_diagnosis)
{
	int original_top= lua_gettop(L);
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	if(!lua_istable(L, taps_index))
	{
		RETURN_NOT_SANE("Tap set is not a table.");
	}
	string sub_sanity;
	tap_set.resize(NUM_NoteSkinTapPart);
	for(size_t part= NSTP_Tap; part < NUM_NoteSkinTapPart; ++part)
	{
		Enum::Push(L, static_cast<NoteSkinTapPart>(part));
		lua_gettable(L, taps_index);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Part %s not returned.",
					NoteSkinTapPartToString(static_cast<NoteSkinTapPart>(part)).c_str()));
		}
		if(!tap_set[part].load_from_lua(L, lua_gettop(L), sub_sanity))
		{
			RETURN_NOT_SANE(fmt::sprintf("Error loading part %s: %s",
					NoteSkinTapPartToString(static_cast<NoteSkinTapPart>(part)).c_str(),
					sub_sanity.c_str()));
		}
	}
#undef RETURN_NOT_SANE
	return true;
}

bool NoteSkinColumn::load_from_lua(lua_State* L, int index, NoteSkinLoader const* load_skin, string& insanity_diagnosis)
{
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, "taps");
	if(!lua_istable(L, -1))
	{
		RETURN_NOT_SANE("No taps given.");
	}
	int taps_index= lua_gettop(L);
	string sub_sanity;
	if(!load_tap_set_from_lua(L, taps_index, m_taps, sub_sanity))
	{
		RETURN_NOT_SANE(sub_sanity);
	}
	lua_settop(L, taps_index-1);
	lua_getfield(L, index, "reverse_taps");
	if(lua_istable(L, -1))
	{
		if(!load_tap_set_from_lua(L, lua_gettop(L), m_reverse_taps, sub_sanity))
		{
			RETURN_NOT_SANE(sub_sanity);
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, index, "optional_taps");
	int optional_taps_index= lua_gettop(L);
	// Leaving out the optional field is not an error.
	if(lua_istable(L, -1))
	{
		for(size_t part= NSTOP_HoldHead; part < NUM_NoteSkinTapOptionalPart; ++part)
		{
			Enum::Push(L, static_cast<NoteSkinTapOptionalPart>(part));
			lua_gettable(L, optional_taps_index);
			if(lua_istable(L, -1))
			{
				QuantizedTap* temp= new QuantizedTap;
				if(!temp->load_from_lua(L, lua_gettop(L), sub_sanity))
				{
					Rage::safe_delete(temp);
					temp= nullptr;
				}
				m_optional_taps[part]= temp;
			}
		}
	}
	lua_settop(L, optional_taps_index-1);
	lua_getfield(L, index, "reverse_optional_taps");
	int rev_opt_taps_index= lua_gettop(L);
	if(lua_istable(L, -1))
	{
		m_reverse_optional_taps.resize(NUM_NoteSkinTapOptionalPart);
		for(size_t part= NSTOP_HoldHead; part < NUM_NoteSkinTapOptionalPart; ++part)
		{
			Enum::Push(L, static_cast<NoteSkinTapOptionalPart>(part));
			lua_gettable(L, optional_taps_index);
			if(lua_istable(L, -1))
			{
				QuantizedTap* temp= new QuantizedTap;
				if(!temp->load_from_lua(L, lua_gettop(L), sub_sanity))
				{
					Rage::safe_delete(temp);
					temp= nullptr;
				}
				m_reverse_optional_taps[part]= temp;
			}
		}
	}
	lua_settop(L, rev_opt_taps_index-1);
	if(!load_holds_from_lua(L, index, m_holds, "holds", load_skin,
			insanity_diagnosis))
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	if(!load_holds_from_lua(L, index, m_reverse_holds, "reverse_holds", load_skin,
			insanity_diagnosis))
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	if(!load_texs_from_lua(L, index, m_hold_player_masks, "hold_masks", load_skin, insanity_diagnosis))
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	if(!load_texs_from_lua(L, index, m_hold_reverse_player_masks, "hold_reverse_masks", load_skin, insanity_diagnosis))
	{
		RETURN_NOT_SANE(insanity_diagnosis);
	}
	m_width= get_optional_double(L, index, "width", default_column_width);
	m_padding= get_optional_double(L, index, "padding", default_column_padding);
	m_use_custom_x= false;
	lua_getfield(L, index, "custom_x");
	if(lua_isnumber(L, -1))
	{
		m_use_custom_x= true;
		m_custom_x= lua_tonumber(L, -1);
	}
	lua_pop(L, 1);
	m_hold_gray_percent= get_optional_double(L, index, "hold_gray_percent", default_hold_gray_percent);
	m_anim_mult= get_optional_double(L, index, "anim_time", default_anim_time);
	if(m_anim_mult <= min_anim_time)
	{
		m_anim_mult= 1.0;
	}
	m_anim_mult= 1.0 / m_anim_mult;
	m_quantum_mult= get_optional_double(L, index, "quantum_time", default_quantum_time);
	if(m_quantum_mult <= min_quantum_time)
	{
		m_quantum_mult= 1.0;
	}
	m_quantum_mult= 1.0 / m_quantum_mult;
	m_anim_uses_beats= get_optional_bool(L, index, "anim_uses_beats", false);
	m_use_hold_heads_for_taps_on_row= get_optional_bool(L, index, "use_hold_heads_for_taps_on_row", false);
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool NoteSkinLayer::load_from_lua(lua_State* L, int index, size_t columns,
	std::string& insanity_diagnosis)
{
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	size_t num_columns= get_table_len(L, index, NoteSkinData::max_columns, "layer actors", insanity_diagnosis);
	if(num_columns != columns)
	{
		RETURN_NOT_SANE(fmt::sprintf("Invalid number of columns: %s", insanity_diagnosis.c_str()));
	}
	m_actors.resize(num_columns);
	for(size_t c= 0; c < num_columns; ++c)
	{
		lua_rawgeti(L, index, c+1);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE("Actor not found.");
		}
		std::unique_ptr<XNode> node(XmlFileUtil::XNodeFromTable(L));
		if(node.get() == nullptr)
		{
			RETURN_NOT_SANE("Actor not valid.");
		}
		Actor* act= ActorUtil::LoadFromNode(node.get(), nullptr);
		if(act == nullptr)
		{
			RETURN_NOT_SANE("Error loading actor.");
		}
		m_actors[c]= act;
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

NoteSkinData::NoteSkinData()
	:m_children_owned_by_field_now(false), m_loaded(false)
{
	
}

NoteSkinData::~NoteSkinData()
{
	clear();
}

void NoteSkinData::clear()
{
	// The taps are cleared by AutoActor deleting them.
	m_columns.clear();
	m_player_colors.clear();
	if(m_children_owned_by_field_now)
	{
		m_layers.clear();
		m_field_layers.clear();
		return;
	}
	for(auto&& layer : m_layers)
	{
		for(auto&& act : layer.m_actors)
		{
			delete act;
		}
	}
	m_layers.clear();
	for(auto&& act : m_field_layers)
	{
		delete act;
	}
	m_field_layers.clear();
}

void NoteSkinData::swap(NoteSkinData& other)
{
	m_layers.swap(other.m_layers);
	m_field_layers.swap(other.m_field_layers);
	m_player_colors.swap(other.m_player_colors);
	m_columns.swap(other.m_columns);
	m_skin_parameters.swap(other.m_skin_parameters);
	bool temp_loaded= m_loaded;
	m_loaded= other.m_loaded;
	other.m_loaded= temp_loaded;
}

bool NoteSkinData::load_taps_from_lua(lua_State* L, int index, size_t columns,
	NoteSkinLoader const* load_skin, string& insanity_diagnosis)
{
	//lua_pushvalue(L, index);
	//LuaHelpers::rec_print_table(L, "newskin_data", "");
	//lua_pop(L, 1);
	// Loading is atomic:  If a single error occurs during loading the data,
	// none of it is used.
	// Pop the table we're loading from off the stack when returning.
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	lua_getfield(L, index, "columns");
	size_t num_columns= get_table_len(L, -1, max_columns, "columns", insanity_diagnosis);
	if(num_columns != columns)
	{
		RETURN_NOT_SANE(fmt::sprintf("Invalid number of columns: %s", insanity_diagnosis.c_str()));
	}
	m_columns.resize(num_columns);
	int columns_index= lua_gettop(L);
	string sub_sanity;
	for(size_t c= 0; c < num_columns; ++c)
	{
		lua_rawgeti(L, columns_index, c+1);
		if(!lua_istable(L, -1))
		{
			RETURN_NOT_SANE(fmt::sprintf("Nothing given for column %zu.", c+1));
		}
		if(!m_columns[c].load_from_lua(L, lua_gettop(L), load_skin, sub_sanity))
		{
			RETURN_NOT_SANE(fmt::sprintf("Error loading column %zu: %s", c+1, sub_sanity.c_str()));
		}
	}
	lua_settop(L, columns_index-1);
	lua_getfield(L, index, "vivid_operation");
	if(lua_isboolean(L, -1))
	{
		bool vivid= lua_toboolean(L, -1);
		for(auto&& column : m_columns)
		{
			column.vivid_operation(vivid);
		}
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	m_loaded= true;
	return true;
}

void NoteSkinLoader::swap(NoteSkinLoader& other)
{
	m_skin_name.swap(other.m_skin_name);
	m_fallback_skin_name.swap(other.m_fallback_skin_name);
	m_load_path.swap(other.m_load_path);
	m_notes_loader.swap(other.m_notes_loader);
	m_layer_loaders.swap(other.m_layer_loaders);
	m_player_colors.swap(other.m_player_colors);
	m_field_layer_names.swap(other.m_field_layer_names);
	m_supported_buttons.swap(other.m_supported_buttons);
	m_skin_parameters.swap(other.m_skin_parameters);
	m_skin_parameter_info.swap(other.m_skin_parameter_info);
	bool temp_sup= m_supports_all_buttons;
	m_supports_all_buttons= other.m_supports_all_buttons;
	other.m_supports_all_buttons= temp_sup;
}

bool NoteSkinLoader::load_from_file(std::string const& path)
{
	if(!FILEMAN->IsAFile(path))
	{
		LuaHelpers::ReportScriptError("Noteskin '" + path + "' does not exist.");
		return false;
	}
	string temp_load_path= Rage::dir_name(path);
	std::string skin_text;
	if(!GetFileContents(path, skin_text))
	{
		LuaHelpers::ReportScriptError("Could not load noteskin '" + path + "'.");
		return false;
	}
	std::string error= "Error loading noteskin '" + path + "': ";
	Lua* L= LUA->Get();
	if(!LuaHelpers::RunScript(L, skin_text, "@" + path, error, 0, 1, true))
	{
		lua_settop(L, 0);
		LUA->Release(L);
		return false;
	}
	auto path_parts = Rage::split(path, "/");
	size_t name_index= 0;
	if(path_parts.size() > 1)
	{
		name_index= path_parts.size() - 2;
	}
	string sanity;
	if(!load_from_lua(L, lua_gettop(L), path_parts[name_index], temp_load_path,
			sanity))
	{
		LuaHelpers::ReportScriptError("Error loading noteskin '" + path + "': "
			+ sanity);
		lua_settop(L, 0);
		LUA->Release(L);
		return false;
	}
	lua_settop(L, 0);
	LUA->Release(L);
	return true;
}

bool NoteSkinLoader::load_from_lua(lua_State* L, int index, string const& name,
	string const& path, string& insanity_diagnosis)
{
	int original_top= lua_gettop(L) - 1;
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	if(!lua_istable(L, index))
	{
		RETURN_NOT_SANE("Noteskin data is not a table.");
	}
	lua_getfield(L, index, "buttons");
	// If there is no buttons table, it's not an error because a noteskin that
	// supports all buttons can consider it more convenient to just use the
	// supports_all_buttons flag.
	if(lua_istable(L, -1))
	{
		size_t num_buttons= lua_objlen(L, -1);
		for(size_t b= 0; b < num_buttons; ++b)
		{
			lua_rawgeti(L, -1, b+1);
			string button_name= lua_tostring(L, -1);
			m_supported_buttons.insert(button_name);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, index, "layers");
	if(lua_istable(L, -1))
	{
		string sub_sanity;
		if(!load_string_table(L, lua_gettop(L), max_layers, m_layer_loaders,
				"layers", sub_sanity))
		{
			RETURN_NOT_SANE("Error in layers table: " + sub_sanity);
		}
	}
	else
	{
		lua_pop(L, 1);
	}
	lua_getfield(L, index, "field_layers");
	if(lua_istable(L, -1))
	{
		string sub_sanity;
		if(!load_string_table(L, lua_gettop(L), max_layers, m_field_layer_names,
				"field_layers", sub_sanity))
		{
			RETURN_NOT_SANE("Error in field_layers table: " + sub_sanity);
		}
	}
	else
	{
		lua_pop(L, 1);
	}
	lua_getfield(L, index, "notes");
	if(!lua_isstring(L, -1))
	{
		RETURN_NOT_SANE("No notes loader found.");
	}
	m_notes_loader= lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, index, "fallback");
	if(lua_isstring(L, -1))
	{
		m_fallback_skin_name= lua_tostring(L, -1);
	}
	else
	{
		m_fallback_skin_name.clear();
	}
	lua_pop(L, 1);
	// The player_colors are optional.
	lua_getfield(L, index, "player_colors");
	int colors_index= lua_gettop(L);
	if(lua_istable(L, colors_index))
	{
		size_t num_colors= lua_objlen(L, colors_index);
		m_player_colors.resize(num_colors);
		for(size_t c= 0; c < num_colors; ++c)
		{
			lua_rawgeti(L, colors_index, c+1);
			FromStack(m_player_colors[c], L, lua_gettop(L));
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, index, "supports_all_buttons");
	m_supports_all_buttons= lua_toboolean(L, -1);
	lua_getfield(L, index, "skin_parameters");
	m_skin_parameters.SetFromStack(L);
	lua_getfield(L, index, "skin_parameter_info");
	m_skin_parameter_info.SetFromStack(L);
	lua_settop(L, original_top);
#undef RETURN_NOT_SANE
	m_skin_name= name;
	m_load_path= path;
	return true;
}

// TODO:  Move the button lists for stepstypes to lua data files.
// This hardcoded list is just temporary so that noteskins can be made and
// tested while other areas are under construction.  My plan is to get rid of
// styles and move all stepstype data to lua files to be loaded at startup.
static vector<vector<string> > button_lists = {
// StepsType_dance_single,
	{"Left", "Down", "Up", "Right"},
// StepsType_dance_double,
	{"Left", "Down", "Up", "Right", "Left", "Down", "Up", "Right"},
// StepsType_dance_couple,
	{"Left", "Down", "Up", "Right", "Left", "Down", "Up", "Right"},
// StepsType_dance_solo,
	{"Left", "UpLeft", "Down", "Up", "UpRight", "Right"},
// StepsType_dance_threepanel,
	{"UpLeft", "Down", "UpRight"},
// StepsType_dance_routine,
	{"Left", "Down", "Up", "Right", "Left", "Down", "Up", "Right"},
// StepsType_pump_single,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_pump_halfdouble,
	{"Center", "UpRight", "DownRight", "DownLeft", "UpLeft", "Center"},
// StepsType_pump_double,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight", "DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_pump_couple,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight", "DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_pump_routine,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight", "DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_kb7_single,
	{"Key1", "Key2", "Key3", "Key4", "Key5", "Key6", "Key7"},
	// ez2 buttons are probably wrong because the button mapping logic in Style
	// is too convoluted.
// StepsType_ez2_single,
	{"FootUpLeft", "HandUpLeft", "FootDown", "HandUpRight", "FootUpRight"},
// StepsType_ez2_double,
	{"FootUpLeft", "HandUpLeft", "FootDown", "HandUpRight", "FootUpRight", "FootUpLeft", "HandUpLeft", "FootDown", "HandUpRight", "FootUpRight"},
// StepsType_ez2_real,
	{"FootUpLeft", "HandLrLeft", "HandUpLeft", "FootDown", "HandUpRight", "HandLrRight", "FootUpRight"},
// StepsType_para_single,
	{"ParaLeft", "ParaUpLeft", "ParaUp", "ParaUpRight", "ParaRight"},
// StepsType_ds3ddx_single,
	{"HandLeft", "FootDownLeft", "FootUpLeft", "HandUp", "HandDown", "FootUpRight", "FootDownRight", "HandRight"},
// StepsType_beat_single5,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5"},
// StepsType_beat_versus5,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5"},
// StepsType_beat_double5,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5", "Key5", "Key4", "Key3", "Key2", "Key1", "scratch"},
// StepsType_beat_single7,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5", "Key6", "Key7"},
// StepsType_beat_versus7,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5", "Key6", "Key7"},
// StepsType_beat_double7,
	{"scratch", "Key1", "Key2", "Key3", "Key4", "Key5", "Key6", "Key7", "Key7", "Key6", "Key5", "Key4", "Key3", "Key2", "Key1", "scratch"},
// StepsType_maniax_single,
	{"HandLrLeft", "HandUpLeft", "HandUpRight", "HandLrRight"},
// StepsType_maniax_double,
	{"HandLrLeft", "HandUpLeft", "HandUpRight", "HandLrRight", "HandLrLeft", "HandUpLeft", "HandUpRight", "HandLrRight"},
// StepsType_techno_single4,
	{"Left", "Down", "Up", "Right"},
// StepsType_techno_single5,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_techno_single8,
	{"DownLeft", "Left", "UpLeft", "Down", "Up", "UpRight", "Right", "DownRight"},
// StepsType_techno_double4,
	{"Left", "Down", "Up", "Right", "Left", "Down", "Up", "Right"},
// StepsType_techno_double5,
	{"DownLeft", "UpLeft", "Center", "UpRight", "DownRight", "DownLeft", "UpLeft", "Center", "UpRight", "DownRight"},
// StepsType_techno_double8,
	{"DownLeft", "Left", "UpLeft", "Down", "Up", "UpRight", "Right", "DownRight", "DownLeft", "Left", "UpLeft", "Down", "Up", "UpRight", "Right", "DownRight"},
// StepsType_popn_five,
	{"Left Green", "Left Blue", "Red", "Right Blue", "Right Green"},
// StepsType_popn_nine,
	{"Left White", "Left Yellow", "Left Green", "Left Blue", "Red", "Right Blue", "Right Green", "Right Yellow", "Right White"},
// StepsType_lights_cabinet,
	{"MarqueeUpLeft", "MarqueeUpRight", "MarqueeLrLeft", "MarqueeLrRight", "ButtonsLeft", "ButtonsRight", "BassLeft", "BassRight"},
// StepsType_kickbox_human,
	{"LeftFoot", "LeftFist", "RightFist", "RightFoot"},
// StepsType_kickbox_quadarm,
	{"UpLeftFist", "DownLeftFist", "DownRightFist", "UpRightFist"},
// StepsType_kickbox_insect,
	{"LeftFoot", "UpLeftFist", "DownLeftFist", "DownRightFist", "UpRightFist", "RightFoot"},
// StepsType_kickbox_arachnid,
	{"DownLeftFoot", "UpLeftFoot", "UpLeftFist", "DownLeftFist", "DownRightFist", "UpRightFist", "UpRightFoot", "DownRightFoot"}
};

bool NoteSkinLoader::supports_needed_buttons(StepsType stype, bool disable_supports_all) const
{
	if(m_supports_all_buttons && !disable_supports_all)
	{
		return true;
	}
	std::vector<std::string> const& button_list= button_lists[stype];
	for(auto&& button_name : button_list)
	{
		if(m_supported_buttons.find(button_name) == m_supported_buttons.end())
		{
			return false;
		}
	}
	return true;
}

bool NoteSkinLoader::push_loader_function(lua_State* L, string const& loader) const
{
	if(loader.empty())
	{
		return false;
	}
	string found_path= NOTESKIN->get_path(this, loader);
	if(found_path.empty())
	{
		LuaHelpers::ReportScriptError("Noteskin " + m_skin_name + " points to a"
			" loader file that does not exist: " + loader);
		return false;
	}
	std::string script_text;
	if(!GetFileContents(found_path, script_text))
	{
		LuaHelpers::ReportScriptError("Noteskin " + m_skin_name + " points to a"
			" loader file " + found_path + " could not be loaded.");
		return false;
	}
	std::string error= "Error loading " + found_path + ": ";
	if(!LuaHelpers::RunScript(L, script_text, "@" + found_path, error, 0, 1, true))
	{
		return false;
	}
	return true;
}

bool NoteSkinLoader::load_layer_set_into_data(lua_State* L,
	LuaReference& skin_params, int button_list_index, int stype_index,
	size_t columns, vector<string> const& loader_set,
	vector<NoteSkinLayer>& dest, string& insanity_diagnosis) const
{
	int original_top= lua_gettop(L);
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); insanity_diagnosis= message; return false;
	dest.resize(loader_set.size());
	string sub_sanity;
	for(size_t i= 0; i < loader_set.size(); ++i)
	{
		if(!push_loader_function(L, loader_set[i]))
		{
			RETURN_NOT_SANE("Could not load loader " + loader_set[i]);
		}
		std::string error= "Error running " + m_load_path + loader_set[i] + ": ";
		lua_pushvalue(L, button_list_index);
		lua_pushvalue(L, stype_index);
		skin_params.PushSelf(L);
		if(!LuaHelpers::RunScriptOnStack(L, error, 3, 1, true))
		{
			RETURN_NOT_SANE("Error running loader " + loader_set[i]);
		}
		if(!dest[i].load_from_lua(L, lua_gettop(L), columns, sub_sanity))
		{
			RETURN_NOT_SANE("Error in layer data: " + sub_sanity);
		}
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	return true;
}

bool NoteSkinLoader::load_into_data(StepsType stype,
	LuaReference& skin_params, NoteSkinData& dest, string& insanity_diagnosis) const
{
	vector<string> const& button_list= button_lists[stype];
	Lua* L= LUA->Get();
	int original_top= lua_gettop(L);
	sanitize_skin_parameters(L, skin_params);
#define RETURN_NOT_SANE(message) lua_settop(L, original_top); LUA->Release(L); insanity_diagnosis= message; return false;
	LuaThreadVariable skin_var("skin_name", m_skin_name);
	lua_createtable(L, button_list.size(), 0);
	for(size_t b= 0; b < button_list.size(); ++b)
	{
		lua_pushstring(L, button_list[b].c_str());
		lua_rawseti(L, -2, b+1);
	}
	int button_list_index= lua_gettop(L);
	Enum::Push(L, stype);
	int stype_index= lua_gettop(L);
	if(!push_loader_function(L, m_notes_loader))
	{
		RETURN_NOT_SANE("Could not load tap loader.");
	}
	std::string error= "Error running " + m_load_path + m_notes_loader + ": ";
	lua_pushvalue(L, button_list_index);
	lua_pushvalue(L, stype_index);
	skin_params.PushSelf(L);
	if(!LuaHelpers::RunScriptOnStack(L, error, 3, 1, true))
	{
		RETURN_NOT_SANE("Error running loader for notes.");
	}
	string sub_sanity;
	if(!dest.load_taps_from_lua(L, lua_gettop(L), button_list.size(), this, sub_sanity))
	{
		RETURN_NOT_SANE("Invalid data from loader: " + sub_sanity);
	}
	if(!load_layer_set_into_data(L, skin_params, button_list_index, stype_index, button_list.size(), m_layer_loaders,
			dest.m_layers, sub_sanity))
	{
		RETURN_NOT_SANE("Error running layer loaders: " + sub_sanity);
	}
	dest.m_field_layers.reserve(m_field_layer_names.size());
	for(size_t lid= 0; lid < m_field_layer_names.size(); ++lid)
	{
		if(!push_loader_function(L, m_field_layer_names[lid]))
		{
			RETURN_NOT_SANE("Could not load loader " + m_field_layer_names[lid]);
		}
		std::string error= "Error running " + m_load_path + m_field_layer_names[lid] + ": ";
		lua_pushvalue(L, button_list_index);
		lua_pushvalue(L, stype_index);
		skin_params.PushSelf(L);
		if(!LuaHelpers::RunScriptOnStack(L, error, 3, 1, true))
		{
			RETURN_NOT_SANE("Error running loader " + m_field_layer_names[lid]);
		}
		std::unique_ptr<XNode> node(XmlFileUtil::XNodeFromTable(L));
		if(node.get() == nullptr)
		{
			RETURN_NOT_SANE("Actor not valid.");
		}
		Actor* act= ActorUtil::LoadFromNode(node.get(), nullptr);
		if(act == nullptr)
		{
			RETURN_NOT_SANE("Error loading actor.");
		}
		dest.m_field_layers.push_back(act);
	}
#undef RETURN_NOT_SANE
	lua_settop(L, original_top);
	LUA->Release(L);
	dest.m_player_colors= m_player_colors;
	return true;
}

void NoteSkinLoader::recursive_sanitize_skin_parameters(lua_State* L,
	std::unordered_set<void const*>& visited_tables, int curr_depth,
	int curr_param_set_info, int curr_param_set_defaults,
	int curr_param_set_dest) const
{
	// max_depth is a protection against someone creating a pathologically deep
	// table of parameters that could cause a stack overflow.
	static const int max_depth= 20;
	lua_pushnil(L);
	while(lua_next(L, curr_param_set_defaults) != 0)
	{
		// stack: field_key, field_default
		int field_default= lua_gettop(L);
		int field_key= field_default - 1;
#define SAFE_CONTINUE lua_settop(L, field_key); continue;
		int key_type= lua_type(L, field_key);
		int field_type= lua_type(L, field_default);
		lua_pushvalue(L, field_key);
		lua_gettable(L, curr_param_set_info);
		int field_info= lua_gettop(L);
		if(lua_type(L, field_info) != LUA_TTABLE)
		{
			// Push a copy of the key onto the stack so that lua_tostring
			// won't affect the key being used to iterate through the table.
			lua_pushvalue(L, field_key);
			LuaHelpers::ReportScriptErrorFmt("Field %s in noteskin parameters has invalid type info.", lua_tostring(L, -1));
			SAFE_CONTINUE;
		}
		// Ignore stuff that doesn't have a string or number index.
		if(key_type != LUA_TSTRING && key_type != LUA_TNUMBER)
		{
			SAFE_CONTINUE;
		}
		switch(field_type)
		{
			case LUA_TTABLE:
				{
					void const* table= lua_topointer(L, field_default);
					bool visited= (visited_tables.find(table) != visited_tables.end());
					if(visited)
					{
						SAFE_CONTINUE;
					}
					if(curr_depth >= max_depth)
					{
						LuaHelpers::ReportScriptError("Noteskin parameter table exceeded maximum depth, ignoring deeper parameters.");
						SAFE_CONTINUE;
					}
					lua_pushvalue(L, field_key); // stack: field_key, field_default, field_info, field_key
					lua_gettable(L, curr_param_set_dest); // stack: field_key, field_default, field_info, next_param_set_dest
					int next_param_set_dest= lua_gettop(L);
					if(lua_type(L, next_param_set_dest) != LUA_TTABLE)
					{
						lua_pop(L, 1); // stack: field_key, field_default, field_info
						lua_pushvalue(L, field_key); // stack: field_key, field_default, field_info, field_key
						lua_newtable(L); // stack: field_key, field_default, field_info, field_key, table
						lua_settable(L, curr_param_set_dest); // stack: field_key, field_default, field_info
						lua_pushvalue(L, field_key); // stack: field_key, field_default, field_info, field_key
						lua_gettable(L, curr_param_set_dest); // stack: field_key, field_default, field_info, next_param_set_dest
						next_param_set_dest= lua_gettop(L);
					}
					visited_tables.insert(table);
					recursive_sanitize_skin_parameters(L, visited_tables, curr_depth+1,
						field_info, field_default, next_param_set_dest);
					// Pop next_param_set_dest.
					lua_pop(L, 1); // stack: field_key, field_default, field_info
				}
				break;
			case LUA_TSTRING:
			case LUA_TNUMBER:
			case LUA_TBOOLEAN:
				{
#define SET_FIELD_TO_DEFAULT \
	lua_pushvalue(L, field_key); \
	lua_pushvalue(L, field_default); \
	lua_settable(L, curr_param_set_dest);
					lua_pushvalue(L, field_key);
					lua_gettable(L, curr_param_set_dest);
					int dest_value= lua_gettop(L);
					if(lua_type(L, dest_value) != field_type)
					{
						SET_FIELD_TO_DEFAULT;
					}
					else if(field_type != LUA_TBOOLEAN)
					{
						lua_getfield(L, field_info, "choices");
						// The choices table is optional, if it exists, the field must be
						// one of the choices in it.
						int choices_table= lua_gettop(L);
						if(lua_type(L, choices_table) == LUA_TTABLE)
						{
							if(!value_is_in_table(L, dest_value, choices_table))
							{
								SET_FIELD_TO_DEFAULT;
							}
						}
						else
						{
							lua_getfield(L, field_info, "min");
							int min_value= lua_gettop(L);
							lua_getfield(L, field_info, "max");
							int max_value= lua_gettop(L);
							if(lua_type(L, min_value) == field_type && lua_type(L, max_value) == field_type)
							{
								bool in_range= true;
								if(lua_lessthan(L, dest_value, min_value))
								{
									in_range= false;
								}
								else if(lua_lessthan(L, max_value, dest_value))
								{
									in_range= false;
								}
								if(!in_range)
								{
									SET_FIELD_TO_DEFAULT;
								}
							}
							// Pop the min and max values.
							lua_pop(L, 2);
						}
						// Pop the choices table.
						lua_pop(L, 1);
					}
					lua_pop(L, 1);
#undef SET_FIELD_TO_DEFAULT
				}
				break;
		}
		lua_settop(L, field_key);
#undef SAFE_CONTINUE
	}
}

void NoteSkinLoader::sanitize_skin_parameters(lua_State* L, LuaReference& params) const
{
	if(m_skin_parameter_info.IsNil() || m_skin_parameters.IsNil())
	{
		params.SetFromNil();
		return;
	}
	if(params.GetLuaType() != LUA_TTABLE)
	{
		m_skin_parameters.PushSelf(L);
		params.SetFromStack(L);
		params.DeepCopy();
		return;
	}
	m_skin_parameter_info.PushSelf(L);
	int curr_param_set_info= lua_gettop(L);
	if(lua_type(L, curr_param_set_info) != LUA_TTABLE)
	{
		LuaHelpers::ReportScriptError("Noteskin parameters info must be a table.");
		lua_pop(L, 1);
		m_skin_parameters.PushSelf(L);
		params.SetFromStack(L);
		params.DeepCopy();
		return;
	}
	m_skin_parameters.PushSelf(L);
	int curr_param_set_defaults= lua_gettop(L);
	params.PushSelf(L);
	int curr_param_set_dest= lua_gettop(L);
	std::unordered_set<void const*> visited_tables;
	recursive_sanitize_skin_parameters(L, visited_tables, 0,
		curr_param_set_info, curr_param_set_defaults, curr_param_set_dest);
}

void NoteSkinLoader::push_skin_parameter_info(lua_State* L) const
{
	if(m_skin_parameter_info.IsNil())
	{
		lua_pushnil(L);
		return;
	}
	// Make a copy of the parameter info so the theme can't corrupt it.
	LuaReference param_info= m_skin_parameter_info;
	param_info.DeepCopy();
	param_info.PushSelf(L);
}

void NoteSkinLoader::push_skin_parameter_defaults(lua_State* L) const
{
	if(m_skin_parameters.IsNil())
	{
		lua_pushnil(L);
		return;
	}
	// Make a copy of the parameter defaults so the theme can't corrupt it.
	LuaReference param_info= m_skin_parameters;
	param_info.DeepCopy();
	param_info.PushSelf(L);
}
