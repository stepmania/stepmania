#ifndef MOD_VALUE_H
#define MOD_VALUE_H

#include <initializer_list>
#include <list>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "CubicSpline.h"
#include "RageTimer.h"
#include "RageTypes.h"
#include "TimingData.h"

struct lua_State;
struct ModFunction;
struct ModifiableValue;

// invalid_modfunction_time exists so that the loading code can tell when a
// start or end time was provided.
static CONSTEXPR_VARIABLE double invalid_modfunction_time= -1000.0;

struct ModManager
{
	size_t column;
	struct func_and_parent
	{
		ModFunction* func;
		ModifiableValue* parent;
		func_and_parent() {}
		func_and_parent(ModFunction* f, ModifiableValue* p)
			:func(f), parent(p)
		{}
	};
	ModManager()
		:m_prev_curr_second(invalid_modfunction_time)
	{}
	void update(double curr_beat, double curr_second);
	void add_mod(ModFunction* func, ModifiableValue* parent);
	void remove_mod(ModFunction* func);
	void remove_all_mods(ModifiableValue* parent);

	void dump_list_status();

	void add_to_per_frame_update(ModFunction* func);
	void remove_from_per_frame_update(ModFunction* func);

private:
	void insert_into_past(ModFunction* func, ModifiableValue* parent);
	void insert_into_present(ModFunction* func, ModifiableValue* parent);
	void insert_into_future(ModFunction* func, ModifiableValue* parent);
#define INSERT_FAP(time_name) \
	void insert_into_##time_name(func_and_parent& fap) \
	{ insert_into_##time_name(fap.func, fap.parent); }
	INSERT_FAP(past);
	INSERT_FAP(present);
	INSERT_FAP(future);
#undef INSERT_FAP
	void remove_from_present(std::list<func_and_parent>::iterator fapi);

	double m_prev_curr_second;
	std::list<func_and_parent> m_past_funcs;
	std::list<func_and_parent> m_present_funcs;
	std::list<func_and_parent> m_future_funcs;

	std::unordered_set<ModFunction*> m_per_frame_update_funcs;
};

struct mod_val_inputs
{
	double scalar;
	double eval_beat;
	double eval_second;
	double music_beat;
	double music_second;
	double dist_beat;
	double dist_second;
	double y_offset;
	double start_dist_beat;
	double start_dist_second;
	double end_dist_beat;
	double end_dist_second;
	mod_val_inputs(double const mb, double const ms)
		:scalar(1.0), eval_beat(mb), eval_second(ms), music_beat(mb),
		music_second(ms), dist_beat(0.0), dist_second(0.0), y_offset(0.0)
	{}
	mod_val_inputs(double const eb, double const es, double const mb, double const ms)
		:scalar(1.0), eval_beat(eb), eval_second(es), music_beat(mb),
		music_second(ms), dist_beat(eb-mb), dist_second(es-ms), y_offset(0.0)
	{}
	mod_val_inputs(double const eb, double const es, double const mb, double const ms, double const yoff)
		:scalar(1.0), eval_beat(eb), eval_second(es), music_beat(mb),
		music_second(ms), dist_beat(eb-mb), dist_second(es-ms), y_offset(yoff)
	{}
#define PART_DIFF(result, after, before, type) \
	result##_dist_##type= after##_##type - before##_##type;
#define BS_DIFF(result, after, before) \
	PART_DIFF(result, after, before, beat); \
	PART_DIFF(result, after, before, second);
	void set_time(double start_beat, double start_second, double curr_beat,
		double curr_second, double end_beat, double end_second)
	{
		BS_DIFF(start, curr, start);
		BS_DIFF(end, end, curr);
	}
};

enum ModInputType
{
	MIT_Scalar,
	MIT_EvalBeat,
	MIT_EvalSecond,
	MIT_MusicBeat,
	MIT_MusicSecond,
	MIT_DistBeat,
	MIT_DistSecond,
	MIT_YOffset,
	// TODO:  Split Start/End dist types into Music and Eval subtypes.
	MIT_StartDistBeat,
	MIT_StartDistSecond,
	MIT_EndDistBeat,
	MIT_EndDistSecond,
	NUM_ModInputType,
	ModInputType_Invalid
};
std::string const ModInputTypeToString(ModInputType fmt);
LuaDeclareType(ModInputType);

enum ModInputMetaType
{
	MIMT_Scalar,
	MIMT_PerFrame,
	MIMT_PerNote,
	MIMT_Invalid
};

struct ModInput
{
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
	ModInput()
		:m_parent(nullptr), m_type(ModInputType_Invalid), m_scalar(0.0),
		m_offset(0.0), m_rep_begin(0.0), m_rep_end(0.0),
		m_loop_spline(false), m_polygonal_spline(false),
		rep_apple(nullptr), phase_apple(nullptr), spline_apple(nullptr),
		choice(&mod_val_inputs::scalar)
	{}
	ModInputMetaType get_meta_type()
	{
		switch(m_type)
		{
			case MIT_Scalar:
				return MIMT_Scalar;
			case MIT_MusicBeat:
			case MIT_MusicSecond:
			case MIT_StartDistBeat:
			case MIT_StartDistSecond:
			case MIT_EndDistBeat:
			case MIT_EndDistSecond:
				return MIMT_PerFrame;
			case MIT_EvalBeat:
			case MIT_EvalSecond:
			case MIT_DistBeat:
			case MIT_DistSecond:
			case MIT_YOffset:
				return MIMT_PerNote;
			default:
				return MIMT_Invalid;
		}
		return MIMT_Scalar;
	}
	void clear();
	void push_phase(lua_State* L, size_t phase);
	void push_def_phase(lua_State* L);
	void load_rep(lua_State* L, int index);
	void load_one_phase(lua_State* L, int index, size_t phase);
	void load_def_phase(lua_State* L, int index);
	void load_phases(lua_State* L, int index);
	void sort_phases();
	void load_spline(lua_State* L, int index);
	void load_from_lua(lua_State* L, int index, ModFunction* parent);
	phase const* find_phase(double input);
	double apply_rep(double input);
	double apply_phase(double input)
	{
		phase const* curr= find_phase(input);
		return ((input - curr->start) * curr->mult) + curr->offset;
	}
	double apply_spline(double input)
	{
		if(m_spline.empty())
		{
			return input;
		}
		return static_cast<double>(m_spline.evaluate(static_cast<float>(input), m_loop_spline));
	}
	double apply_nothing(double input)
	{
		return input;
	}
	double pick(mod_val_inputs const& input)
	{
		double ret= input.*choice;
		if(rep_apple != nullptr)
		{
			ret= (this->*rep_apple)(ret);
		}
		if(phase_apple != nullptr)
		{
			ret= (this->*phase_apple)(ret);
		}
		ret*= m_scalar;
		if(spline_apple != nullptr)
		{
			ret= (this->*spline_apple)(ret);
		}
		return ret + m_offset;
	}
	virtual void PushSelf(lua_State* L);

	void send_repick();
	void send_spline_repick();
	ModInputType get_type() { return m_type; }
	void set_type(ModInputType t);
	double get_scalar() { return m_scalar; }
	void set_scalar(double s);
	double get_offset() { return m_offset; }
	void set_offset(double s);
	void push_rep(lua_State* L);
	void push_all_phases(lua_State* L);
	size_t get_num_phases() { return m_phases.size(); }
	void enable_phases();
	void disable_phases();
	void check_disable_phases();
	bool get_enable_phases() { return phase_apple != nullptr; }
	void remove_phase(size_t phase);
	void clear_phases();
	void enable_spline();
	void disable_spline();
	bool get_enable_spline() { return spline_apple != nullptr; }
	bool get_spline_loop() { return m_loop_spline; }
	void set_spline_loop(bool b);
	bool get_spline_polygonal() { return m_polygonal_spline; }
	void set_spline_polygonal(bool b);
	size_t get_spline_size() { return m_spline.size(); }
	bool get_spline_empty() { return m_spline.empty(); }
	double get_spline_point(size_t p);
	void set_spline_point(size_t p, double value);
	void add_spline_point(double value);
	void remove_spline_point(size_t p);
	void push_spline(lua_State* L);

private:
	ModFunction* m_parent;
	ModInputType m_type;
	double m_scalar;
	double m_offset;

	// The input value can be passed through a couple of modifiers to change
	// its range.  These modifiers are applied before the scalar and offset.
	// So it works like this:
	//   result= apply_rep_mod(input)
	//   result= apply_phase_mod(result)
	//   return (result * scalar) + offset
	// These input modifiers are necessary for mods like beat and hidden,

	// The rep modifier makes a sub-range repeat.  rep_begin is the beginning
	// of the range, rep_end is the end.  The result of the rep modifier will
	// never equal rep_end.
	// Example:
	//   rep_begin is 1.
	//   rep_end is 2.
	//     input is 2, result is 1.
	//     input is .25, result is 1.25.
	//     input is -.25, result is 1.75.
	double m_rep_begin;
	double m_rep_end;
	// The phase modifier applies a multiplier and an offset in its range.
	// Equation: result= ((input - phase_start) * multiplier) + offset
	// The range includes the beginning, but not the end.
	// Input outside its range is not modified.
	// A ModInput can have multiple phases to simplify creating mods that need
	// multiple phases. (say, when beat ramps up the amplitude on a sine wave,
	// then ramps it down.  Between beats is one phase, ramp up is another, and
	// ramp down is a third.)
	// If two phases overlap, the one that is used is undefined.
	// Example:
	//   phase start is .5.
	//   phase finish is 1.
	//   phase mult is 2.
	//   phase offset is .5.
	//     input is .4, result is .4 (outside the phase).
	//     input is .5, result is .5 (.5 - .5 is 0, 0 * 2 is 0, 0 + .5 is .5)
	//     input is .6, result is .7 (.6 - .5 is .1, .1 * 2 is .2, .2 + .5 is .7)
	//     input is 1, result is 1 (outside the phase).
	std::vector<phase> m_phases;
	phase m_default_phase;

	// Special stuff for ModInputType_Spline.
	CubicSpline m_spline;
	bool m_loop_spline;
	bool m_polygonal_spline;

	double (ModInput::* rep_apple)(double);
	double (ModInput::* phase_apple)(double);
	double (ModInput::* spline_apple)(double);
	double mod_val_inputs::* choice;
};

enum ModFunctionType
{
	MFT_Constant,
	MFT_Product,
	MFT_Power,
	MFT_Log,
	MFT_Sine,
	MFT_Tan,
	MFT_Square,
	MFT_Triangle,
	MFT_Spline,
	NUM_ModFunctionType,
	ModFunctionType_Invalid
};
std::string const ModFunctionTypeToString(ModFunctionType fmt);
LuaDeclareType(ModFunctionType);

enum ModSumType
{
	MST_Add,
	MST_Subtract,
	MST_Multiply,
	MST_Divide,
	NUM_ModSumType,
	ModSumType_Invalid
};
std::string const ModSumTypeToString(ModSumType mvst);
LuaDeclareType(ModSumType);

struct ModFunction
{
	ModFunction(ModifiableValue* parent)
		:m_start_beat(invalid_modfunction_time),
		m_start_second(invalid_modfunction_time),
		m_end_beat(invalid_modfunction_time),
		m_end_second(invalid_modfunction_time),
		m_sum_type(MST_Add),
		m_sub_eval(&ModFunction::noop_eval),
		m_pfu(&ModFunction::per_frame_update_normal),
		m_pnu(&ModFunction::per_note_update_normal),
		m_parent(parent)
	{}
	~ModFunction() {}

	void calc_unprovided_times(TimingData const* timing);

	std::string const& get_name() { return m_name; }
	// needs_per_frame_update exists so that ModifiableValue can check after
	// creating a ModFunction to see if it needs to be added to the manager for
	// solving every frame.
	bool needs_per_frame_update() { return !m_per_frame_inputs.empty(); }
	void per_frame_update(mod_val_inputs const& input);
	size_t find_child(ModInput* child);
	void remove_child_from_percoset(size_t child_index,
		vector<size_t>& percoset);
	void recategorize(ModInput* child, ModInputMetaType old_meta,
		ModInputMetaType new_meta);
	void repick(ModInput* child);

	double evaluate(mod_val_inputs const& input)
	{
		if(!m_per_note_inputs.empty())
		{
			(this->*m_pnu)(input);
		}
		return (this->*m_sub_eval)();
	}
	double evaluate_with_time(mod_val_inputs const& input)
	{
		if(!m_per_note_inputs.empty())
		{
			const_cast<mod_val_inputs&>(input).set_time(m_start_beat,
				m_start_second, input.music_beat, input.music_second, m_end_beat,
				m_end_second);
			(this->*m_pnu)(input);
		}
		return (this->*m_sub_eval)();
	}
	double noop_eval()
	{
		return 0.0;
	}
	double constant_eval();
	double product_eval();
	double power_eval();
	double log_eval();
	double sine_eval();
	double tan_eval();
	double square_eval();
	double triangle_eval();
	double spline_eval();
	void set_type(ModFunctionType type);
	ModFunctionType get_type() { return m_type; }
	bool load_from_lua(lua_State* L, int index);
	void push_inputs(lua_State* L, int table_index);
	size_t num_inputs() { return m_inputs.size(); }
	void PushSelf(lua_State* L);

	double m_start_beat;
	double m_start_second;
	double m_end_beat;
	double m_end_second;
	ModSumType m_sum_type;

private:
	void update_input_set(mod_val_inputs const& input,
		vector<size_t>& input_set);
	void update_input_set_in_spline(mod_val_inputs const& input,
		vector<size_t>& input_set);
	void per_frame_update_normal(mod_val_inputs const& input);
	void per_note_update_normal(mod_val_inputs const& input);
	void per_frame_update_spline(mod_val_inputs const& input);
	void per_note_update_spline(mod_val_inputs const& input);

	bool m_being_loaded;
	ModFunctionType m_type;
	vector<ModInput> m_inputs;
	vector<size_t> m_per_note_inputs;
	vector<size_t> m_per_frame_inputs;
	vector<double> m_picked_inputs;
	double (ModFunction::* m_sub_eval)();
	void (ModFunction::* m_pfu)(mod_val_inputs const& input);
	void (ModFunction::* m_pnu)(mod_val_inputs const& input);

	CubicSpline m_spline;
	bool m_loop_spline;
	bool m_polygonal_spline;
	bool m_has_point_per_frame_input;

	std::string m_name;
	ModifiableValue* m_parent;
};

struct ModifiableValue
{
	ModifiableValue(ModManager* man, double value)
		:m_value(value), m_manager(man), m_timing(nullptr)
	{}
	~ModifiableValue();
	void set_timing(TimingData const* timing)
	{
		m_timing= timing;
	}
	double evaluate(mod_val_inputs const& input);
	std::list<ModFunction*>::iterator find_mod(std::string const& name);
	ModFunction* add_mod(lua_State* L, int index);
	ModFunction* get_mod(std::string const& name);
	void remove_mod(std::string const& name);
	void clear_mods();

	ModFunction* add_managed_mod(lua_State* L, int index);
	ModFunction* get_managed_mod(std::string const& name);
	void remove_managed_mod(std::string const& name);
	void clear_managed_mods();
	void add_mod_to_active_list(ModFunction* mod);
	void remove_mod_from_active_list(ModFunction* mod);

	virtual void PushSelf(lua_State* L);

	double m_value;
private:
	ModFunction* add_mod_internal(lua_State* L, int index);

	ModManager* m_manager;
	TimingData const* m_timing;
	std::list<ModFunction*> m_mods;
	std::unordered_map<std::string, ModFunction*> m_managed_mods;
	std::unordered_set<ModFunction*> m_active_managed_mods;
};

struct ModifiableVector3
{
	ModifiableVector3(ModManager* man, double value)
		:x_mod(man, value), y_mod(man, value), z_mod(man, value)
	{}
	void evaluate(mod_val_inputs const& input, Rage::Vector3& out)
	{
		out.x = static_cast<float>(x_mod.evaluate(input));
		out.y = static_cast<float>(y_mod.evaluate(input));
		out.z = static_cast<float>(z_mod.evaluate(input));
	}
	void set_timing(TimingData const* timing)
	{
		x_mod.set_timing(timing);
		y_mod.set_timing(timing);
		z_mod.set_timing(timing);
	}
	ModifiableValue x_mod;
	ModifiableValue y_mod;
	ModifiableValue z_mod;
};

struct ModifiableTransform
{
	ModifiableTransform(ModManager* man)
		:pos_mod(man, 0.0), rot_mod(man, 0.0), zoom_mod(man, 1.0)
	{}
	void set_timing(TimingData const* timing)
	{
		pos_mod.set_timing(timing);
		rot_mod.set_timing(timing);
		zoom_mod.set_timing(timing);
	}
	void evaluate(mod_val_inputs const& input, Rage::transform& out)
	{
		pos_mod.evaluate(input, out.pos);
		rot_mod.evaluate(input, out.rot);
		zoom_mod.evaluate(input, out.zoom);
	}
	void hold_render_eval(mod_val_inputs const& input, Rage::transform& out, bool do_rot)
	{
		pos_mod.evaluate(input, out.pos);
		if(do_rot)
		{
			out.rot.y = static_cast<float>(rot_mod.y_mod.evaluate(input));
		}
		out.zoom.x = static_cast<float>(zoom_mod.x_mod.evaluate(input));
	}
	ModifiableVector3 pos_mod;
	ModifiableVector3 rot_mod;
	ModifiableVector3 zoom_mod;
};

#endif
