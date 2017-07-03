#ifndef MOD_VALUE_INTERNAL_H
#define MOD_VALUE_INTERNAL_H

// Things that logically belong in a header file, but should not be used
// outside of ModValue.cpp.

enum mut
{
	mut_never,
	mut_frame,
	mut_note
};

enum mit
{
	mit_number,
	mit_music_rate,
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
	mit_length_beats,
	mit_length_seconds,
	mit_end_beat,
	mit_end_second,
	mit_prefunres
};

enum mot
{
	mot_replace,
	mot_add,
	mot_subtract,
	mot_multiply,
	mot_divide,
	mot_exp,
	mot_log,
	mot_min,
	mot_max,
	mot_sin,
	mot_cos,
	mot_tan,
	mot_square,
	mot_triangle,
	mot_asin,
	mot_acos,
	mot_atan,
	mot_asquare,
	mot_atriangle,
	mot_random,
	mot_phase,
	mot_repeat,
	mot_abs,
	mot_mod,
	mot_floor,
	mot_ceil,
	mot_round,
	mot_spline,
	mot_lua
};

mit str_to_mit(std::string const& str);
mot str_to_mot(std::string const& str);

struct mod_operand
{
	virtual ~mod_operand() {}
	virtual mut get_update_type()= 0;
	virtual double evaluate(mod_val_inputs& input)= 0;
	virtual bool needs_beat()= 0;
	virtual bool needs_second()= 0;
	virtual bool needs_y_offset()= 0;
};

struct mod_input_number : mod_operand
{
	double m_value;
	virtual double evaluate(mod_val_inputs&)
	{ return m_value; }
	virtual mut get_update_type()
	{ return mut_never; }
	virtual bool needs_beat()
	{ return false; }
	virtual bool needs_second()
	{ return false; }
	virtual bool needs_y_offset()
	{ return false; }
};

struct mod_input_music_rate : mod_operand
{
	virtual double evaluate(mod_val_inputs&)
	{ return GAMESTATE->get_hasted_music_rate(); }
	virtual mut get_update_type()
	{ return mut_frame; }
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
// start dist and end dist can be done with:
// {"subtract", "music_beat" "start_beat"}
SIMPLE_MOD_INPUT_TYPE(start_beat, mut_never, true, false, false);
SIMPLE_MOD_INPUT_TYPE(start_second, mut_never, false, true, false);
SIMPLE_MOD_INPUT_TYPE(length_beats, mut_never, true, false, false);
SIMPLE_MOD_INPUT_TYPE(length_seconds, mut_never, false, true, false);
SIMPLE_MOD_INPUT_TYPE(end_beat, mut_never, true, false, false);
SIMPLE_MOD_INPUT_TYPE(end_second, mut_never, false, true, false);
SIMPLE_MOD_INPUT_TYPE(prefunres, mut_note, false, false, false);

#undef SIMPLE_MOD_INPUT_TYPE

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

mod_operand* create_mod_operator(mod_function* parent, lua_State* L, int index);
mod_operand* create_mod_input(lua_State* L, int index);
mod_operand* load_operand_on_stack(mod_function* parent, lua_State* L);
mod_operand* load_single_operand(mod_function* parent, lua_State* L, int operator_index,
	size_t operand_index);
void load_operands_into_container(mod_function* parent, lua_State* L, int index,
	vector<mod_operand*>& container);
void organize_simple_operands(mod_function* parent, mod_operator* self);

#define PER_NOTE_UPDATE_IF if(!m_per_note_operands.empty()) { per_note_update(input); }
#define EVAL_RESULT_EVAL \
virtual double evaluate(mod_val_inputs& input) \
{ \
	PER_NOTE_UPDATE_IF; \
	return m_eval_result; \
}

struct mod_operator_replace : mod_operator
{
	virtual void load_from_lua(mod_function*, lua_State*, int)
	{
		throw std::string("mod operator replace really just exists to be used as a sum type.");
	}
};

#define plus_wrap(left, right) (left + right)
#define subtract_wrap(left, right) (left - right)
#define multiply_wrap(left, right) (left * right)

double divide_wrap(double left, double right)
{
	if(right == 0.0)
	{
		return 0.0;
	}
	return left / right;
}

#define SIMPLE_OPERATOR(op_name, eval, single_op) \
struct mod_operator_##op_name : mod_operator \
{ \
	virtual double evaluate(mod_val_inputs& input) \
	{ \
		PER_NOTE_UPDATE_IF; \
		double ret= single_op(m_operand_results[0]); \
		size_t size= m_operand_results.size(); \
		for(size_t cur= 1; cur < size; ++cur) \
		{ \
			ret= eval(ret, m_operand_results[cur]); \
		} \
		return ret; \
	} \
};

double log_wrapper(double left, double right)
{
	return left / log(right);
}

double log_single_op(double left)
{
	return log(left);
}

double pow_wrapper(double left, double right)
{
	double result= std::pow(left, right);
	if(std::isnan(result) || !std::isfinite(result))
	{
		return 0.0;
	}
	return result;
}

#define same_thing_single_op(thing) thing

SIMPLE_OPERATOR(add, plus_wrap, same_thing_single_op);
SIMPLE_OPERATOR(subtract, subtract_wrap, same_thing_single_op);
SIMPLE_OPERATOR(multiply, multiply_wrap, same_thing_single_op);
SIMPLE_OPERATOR(divide, divide_wrap, same_thing_single_op);
SIMPLE_OPERATOR(exp, pow_wrapper, same_thing_single_op);
SIMPLE_OPERATOR(log, log_wrapper, log_single_op);
SIMPLE_OPERATOR(min, std::min, same_thing_single_op);
SIMPLE_OPERATOR(max, std::max, same_thing_single_op);

#undef plus_wrap
#undef subtract_wrap
#undef multiply_wrap
#undef SIMPLE_OPERATOR

#define SINGLE_OPERATOR(op_name, eval) \
struct mod_operator_##op_name : mod_operator \
{ \
	virtual double evaluate(mod_val_inputs& input) \
	{ \
		PER_NOTE_UPDATE_IF; \
		return eval(m_operand_results[0]); \
	} \
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index) \
	{ \
		mod_operator::load_from_lua(parent, L, index); \
		if(m_operands.size() != 1) \
		{ \
			throw std::string(#op_name " operator must have exactly 1 operand."); \
		} \
	} \
};

SINGLE_OPERATOR(abs, fabs);

#undef SINGLE_OPERATOR

#define PAIR_OPERATOR(op_name, eval) \
struct mod_operator_##op_name : mod_operator \
{ \
	virtual double evaluate(mod_val_inputs& input) \
	{ \
		PER_NOTE_UPDATE_IF; \
		return eval(m_operand_results[0], m_operand_results[1]); \
	} \
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index) \
	{ \
		mod_operator::load_from_lua(parent, L, index); \
		if(m_operands.size() != 2) \
		{ \
			throw std::string(#op_name " operator must have exactly 2 operands."); \
		} \
	} \
};

double fmod_wrapper(double left, double right)
{
	if(right == 0.0)
	{
		return 0.0;
	}
	return fmod(left, right);
}

// Pretty sure checking for 1.0 is slower than the divide and multiply.
#define OBLONG_WRAPPER(name) \
double name##_wrapper(double left, double right) \
{ \
	if(right == 0.0) { return left; } \
	return std::name(left / right) * right; \
}

OBLONG_WRAPPER(floor);
OBLONG_WRAPPER(ceil);
OBLONG_WRAPPER(round);

#undef OBLONG_WRAPPER

PAIR_OPERATOR(mod, fmod_wrapper);
PAIR_OPERATOR(floor, floor_wrapper);
PAIR_OPERATOR(ceil, ceil_wrapper);
PAIR_OPERATOR(round, round_wrapper);

#undef PAIR_OPERATOR

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

double square_wave(double angle)
{
	angle= fmod(angle, 2.0);
	if(angle < 0.0)
	{
		angle+= 2.0;
	}
	return angle >= 1.0 ? -1.0 : 1.0;
}

double triangle_wave(double angle)
{
	angle= fmod(angle, 2.0);
	if(angle < 0.0)
	{
		angle+= 2.0;
	}
	if(angle < .5)
	{
		return angle * 2.0;
	}
	else if(angle < 1.5)
	{
		return 1.0 - ((angle - .5) * 2.0);
	}
	else
	{
		return -4.0 + (angle * 2.0);
	}
}

// Angle is multiplied by pi for trig functions so that the lua side doesn't
// have to multiply by pi.
double sine_wave(double angle)
{
	return Rage::FastSin(angle * Rage::D_PI);
}

double cosine_wave(double angle)
{
	return Rage::FastCos(angle * Rage::D_PI);
}

double tan_wave(double angle)
{
	return tan(angle * Rage::D_PI);
}

double atriangle(double pos)
{
	if(pos > 1.0)
	{
		return .5;
	}
	if(pos < -1.0)
	{
		return -.5;
	}
	return pos * .5;
}

double asquare(double pos)
{
	if(pos > 0.0)
	{
		return .5;
	}
	return -.5;
}

double asin_wave(double pos)
{
	if(pos > 1.0)
	{
		return .5;
	}
	if(pos < -1.0)
	{
		return -.5;
	}
	return std::asin(pos) / Rage::D_PI;
}

double acos_wave(double pos)
{
	if(pos > 1.0)
	{
		return 0.0;
	}
	if(pos < -1.0)
	{
		return 1.0;
	}
	return std::acos(pos) / Rage::D_PI;
}

double atan_wave(double pos)
{
	return std::atan(pos) / Rage::D_PI;
}

WAVE_OPERATOR(sin, (sine_wave(angle)));
WAVE_OPERATOR(cos, (cosine_wave(angle)));
WAVE_OPERATOR(tan, (tan_wave(angle)));
WAVE_OPERATOR(square, (square_wave(angle)));
WAVE_OPERATOR(triangle, (triangle_wave(angle)));
WAVE_OPERATOR(asin, (asin_wave(angle)));
WAVE_OPERATOR(acos, (acos_wave(angle)));
WAVE_OPERATOR(atan, (atan_wave(angle)));
WAVE_OPERATOR(asquare, (asquare(angle)));
WAVE_OPERATOR(atriangle, (atriangle(angle)));

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
	EVAL_RESULT_EVAL;
	void repeat_eval();
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);
	double m_eval_result;
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

struct mod_operator_lua : mod_operator
{
	EVAL_RESULT_EVAL;
	void lua_eval();
	virtual void load_from_lua(mod_function* parent, lua_State* L, int index);
	void per_frame_update(mod_val_inputs& input);
	void per_note_update(mod_val_inputs& input);

	double m_eval_result;
	LuaReference m_func;
};

#undef EVAL_RESULT_EVAL

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
	void load_from_lua(lua_State* L, int index, TimingData const* timing_data);
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
	double m_length_beats;
	double m_length_seconds;
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

mod_function* create_mod_function(ModifiableValue* parent, lua_State* L,
	int index, double col);

#endif
