#ifndef MOD_VALUE_H
#define MOD_VALUE_H

#include <initializer_list>
#include <list>
#include <map>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "CubicSpline.h"
#include "RageTimer.h"
#include "RageTypes.h"
#include "TimingData.h"

struct lua_State;
struct ModifiableValue;
struct mod_function;

// invalid_modfunction_time exists so that the loading code can tell when a
// start or end time was provided.
static CONSTEXPR_VARIABLE double invalid_modfunction_time= -1000.0;

struct ModManager
{
	size_t column;
	struct func_and_parent
	{
		mod_function* func;
		ModifiableValue* parent;
		func_and_parent() {}
		func_and_parent(mod_function* f, ModifiableValue* p)
			:func(f), parent(p)
		{}
	};
	ModManager()
		:m_prev_curr_second(invalid_modfunction_time)
	{}
	void update(double curr_beat, double curr_second);
	void add_mod(mod_function* func, ModifiableValue* parent);
	void remove_mod(mod_function* func);
	void remove_all_mods(ModifiableValue* parent);

	void dump_list_status();

	void add_to_per_frame_update(mod_function* func);
	void remove_from_per_frame_update(mod_function* func);

private:
	void insert_into_past(mod_function* func, ModifiableValue* parent);
	void insert_into_present(mod_function* func, ModifiableValue* parent);
	void insert_into_future(mod_function* func, ModifiableValue* parent);
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

	std::unordered_set<mod_function*> m_per_frame_update_funcs;
};

struct mod_val_inputs
{
	double column;
	double y_offset;
	double note_id_in_chart;
	double note_id_in_column;
	double row_id;
	double eval_beat;
	double eval_second;
	double music_beat;
	double music_second;
	double dist_beat;
	double dist_second;
	double start_beat;
	double start_second;
	double end_beat;
	double end_second;
	mod_val_inputs()
		:column(0.0), y_offset(0.0),
		note_id_in_chart(0.0), note_id_in_column(0.0), row_id(0.0),
		eval_beat(0.0), eval_second(0.0), music_beat(0.0),
		music_second(0.0), dist_beat(0.0), dist_second(0.0)
	{}
	mod_val_inputs(double const mb, double const ms)
		:column(0.0), y_offset(0.0),
		note_id_in_chart(0.0), note_id_in_column(0.0), row_id(0.0),
		eval_beat(mb), eval_second(ms), music_beat(mb),
		music_second(ms), dist_beat(0.0), dist_second(0.0)
	{}
	mod_val_inputs(double const eb, double const es, double const mb,
		double const ms)
		:column(0.0), y_offset(0.0),
		note_id_in_chart(0.0), note_id_in_column(0.0), row_id(0.0),
		eval_beat(eb), eval_second(es), music_beat(mb),
		music_second(ms), dist_beat(eb-mb), dist_second(es-ms)
	{}
	mod_val_inputs(double const eb, double const es, double const mb,
		double const ms, double const yoff)
		:column(0.0), y_offset(yoff),
		note_id_in_chart(0.0), note_id_in_column(0.0), row_id(0.0),
		eval_beat(eb), eval_second(es), music_beat(mb),
		music_second(ms), dist_beat(eb-mb), dist_second(es-ms)
	{}
	void init(double const eb, double const es, double const mb,
		double const ms, TapNote const* note)
	{
		eval_beat= eb;  eval_second= es;
		music_beat= mb;  music_second= ms;
		dist_beat= eb-mb;  dist_second= es-ms;
		set_note(note);
	}
	void change_eval_beat(double const eb)
	{
		eval_beat= eb;
		dist_beat= eb-music_beat;
	}
	void change_eval_time(double const eb, double const es)
	{
		change_eval_beat(eb);
		eval_second= es;
		dist_second= es-music_second;
	}
	void set_time(double sb, double ss, double eb, double es)
	{
		start_beat= sb;
		start_second= ss;
		end_beat= eb;
		end_second= es;
	}
	void set_note(TapNote const* note)
	{
		if(note != nullptr)
		{
			note_id_in_chart= note->id_in_chart;
			note_id_in_column= note->id_in_column;
			row_id= note->row_id;
		}
	}
};

struct ModifiableValue
{
	ModifiableValue(ModManager* man, double value);
	~ModifiableValue();
	void set_timing(TimingData const* timing)
	{
		m_timing= timing;
	}
	void set_column(uint32_t col)
	{
		m_column= col;
	}
	double evaluate(mod_val_inputs& input);
	std::list<mod_function*>::iterator find_mod(std::string const& name);
	void add_mod(lua_State* L, int index);
	void remove_mod(std::string const& name);
	void clear_mods();
	bool empty() { return m_mods.empty() && m_active_managed_mods.empty(); }

	void add_managed_mod(lua_State* L, int index);
	void remove_managed_mod(std::string const& name);
	void clear_managed_mods();
	void add_mod_to_active_list(mod_function* mod);
	void remove_mod_from_active_list(mod_function* mod);
	void add_simple_mod(std::string const& name, std::string const& input_type,
		double value);
	void take_over_mods(ModifiableValue& other);

	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();

	virtual void PushSelf(lua_State* L);

private:
	void insert_mod_internal(mod_function* new_mod);

	ModManager* m_manager;
	TimingData const* m_timing;
	double m_column;
	std::list<mod_function*> m_mods;
	std::unordered_map<std::string, mod_function*> m_managed_mods;
	std::multimap<int, mod_function*> m_active_managed_mods;

	bool m_needs_beat;
	bool m_needs_second;
	bool m_needs_y_offset;
};

struct ModifiableVector3
{
	ModifiableVector3(ModManager* man, double value)
		:x_mod(man, value), y_mod(man, value), z_mod(man, value)
	{}
	void evaluate(mod_val_inputs& input, Rage::Vector3& out)
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
	void set_column(uint32_t col)
	{
		x_mod.set_column(col);
		y_mod.set_column(col);
		z_mod.set_column(col);
	}
	void take_over_mods(ModifiableVector3& other)
	{
		x_mod.take_over_mods(other.x_mod);
		y_mod.take_over_mods(other.y_mod);
		z_mod.take_over_mods(other.z_mod);
	}
	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();

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
	void set_column(uint32_t col)
	{
		pos_mod.set_column(col);
		rot_mod.set_column(col);
		zoom_mod.set_column(col);
	}
	void evaluate(mod_val_inputs& input, Rage::transform& out)
	{
		pos_mod.evaluate(input, out.pos);
		rot_mod.evaluate(input, out.rot);
		zoom_mod.evaluate(input, out.zoom);
	}
	void hold_render_eval(mod_val_inputs& input, Rage::transform& out, bool do_rot)
	{
		pos_mod.evaluate(input, out.pos);
		if(do_rot)
		{
			out.rot.y = static_cast<float>(rot_mod.y_mod.evaluate(input));
		}
		out.zoom.x = static_cast<float>(zoom_mod.x_mod.evaluate(input));
	}
	void take_over_mods(ModifiableTransform& other)
	{
		pos_mod.take_over_mods(other.pos_mod);
		rot_mod.take_over_mods(other.rot_mod);
		zoom_mod.take_over_mods(other.zoom_mod);
	}
	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();

	ModifiableVector3 pos_mod;
	ModifiableVector3 rot_mod;
	ModifiableVector3 zoom_mod;
};

#endif
