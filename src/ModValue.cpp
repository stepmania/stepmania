#include "global.h"
#include "CubicSpline.h"
#include "EnumHelper.h"
#include "ModValue.h"
#include "RageLog.h"
#include "RageMath.h"
#include "LuaBinding.h"

using std::vector;

/*
static const vector<string> mod_names= {
	"alternate", "beat", "blind", "blink", "boomerang", "boost", "brake",
	"bumpy", "centered", "confusion", "cover", "cross", "dark", "dizzy",
	"drunk", "expand", "flip", "hidden", "hidden_offset", "invert",
	"max_scroll_bpm", "mini", "no_attack", "pass_mark", "player_autoplayer",
	"rand_attack", "random_speed", "random_vanish", "reverse", "roll",
	"scroll_speed", "skew", "split", "stealth", "sudden", "sudden_offset",
	"tilt", "time_spacing", "tiny", "tipsy", "tornado", "twirl", "wave",
	"xmode"
};
*/

static const char* ModInputTypeNames[] = {
	"Scalar",
	"EvalBeat",
	"EvalSecond",
	"MusicBeat",
	"MusicSecond",
	"DistBeat",
	"DistSecond",
	"YOffset",
	"StartDistBeat",
	"StartDistSecond",
	"EndDistBeat",
	"EndDistSecond",
};
XToString(ModInputType);
LuaXType(ModInputType);

static const char* ModFunctionTypeNames[] = {
	"Constant",
	"Product",
	"Power",
	"Log",
	"Sine",
	"Tan",
	"Square",
	"Triangle",
	"Spline",
};
XToString(ModFunctionType);
LuaXType(ModFunctionType);

static const char* ModSumTypeNames[] = {
	"Add",
	"Subtract",
	"Multiply",
	"Divide",
};
XToString(ModSumType);
LuaXType(ModSumType);

void ModManager::update(double curr_beat, double curr_second)
{
	double const time_diff= curr_second - m_prev_curr_second;
	if(time_diff == 0)
	{
		return;
	}
	if(time_diff > 0)
	{
		// Time is moving forwards.
		for(auto fap= m_present_funcs.begin(); fap != m_present_funcs.end();)
		{
			auto this_fap= fap++;
			if(this_fap->func->m_end_second < curr_second)
			{
				insert_into_past(*this_fap);
				remove_from_present(this_fap);
			}
			else
			{
				break;
			}
		}
		for(auto fap= m_future_funcs.begin(); fap != m_future_funcs.end();)
		{
			auto this_fap= fap++;
			if(this_fap->func->m_start_second <= curr_second)
			{
				if(this_fap->func->m_end_second < curr_second)
				{
					insert_into_past(*this_fap);
				}
				else
				{
					insert_into_present(*this_fap);
				}
				m_future_funcs.erase(this_fap);
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// Time is moving backwards.
		for(auto fap= m_present_funcs.begin(); fap != m_present_funcs.end();)
		{
			auto this_fap= fap++;
			if(this_fap->func->m_end_second < curr_second)
			{
				insert_into_past(*this_fap);
				remove_from_present(this_fap);
			}
			else if(this_fap->func->m_start_second > curr_second)
			{
				insert_into_future(*this_fap);
				remove_from_present(this_fap);
			}
		}
		for(auto fap= m_past_funcs.begin(); fap != m_past_funcs.end();)
		{
			auto this_fap= fap++;
			if(this_fap->func->m_end_second >= curr_second)
			{
				if(this_fap->func->m_start_second > curr_second)
				{
					insert_into_future(*this_fap);
				}
				else
				{
					insert_into_present(*this_fap);
				}
				m_past_funcs.erase(this_fap);
			}
			else
			{
				break;
			}
		}
	}
	m_prev_curr_second= curr_second;
	if(!m_per_frame_update_funcs.empty())
	{
		mod_val_inputs input(curr_beat, curr_second);
		for(auto&& func : m_per_frame_update_funcs)
		{
			func->per_frame_update(input);
		}
	}
}

void ModManager::add_mod(ModFunction* func, ModifiableValue* parent)
{
	if(func->m_start_second > m_prev_curr_second)
	{
		insert_into_future(func, parent);
	}
	else if(func->m_end_second < m_prev_curr_second)
	{
		insert_into_past(func, parent);
	}
	else
	{
		insert_into_present(func, parent);
	}
}

void ModManager::remove_mod(ModFunction* func)
{
	for(auto&& managed_list : {&m_past_funcs, &m_present_funcs, &m_future_funcs})
	{
		for(auto fap= managed_list->begin(); fap != managed_list->end();)
		{
			auto this_fap= fap++;
			if(this_fap->func == func)
			{
				managed_list->erase(this_fap);
			}
		}
	}
}

void ModManager::remove_all_mods(ModifiableValue* parent)
{
	for(auto&& managed_list : {&m_past_funcs, &m_present_funcs, &m_future_funcs})
	{
		for(auto fap= managed_list->begin(); fap != managed_list->end();)
		{
			auto this_fap= fap++;
			if(this_fap->parent == parent)
			{
				managed_list->erase(this_fap);
			}
		}
	}
}

void ModManager::add_to_per_frame_update(ModFunction* func)
{
	if(func->needs_per_frame_update())
	{
		m_per_frame_update_funcs.insert(func);
	}
}

void ModManager::remove_from_per_frame_update(ModFunction* func)
{
	auto entry= m_per_frame_update_funcs.find(func);
	if(entry != m_per_frame_update_funcs.end())
	{
		m_per_frame_update_funcs.erase(entry);
	}
}

void ModManager::dump_list_status()
{
	LOG->Trace("ModManager::dump_list_status:");
	for(auto&& managed_list : {&m_past_funcs, &m_present_funcs, &m_future_funcs})
	{
		for(auto fap= managed_list->begin(); fap != managed_list->end(); ++fap)
		{
			LOG->Trace("%f, %f : %f, %f", fap->func->m_start_beat, fap->func->m_start_second, fap->func->m_end_beat, fap->func->m_end_second);
		}
		LOG->Trace("list over");
	}
}

void ModManager::insert_into_past(ModFunction* func, ModifiableValue* parent)
{
	// m_past_funcs is sorted in descending end second order.  Entries with the
	// same end second are sorted in undefined order.
	// This way, when time flows backwards, traversing from beginning to end
	// gives the entries that should go into present.
	// When time flows forwards, this ends up being inserting at the front.
	for(auto fap= m_past_funcs.begin(); fap != m_past_funcs.end(); ++fap)
	{
		if(fap->func->m_end_second < func->m_end_second)
		{
			m_past_funcs.insert(fap, func_and_parent(func, parent));
			return;
		}
	}
	m_past_funcs.push_back(func_and_parent(func, parent));
}

void ModManager::insert_into_present(ModFunction* func, ModifiableValue* parent)
{
	add_to_per_frame_update(func);
	parent->add_mod_to_active_list(func);
	// m_present_funcs is sorted in ascending end second order.  Entries with
	// the same end second are sorted in ascending start second order.
	for(auto fap= m_present_funcs.begin(); fap != m_present_funcs.end(); ++fap)
	{
		if(fap->func->m_end_second > func->m_end_second ||
			(fap->func->m_end_second == func->m_end_second &&
				fap->func->m_start_second > func->m_start_second))
		{
			m_present_funcs.insert(fap, func_and_parent(func, parent));
			return;
		}
	}
	m_present_funcs.push_back(func_and_parent(func, parent));
}

void ModManager::insert_into_future(ModFunction* func, ModifiableValue* parent)
{
	// m_future_funcs is sorted in ascending start second order.  Entries with
	// the same start second are sorted in undefined order.
	for(auto fap= m_future_funcs.begin(); fap != m_future_funcs.end(); ++fap)
	{
		if(fap->func->m_start_second > func->m_start_second)
		{
			m_future_funcs.insert(fap, func_and_parent(func, parent));
			return;
		}
	}
	m_future_funcs.push_back(func_and_parent(func, parent));
}

void ModManager::remove_from_present(std::list<func_and_parent>::iterator fapi)
{
	remove_from_per_frame_update(fapi->func);
	fapi->parent->remove_mod_from_active_list(fapi->func);
	m_present_funcs.erase(fapi);
}

void ModInput::init_simple(ModFunction* parent, ModInputType input_type,
	double input_scalar)
{
	clear();
	m_parent= parent;
	set_type(input_type);
	m_scalar= input_scalar;
}

void ModInput::clear()
{
	m_type= MIT_Scalar;
	m_scalar= 0.0;
	m_offset= 0.0;
	m_rep_begin= 0.0;
	m_rep_end= 0.0;
	m_phases.clear();
}

static void get_numbers(lua_State* L, int index, vector<double*> const& ret)
{
	for(size_t i= 0; i < ret.size(); ++i)
	{
		lua_rawgeti(L, index, i+1);
		(*ret[i]) = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
}

static void push_numbers(lua_State* L, vector<double*> const& noms)
{
	lua_createtable(L, noms.size(), 0);
	for(size_t i= 0; i < noms.size(); ++i)
	{
		lua_pushnumber(L, *noms[i]);
		lua_rawseti(L, -2, i+1);
	}
}

void ModInput::push_phase(lua_State* L, size_t phase)
{
	push_numbers(L, {&m_phases[phase].start, &m_phases[phase].finish,
					&m_phases[phase].mult, &m_phases[phase].offset});
}

void ModInput::push_def_phase(lua_State* L)
{
	push_numbers(L, {&m_default_phase.start, &m_default_phase.finish,
					&m_default_phase.mult, &m_default_phase.offset});
}

void ModInput::load_rep(lua_State* L, int index)
{
	if(lua_istable(L, index))
	{
		rep_apple= &ModInput::apply_rep;
		get_numbers(L, index, {&m_rep_begin, &m_rep_end});
	}
	else
	{
		rep_apple= nullptr;
	}
}

void ModInput::load_one_phase(lua_State* L, int index, size_t phase)
{
	if(lua_istable(L, index))
	{
		get_numbers(L, index, {&m_phases[phase].start, &m_phases[phase].finish,
					&m_phases[phase].mult, &m_phases[phase].offset});
	}
}

void ModInput::load_def_phase(lua_State* L, int index)
{
	if(lua_istable(L, index))
	{
		get_numbers(L, index, {&m_default_phase.start, &m_default_phase.finish,
					&m_default_phase.mult, &m_default_phase.offset});
	}
}

void ModInput::load_phases(lua_State* L, int index)
{
	if(lua_istable(L, index))
	{
		enable_phases();
		lua_getfield(L, index, "default");
		load_def_phase(L, lua_gettop(L));
		lua_pop(L, 1);
		size_t num_phases= lua_objlen(L, index);
		m_phases.resize(num_phases);
		for(size_t i= 0; i < num_phases; ++i)
		{
			lua_rawgeti(L, index, i+1);
			load_one_phase(L, lua_gettop(L), i);
			lua_pop(L, 1);
		}
		sort_phases();
	}
}

bool compare_phases(ModInput::phase left, ModInput::phase right)
{
	return left.start < right.start;
}

void ModInput::sort_phases()
{
	stable_sort(m_phases.begin(), m_phases.end(), compare_phases);
}

void ModInput::load_spline(lua_State* L, int index)
{
	if(lua_istable(L, index))
	{
		enable_spline();
		m_loop_spline= get_optional_bool(L, index, "loop");
		m_polygonal_spline= get_optional_bool(L, index, "polygonal");
		size_t num_points= lua_objlen(L, index);
		m_spline.resize(num_points);
		for(size_t p= 0; p < num_points; ++p)
		{
			lua_rawgeti(L, index, p+1);
			m_spline.set_point(p, static_cast<float>(lua_tonumber(L, -1)));
			lua_pop(L, 1);
		}
		m_spline.solve(m_loop_spline, m_polygonal_spline);
	}
}

void ModInput::load_from_lua(lua_State* L, int index, ModFunction* parent)
{
	m_parent= parent;
	if(lua_isnumber(L, index))
	{
		set_type(MIT_Scalar);
		m_scalar= lua_tonumber(L, index);
		return;
	}
	if(lua_istable(L, index))
	{
		lua_rawgeti(L, index, 1);
		set_type(Enum::Check<ModInputType>(L, -1));
		lua_pop(L, 1);
		// The use of lua_tonumber is deliberate.  If the scalar or offset value
		// does not exist, lua_tonumber will return 0.
		lua_rawgeti(L, index, 2);
		m_scalar= lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);
		m_offset= lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, index, "rep");
		load_rep(L, lua_gettop(L));
		lua_pop(L, 1);
		lua_getfield(L, index, "phases");
		load_phases(L, lua_gettop(L));
		lua_pop(L, 1);
		lua_getfield(L, index, "spline");
		load_spline(L, lua_gettop(L));
		lua_pop(L, 1);
	}
}

void ModInput::set_type(ModInputType t)
{
	ModInputMetaType old_meta_type= get_meta_type();
	m_type= t;
	switch(m_type)
	{
		case MIT_Scalar:
			choice= &mod_val_inputs::scalar;
			break;
		case MIT_EvalBeat:
			choice= &mod_val_inputs::eval_beat;
			break;
		case MIT_EvalSecond:
			choice= &mod_val_inputs::eval_second;
			break;
		case MIT_MusicBeat:
			choice= &mod_val_inputs::music_beat;
			break;
		case MIT_MusicSecond:
			choice= &mod_val_inputs::music_second;
			break;
		case MIT_DistBeat:
			choice= &mod_val_inputs::dist_beat;
			break;
		case MIT_DistSecond:
			choice= &mod_val_inputs::dist_second;
			break;
		case MIT_YOffset:
			choice= &mod_val_inputs::y_offset;
			break;
		case MIT_StartDistBeat:
			choice= &mod_val_inputs::start_dist_beat;
			break;
		case MIT_StartDistSecond:
			choice= &mod_val_inputs::start_dist_second;
			break;
		case MIT_EndDistBeat:
			choice= &mod_val_inputs::end_dist_beat;
			break;
		case MIT_EndDistSecond:
			choice= &mod_val_inputs::end_dist_second;
			break;
		default:
			break;
	}
	ModInputMetaType new_meta_type= get_meta_type();
	if(m_parent != nullptr && old_meta_type != new_meta_type)
	{
		m_parent->recategorize(this, old_meta_type, new_meta_type);
	}
}

ModInput::phase const* ModInput::find_phase(double input)
{
	if(m_phases.empty() || input < m_phases.front().start || input >= m_phases.back().finish)
	{
		return &m_default_phase;
	}
	// Every time I have to do a binary search, there's some odd wrinkle that
	// forces the implementation to be different.  In this case, input is not
	// guaranteed to be in phase.  For example, if the phase ranges are [0, 1),
	// [2, 3), and the input is 1.5, then no phase should be applied.
	size_t lower= 0;
	size_t upper= m_phases.size()-1;
	if(input < m_phases[lower].finish)
	{
		return &m_phases[lower];
	}
	if(input >= m_phases[upper].start)
	{
		return &m_phases[upper];
	}
	while(lower != upper)
	{
		size_t mid= (upper + lower) / 2;
		if(input < m_phases[mid].start)
		{
			if(mid > lower)
			{
				if(input >= m_phases[mid-1].finish)
				{
					return &m_default_phase;
				}
			}
			else
			{
				return &m_default_phase;
			}
			upper= mid;
		}
		else if(input >= m_phases[mid].finish)
		{
			// mid is mathematically guaranteed to be less than upper.
			if(input < m_phases[mid+1].start)
			{
				return &m_default_phase;
			}
			lower= mid;
		}
		else
		{
			return &m_phases[mid];
		}
	}
	return &m_phases[lower];
}

double ModInput::apply_rep(double input)
{
	double const dist= m_rep_end - m_rep_begin;
	double const mod_res= fmod(input, dist);
	if(mod_res < 0.0)
	{
		return mod_res + dist + m_rep_begin;
	}
	return mod_res + m_rep_begin;
}

void ModInput::send_repick()
{
	if(get_meta_type() == MIMT_Scalar && m_parent != nullptr)
	{
		m_parent->repick(this);
	}
}

void ModInput::send_spline_repick()
{
	m_spline.solve(m_loop_spline, m_polygonal_spline);
	send_repick();
}

void ModInput::set_scalar(double s)
{
	m_scalar= s;
	send_repick();
}

void ModInput::set_offset(double s)
{
	m_offset= s;
	send_repick();
}

void ModInput::push_rep(lua_State* L)
{
	push_numbers(L, {&m_rep_begin, &m_rep_end});
}

void ModInput::push_all_phases(lua_State* L)
{
	lua_createtable(L, m_phases.size(), 0);
	for(size_t i= 0; i < m_phases.size(); ++i)
	{
		push_phase(L, i);
		lua_rawseti(L, -2, i+1);
	}
}

void ModInput::enable_phases()
{
	phase_apple= &ModInput::apply_phase;
	send_repick();
}

void ModInput::disable_phases()
{
	phase_apple= nullptr;
	send_repick();
}

void ModInput::check_disable_phases()
{
	if(m_phases.empty() && m_default_phase.mult == 1.0 && m_default_phase.offset == 0.0)
	{
		disable_phases();
	}
}

void ModInput::remove_phase(size_t phase)
{
	if(phase < m_phases.size())
	{
		m_phases.erase(m_phases.begin() + phase);
	}
	check_disable_phases();
	send_repick();
}

void ModInput::clear_phases()
{
	m_phases.clear();
	check_disable_phases();
	send_repick();
}

void ModInput::enable_spline()
{
	spline_apple= &ModInput::apply_spline;
	send_repick();
}

void ModInput::disable_spline()
{
	spline_apple= nullptr;
	send_repick();
}

void ModInput::set_spline_loop(bool b)
{
	m_loop_spline= b;
	send_spline_repick();
}

void ModInput::set_spline_polygonal(bool b)
{
	m_polygonal_spline= b;
	m_spline.solve(m_loop_spline, m_polygonal_spline);
	send_spline_repick();
}

double ModInput::get_spline_point(size_t p)
{
	float as_t= static_cast<float>(p);
	return m_spline.evaluate(as_t, m_loop_spline);
}

void ModInput::set_spline_point(size_t p, double value)
{
	if(p < m_spline.size())
	{
		m_spline.set_point(p, static_cast<float>(value));
		m_spline.solve(m_loop_spline, m_polygonal_spline);
		send_spline_repick();
	}
}

void ModInput::add_spline_point(double value)
{
	m_spline.resize(m_spline.size()+1);
	m_spline.set_point(m_spline.size()-1, static_cast<float>(value));
	m_spline.solve(m_loop_spline, m_polygonal_spline);
	send_spline_repick();
}

void ModInput::remove_spline_point(size_t p)
{
	p= std::min(p, m_spline.size());
	m_spline.remove_point(p);
	m_spline.solve(m_loop_spline, m_polygonal_spline);
	send_spline_repick();
}

void ModInput::push_spline(lua_State* L)
{
	lua_createtable(L, m_spline.size(), 2);
	lua_pushboolean(L, m_loop_spline);
	lua_setfield(L, -2, "loop");
	lua_pushboolean(L, m_polygonal_spline);
	lua_setfield(L, -2, "polygonal");
	for(size_t p= 0; p < m_spline.size(); ++p)
	{
		lua_pushnumber(L, get_spline_point(p));
		lua_rawseti(L, -2, p+1);
	}
}

void ModFunction::update_input_set(mod_val_inputs const& input,
	vector<size_t>& input_set)
{
	const_cast<mod_val_inputs&>(input).set_time(m_start_beat, m_start_second,
		input.music_beat, input.music_second, m_end_beat, m_end_second);
	for(auto pindex : input_set)
	{
		m_picked_inputs[pindex]= m_inputs[pindex].pick(input);
	}
}

void ModFunction::update_input_set_in_spline(mod_val_inputs const& input,
	vector<size_t>& input_set)
{
	const_cast<mod_val_inputs&>(input).set_time(m_start_beat, m_start_second,
		input.music_beat, input.music_second, m_end_beat, m_end_second);
	for(auto pindex : input_set)
	{
		m_picked_inputs[pindex]= m_inputs[pindex].pick(input);
		// The first input is the t value.
		if(pindex > 0)
		{
			m_spline.set_point(pindex-1, static_cast<float>(m_picked_inputs[pindex]));
		}
	}
}

size_t ModFunction::find_child(ModInput* child)
{
	for(size_t p= 0; p < m_inputs.size(); ++p)
	{
		if(child == &(m_inputs[p]))
		{
			return p;
		}
	}
	return m_inputs.size();
}

void ModFunction::remove_child_from_percoset(size_t child_index,
	vector<size_t>& percoset)
{
	for(auto pill= percoset.begin(); pill != percoset.end(); ++pill)
	{
		if(*pill == child_index)
		{
			percoset.erase(pill);
			return;
		}
	}
}

void ModFunction::recategorize(ModInput* child, ModInputMetaType old_meta,
	ModInputMetaType new_meta)
{
	if(m_being_loaded)
	{
		return;
	}
	size_t index= find_child(child);
	if(index >= m_inputs.size())
	{
		return;
	}
	if(old_meta == MIMT_PerFrame)
	{
		remove_child_from_percoset(index, m_per_frame_inputs);
	}
	else if(old_meta == MIMT_PerNote)
	{
		remove_child_from_percoset(index, m_per_note_inputs);
	}
	if(new_meta == MIMT_PerFrame)
	{
		m_per_frame_inputs.push_back(index);
	}
	else if(new_meta == MIMT_PerNote)
	{
		m_per_note_inputs.push_back(index);
	}
	else
	{
		mod_val_inputs scalar_input(0.0, 0.0);
		m_picked_inputs[index]= child->pick(scalar_input);
	}
}

void ModFunction::repick(ModInput* child)
{
	size_t index= find_child(child);
	if(index < m_inputs.size())
	{
		mod_val_inputs scalar_input(0.0, 0.0);
		m_picked_inputs[index]= m_inputs[index].pick(scalar_input);
	}
}

void ModFunction::per_frame_update(mod_val_inputs const& input)
{
	if(!m_per_frame_inputs.empty())
	{
		(this->*m_pfu)(input);
	}
}

void ModFunction::per_frame_update_normal(mod_val_inputs const& input)
{
	update_input_set(input, m_per_frame_inputs);
}

void ModFunction::per_note_update_normal(mod_val_inputs const& input)
{
	update_input_set(input, m_per_note_inputs);
}

void ModFunction::per_frame_update_spline(mod_val_inputs const& input)
{
	update_input_set_in_spline(input, m_per_frame_inputs);
	if(m_per_note_inputs.empty() && m_has_point_per_frame_input)
	{
		m_spline.solve(m_loop_spline, m_polygonal_spline);
	}
}

void ModFunction::per_note_update_spline(mod_val_inputs const& input)
{
	update_input_set_in_spline(input, m_per_note_inputs);
	m_spline.solve(m_loop_spline, m_polygonal_spline);
}

double ModFunction::constant_eval()
{
	return m_picked_inputs[0];
}

double ModFunction::product_eval()
{
	return m_picked_inputs[0] * m_picked_inputs[1];
}

double ModFunction::power_eval()
{
	return pow(m_picked_inputs[0], m_picked_inputs[1]);
}

double ModFunction::log_eval()
{
	return log(m_picked_inputs[0]) / log(m_picked_inputs[1]);
}

#define ZERO_AMP_WAVE_RETURN \
if(m_picked_inputs[2] == 0.0) \
{ \
	return m_picked_inputs[3]; \
}
#define WAVE_ANGLE_CALC \
double angle= fmod(m_picked_inputs[0] + m_picked_inputs[1], Rage::D_PI * 2.0); \
if(angle < 0.0) \
{ \
	angle+= Rage::D_PI * 2.0; \
}
#define WAVE_RET return (wave_res * m_picked_inputs[2]) + m_picked_inputs[3];

double ModFunction::sine_eval()
{
	ZERO_AMP_WAVE_RETURN;
	WAVE_ANGLE_CALC;
	double const wave_res= static_cast<double>(Rage::FastSin(angle));
	WAVE_RET;
}

double ModFunction::tan_eval()
{
	ZERO_AMP_WAVE_RETURN;
	WAVE_ANGLE_CALC;
	double const wave_res= tan(angle);
	WAVE_RET;
}

double ModFunction::square_eval()
{
	ZERO_AMP_WAVE_RETURN;
	WAVE_ANGLE_CALC;
	double const wave_res= (angle >= Rage::D_PI ? -1.0 : 1.0);
	WAVE_RET;
}

double ModFunction::triangle_eval()
{
	ZERO_AMP_WAVE_RETURN;
	WAVE_ANGLE_CALC;
	double wave_res= angle * Rage::D_PI_REC;
	if(wave_res < .5)
	{
		wave_res= wave_res * 2.0;
	}
	else if(wave_res < 1.5)
	{
		wave_res= 1.0 - ((wave_res - .5) * 2.0);
	}
	else
	{
		wave_res= -4.0 + (wave_res * 2.0);
	}
	WAVE_RET;
}

#undef WAVE_RET
#undef WAVE_ANGLE_CALC
#undef ZERO_AMP_WAVE_RETURN

double ModFunction::spline_eval()
{
	return m_spline.evaluate(static_cast<float>(m_picked_inputs[0]), m_loop_spline);
}

void ModFunction::set_type(ModFunctionType type)
{
	m_type= type;
	size_t num_inputs= 0;
	switch(m_type)
	{
		case MFT_Constant:
			m_sub_eval= &ModFunction::constant_eval;
			num_inputs= 1;
			break;
		case MFT_Product:
			m_sub_eval= &ModFunction::product_eval;
			num_inputs= 2;
			break;
		case MFT_Power:
			m_sub_eval= &ModFunction::power_eval;
			num_inputs= 2;
			break;
		case MFT_Log:
			m_sub_eval= &ModFunction::log_eval;
			num_inputs= 2;
			break;
		case MFT_Sine:
			m_sub_eval= &ModFunction::sine_eval;
			num_inputs= 4;
			break;
		case MFT_Tan:
			m_sub_eval= &ModFunction::tan_eval;
			num_inputs= 4;
			break;
		case MFT_Square:
			m_sub_eval= &ModFunction::square_eval;
			num_inputs= 4;
			break;
		case MFT_Triangle:
			m_sub_eval= &ModFunction::triangle_eval;
			num_inputs= 4;
			break;
		case MFT_Spline:
			m_sub_eval= &ModFunction::spline_eval;
			m_pfu= &ModFunction::per_frame_update_spline;
			m_pnu= &ModFunction::per_note_update_spline;
			break;
		default:
			return;
	}
	if(m_type != MFT_Spline)
	{
		m_inputs.resize(num_inputs);
	}
}

void ModFunction::init_picked_inputs()
{
	m_picked_inputs.resize(m_inputs.size());
	mod_val_inputs scalar_input(0.0, 0.0);
	for(size_t p= 0; p < m_inputs.size(); ++p)
	{
		ModInputMetaType mt= m_inputs[p].get_meta_type();
		switch(mt)
		{
			case MIMT_Scalar:
				m_picked_inputs[p]= m_inputs[p].pick(scalar_input);
				break;
			case MIMT_PerFrame:
				m_per_frame_inputs.push_back(p);
				break;
			case MIMT_PerNote:
				m_per_note_inputs.push_back(p);
				break;
			default:
				break;
		}
	}
}

bool ModFunction::load_from_lua(lua_State* L, int index)
{
	m_being_loaded= true;
	lua_rawgeti(L, index, 1);
	ModFunctionType type= Enum::Check<ModFunctionType>(L, -1, true, true);
	lua_pop(L, 1);
	if(type == ModFunctionType_Invalid)
	{
		m_being_loaded= false;
		return false;
	}
	set_type(type);
	// The lua table looks like this:
	// {
	//   name= "string", -- optional
	//   start_beat= 5, -- optional
	//   start_sec= 5, -- optional
	//   end_beat= 5, -- optional
	//   end_sec= 5, -- optional
	//   sum_type= "ModValSumType_Add", -- optional
	//   type, input, ...
	// }
	// name, the start and end values, and sum_type are optional.
	// The ... is for the inputs after the first.
	// So the first input is at lua table index 2.
	lua_getfield(L, index, "name");
	if(lua_isstring(L, -1))
	{
		m_name= lua_tostring(L, -1);
	}
	else
	{
		m_name= unique_name("mod");
	}
	lua_pop(L, 1);
	m_start_beat= get_optional_double(L, index, "start_beat", invalid_modfunction_time);
	m_start_second= get_optional_double(L, index, "start_second", invalid_modfunction_time);
	m_end_beat= get_optional_double(L, index, "end_beat", invalid_modfunction_time);
	m_end_second= get_optional_double(L, index, "end_second", invalid_modfunction_time);
	lua_getfield(L, index, "sum_type");
	if(lua_isstring(L, -1))
	{
		ModSumType sum_type= Enum::Check<ModSumType>(L, -1, true, true);
		if(sum_type == ModSumType_Invalid)
		{
			sum_type= MST_Add;
		}
		m_sum_type= sum_type;
	}
	if(m_type != MFT_Spline)
	{
		size_t elements= lua_objlen(L, index);
		size_t limit= std::min(elements, m_inputs.size()+1);
		for(size_t el= 2; el <= limit; ++el)
		{
			lua_rawgeti(L, index, el);
			m_inputs[el-2].load_from_lua(L, lua_gettop(L), this);
			lua_pop(L, 1);
		}
	}
	else
	{
		// The first element of the table is the type.  So the number of points
		// is one less than the size of the table.
		size_t num_points= lua_objlen(L, index) - 1;
		// The t value input is going to be put in the first slot.  So there is
		// one more input than the number of points.
		m_inputs.resize(num_points+1);
		m_spline.resize(num_points);
		lua_getfield(L, index, "t");
		m_inputs[0].load_from_lua(L, lua_gettop(L), this);
		lua_pop(L, 1);
		m_loop_spline= get_optional_bool(L, index, "loop");
		m_polygonal_spline= get_optional_bool(L, index, "polygonal");
		for(size_t el= 2; el <= num_points+1; ++el)
		{
			lua_rawgeti(L, index, el);
			m_inputs[el-1].load_from_lua(L, lua_gettop(L), this);
			lua_pop(L, 1);
		}
	}
	init_picked_inputs();
	if(m_type == MFT_Spline)
	{
		// All scalar inputs are sent to the spline on loading.  So the ones that
		// are not scalars are listed in per_note_inputs and per_frame_inputs so
		// those stages only send the ones they need to.
		// If all the input points are scalars, then they only need to be copied
		// into the spline once, and the spline only has to be solved once ever.
		// The t value input is in the first slot.  So there is one more input
		// than the number of points.
		for(size_t p= 1; p < m_picked_inputs.size(); ++p)
		{
			m_spline.set_point(p-1, static_cast<float>(m_picked_inputs[p]));
		}
		m_has_point_per_frame_input= false;
		for(size_t p= 0; p < m_per_frame_inputs.size(); ++p)
		{
			if(m_per_frame_inputs[p] > 0)
			{
				m_has_point_per_frame_input= true;
				break;
			}
		}
		if(!m_has_point_per_frame_input && m_per_note_inputs.empty())
		{
			m_spline.solve(m_loop_spline, m_polygonal_spline);
		}
	}
	m_being_loaded= false;
	return true;
}

void ModFunction::push_inputs(lua_State* L, int table_index)
{
	size_t curr_input= 0;
	int out_index= 1;
	// For splines, the first input is the t value input.  But the returned
	// inputs table should look like the table the ModFunction was created
	// from.  So a spline starts at input 1, and puts the t input in a field.
	if(m_type == MFT_Spline)
	{
		curr_input= 1;
		m_inputs[0].PushSelf(L);
		lua_setfield(L, table_index, "t");
	}
	for(; curr_input < m_inputs.size(); ++curr_input)
	{
		m_inputs[curr_input].PushSelf(L);
		lua_rawseti(L, table_index, out_index);
		++out_index;
	}
}

void ModFunction::init_single_input(std::string const& name,
	ModInputType input_type, double input_scalar)
{
	m_being_loaded= true;
	set_type(MFT_Constant);
	m_name= name;
	m_sum_type= MST_Add;
	m_inputs[0].init_simple(this, input_type, input_scalar);
	init_picked_inputs();
	m_being_loaded= false;
}

static void calc_timing_pair(TimingData const* timing, double& beat, double& second)
{
	bool beat_needed= (beat == invalid_modfunction_time);
	bool second_needed= (second == invalid_modfunction_time);
	if(beat_needed && !second_needed)
	{
		beat= timing->GetBeatFromElapsedTime(static_cast<float>(second));
	}
	else if(!beat_needed && second_needed)
	{
		second= timing->GetElapsedTimeFromBeat(static_cast<float>(beat));
	}
}

void ModFunction::calc_unprovided_times(TimingData const* timing)
{
	calc_timing_pair(timing, m_start_beat, m_start_second);
	calc_timing_pair(timing, m_end_beat, m_end_second);
}

#define MF_NEEDS_THING(thing) \
bool ModFunction::needs_##thing() \
{ \
	for(auto&& input : m_inputs) \
	{ \
		if(input.needs_##thing()) \
		{ \
			return true; \
		} \
	} \
	return false; \
}

MF_NEEDS_THING(beat);
MF_NEEDS_THING(second);
MF_NEEDS_THING(y_offset);
#undef MF_NEEDS_THING

static ModFunction* create_field_mod(ModifiableValue* parent, lua_State* L, int index)
{
	ModFunction* ret= new ModFunction(parent);
	if(!ret->load_from_lua(L, index))
	{
		delete ret;
		return nullptr;
	}
	return ret;
}

ModifiableValue::~ModifiableValue()
{
	clear_mods();
	clear_managed_mods();
}

double ModifiableValue::evaluate(mod_val_inputs const& input)
{
	double sum= m_value;
	if(!m_mods.empty())
	{
		for(auto&& mod : m_mods)
		{
			switch(mod->m_sum_type)
			{
				case MST_Add:
					sum+= mod->evaluate(input);
					break;
				case MST_Subtract:
					sum-= mod->evaluate(input);
					break;
				case MST_Multiply:
					sum*= mod->evaluate(input);
					break;
				case MST_Divide:
					sum/= mod->evaluate(input);
					break;
				default:
					FAIL_M(fmt::sprintf("Invalid ModSumType: %i", static_cast<int>(mod->m_sum_type)));
					break;
			}
		}
	}
	if(!m_active_managed_mods.empty())
	{
		for(auto&& mod : m_active_managed_mods)
		{
			switch(mod->m_sum_type)
			{
				case MST_Add:
					sum+= mod->evaluate_with_time(input);
					break;
				case MST_Subtract:
					sum-= mod->evaluate_with_time(input);
					break;
				case MST_Multiply:
					sum*= mod->evaluate_with_time(input);
					break;
				case MST_Divide:
					sum/= mod->evaluate_with_time(input);
					break;
				default:
					FAIL_M(fmt::sprintf("Invalid ModSumType: %i", static_cast<int>(mod->m_sum_type)));
					break;
			}
		}
	}
	return sum;
}

ModFunction* ModifiableValue::add_mod_internal(lua_State* L, int index)
{
	ModFunction* new_mod= create_field_mod(this, L, index);
	if(new_mod == nullptr)
	{
		LuaHelpers::ReportScriptError("Problem creating modifier: unknown type.");
	}
	return new_mod;
}

ModFunction* ModifiableValue::insert_mod_internal(ModFunction* new_mod)
{
	ModFunction* ret= nullptr;
	auto insert_point= find_mod(new_mod->get_name());
	if(insert_point == m_mods.end())
	{
		m_mods.push_back(new_mod);
		ret= new_mod;
	}
	else
	{
		(*(*insert_point)) = (*new_mod);
		delete new_mod;
		ret= (*insert_point);
	}
	m_manager->add_to_per_frame_update(ret);
	return ret;
}

std::list<ModFunction*>::iterator ModifiableValue::find_mod(std::string const& name)
{
	for(auto iter= m_mods.begin(); iter != m_mods.end(); ++iter)
	{
		if((*iter)->get_name() == name)
		{
			return iter;
		}
	}
	return m_mods.end();
}

ModFunction* ModifiableValue::add_mod(lua_State* L, int index)
{
	ModFunction* new_mod= add_mod_internal(L, index);
	if(new_mod == nullptr)
	{
		return nullptr;
	}
	return insert_mod_internal(new_mod);
}

ModFunction* ModifiableValue::get_mod(std::string const& name)
{
	auto mod= find_mod(name);
	if(mod != m_mods.end())
	{
		return *mod;
	}
	return nullptr;
}

void ModifiableValue::remove_mod(std::string const& name)
{
	auto mod= find_mod(name);
	if(mod != m_mods.end())
	{
		delete *mod;
		m_mods.erase(mod);
	}
}

void ModifiableValue::clear_mods()
{
	for(auto&& mod : m_mods)
	{
		delete mod;
	}
	m_mods.clear();
}

ModFunction* ModifiableValue::add_managed_mod(lua_State* L, int index)
{
	ModFunction* new_mod= add_mod_internal(L, index);
	if(new_mod == nullptr)
	{
		return nullptr;
	}
	new_mod->calc_unprovided_times(m_timing);
	ModFunction* ret= nullptr;
	auto mod= m_managed_mods.find(new_mod->get_name());
	if(mod == m_managed_mods.end())
	{
		m_managed_mods.insert(make_pair(new_mod->get_name(), new_mod));
		ret= new_mod;
	}
	else
	{
		(*(mod->second)) = (*new_mod);
		delete new_mod;
		ret= mod->second;
	}
	m_manager->add_mod(ret, this);
	//m_manager->dump_list_status();
	return ret;
}

ModFunction* ModifiableValue::get_managed_mod(std::string const& name)
{
	auto mod= m_managed_mods.find(name);
	if(mod != m_managed_mods.end())
	{
		return mod->second;
	}
	return nullptr;
}

void ModifiableValue::remove_managed_mod(std::string const& name)
{
	auto mod= m_managed_mods.find(name);
	if(mod != m_managed_mods.end())
	{
		m_manager->remove_mod(mod->second);
		remove_mod_from_active_list(mod->second);
		delete mod->second;
		m_managed_mods.erase(mod);
	}
}

void ModifiableValue::clear_managed_mods()
{
	m_manager->remove_all_mods(this);
	for(auto&& mod : m_managed_mods)
	{
		delete mod.second;
	}
	m_managed_mods.clear();
}

void ModifiableValue::add_mod_to_active_list(ModFunction* mod)
{
	m_active_managed_mods.insert(mod);
}

void ModifiableValue::remove_mod_from_active_list(ModFunction* mod)
{
	auto iter= m_active_managed_mods.find(mod);
	if(iter != m_active_managed_mods.end())
	{
		m_active_managed_mods.erase(iter);
	}
}

void ModifiableValue::add_simple_mod(std::string const& name,
	ModInputType input_type, double input_scalar)
{
	ModFunction* new_mod= new ModFunction(this);
	new_mod->init_single_input(name, input_type, input_scalar);
	insert_mod_internal(new_mod);
}

void ModifiableValue::take_over_mods(ModifiableValue& other)
{
	m_mods.swap(other.m_mods);
	for(auto&& mod : m_mods)
	{
		mod->change_parent(this);
	}
	m_managed_mods.swap(other.m_managed_mods);
	for(auto&& mod : m_managed_mods)
	{
		m_manager->add_mod(mod.second, this);
	}
	m_active_managed_mods.swap(other.m_active_managed_mods);
	other.clear_managed_mods();
}

#define MV_NEEDS_THING(thing) \
bool ModifiableValue::needs_##thing() \
{ \
	for(auto&& fun : m_mods) \
	{ \
		if(fun->needs_##thing()) \
		{ \
			return true; \
		} \
	} \
	for(auto&& fun : m_active_managed_mods) \
	{ \
		if(fun->needs_##thing()) \
		{ \
			return true; \
		} \
	} \
	return false; \
}

MV_NEEDS_THING(beat);
MV_NEEDS_THING(second);
MV_NEEDS_THING(y_offset);
#undef MV_NEEDS_THING

#define MV3_NEEDS_THING(thing) \
bool ModifiableVector3::needs_##thing() \
{ \
	return x_mod.needs_##thing() || y_mod.needs_##thing() || z_mod.needs_##thing(); \
}

MV3_NEEDS_THING(beat);
MV3_NEEDS_THING(second);
MV3_NEEDS_THING(y_offset);
#undef MV3_NEEDS_THING

#define MT_NEEDS_THING(thing) \
bool ModifiableTransform::needs_##thing() \
{ \
	return pos_mod.needs_##thing() || rot_mod.needs_##thing() || zoom_mod.needs_##thing(); \
}

MT_NEEDS_THING(beat);
MT_NEEDS_THING(second);
MT_NEEDS_THING(y_offset);
#undef MT_NEEDS_THING


// lua start
struct LunaModInput : Luna<ModInput>
{
	static int get_type(T* p, lua_State* L)
	{
		Enum::Push(L, p->get_type());
		return 1;
	}
	static int set_type(T* p, lua_State* L)
	{
		p->set_type(Enum::Check<ModInputType>(L, 1));
		COMMON_RETURN_SELF;
	}
	GETTER_SETTER_FLOAT_METHOD(scalar);
	GETTER_SETTER_FLOAT_METHOD(offset);
	static int get_rep(T* p, lua_State* L)
	{
		p->push_rep(L);
		return 1;
	}
	static int set_rep(T* p, lua_State* L)
	{
		p->load_rep(L, 1);
		COMMON_RETURN_SELF;
	}
	static int get_all_phases(T* p, lua_State* L)
	{
		p->push_all_phases(L);
		return 1;
	}
	static int get_phase(T* p, lua_State* L)
	{
		size_t phase= static_cast<size_t>(IArg(1)) - 1;
		if(phase >= p->get_num_phases())
		{
			lua_pushnil(L);
		}
		else
		{
			p->push_phase(L, phase);
		}
		return 1;
	}
	static int get_default_phase(T* p, lua_State* L)
	{
		p->push_def_phase(L);
		return 1;
	}
	static int get_num_phases(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_num_phases());
		return 1;
	}
	static int set_all_phases(T* p, lua_State* L)
	{
		p->load_phases(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_phase(T* p, lua_State* L)
	{
		size_t phase= static_cast<size_t>(IArg(1)) - 1;
		if(phase >= p->get_num_phases() || !lua_istable(L, 2))
		{
			luaL_error(L, "Args to ModInput:set_phase must be an index and a table.");
		}
		p->load_one_phase(L, 2, phase);
		p->sort_phases();
		COMMON_RETURN_SELF;
	}
	static int set_default_phase(T* p, lua_State* L)
	{
		p->load_def_phase(L, 1);
		COMMON_RETURN_SELF;
	}
	static int remove_phase(T* p, lua_State* L)
	{
		size_t phase= static_cast<size_t>(IArg(1)) - 1;
		p->remove_phase(phase);
		COMMON_RETURN_SELF;
	}
	static int clear_phases(T* p, lua_State* L)
	{
		(void)L;
		p->clear_phases();
		COMMON_RETURN_SELF;
	}
	static int set_enable_phases(T* p, lua_State* L)
	{
		bool enable= BArg(1);
		if(enable)
		{
			p->enable_phases();
		}
		else
		{
			p->disable_phases();
		}
		COMMON_RETURN_SELF;
	}
	static int get_enable_phases(T* p, lua_State* L)
	{
		lua_pushboolean(L, p->get_enable_phases());
		return 1;
	}
	static int get_enable_spline(T* p, lua_State* L)
	{
		lua_pushboolean(L, p->get_enable_spline());
		return 1;
	}
	static int set_enable_spline(T* p, lua_State* L)
	{
		bool enable= BArg(1);
		if(enable)
		{
			p->enable_spline();
		}
		else
		{
			p->disable_spline();
		}
		COMMON_RETURN_SELF;
	}
	GETTER_SETTER_BOOL_METHOD(spline_loop);
	GETTER_SETTER_BOOL_METHOD(spline_polygonal);
	static size_t spline_point_index(T* p, lua_State* L, int index)
	{
		int i= IArg(index);
		if(i < 1)
		{
			luaL_error(L, "Invalid spline point index.");
		}
		size_t ret= static_cast<size_t>(i) - 1;
		if(ret >= p->get_spline_size())
		{
			luaL_error(L, "Invalid spline point index.");
		}
		return ret;
	}
	static int get_spline_size(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->get_spline_size());
		return 1;
	}
	static int get_spline_point(T* p, lua_State* L)
	{
		size_t point= spline_point_index(p, L, 1);
		lua_pushnumber(L, p->get_spline_point(point));
		return 1;
	}
	static int set_spline_point(T* p, lua_State* L)
	{
		size_t point= spline_point_index(p, L, 1);
		p->set_spline_point(point, FArg(2));
		COMMON_RETURN_SELF;
	}
	static int add_spline_point(T* p, lua_State* L)
	{
		p->add_spline_point(FArg(1));
		COMMON_RETURN_SELF;
	}
	static int remove_spline_point(T* p, lua_State* L)
	{
		if(p->get_spline_empty())
		{
			COMMON_RETURN_SELF;
		}
		size_t point= 0;
		if(lua_isnil(L, 1))
		{
			point= p->get_spline_size()-1;
		}
		else
		{
			point= spline_point_index(p, L, 1);
		}
		p->remove_spline_point(point);
		COMMON_RETURN_SELF;
	}
	static int get_spline(T* p, lua_State* L)
	{
		p->push_spline(L);
		return 1;
	}
	static int set_spline(T* p, lua_State* L)
	{
		p->load_spline(L, 1);
		COMMON_RETURN_SELF;
	}
	LunaModInput()
	{
		ADD_GET_SET_METHODS(type);
		ADD_GET_SET_METHODS(scalar);
		ADD_GET_SET_METHODS(offset);
		ADD_GET_SET_METHODS(rep);
		ADD_GET_SET_METHODS(all_phases);
		ADD_GET_SET_METHODS(phase);
		ADD_GET_SET_METHODS(default_phase);
		ADD_METHOD(get_num_phases);
		ADD_METHOD(remove_phase);
		ADD_METHOD(clear_phases);
		ADD_GET_SET_METHODS(enable_phases);
		ADD_METHOD(get_spline_size);
		ADD_GET_SET_METHODS(spline_point);
		ADD_METHOD(add_spline_point);
		ADD_METHOD(remove_spline_point);
		ADD_GET_SET_METHODS(spline);
	}
};
LUA_REGISTER_CLASS(ModInput);

struct LunaModFunction : Luna<ModFunction>
{
	static int get_inputs(T* p, lua_State* L)
	{
		lua_createtable(L, p->num_inputs(), 0);
		p->push_inputs(L, lua_gettop(L));
		return 1;
	}
	GET_SET_ENUM_METHOD(sum_type, ModSumType, m_sum_type);
	LunaModFunction()
	{
		ADD_METHOD(get_inputs);
		ADD_GET_SET_METHODS(sum_type);
	}
};
LUA_REGISTER_CLASS(ModFunction);

struct LunaModifiableValue : Luna<ModifiableValue>
{
	static int add_mod(T* p, lua_State* L)
	{
		p->add_mod(L, lua_gettop(L));
		COMMON_RETURN_SELF;
	}
	static int add_get_mod(T* p, lua_State* L)
	{
		ModFunction* mod= p->add_mod(L, lua_gettop(L));
		if(mod == nullptr)
		{
			lua_pushnil(L);
		}
		else
		{
			mod->PushSelf(L);
		}
		return 1;
	}
	static int get_mod(T* p, lua_State* L)
	{
		ModFunction* mod= p->get_mod(SArg(1));
		if(mod == nullptr)
		{
			lua_pushnil(L);
		}
		else
		{
			mod->PushSelf(L);
		}
		return 1;
	}
	static int remove_mod(T* p, lua_State* L)
	{
		p->remove_mod(SArg(1));
		COMMON_RETURN_SELF;
	}
	static int clear_mods(T* p, lua_State* L)
	{
		p->clear_mods();
		COMMON_RETURN_SELF;
	}
	static int add_managed_mod(T* p, lua_State* L)
	{
		p->add_managed_mod(L, lua_gettop(L));
		COMMON_RETURN_SELF;
	}
	static int add_managed_mod_set(T* p, lua_State* L)
	{
		if(!lua_istable(L, 1))
		{
			luaL_error(L, "Arg for add_managed_mod_set must be a table of ModFunctins.");
		}
		size_t num_mods= lua_objlen(L, 1);
		for(size_t m= 0; m < num_mods; ++m)
		{
			lua_rawgeti(L, 1, m+1);
			p->add_managed_mod(L, lua_gettop(L));
			lua_pop(L, 1);
		}
		COMMON_RETURN_SELF;
	}
	static int add_get_managed_mod(T* p, lua_State* L)
	{
		ModFunction* mod= p->add_managed_mod(L, lua_gettop(L));
		if(mod == nullptr)
		{
			lua_pushnil(L);
		}
		else
		{
			mod->PushSelf(L);
		}
		return 1;
	}
	static int get_managed_mod(T* p, lua_State* L)
	{
		ModFunction* mod= p->get_managed_mod(SArg(1));
		if(mod == nullptr)
		{
			lua_pushnil(L);
		}
		else
		{
			mod->PushSelf(L);
		}
		return 1;
	}
	static int remove_managed_mod(T* p, lua_State* L)
	{
		p->remove_managed_mod(SArg(1));
		COMMON_RETURN_SELF;
	}
	static int clear_managed_mods(T* p, lua_State* L)
	{
		p->clear_managed_mods();
		COMMON_RETURN_SELF;
	}
	static int get_value(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->m_value);
		return 1;
	}
	static int set_value(T* p, lua_State* L)
	{
		p->m_value= FArg(1);
		COMMON_RETURN_SELF;
	}
	static int evaluate(T* p, lua_State* L)
	{
		mod_val_inputs input(FArg(1), FArg(2), FArg(3), FArg(4), FArg(5));
		lua_pushnumber(L, p->evaluate(input));
		return 1;
	}
	LunaModifiableValue()
	{
		ADD_METHOD(add_mod);
		ADD_METHOD(add_get_mod);
		ADD_METHOD(get_mod);
		ADD_METHOD(remove_mod);
		ADD_METHOD(clear_mods);
		ADD_METHOD(add_managed_mod);
		ADD_METHOD(add_managed_mod_set);
		ADD_METHOD(add_get_managed_mod);
		ADD_METHOD(get_managed_mod);
		ADD_METHOD(remove_managed_mod);
		ADD_METHOD(clear_managed_mods);
		ADD_GET_SET_METHODS(value);
		ADD_METHOD(evaluate);
	}
};
LUA_REGISTER_CLASS(ModifiableValue);
