#include "global.h"
#include "CubicSpline.h"
#include "EnumHelper.h"
#include "GameState.h"
#include "ModValue.h"
#include "RageLog.h"
#include "RageMath.h"
#include "LuaBinding.h"

enum mut
{
	mut_never,
	mut_frame,
	mut_note
};

struct mod_operand
{
	virtual ~mod_operand() {}
	virtual mut get_update_type()= 0;
	virtual double evaluate(mod_val_inputs& input)= 0;
	virtual bool needs_beat()= 0;
	virtual bool needs_second()= 0;
	virtual bool needs_y_offset()= 0;

	mut m_update_type;
};

struct mod_input_number : mod_operand
{
	double m_value;
	virtual double evaluate(mod_val_inputs&)
	{ return m_value;	}
	virtual mut get_update_type()
	{ return mut_never; }
	virtual bool needs_beat()
	{ return false; }
	virtual bool needs_second()
	{ return false; }
	virtual bool needs_y_offset()
	{ return false; }
};

#define SIMPLE_MOD_INPUT_TYPE(field_name, update, beat, second, yoff)	\
struct mod_input_##field_name : mod_operand \
{ \
	virtual double evaluate(mod_val_inputs& input) \
	{ return input.field_name; } \
	virtual mut get_update_type() \
	{ return update; } \
	virtual bool needs_beat() \
	{ return beat; } \
	virtual bool needs_second() \
	{ return second; } \
	virtual bool needs_y_offset() \
	{ return yoff; } \
};

SIMPLE_MOD_INPUT_TYPE(column, mut_never, false, false, false);
SIMPLE_MOD_INPUT_TYPE(y_offset, mut_note, false, false, true);
SIMPLE_MOD_INPUT_TYPE(note_id_in_chart, mut_note, false, false, false);
SIMPLE_MOD_INPUT_TYPE(note_id_in_column, mut_note, false, false, false);
SIMPLE_MOD_INPUT_TYPE(row_id, mut_note, false, false, false);
SIMPLE_MOD_INPUT_TYPE(eval_beat, mut_note, true, false, false);
SIMPLE_MOD_INPUT_TYPE(eval_second, mut_note, false, true, false);
SIMPLE_MOD_INPUT_TYPE(music_beat, mut_frame, true, false, false);
SIMPLE_MOD_INPUT_TYPE(music_second, mut_frame, false, true, false);
SIMPLE_MOD_INPUT_TYPE(dist_beat, mut_note, true, false, false);
SIMPLE_MOD_INPUT_TYPE(dist_second, mut_note, false, true, false);
// start_dist and end dist can be done with:
// {"MO_subtract", "MI_music_beat" "MI_start_beat"}
SIMPLE_MOD_INPUT_TYPE(start_beat, mut_never, true, false, false);
SIMPLE_MOD_INPUT_TYPE(start_second, mut_never, false, true, false);
SIMPLE_MOD_INPUT_TYPE(end_beat, mut_never, true, false, false);
SIMPLE_MOD_INPUT_TYPE(end_second, mut_never, false, true, false);

#undef SIMPLE_MOD_INPUT_TYPE

enum mit
{
	mit_number,
	mit_column,
	mit_y_offset,
	mit_note_id_in_chart,
	mit_note_id_in_column,
	mit_row_id,
	mit_eval_beat,
	mit_eval_second,
	mit_music_beat,
	mit_music_second,
	mit_dist_beat,
	mit_dist_second,
	mit_start_beat,
	mit_start_second,
	mit_end_beat,
	mit_end_second
};

struct mod_operator : mod_operand
{
	virtual ~mod_operator()
	{
		for(auto&& op : m_operands)
		{
			delete op;
			op= nullptr;
		}
	}
	virtual double evaluate(mod_val_inputs& input)= 0;
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	mut get_update_type() { return m_update_type; }
	virtual void per_frame_update(mod_val_inputs& input);
	virtual void per_note_update(mod_val_inputs& input);
	virtual bool needs_beat();
	virtual bool needs_second();
	virtual bool needs_y_offset();

	mut m_update_type;
	vector<mod_operand*> m_operands;
	vector<double> m_operand_results;
	vector<size_t> m_per_note_operands;
	vector<size_t> m_per_frame_operands;
};

static mod_operand* load_single_operand(mod_function* parent, lua_State* L, int operator_index,
	size_t operand_index);
static void load_operands_into_container(mod_function* parent, lua_State* L, int index,
	vector<mod_operand*>& container);
static void organize_simple_operands(mod_function* parent, mod_operator* self);
static mod_operand* create_simple_input(std::string const& input_type, double value);

#define PER_NOTE_UPDATE_IF if(!m_per_note_operands.empty()) { per_note_update(input); }
#define EVAL_RESULT_EVAL \
virtual double evaluate(mod_val_inputs& input) \
{ \
	PER_NOTE_UPDATE_IF; \
	return m_eval_result; \
}

#define plus_wrap(left, right) (left + right)
#define subtract_wrap(left, right) (left - right)
#define multiply_wrap(left, right) (left * right)
#define divide_wrap(left, right) (left / right)

#define SIMPLE_OPERATOR(op_name, eval) \
struct mod_operator_##op_name : mod_operator \
{ \
	virtual double evaluate(mod_val_inputs& input) \
	{ \
		PER_NOTE_UPDATE_IF; \
		double ret= m_operand_results[0]; \
		for(size_t cur= 1; cur < m_operand_results.size(); ++cur) \
		{ \
			ret= eval(ret, m_operand_results[cur]); \
		} \
		return ret; \
	} \
	void add_simple_operand(std::string const& input_type, double value) \
	{ m_operands.push_back(create_simple_input(input_type, value)); } \
};

static double log_wrapper(double left, double right)
{
	return log(left) / log(right);
}

SIMPLE_OPERATOR(add, plus_wrap);
SIMPLE_OPERATOR(subtract, subtract_wrap);
SIMPLE_OPERATOR(multiply, multiply_wrap);
SIMPLE_OPERATOR(divide, divide_wrap);
SIMPLE_OPERATOR(exp, pow);
SIMPLE_OPERATOR(log, log_wrapper);

#undef plus_wrap
#undef subtract_wrap
#undef multiply_wrap
#undef divide_wrap
#undef SIMPLE_OPERATOR

#define WAVE_OPERATOR(op_name, wave_eval) \
struct mod_operator_##op_name : mod_operator \
{ \
	EVAL_RESULT_EVAL; \
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index) \
	{ \
		mod_operator::load_from_lua(parent, L, index); \
		if(m_operands.size() != 1) \
		{ \
			throw std::string("Wave functions only take one operand."); \
		} \
		if(m_update_type == mut_never) \
		{ \
			double angle= m_operand_results[0]; \
			m_eval_result= (wave_eval); \
		} \
	} \
	virtual void per_frame_update(mod_val_inputs& input) \
	{ \
		mod_operator::per_frame_update(input); \
		double angle= m_operand_results[0]; \
		m_eval_result= (wave_eval); \
	} \
	virtual void per_note_update(mod_val_inputs& input) \
	{ \
		mod_operator::per_note_update(input); \
		double angle= m_operand_results[0]; \
		m_eval_result= (wave_eval); \
	} \
	double m_eval_result; \
};

static double square_wave(double angle)
{
	angle= fmod(angle, Rage::D_PI * 2.0);
	if(angle < 0.0)
	{
		angle+= Rage::D_PI * 2.0;
	}
	return angle >= Rage::D_PI ? -1.0 : 1.0;
}

static double triangle_wave(double angle)
{
	angle= fmod(angle, Rage::D_PI * 2.0);
	if(angle < 0.0)
	{
		angle+= Rage::D_PI * 2.0;
	}
	double result= angle * Rage::D_PI_REC;
	if(result < .5)
	{
		return result * 2.0;
	}
	else if(result < 1.5)
	{
		return 1.0 - ((result - .5) * 2.0);
	}
	else
	{
		return -4.0 + (result * 2.0);
	}
}

WAVE_OPERATOR(sine, (Rage::FastSin(angle)));
WAVE_OPERATOR(cos, (Rage::FastCos(angle)));
WAVE_OPERATOR(tan, (tan(angle)));
WAVE_OPERATOR(square, (square_wave(angle)));
WAVE_OPERATOR(triangle, (triangle_wave(angle)));

#undef WAVE_OPERATOR

struct mod_operator_random : mod_operator
{
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index)
	{
		mod_operator::load_from_lua(parent, L, index);
		if(m_operands.size() != 1)
		{
			throw std::string("Random operator only takes one operand.");
		}
		if(m_update_type == mut_never)
		{
			m_eval_result= GAMESTATE->simple_stage_frandom(m_operand_results[0]);
		}
	}
	EVAL_RESULT_EVAL;
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);
	double m_eval_result;
};

struct mod_operator_phase : mod_operator
{
	EVAL_RESULT_EVAL;
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	struct phase
	{
		phase()
			:start(0.0), finish(0.0), mult(1.0), offset(0.0)
		{}
		double start;
		double finish;
		double mult;
		double offset;
	};
	phase const* find_phase(double input);
	void phase_eval();
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);
	double m_eval_result;
	vector<phase> m_phases;
	phase m_default_phase;
};

struct mod_operator_repeat : mod_operator
{
	mod_operator_repeat()
		:m_rep_begin(0.0), m_rep_end(0.0)
	{}
	EVAL_RESULT_EVAL;
	void repeat_eval();
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);
	double m_eval_result;
	double m_rep_begin;
	double m_rep_end;
};

struct mod_operator_spline : mod_operator
{
	mod_operator_spline()
		:m_loop(false), m_polygonal(false), m_has_per_frame_points(false),
		 m_has_per_note_points(false)
	{}
	EVAL_RESULT_EVAL;
	void spline_eval();
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);

	double m_eval_result;
	CubicSpline m_spline;
	bool m_loop;
	bool m_polygonal;
	bool m_has_per_frame_points;
	bool m_has_per_note_points;
};

#undef EVAL_RESULT_EVAL

enum mot
{
	mot_add,
	mot_subtract,
	mot_multiply,
	mot_divide,
	mot_exp,
	mot_log,
	mot_sine,
	mot_cos,
	mot_tan,
	mot_square,
	mot_triangle,
	mot_random,
	mot_phase,
	mot_repeat,
	mot_spline
};

static mod_operand* create_mod_operator(mod_function* parent, lua_State* L, int index);
static mod_operand* create_mod_input(lua_State* L, int index);
static mod_operand* load_operand_on_stack(mod_function* parent, lua_State* L);

struct mod_function
{
	mod_function(ModifiableValue* parent, double col)
		:m_priority(0), m_sum_type(mot_add),
		 m_start_beat(invalid_modfunction_time),
		 m_start_second(invalid_modfunction_time),
		 m_end_beat(invalid_modfunction_time),
		 m_end_second(invalid_modfunction_time),
		 m_base_operand(nullptr), m_column(col),
		 m_parent(parent)
	{}
	~mod_function();
	void change_parent(ModifiableValue* new_parent)
	{
		m_parent= new_parent;
	}


	double evaluate(mod_val_inputs& input);
	double evaluate_with_time(mod_val_inputs& input);
	void load_from_lua(lua_State* L, int index, uint32_t col);
	void simple_load(std::string const& name, std::string const& input_type,
		double value);
	void add_per_frame_operator(mod_operator* op);

	void calc_unprovided_times(TimingData const* timing);
	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();
	std::string const& get_name() { return m_name; }
	void set_input_fields(mod_val_inputs& input)
	{
		input.column= m_column;
		input.set_time(m_start_beat, m_start_second, m_end_beat, m_end_second);
	}

	// needs_per_frame_update exists so that ModifiableValue can check after
	// creating a mod_function to see if it needs to be added to the manager for
	// solving every frame.
	bool needs_per_frame_update();
	void per_frame_update(mod_val_inputs& input);

	int m_priority;
	mot m_sum_type;
	double m_start_beat;
	double m_start_second;
	double m_end_beat;
	double m_end_second;

private:
	// do not use mod_function::operator=, too much pointer handling.
	mod_function& operator=(mod_function&)
	{return *this;}
	mod_operand* m_base_operand;
	vector<mod_operator*> m_per_frame_operators;
	double m_column;

	std::string m_name;
	ModifiableValue* m_parent;
};


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

void ModManager::remove_from_present(std::list<func_and_parent>::iterator fapi)
{
	remove_from_per_frame_update(fapi->func);
	fapi->parent->remove_mod_from_active_list(fapi->func);
	m_present_funcs.erase(fapi);
}


static mot str_to_mot(std::string const& str)
{
#define CONVENT(field_name) {#field_name, mot_##field_name}
	static std::unordered_map<std::string, mot> conversion= {
		{"+", mot_add},
		{"-", mot_subtract},
		{"*", mot_multiply},
		{"/", mot_divide},
		{"^", mot_exp},
		{"v", mot_log},
		CONVENT(add),
		CONVENT(subtract),
		CONVENT(multiply),
		CONVENT(divide),
		CONVENT(exp),
		CONVENT(log),
		CONVENT(sine),
		CONVENT(cos),
		CONVENT(tan),
		CONVENT(square),
		CONVENT(triangle),
		CONVENT(random),
		CONVENT(phase),
		CONVENT(repeat),
		CONVENT(spline)
	};
#undef CONVENT
	auto entry= conversion.find(str);
	if(entry == conversion.end())
	{
		throw std::string(fmt::sprintf("Invalid mod operator type: %s", str.c_str()));
	}
	return entry->second;
}

static mod_operand* create_mod_operator(mod_function* parent, lua_State* L, int index)
{
	lua_rawgeti(L, index, 1);
	if(lua_type(L, -1) != LUA_TSTRING)
	{
		lua_pop(L, 1);
		throw std::string("Mod operator type must be a string.");
	}
	std::string str_type= lua_tostring(L, -1);
	lua_pop(L, 1);
	mot op_type= str_to_mot(str_type);
	mod_operator* new_oper= nullptr;
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
		SET_NEW(sine);
		SET_NEW(cos);
		SET_NEW(tan);
		SET_NEW(square);
		SET_NEW(triangle);
		SET_NEW(random);
		SET_NEW(phase);
		SET_NEW(repeat);
		SET_NEW(spline);
		default: break;
	}
#undef SET_NEW
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

static mit str_to_mit(std::string const& str)
{
#define CONVENT(field_name) {#field_name, mit_##field_name}
	static std::unordered_map<std::string, mit> conversion= {
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
		CONVENT(end_beat),
		CONVENT(end_second)
	};
#undef CONVENT
	auto entry= conversion.find(str);
	if(entry == conversion.end())
	{
		throw std::string(fmt::sprintf("Invalid mod input type: %s", str.c_str()));
	}
	return entry->second;
}

static mod_operand* create_mod_input(std::string const& str_type)
{
	mit type= str_to_mit(str_type);
#define RET_ENTRY(field_name) \
	case mit_##field_name: return new mod_input_##field_name;
#define ENTRY_BS_PAIR(base) RET_ENTRY(base##_beat); RET_ENTRY(base##_second);
	switch(type)
	{
		RET_ENTRY(column);
		RET_ENTRY(y_offset);
		RET_ENTRY(note_id_in_chart);
		RET_ENTRY(note_id_in_column);
		RET_ENTRY(row_id);
		ENTRY_BS_PAIR(eval);
		ENTRY_BS_PAIR(music);
		ENTRY_BS_PAIR(dist);
		ENTRY_BS_PAIR(start);
		ENTRY_BS_PAIR(end);
		default: break;
	}
#undef ENTRY_BS_PAIR
#undef RET_ENTRY
	throw std::string(fmt::sprintf("Unknown mod input type: %s", str_type.c_str()));
}

static mod_operand* create_mod_input(lua_State* L, int index)
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

static mod_operand* create_simple_input(std::string const& input_type, double value)
{
	if(input_type == "number")
	{
		mod_input_number* op= new mod_input_number;
		op->m_value= value;
		return op;
	}
	else
	{
		try
		{
			mod_operand* op= create_mod_input(input_type);
			return op;
		}
		catch(std::string& err)
		{
			LuaHelpers::ReportScriptError("lol kyz messed up");
			LuaHelpers::ReportScriptError(err);
			mod_input_number* op= new mod_input_number;
			op->m_value= value;
			return op;
		}
	}
}

static mod_operand* load_operand_on_stack(mod_function* parent, lua_State* L)
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

static mod_operand* load_single_operand(mod_function* parent, lua_State* L, int operator_index,
	size_t operand_index)
{
	lua_rawgeti(L, operator_index, operand_index);
	return load_operand_on_stack(parent, L);
}

static void load_operands_into_container(mod_function* parent, lua_State* L, int index,
	vector<mod_operand*>& container)
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

static void organize_simple_operands(mod_function* parent, mod_operator* self)
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

static void get_numbers(lua_State* L, int index, vector<double*> const& ret)
{
	for(size_t i= 0; i < ret.size(); ++i)
	{
		lua_rawgeti(L, index, i+1);
		(*ret[i]) = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
}

static void load_one_phase(lua_State* L, int index,
	mod_operator_phase::phase& dest)
{
	if(lua_istable(L, index))
	{
		get_numbers(L, index, {&dest.start, &dest.finish, &dest.mult, &dest.offset});
	}
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
	lua_getfield(L, phase_index, "default");
	load_one_phase(L, lua_gettop(L), m_default_phase);
	lua_pop(L, 1);
	size_t num_phases= lua_objlen(L, phase_index);
	m_phases.resize(num_phases);
	for(size_t i= 0; i < num_phases; ++i)
	{
		lua_rawgeti(L, phase_index, i+1);
		load_one_phase(L, lua_gettop(L), m_phases[i]);
		lua_pop(L, 1);
	}
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
	m_operands.push_back(load_single_operand(parent, L, index, 2));
	organize_simple_operands(parent, this);
	m_rep_begin= 0.0;
	m_rep_end= 0.0;
	lua_rawgeti(L, index, 3);
	if(lua_isnumber(L, -1))
	{
		m_rep_begin= lua_tonumber(L, -1);
	}
	lua_pop(L, 1);
	lua_rawgeti(L, index, 4);
	if(lua_isnumber(L, -1))
	{
		m_rep_end= lua_tonumber(L, -1);
	}
	lua_pop(L, 1);

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
	double const dist= m_rep_end - m_rep_begin;
	double const mod_res= fmod(op_result, dist);
	if(mod_res < 0.0)
	{
		m_eval_result= mod_res + dist + m_rep_begin;
	}
	else
	{
		m_eval_result= mod_res + m_rep_begin;
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
	m_loop= get_optional_bool(L, index, "loop");
	m_polygonal= get_optional_bool(L, index, "polygonal");
	if(!m_has_per_frame_points && !m_has_per_note_points)
	{
		m_spline.solve(m_loop, m_polygonal);
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

mod_function::~mod_function()
{
	delete m_base_operand;
}

void mod_function::load_from_lua(lua_State* L, int index, uint32_t col)
{
	// {start_beat= 4, end_beat= 8, name= "frogger", priority= 0,
	//   sum_type= "add", {"add", 5, "music_beat"}}
	lua_getfield(L, index, "name");
	if(lua_type(L, -1) == LUA_TSTRING)
	{
		m_name= lua_tostring(L, -1);
	}
	else
	{
		m_name= unique_name("mod");
	}
	lua_pop(L, 1);
	lua_getfield(L, index, "sum_type");
	if(lua_type(L, -1) == LUA_TSTRING)
	{
		std::string str_type= lua_tostring(L, -1);
		lua_pop(L, 1);
		m_sum_type= str_to_mot(str_type);
		switch(m_sum_type)
		{
			case mot_add:
			case mot_subtract:
			case mot_multiply:
			case mot_divide:
				break;
			default:
				throw std::string("sum_type not supported for mod function.");
		}
	}
	else
	{
		lua_pop(L, 1);
	}
	m_priority= get_optional_int(L, index, "priority", 0);
	m_start_beat= get_optional_double(L, index, "start_beat", invalid_modfunction_time);
	m_start_second= get_optional_double(L, index, "start_second", invalid_modfunction_time);
	m_end_beat= get_optional_double(L, index, "end_beat", invalid_modfunction_time);
	m_end_second= get_optional_double(L, index, "end_second", invalid_modfunction_time);
	m_column= get_optional_double(L, index, "column", col);
	lua_rawgeti(L, index, 1);
	m_base_operand= load_operand_on_stack(this, L);
}

void mod_function::simple_load(std::string const& name, std::string const& input_type, double value)
{
	m_name= name;
	if(input_type == "number")
	{
		mod_input_number* op= new mod_input_number;
		op->m_value= value;
		m_base_operand= op;
	}
	else
	{
		mod_operator_multiply* op= new mod_operator_multiply;
		op->add_simple_operand(input_type, 0.0);
		op->add_simple_operand("number", value);
		m_base_operand= op;
	}
}

void mod_function::add_per_frame_operator(mod_operator* op)
{
	m_per_frame_operators.push_back(op);
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

void mod_function::calc_unprovided_times(TimingData const* timing)
{
	calc_timing_pair(timing, m_start_beat, m_start_second);
	calc_timing_pair(timing, m_end_beat, m_end_second);
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

static mod_function* create_mod_function(ModifiableValue* parent,
	lua_State* L, int index, double col)
{
	mod_function* ret= new mod_function(parent, col);
	try
	{
		ret->load_from_lua(L, index, col);
	}
	catch(std::string& err)
	{
		delete ret;
		throw;
	}
	return ret;
}

static mod_function* create_simple_mod_function(ModifiableValue* parent,
	std::string const& name, std::string const& input_type, double value)
{
	mod_function* ret= new mod_function(parent, 0.0);
	ret->simple_load(name, input_type, value);
	return ret;
}

ModifiableValue::ModifiableValue(ModManager* man, double value)
	:m_manager(man)
{
	if(value != 0.0)
	{
		add_simple_mod("base_value", "number", value);
	}
}

ModifiableValue::~ModifiableValue()
{
	clear_mods();
	clear_managed_mods();
}

double ModifiableValue::evaluate(mod_val_inputs& input)
{
	double sum= 0;
	if(!m_mods.empty())
	{
		for(auto&& mod : m_mods)
		{
			input.column= m_column;
			switch(mod->m_sum_type)
			{
				case mot_add:
					sum+= mod->evaluate(input);
					break;
				case mot_subtract:
					sum-= mod->evaluate(input);
					break;
				case mot_multiply:
					sum*= mod->evaluate(input);
					break;
				case mot_divide:
					sum/= mod->evaluate(input);
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
				case mot_add:
					sum+= mod.second->evaluate_with_time(input);
					break;
				case mot_subtract:
					sum-= mod.second->evaluate_with_time(input);
					break;
				case mot_multiply:
					sum*= mod.second->evaluate_with_time(input);
					break;
				case mot_divide:
					sum/= mod.second->evaluate_with_time(input);
					break;
				default:
					break;
			}
		}
	}
	return sum;
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

void ModifiableValue::insert_mod_internal(mod_function* new_mod)
{
	auto insert_point= find_mod(new_mod->get_name());
	if(insert_point == m_mods.end())
	{
		m_mods.push_back(new_mod);
	}
	else
	{
		m_manager->remove_from_per_frame_update(*insert_point);
		delete *insert_point;
		*insert_point= new_mod;
	}
	m_manager->add_to_per_frame_update(new_mod);
#define MF_SET_NEEDS(thing) \
	if(new_mod->needs_##thing()) { m_needs_##thing= true; }
	MF_SET_NEEDS(beat);
	MF_SET_NEEDS(second);
	MF_SET_NEEDS(y_offset);
#undef MF_SET_NEEDS
}

void ModifiableValue::add_mod(lua_State* L, int index)
{
	insert_mod_internal(create_mod_function(this, L, index, m_column));
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

void ModifiableValue::add_managed_mod(lua_State* L, int index)
{
	mod_function* new_mod= create_mod_function(this, L, index, m_column);
	new_mod->calc_unprovided_times(m_timing);
	auto mod= m_managed_mods.find(new_mod->get_name());
	if(mod == m_managed_mods.end())
	{
		m_managed_mods.insert(make_pair(new_mod->get_name(), new_mod));
	}
	else
	{
		m_manager->remove_mod(mod->second);
		delete mod->second;
		mod->second= new_mod;
	}
	m_manager->add_mod(new_mod, this);
	//m_manager->dump_list_status();
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
	m_active_managed_mods.clear();
	m_managed_mods.clear();
}

void ModifiableValue::add_mod_to_active_list(mod_function* mod)
{
	m_active_managed_mods.insert(std::make_pair(mod->m_priority, mod));
}

void ModifiableValue::remove_mod_from_active_list(mod_function* mod)
{
	auto iter= m_active_managed_mods.find(mod->m_priority);
	for(; iter != m_active_managed_mods.end(); ++iter)
	{
		if(iter->second == mod)
		{
			m_active_managed_mods.erase(iter);
			return;
		}
	}
}

void ModifiableValue::add_simple_mod(std::string const& name,
	std::string const& input_type, double value)
{
	insert_mod_internal(create_simple_mod_function(this, name, input_type, value));
}

void ModifiableValue::take_over_mods(ModifiableValue& other)
{
	m_mods.swap(other.m_mods);
	for(auto&& mod : m_mods)
	{
		mod->change_parent(this);
		m_manager->add_to_per_frame_update(mod);
	}
	m_managed_mods.swap(other.m_managed_mods);
	for(auto&& mod : m_managed_mods)
	{
		mod.second->change_parent(this);
		m_manager->add_mod(mod.second, this);
	}
	m_active_managed_mods.swap(other.m_active_managed_mods);
	other.clear_managed_mods();
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
	return pos_mod.needs_##thing() || rot_mod.needs_##thing() || zoom_mod.needs_##thing(); \
}

MT_NEEDS_THING(beat);
MT_NEEDS_THING(second);
MT_NEEDS_THING(y_offset);
#undef MT_NEEDS_THING


struct LunaModifiableValue : Luna<ModifiableValue>
{
	static int add_mod(T* p, lua_State* L)
	{
		try
		{
			p->add_mod(L, lua_gettop(L));
		}
		catch(std::string& err)
		{
			luaL_error(L, err.c_str());
		}
		COMMON_RETURN_SELF;
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
		try
		{
			p->add_managed_mod(L, lua_gettop(L));
		}
		catch(std::string& err)
		{
			luaL_error(L, err.c_str());
		}
		COMMON_RETURN_SELF;
	}
	static int add_managed_mod_set(T* p, lua_State* L)
	{
		if(!lua_istable(L, 1))
		{
			luaL_error(L, "Arg for add_managed_mod_set must be a table of ModFunctins.");
		}
		size_t num_mods= lua_objlen(L, 1);
		for(size_t m= 1; m <= num_mods; ++m)
		{
			lua_rawgeti(L, 1, m);
			try
			{
				p->add_managed_mod(L, lua_gettop(L));
			}
			catch(std::string& err)
			{
				lua_pop(L, 1);
				luaL_error(L, err.c_str());
			}
			lua_pop(L, 1);
		}
		COMMON_RETURN_SELF;
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
	static int evaluate(T* p, lua_State* L)
	{
		mod_val_inputs input(FArg(1), FArg(2), FArg(3), FArg(4), FArg(5), nullptr);
		lua_pushnumber(L, p->evaluate(input));
		return 1;
	}
	LunaModifiableValue()
	{
		ADD_METHOD(add_mod);
		ADD_METHOD(remove_mod);
		ADD_METHOD(clear_mods);
		ADD_METHOD(add_managed_mod);
		ADD_METHOD(add_managed_mod_set);
		ADD_METHOD(remove_managed_mod);
		ADD_METHOD(clear_managed_mods);
		ADD_METHOD(evaluate);
	}
};
LUA_REGISTER_CLASS(ModifiableValue);
