#include "global.h"
#include "CubicSpline.h"
#include "EnumHelper.h"
#include "GameState.h"
#include "ModValue.h"
#include "ModValueInternal.h"
#include "RageLog.h"
#include "RageMath.h"
#include "LuaBinding.h"

#include "Actor.h"
#include "ActorUtil.h"

ModManager::~ModManager()
{
	for(auto&& container : {&m_past_funcs, &m_present_funcs, &m_past_funcs})
	{
		for(auto&& fap : *container)
		{
			delete fap.func;
		}
		container->clear();
	}
}

void ModManager::update(double curr_beat, double curr_second)
{
	double const time_diff= curr_second - m_prev_curr_second;
	// Do not simply return early when time_diff is 0 because per frame
	// mod_functions still need to be updated when they are changed while the
	// game is paused. -Kyz
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
	else if(time_diff < 0)
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

void ModManager::add_mod(mod_function* func, ModifiableValue* parent)
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

void ModManager::register_moddable(std::string const& name, ModifiableValue* thing)
{
	m_registered_targets.insert(std::make_pair(name, thing));
}

void ModManager::set_base_values(lua_State* L, int value_set)
{
	lua_pushnil(L);
	while(lua_next(L, value_set) != 0)
	{
		if(lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER)
		{
			std::string target= lua_tostring(L, -2);
			auto target_registry_entry= m_registered_targets.find(target);
			if(target_registry_entry != m_registered_targets.end())
			{
				double value= lua_tonumber(L, -1);
				target_registry_entry->second->set_base_value(value);
			}
			else
			{
				LuaHelpers::ReportScriptErrorFmt("Mod target %s not found.", target.c_str());
			}
		}
		lua_pop(L, 1);
	}
}

void ModManager::add_permanent_mods(lua_State* L, int mod_set)
{
	size_t num_entries= lua_objlen(L, mod_set);
	std::vector<std::pair<size_t, std::string> > failure_backlog;
	for(size_t mod_id= 1; mod_id <= num_entries; ++mod_id)
	{
		lua_rawgeti(L, mod_set, mod_id);
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			int entry_table= lua_gettop(L);
			std::string target_name;
			get_optional_string(L, entry_table, "target", target_name);
			if(!target_name.empty())
			{
				auto target_registry_entry= m_registered_targets.find(target_name);
				if(target_registry_entry != m_registered_targets.end())
				{
					auto target= target_registry_entry->second;
					std::string failure;
					mod_function* temp_func= new mod_function(target, target->get_column());
					try
					{
						temp_func->load_from_lua(L, entry_table, nullptr);
						target->add_mod(temp_func, *this);
					}
					catch(std::string& err)
					{
						failure_backlog.push_back(std::make_pair(mod_id, err));
						delete temp_func;
					}
				}
			}
		}
		lua_pop(L, 1);
	}
	if(!failure_backlog.empty())
	{
		LOG->Trace("Malformed permanent mods:");
		for(auto&& failblog : failure_backlog)
		{
			LOG->Trace("%i: %s", failblog.first, failblog.second.c_str());
		}
		LuaHelpers::ReportScriptError("Check log for list of mods that could not be loaded.");
	}
}

void ModManager::add_timed_mods(lua_State* L, int mod_set)
{
	size_t num_entries= lua_objlen(L, mod_set);
	std::vector<std::pair<size_t, std::string> > failure_backlog;
	for(size_t mod_id= 1; mod_id <= num_entries; ++mod_id)
	{
		lua_rawgeti(L, mod_set, mod_id);
		if(lua_type(L, -1) == LUA_TTABLE)
		{
			int entry_table= lua_gettop(L);
			std::string target_name;
			get_optional_string(L, entry_table, "target", target_name);
			if(!target_name.empty())
			{
				auto target_registry_entry= m_registered_targets.find(target_name);
				if(target_registry_entry != m_registered_targets.end())
				{
					auto target= target_registry_entry->second;
					mod_function* temp_func= new mod_function(target, target->get_column());
					try
					{
						temp_func->load_from_lua(L, entry_table, m_timing_data);
						add_mod(temp_func, target);
					}
					catch(std::string& err)
					{
						delete temp_func;
						failure_backlog.push_back(std::make_pair(mod_id, err));
					}
				}
			}
		}
		lua_pop(L, 1);
	}
	if(!failure_backlog.empty())
	{
		LOG->Trace("Malformed timed mods:");
		for(auto&& failblog : failure_backlog)
		{
			LOG->Trace("%i: %s", failblog.first, failblog.second.c_str());
		}
		LuaHelpers::ReportScriptError("Check log for list of mods that could not be loaded.");
	}
}

void ModManager::push_target_info(lua_State* L)
{
	// Creates a table like this:
	// { name= true, [1]= name, ... }
	// So lua that needs the target info can use 'if info.name then' to test
	// whether a target exists, or iterate through the table to show a list.
	lua_createtable(L, m_registered_targets.size(), m_registered_targets.size());
	int info_table= lua_gettop(L);
	int next_id= 1;
	for(auto&& target_entry : m_registered_targets)
	{
		lua_pushboolean(L, true);
		lua_setfield(L, info_table, target_entry.first.c_str());
		++next_id;
	}
}


void ModManager::remove_permanent_mods(lua_State* L, int mod_set)
{
	// mod_set is a table of target, name pair tables.
	// {{target= "speed", name= "speed"}, {target= "note_transform", name= "hat"}}
	size_t num_entries= lua_objlen(L, mod_set);
	for(size_t mod_id= 1; mod_id <= num_entries; ++mod_id)
	{
		lua_rawgeti(L, mod_set, mod_id);
		int mod_entry= lua_gettop(L);
		if(lua_type(L, mod_entry) == LUA_TTABLE)
		{
			std::string target;
			get_optional_string(L, mod_entry, "target", target);
			std::string name;
			get_optional_string(L, mod_entry, "name", name);
			if(!target.empty() && !name.empty())
			{
				if(target == "all")
				{
					for(auto&& entry : m_registered_targets)
					{
						entry.second->remove_mod(name, *this);
					}
				}
				else
				{
					auto target_registry_entry= m_registered_targets.find(target);
					if(target_registry_entry != m_registered_targets.end())
					{
						target_registry_entry->second->remove_mod(name, *this);
					}
				}
			}
		}
		lua_pop(L, 1);
	}
}

void ModManager::clear_permanent_mods(lua_State* L, int mod_set)
{
	// mod_set is a table of targets to clear all mods from.
	// {"reverse", "note_transform", "note_visibility"}
	size_t num_entries= lua_objlen(L, mod_set);
	for(size_t mod_id= 1; mod_id <= num_entries; ++mod_id)
	{
		lua_rawgeti(L, mod_set, mod_id);
		int mod_entry= lua_gettop(L);
		if(lua_type(L, mod_entry) == LUA_TSTRING)
		{
			std::string target= lua_tostring(L, mod_entry);
			if(target == "all")
			{
				for(auto&& entry : m_registered_targets)
				{
					entry.second->clear_mods(*this);
				}
			}
			else
			{
				auto target_registry_entry= m_registered_targets.find(target);
				if(target_registry_entry != m_registered_targets.end())
				{
					target_registry_entry->second->clear_mods(*this);
				}
			}
		}
		lua_pop(L, 1);
	}
}

void ModManager::clear_timed_mods()
{
	for(auto&& target : m_registered_targets)
	{
		target.second->clear_active_list();
	}
	for(auto&& container : {&m_past_funcs, &m_present_funcs, &m_future_funcs})
	{
		for(auto&& entry : *container)
		{
			auto per_frame_entry= m_per_frame_update_funcs.find(entry.func);
			if(per_frame_entry != m_per_frame_update_funcs.end())
			{
				m_per_frame_update_funcs.erase(per_frame_entry);
			}
			delete entry.func;
		}
		container->clear();
	}
}


void ModManager::load_time_pair(lua_State* L, int table,
	char const* beat_name, char const* second_name,
	double& beat_result, double& second_result)
{
	beat_result= get_optional_double(L, table, beat_name, invalid_modfunction_time);
	second_result= get_optional_double(L, table, second_name, invalid_modfunction_time);
	if(beat_result == invalid_modfunction_time)
	{
		if(second_result == invalid_modfunction_time)
		{
			return;
		}
		beat_result= m_timing_data->GetBeatFromElapsedTime(static_cast<float>(second_result));
	}
	else
	{
		if(second_result == invalid_modfunction_time)
		{
			second_result= m_timing_data->GetElapsedTimeFromBeat(static_cast<float>(beat_result));
		}
	}
}


void ModManager::load_target_list(lua_State* L, int table,
	std::unordered_set<ModifiableValue*>& target_list)
{
	lua_getfield(L, table, "target");
	if(lua_isstring(L, -1))
	{
		std::string target= lua_tostring(L, -1);
		if(target == "all")
		{
			for(auto iter= m_registered_targets.begin(); iter != m_registered_targets.end(); ++iter)
			{
				target_list.insert(iter->second);
			}
		}
		else
		{
			auto entry= m_registered_targets.find(target);
			if(entry != m_registered_targets.end())
			{
				target_list.insert(entry->second);
			}
		}
	}
	else if(lua_istable(L, -1))
	{
		int list_index= lua_gettop(L);
		size_t entries= lua_objlen(L, list_index);
		for(size_t e= 1; e <= entries; ++e)
		{
			lua_rawgeti(L, list_index, e);
			if(lua_isstring(L, -1))
			{
				std::string target;
				target= lua_tostring(L, -1);
				auto entry= m_registered_targets.find(target);
				if(entry != m_registered_targets.end())
				{
					target_list.insert(entry->second);
				}
			}
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
}


void ModManager::remove_mod(mod_function* func)
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

void ModManager::add_to_per_frame_update(mod_function* func)
{
	if(func->needs_per_frame_update())
	{
		m_per_frame_update_funcs.insert(func);
	}
}

void ModManager::remove_from_per_frame_update(mod_function* func)
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

void ModManager::insert_into_past(mod_function* func, ModifiableValue* parent)
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

void ModManager::insert_into_present(mod_function* func, ModifiableValue* parent)
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

void ModManager::insert_into_future(mod_function* func, ModifiableValue* parent)
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

std::list<ModManager::func_and_parent>::iterator ModManager::remove_from_present(std::list<func_and_parent>::iterator fapi)
{
	remove_from_per_frame_update(fapi->func);
	fapi->parent->remove_mod_from_active_list(fapi->func);
	return m_present_funcs.erase(fapi);
}


#define CONVENT(field_name) {#field_name, mot_##field_name}
static std::unordered_map<std::string, mot> mot_conversion= {
	{"+", mot_add},
	{"-", mot_subtract},
	{"*", mot_multiply},
	{"/", mot_divide},
	{"^", mot_exp},
	{"pow", mot_exp},
	{"v", mot_log},
	{"%", mot_mod},
	{"o", mot_round},
	{"_", mot_floor},
	{"|", mot_abs},
	CONVENT(replace),
	CONVENT(add),
	CONVENT(subtract),
	CONVENT(multiply),
	CONVENT(divide),
	CONVENT(exp),
	CONVENT(log),
	CONVENT(min),
	CONVENT(max),
	CONVENT(sin),
	CONVENT(cos),
	CONVENT(tan),
	CONVENT(square),
	CONVENT(triangle),
	CONVENT(asin),
	CONVENT(acos),
	CONVENT(atan),
	CONVENT(asquare),
	CONVENT(atriangle),
	CONVENT(random),
	CONVENT(phase),
	CONVENT(repeat),
	CONVENT(abs),
	CONVENT(mod),
	CONVENT(floor),
	CONVENT(ceil),
	CONVENT(round),
	CONVENT(spline),
	CONVENT(lua)
};
#undef CONVENT

mot str_to_mot(std::string const& str)
{
	auto entry= mot_conversion.find(str);
	if(entry == mot_conversion.end())
	{
		throw std::string(fmt::sprintf("Invalid mod operator type: %s", str.c_str()));
	}
	return entry->second;
}

mod_operand* create_mod_operator(mod_function* parent, lua_State* L, int index)
{
	lua_rawgeti(L, index, 1);
	int op_field_type= lua_type(L, -1);
	mod_operator* new_oper= nullptr;
	if(op_field_type == LUA_TFUNCTION)
	{
		lua_pop(L, 1);
		new_oper= new mod_operator_lua;
	}
	else if(op_field_type == LUA_TSTRING)
	{
		std::string str_type= lua_tostring(L, -1);
		lua_pop(L, 1);
		mot op_type= str_to_mot(str_type);
#define SET_NEW(op_name) \
		case mot_##op_name: new_oper= new mod_operator_##op_name; break;
		switch(op_type)
		{
			SET_NEW(add);
			SET_NEW(subtract);
			SET_NEW(multiply);
			SET_NEW(divide);
			SET_NEW(exp);
			SET_NEW(log);
			SET_NEW(min);
			SET_NEW(max);
			SET_NEW(sin);
			SET_NEW(cos);
			SET_NEW(tan);
			SET_NEW(square);
			SET_NEW(triangle);
			SET_NEW(asin);
			SET_NEW(acos);
			SET_NEW(atan);
			SET_NEW(asquare);
			SET_NEW(atriangle);
			SET_NEW(random);
			SET_NEW(phase);
			SET_NEW(repeat);
			SET_NEW(abs);
			SET_NEW(mod);
			SET_NEW(floor);
			SET_NEW(ceil);
			SET_NEW(round);
			SET_NEW(spline);
			default: break;
		}
#undef SET_NEW
	}
	else
	{
		lua_pop(L, 1);
		throw std::string("Mod operator type must be a string or function.");
	}
	if(new_oper == nullptr)
	{
		throw std::string("Unknown mod operator type.");
	}
	try
	{
		new_oper->load_from_lua(parent, L, index);
	}
	catch(std::string& err)
	{
		delete new_oper;
		throw;
	}
	return new_oper;
}

#define CONVENT(field_name) {#field_name, mit_##field_name}
static std::unordered_map<std::string, mit> mit_conversion= {
	CONVENT(music_rate),
	CONVENT(column),
	CONVENT(y_offset),
	CONVENT(note_id_in_chart),
	CONVENT(note_id_in_column),
	CONVENT(row_id),
	CONVENT(eval_beat),
	CONVENT(eval_second),
	CONVENT(music_beat),
	CONVENT(music_second),
	CONVENT(dist_beat),
	CONVENT(dist_second),
	CONVENT(start_beat),
	CONVENT(start_second),
	CONVENT(length_beats),
	CONVENT(length_seconds),
	CONVENT(end_beat),
	CONVENT(end_second),
	CONVENT(prefunres)
};
#undef CONVENT

mit str_to_mit(std::string const& str)
{
	auto entry= mit_conversion.find(str);
	if(entry == mit_conversion.end())
	{
		throw std::string(fmt::sprintf("Invalid mod input type: %s", str.c_str()));
	}
	return entry->second;
}

mod_operand* create_mod_input(std::string const& str_type)
{
	mit type= str_to_mit(str_type);
#define RET_ENTRY(field_name) \
	case mit_##field_name: return new mod_input_##field_name;
#define ENTRY_BS_PAIR(base) RET_ENTRY(base##_beat); RET_ENTRY(base##_second);
	switch(type)
	{
		RET_ENTRY(music_rate);
		RET_ENTRY(column);
		RET_ENTRY(y_offset);
		RET_ENTRY(note_id_in_chart);
		RET_ENTRY(note_id_in_column);
		RET_ENTRY(row_id);
		ENTRY_BS_PAIR(eval);
		ENTRY_BS_PAIR(music);
		ENTRY_BS_PAIR(dist);
		ENTRY_BS_PAIR(start);
		RET_ENTRY(length_beats);
		RET_ENTRY(length_seconds);
		ENTRY_BS_PAIR(end);
		RET_ENTRY(prefunres);
		default: break;
	}
#undef ENTRY_BS_PAIR
#undef RET_ENTRY
	throw std::string(fmt::sprintf("Unknown mod input type: %s", str_type.c_str()));
}

mod_operand* create_mod_input(lua_State* L, int index)
{
	if(lua_type(L, index) == LUA_TNUMBER)
	{
		mod_input_number* ret= new mod_input_number;
		ret->m_value= lua_tonumber(L, index);
		return ret;
	}
	std::string str= lua_tostring(L, index);
	return create_mod_input(str);
}

mod_operand* load_operand_on_stack(mod_function* parent, lua_State* L)
{
	int op_index= lua_gettop(L);
	mod_operand* new_oper= nullptr;
	try
	{
		switch(lua_type(L, op_index))
		{
			case LUA_TTABLE:
				if(lua_objlen(L, op_index) < 2)
				{
					throw std::string("Mod operator table must have at least two elements.");
				}
				new_oper= create_mod_operator(parent, L, op_index);
				break;
			case LUA_TNUMBER:
			case LUA_TSTRING:
				new_oper= create_mod_input(L, op_index);
				break;
			default:
				throw std::string("Invalid operand type.");
				break;
		}
	}
	catch(std::string& err)
	{
		lua_pop(L, 1);
		throw;
	}
	lua_pop(L, 1);
	return new_oper;
}

mod_operand* load_single_operand(mod_function* parent, lua_State* L,
	int operator_index, size_t operand_index)
{
	lua_rawgeti(L, operator_index, operand_index);
	return load_operand_on_stack(parent, L);
}

void load_operands_into_container(mod_function* parent, lua_State* L,
	int index, vector<mod_operand*>& container)
{
	// index points to operator table.
	// first entry in the table is the type of the operator.
	size_t last_operand= lua_objlen(L, index);
	container.reserve(last_operand - 1);
	for(size_t cur= 2; cur <= last_operand; ++cur)
	{
		try
		{
			mod_operand* new_oper= load_single_operand(parent, L, index, cur);
			container.push_back(new_oper);
		}
		catch(std::string& err)
		{
			for(auto&& op : container) { delete op; }
			container.clear();
			throw;
		}
	}
}

void organize_simple_operands(mod_function* parent, mod_operator* self)
{
	self->m_operand_results.resize(self->m_operands.size());
	mut most_frequent_update_type= mut_never;
	mod_val_inputs input_for_never(0.0, 0.0);
	parent->set_input_fields(input_for_never);
	for(size_t op= 0; op < self->m_operands.size(); ++op)
	{
		mut op_update= self->m_operands[op]->get_update_type();
		if(op_update > most_frequent_update_type)
		{
			most_frequent_update_type= op_update;
		}
		switch(op_update)
		{
			case mut_never:
				self->m_operand_results[op]= self->m_operands[op]->evaluate(input_for_never);
				break;
			case mut_frame:
				self->m_per_frame_operands.push_back(op);
				break;
			case mut_note:
				self->m_per_note_operands.push_back(op);
				break;
			default:
				break;
		}
	}
	self->m_update_type= most_frequent_update_type;
	if(!self->m_per_frame_operands.empty())
	{
		parent->add_per_frame_operator(self);
	}
}

void mod_operator::load_from_lua(mod_function* parent, lua_State* L, int index)
{
	load_operands_into_container(parent, L, index, m_operands);
	organize_simple_operands(parent, this);
}

void mod_operator::per_frame_update(mod_val_inputs& input)
{
	for(auto&& op : m_per_frame_operands)
	{
		m_operand_results[op]= m_operands[op]->evaluate(input);
	}
}

void mod_operator::per_note_update(mod_val_inputs& input)
{
	for(auto&& op : m_per_note_operands)
	{
		m_operand_results[op]= m_operands[op]->evaluate(input);
	}
}

#define MOD_OP_NEEDS_THING(thing) \
bool mod_operator::needs_##thing() \
{ \
	for(auto&& op : m_operands) \
	{ \
		if(op->needs_##thing()) \
		{ \
			return true; \
		} \
	} \
	return false; \
}

MOD_OP_NEEDS_THING(beat);
MOD_OP_NEEDS_THING(second);
MOD_OP_NEEDS_THING(y_offset);

#undef MOD_OP_NEEDS_THING

void mod_operator_random::per_frame_update(mod_val_inputs& input)
{
	mod_operator::per_frame_update(input);
	m_eval_result= GAMESTATE->simple_stage_frandom(m_operand_results[0]);
}

void mod_operator_random::per_note_update(mod_val_inputs& input)
{
	mod_operator::per_note_update(input);
	m_eval_result= GAMESTATE->simple_stage_frandom(m_operand_results[0]);
}

static bool compare_phases(mod_operator_phase::phase const& left, mod_operator_phase::phase const& right)
{
	return left.start < right.start;
}

void mod_operator_phase::load_from_lua(mod_function* parent, lua_State* L, int index)
{
	// {"phase", operand, {default= {start, finish, mult, offset}, ...}}
	m_operands.push_back(load_single_operand(parent, L, index, 2));
	organize_simple_operands(parent, this);
	lua_rawgeti(L, index, 3);
	int phase_index= lua_gettop(L);
	if(!lua_istable(L, phase_index))
	{
		lua_pop(L, 1);
		throw std::string("Cannot create phase operator without phase table.");
	}
#define SET_PART(part, id) \
	lua_rawgeti(L, source, id); \
	part= lua_tonumber(L, -1); \
	lua_pop(L, 1);
#define LOAD_PHASE(dest) \
	int source= lua_gettop(L); \
	if(lua_istable(L, source)) \
	{ \
		SET_PART(dest.start, 1); \
		SET_PART(dest.finish, 2); \
		SET_PART(dest.mult, 3); \
		SET_PART(dest.offset, 4); \
	} \
	lua_pop(L, 1);
	lua_getfield(L, phase_index, "default");
	LOAD_PHASE(m_default_phase);
	size_t num_phases= lua_objlen(L, phase_index);
	m_phases.resize(num_phases);
	for(size_t i= 0; i < num_phases; ++i)
	{
		lua_rawgeti(L, phase_index, i+1);
		LOAD_PHASE(m_phases[i]);
	}
#undef LOAD_PHASE
#undef SET_PART
	lua_pop(L, 1);
	stable_sort(m_phases.begin(), m_phases.end(), compare_phases);

	if(m_update_type == mut_never)
	{
		phase_eval();
	}
}

mod_operator_phase::phase const* mod_operator_phase::find_phase(double input)
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

void mod_operator_phase::phase_eval()
{
	double res= m_operand_results[0];
	phase const* cur= find_phase(res);
	m_eval_result= ((res - cur->start) * cur->mult) + cur->offset;
}

void mod_operator_phase::per_frame_update(mod_val_inputs& input)
{
	mod_operator::per_frame_update(input);
	phase_eval();
}

void mod_operator_phase::per_note_update(mod_val_inputs& input)
{
	mod_operator::per_note_update(input);
	phase_eval();
}

void mod_operator_repeat::load_from_lua(mod_function* parent, lua_State* L, int index)
{
	mod_operator::load_from_lua(parent, L, index);
	if(m_operands.size() != 3)
	{
		throw std::string("Repeat operator must have exactly 3 operands: input, range begin, range end.");
	}

	if(m_update_type == mut_never)
	{
		repeat_eval();
	}
}

void mod_operator_repeat::per_frame_update(mod_val_inputs& input)
{
	mod_operator::per_frame_update(input);
	repeat_eval();
}

void mod_operator_repeat::per_note_update(mod_val_inputs& input)
{
	mod_operator::per_note_update(input);
	repeat_eval();
}

void mod_operator_repeat::repeat_eval()
{
	double op_result= m_operand_results[0];
	double const dist= m_operand_results[2] - m_operand_results[1];
	double const mod_res= fmod(op_result, dist);
	if(mod_res < 0.0)
	{
		m_eval_result= mod_res + dist + m_operand_results[1];
	}
	else
	{
		m_eval_result= mod_res + m_operand_results[1];
	}
}

void mod_operator_spline::load_from_lua(mod_function* parent, lua_State* L, int index)
{
	mod_operator::load_from_lua(parent, L, index);
	if(m_operands.size() < 2)
	{
		throw std::string("Spline operator must have at least two operands: t value and points.");
	}
	m_spline.resize(m_operands.size() - 1);
	for(size_t p= 1; p < m_operands.size(); ++p)
	{
		m_spline.set_point(p-1, static_cast<float>(m_operand_results[p]));
		switch(m_operands[p]->get_update_type())
		{
			case mut_frame:
				m_has_per_frame_points= true;
				break;
			case mut_note:
				m_has_per_note_points= true;
				break;
			default:
				break;
		}
	}
	m_loop= get_optional_bool(L, index, "loop", false);
	m_polygonal= get_optional_bool(L, index, "polygonal", false);
	if(!m_has_per_frame_points && !m_has_per_note_points)
	{
		m_spline.solve(m_loop, m_polygonal);
	}

	if(m_update_type == mut_never)
	{
		spline_eval();
	}
}

void mod_operator_spline::per_frame_update(mod_val_inputs& input)
{
	for(auto&& op : m_per_frame_operands)
	{
		m_operand_results[op]= m_operands[op]->evaluate(input);
		if(op > 0)
		{
			m_spline.set_point(op-1, m_operand_results[op]);
		}
	}
	if(m_has_per_frame_points && !m_has_per_note_points)
	{
		m_spline.solve(m_loop, m_polygonal);
	}
	if(m_operands[0]->get_update_type() == mut_frame)
	{
		spline_eval();
	}
}

void mod_operator_spline::per_note_update(mod_val_inputs& input)
{
	for(auto&& op : m_per_note_operands)
	{
		m_operand_results[op]= m_operands[op]->evaluate(input);
		if(op > 0)
		{
			m_spline.set_point(op-1, m_operand_results[op]);
		}
	}
	if(m_has_per_note_points)
	{
		m_spline.solve(m_loop, m_polygonal);
	}
	spline_eval();
}

void mod_operator_spline::spline_eval()
{
	m_eval_result= m_spline.evaluate(static_cast<float>(m_operand_results[0]), m_loop);
}

void mod_operator_lua::lua_eval()
{
	Lua* L= LUA->Get();
	m_func.PushSelf(L);
	for(auto&& res : m_operand_results)
	{
		lua_pushnumber(L, res);
	}
	std::string err= "Error running lua mod operator:  ";
	if(LuaHelpers::RunScriptOnStack(L, err, m_operand_results.size(), 1, true))
	{
		if(!lua_isnumber(L, 1))
		{
			LuaHelpers::ReportScriptError("lua mod operator didn't return a number.");
		}
		m_eval_result= lua_tonumber(L, 1);
	}
	lua_settop(L, 0);
	LUA->Release(L);
}

void mod_operator_lua::load_from_lua(mod_function* parent, lua_State* L, int index)
{
	lua_rawgeti(L, index, 1);
	m_func.SetFromStack(L);
	if(m_func.GetLuaType() != LUA_TFUNCTION)
	{
		throw std::string("First operand to mod operator lua must be a function.");
	}
	mod_operator::load_from_lua(parent, L, index);
	if(m_update_type == mut_never)
	{
		lua_eval();
	}
}

void mod_operator_lua::per_frame_update(mod_val_inputs& input)
{
	mod_operator::per_frame_update(input);
	if(m_per_note_operands.empty())
	{
		lua_eval();
	}
}

void mod_operator_lua::per_note_update(mod_val_inputs& input)
{
	mod_operator::per_note_update(input);
	lua_eval();
}

mod_function::~mod_function()
{
	Rage::safe_delete(m_base_operand);
}

void mod_function::load_from_lua(lua_State* L, int index,
	TimingData const* timing_data)
{
	if(m_base_operand != nullptr)
	{
		Rage::safe_delete(m_base_operand);
	}
	// {start_beat= 4, end_beat= 8, name= "frogger", priority= 0,
	//   sum_type= "add", {"add", 5, "music_beat"}}
	get_optional_string(L, index, "name", m_name);
	if(m_name.empty() && timing_data != nullptr)
	{
		m_name= unique_name("mod");
	}
	std::string sum_type;
	get_optional_string(L, index, "sum_type", sum_type);
	if(sum_type.empty())
	{
		m_sum_type= mot_add;
	}
	else
	{
		m_sum_type= str_to_mot(sum_type);
	}
	switch(m_sum_type)
	{
		case mot_replace:
		case mot_add:
		case mot_subtract:
		case mot_multiply:
		case mot_divide:
			break;
		default:
			throw std::string(fmt::sprintf("sum_type %s not supported for mod equation.", sum_type.c_str()));
	}
	m_priority= get_optional_int(L, index, "priority", 0);
	if(timing_data != nullptr)
	{
		double start= get_optional_double(L, index, "start", invalid_modfunction_time);
		if(start == invalid_modfunction_time)
		{
			throw std::string("lacks start time.");
		}
		double length= get_optional_double(L, index, "length", invalid_modfunction_time);
		if(length == invalid_modfunction_time)
		{
			throw std::string("lacks length.");
		}
		std::string time_name;
		get_optional_string(L, index, "time", time_name);
		if(time_name.empty() || time_name == "beat")
		{
			m_start_beat= start;
			m_start_second= timing_data->GetElapsedTimeFromBeat(static_cast<float>(m_start_beat));
			m_end_beat= m_start_beat + length;
			m_end_second= timing_data->GetElapsedTimeFromBeat(static_cast<float>(m_end_beat));
			m_length_beats= length;
			m_length_seconds= m_end_second - m_start_second;
		}
		else
		{
			m_start_second= start;
			m_start_beat= timing_data->GetBeatFromElapsedTime(static_cast<float>(m_start_second));
			m_end_second= m_start_second + length;
			m_end_beat= timing_data->GetBeatFromElapsedTime(static_cast<float>(m_end_second));
			m_length_seconds= length;
			m_length_beats= m_end_beat - m_start_beat;
		}
	}
	m_column= get_optional_double(L, index, "column", m_column);
	lua_rawgeti(L, index, 1);
	m_base_operand= load_operand_on_stack(this, L);
}

void mod_function::add_per_frame_operator(mod_operator* op)
{
	m_per_frame_operators.push_back(op);
}

#define MOD_FUNC_NEEDS_THING(thing) \
bool mod_function::needs_##thing() \
{ \
	return m_base_operand->needs_##thing(); \
}
MOD_FUNC_NEEDS_THING(beat);
MOD_FUNC_NEEDS_THING(second);
MOD_FUNC_NEEDS_THING(y_offset);

bool mod_function::needs_per_frame_update()
{
	return !m_per_frame_operators.empty();
}

void mod_function::per_frame_update(mod_val_inputs& input)
{
	for(auto&& op : m_per_frame_operators)
	{
		op->per_frame_update(input);
	}
}

double mod_function::evaluate(mod_val_inputs& input)
{
	input.column= m_column;
	return m_base_operand->evaluate(input);
}

double mod_function::evaluate_with_time(mod_val_inputs& input)
{
	set_input_fields(input);
	return m_base_operand->evaluate(input);
}

ModifiableValue::ModifiableValue(ModManager& man, std::string const& name,
	double value)
	:m_base_value(value)
{
	man.register_moddable(name, this);
}

ModifiableValue::~ModifiableValue()
{
	for(auto&& mod : m_mods)
	{
		delete mod;
	}
}

double ModifiableValue::evaluate(mod_val_inputs& input)
{
	input.prefunres= m_base_value;
	if(!m_mods.empty())
	{
		for(auto&& mod : m_mods)
		{
			input.column= m_column;
			switch(mod->m_sum_type)
			{
				case mot_replace:
					input.prefunres= mod->evaluate(input);
					break;
				case mot_add:
					input.prefunres+= mod->evaluate(input);
					break;
				case mot_subtract:
					input.prefunres-= mod->evaluate(input);
					break;
				case mot_multiply:
					input.prefunres*= mod->evaluate(input);
					break;
				case mot_divide:
					input.prefunres/= mod->evaluate(input);
					break;
				default:
					break;
			}
		}
	}
	if(!m_active_managed_mods.empty())
	{
		for(auto&& mod : m_active_managed_mods)
		{
			input.column= m_column;
			switch(mod.second->m_sum_type)
			{
				case mot_replace:
					input.prefunres= mod.second->evaluate_with_time(input);
					break;
				case mot_add:
					input.prefunres+= mod.second->evaluate_with_time(input);
					break;
				case mot_subtract:
					input.prefunres-= mod.second->evaluate_with_time(input);
					break;
				case mot_multiply:
					input.prefunres*= mod.second->evaluate_with_time(input);
					break;
				case mot_divide:
					input.prefunres/= mod.second->evaluate_with_time(input);
					break;
				default:
					break;
			}
		}
	}
	return input.prefunres;
}

std::list<mod_function*>::iterator ModifiableValue::find_mod(
	std::string const& name)
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

void ModifiableValue::add_mod(mod_function* func, ModManager& manager)
{
	auto insert_point= find_mod(func->get_name());
	if(insert_point == m_mods.end())
	{
		m_mods.push_back(func);
	}
	else
	{
		manager.remove_from_per_frame_update(*insert_point);
		delete *insert_point;
		*insert_point= func;
	}
	manager.add_to_per_frame_update(func);
#define MF_SET_NEEDS(thing) \
	if(func->needs_##thing()) { m_needs_##thing= true; }
	MF_SET_NEEDS(beat);
	MF_SET_NEEDS(second);
	MF_SET_NEEDS(y_offset);
#undef MF_SET_NEEDS
}

void ModifiableValue::remove_mod(std::string const& name, ModManager& manager)
{
	auto mod= find_mod(name);
	if(mod != m_mods.end())
	{
		manager.remove_from_per_frame_update(*mod);
		delete *mod;
		m_mods.erase(mod);
	}
}

void ModifiableValue::clear_mods(ModManager& manager)
{
	for(auto&& mod : m_mods)
	{
		manager.remove_from_per_frame_update(mod);
		delete mod;
	}
	m_mods.clear();
}

void ModifiableValue::add_mod_to_active_list(mod_function* mod)
{
	m_active_managed_mods.insert(std::make_pair(mod->m_priority, mod));
}

void ModifiableValue::remove_mod_from_active_list(mod_function* mod)
{
	auto iter= m_active_managed_mods.begin();
	for(; iter != m_active_managed_mods.end(); ++iter)
	{
		if(iter->second == mod)
		{
			m_active_managed_mods.erase(iter);
			return;
		}
	}
}

void ModifiableValue::clear_active_list()
{
	m_active_managed_mods.clear();
}

#define MV_NEEDS_THING(thing) \
bool ModifiableValue::needs_##thing() \
{ \
	return m_needs_##thing; \
}
MV_NEEDS_THING(beat);
MV_NEEDS_THING(second);
MV_NEEDS_THING(y_offset);

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
	return pos_mod.needs_##thing() || rot_mod.needs_##thing() || zoom_vmod.needs_##thing() || zoom_mod.needs_##thing(); \
}

MT_NEEDS_THING(beat);
MT_NEEDS_THING(second);
MT_NEEDS_THING(y_offset);
#undef MT_NEEDS_THING

// ModEquation is an object for setting an equation and evaluating it with
// arbitrary input.  Making it an Actor is the only practical way to allow
// the theme to make one and ensure that it is destroyed when it isn't needed
// anymore.
struct ModEquation : Actor
{
	ModEquation();
	~ModEquation() {}
	virtual void PushSelf(lua_State *L);
	virtual ModEquation* Copy() const;
	void set_equation(lua_State* L, int eq_index);
	void set_input_map(lua_State* L, int map_index);
	double evaluate(lua_State* L, int input_index);
	std::unordered_map<size_t, mit> m_input_map;
	mod_val_inputs m_input;
	ModManager m_manager;
	ModifiableValue m_eq_parent;
	mod_function m_equation;
};

ModEquation::ModEquation()
	:m_eq_parent(m_manager, "do_not_use", 1.0),
	m_equation(&m_eq_parent, 0.0)
{
}

REGISTER_ACTOR_CLASS(ModEquation);

void ModEquation::set_equation(lua_State* L, int eq_index)
{
	try
	{
		m_equation.load_from_lua(L, eq_index, nullptr);
	}
	catch(std::string& err)
	{
		luaL_error(L, err.c_str());
	}
}

void ModEquation::set_input_map(lua_State* L, int map_index)
{
	lua_pushnil(L);
	while(lua_next(L, map_index) != 0)
	{
		try
		{
			if(lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TNUMBER)
			{
				std::string mit_name= lua_tostring(L, -2);
				mit to= str_to_mit(mit_name);
				size_t from= lua_tointeger(L, -1);
				m_input_map.insert(std::make_pair(from, to));
			}
		}
		catch(...)
		{
			// don't actually care
		}
		lua_pop(L, 1);
	}
}

double ModEquation::evaluate(lua_State* L, int input_index)
{
	size_t num_inputs= lua_objlen(L, input_index);
	for(size_t id= 1; id <= num_inputs; ++id)
	{
		lua_rawgeti(L, input_index, id);
		double value= lua_tonumber(L, -1);
		lua_pop(L, 1);
		mit type= m_input_map[id];
#define THING_CASE(thing) case mit_##thing: m_input.thing = value; break;
		switch(type)
		{
			THING_CASE(column);
			THING_CASE(y_offset);
			THING_CASE(note_id_in_chart);
			THING_CASE(note_id_in_column);
			THING_CASE(row_id);
			THING_CASE(eval_beat);
			THING_CASE(eval_second);
			THING_CASE(music_beat);
			THING_CASE(music_second);
			THING_CASE(dist_beat);
			THING_CASE(dist_second);
			THING_CASE(start_beat);
			THING_CASE(start_second);
			THING_CASE(length_beats);
			THING_CASE(length_seconds);
			THING_CASE(end_beat);
			THING_CASE(end_second);
			THING_CASE(prefunres);
			default:
				break;
		}
#undef THING_CASE
	}
	m_equation.per_frame_update(m_input);
	return m_equation.evaluate(m_input);
}

struct LunaModEquation : Luna<ModEquation>
{
	static int set_equation(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Must be an equation table.");
		}
		p->set_equation(L, 1);
		COMMON_RETURN_SELF;
	}
	static int set_input_map(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Must be an input map table.");
		}
		p->set_input_map(L, 1);
		COMMON_RETURN_SELF;
	}
	static int evaluate(T* p, lua_State* L)
	{
		if(lua_type(L, 1) != LUA_TTABLE)
		{
			luaL_error(L, "Must be an input table.");
		}
		lua_pushnumber(L, p->evaluate(L, 1));
		return 1;
	}
	LunaModEquation()
	{
		ADD_METHOD(set_equation);
		ADD_METHOD(set_input_map);
		ADD_METHOD(evaluate);
	}
};

LUA_REGISTER_DERIVED_CLASS(ModEquation, Actor);


namespace
{
	static int get_operator_list(lua_State* L)
	{
		lua_createtable(L, 0, mot_conversion.size());
		int table_index= lua_gettop(L);
		for(auto&& entry : mot_conversion)
		{
			lua_pushstring(L, entry.first.c_str());
			lua_pushboolean(L, true);
			lua_settable(L, table_index);
		}
		return 1;
	}
	static int get_input_list(lua_State* L)
	{
		lua_createtable(L, 0, mit_conversion.size());
		int table_index= lua_gettop(L);
		for(auto&& entry : mit_conversion)
		{
			lua_pushstring(L, entry.first.c_str());
			lua_pushboolean(L, true);
			lua_settable(L, table_index);
		}
		return 1;
	}

	const luaL_Reg ModValueTable[] =
	{
		LIST_METHOD(get_operator_list),
		LIST_METHOD(get_input_list),
		{nullptr, nullptr}
	};
}

LUA_REGISTER_NAMESPACE(ModValue);
