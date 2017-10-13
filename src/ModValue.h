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
	~ModManager();
	void set_timing(TimingData const* timing)
	{
		m_timing_data= timing;
	}

	void update(double curr_beat, double curr_second);
	void add_mod(mod_function* func, ModifiableValue* parent);

	// The notefield or column or whatever registers moddable things by name
	// with the manager, then when a mod is set the manager uses the target
	// name to find the moddable thing to add the mod function to.
	// This way, notefield and column don't have to duplicate mod loading logic
	// or both inherit from something that does.

	void register_moddable(std::string const& name, ModifiableValue* thing);
	void set_base_values(lua_State* L, int value_set);
	void add_permanent_mods(lua_State* L, int mod_set);
	void add_timed_mods(lua_State* L, int mod_set);
	void push_target_info(lua_State* L);

	void remove_permanent_mods(lua_State* L, int mod_set);
	void clear_permanent_mods(lua_State* L, int mod_set);
	void clear_timed_mods();

	void load_time_pair(lua_State* L, int table,
		char const* beat_name, char const* second_name,
		double& beat_result, double& second_result);

	void load_target_list(lua_State* L, int table,
		std::unordered_set<ModifiableValue*>& target_list);

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
	std::list<func_and_parent>::iterator remove_from_present(std::list<func_and_parent>::iterator fapi);

	double m_prev_curr_second;
	std::list<func_and_parent> m_past_funcs;
	std::list<func_and_parent> m_present_funcs;
	std::list<func_and_parent> m_future_funcs;

	std::unordered_set<mod_function*> m_per_frame_update_funcs;

	TimingData const* m_timing_data;
	std::unordered_map<std::string, ModifiableValue*> m_registered_targets;
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
	double length_beats;
	double length_seconds;
	double end_beat;
	double end_second;
	double prefunres;
	mod_val_inputs()
		:column(0.0), y_offset(0.0),
		note_id_in_chart(0.0), note_id_in_column(0.0), row_id(0.0),
		eval_beat(0.0), eval_second(0.0), music_beat(0.0),
		music_second(0.0), dist_beat(0.0), dist_second(0.0), prefunres(0.0)
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
		length_beats= eb-sb;
		length_seconds= es-ss;
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
	ModifiableValue(ModManager& man, std::string const& name, double value);
	~ModifiableValue();
	double get_column() { return m_column; }
	void set_column(uint32_t col) { m_column= col; }
	void set_base_value(double value) { m_base_value= value; }
	double get_base_value() { return m_base_value; }
	double evaluate(mod_val_inputs& input);
	std::list<mod_function*>::iterator find_mod(std::string const& name);

	void add_mod(mod_function* func, ModManager& manager);
	void remove_mod(std::string const& name, ModManager& manager);
	void clear_mods(ModManager& manager);
	bool empty() { return m_mods.empty() && m_active_managed_mods.empty(); }

	void add_mod_to_active_list(mod_function* mod);
	void remove_mod_from_active_list(mod_function* mod);
	void clear_active_list();

	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();

	double m_result;

private:
	double m_column;
	std::list<mod_function*> m_mods;
	std::multimap<int, mod_function*> m_active_managed_mods;

	double m_base_value;

	bool m_needs_beat;
	bool m_needs_second;
	bool m_needs_y_offset;
};

struct ModifiableVector3
{
	ModifiableVector3(ModManager& man, std::string const& name, double value)
		:x_mod(man, name + "_x", value), y_mod(man, name + "_y", value),
			z_mod(man, name + "_z", value)
	{}
	ModifiableVector3(ModManager& man, std::string const& name, double x, double y, double z)
		:x_mod(man, name + "_x", x), y_mod(man, name + "_y", y),
			z_mod(man, name + "_z", z)
	{}
	void evaluate(mod_val_inputs& input, Rage::Vector3& out)
	{
		out.x = static_cast<float>(x_mod.evaluate(input));
		out.y = static_cast<float>(y_mod.evaluate(input));
		out.z = static_cast<float>(z_mod.evaluate(input));
	}
	void set_column(uint32_t col)
	{
		x_mod.set_column(col);
		y_mod.set_column(col);
		z_mod.set_column(col);
	}
	void set_base_value(Rage::Vector3 const& value)
	{
		x_mod.set_base_value(value.x);
		y_mod.set_base_value(value.y);
		z_mod.set_base_value(value.z);
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
	ModifiableTransform(ModManager& man, std::string const& name)
		:pos_mod(man, name + "_pos", 0.0), rot_mod(man, name + "_rot", 0.0),
		zoom_vmod(man, name + "_zoom", 1.0), zoom_mod(man, name + "_zoom", 1.0)
	{}
	void set_column(uint32_t col)
	{
		pos_mod.set_column(col);
		rot_mod.set_column(col);
		zoom_vmod.set_column(col);
		zoom_mod.set_column(col);
	}
	void set_base_value(Rage::transform const& value)
	{
		pos_mod.set_base_value(value.pos);
		rot_mod.set_base_value(value.rot);
		zoom_vmod.set_base_value(value.zoom);
		zoom_mod.set_base_value(1.0);
	}
	void evaluate(mod_val_inputs& input, Rage::transform& out)
	{
		pos_mod.evaluate(input, out.pos);
		rot_mod.evaluate(input, out.rot);
		// Multiply by pi so that the lua side doesn't need to.
		out.rot.x*= Rage::D_PI;
		out.rot.y*= Rage::D_PI;
		out.rot.z*= Rage::D_PI;
		zoom_vmod.evaluate(input, out.zoom);
		double zoom= zoom_mod.evaluate(input);
		out.zoom.x*= zoom;
		out.zoom.y*= zoom;
		out.zoom.z*= zoom;
	}
	void hold_render_eval(mod_val_inputs& input, Rage::transform& out, bool do_rot, bool do_y_zoom)
	{
		pos_mod.evaluate(input, out.pos);
		if(do_rot)
		{
			out.rot.y = static_cast<float>(rot_mod.y_mod.evaluate(input));
		}
		double zoom= zoom_mod.evaluate(input);
		out.zoom.x = static_cast<float>(zoom_vmod.x_mod.evaluate(input) * zoom);
		if(do_y_zoom)
		{
			out.zoom.y = static_cast<float>(zoom_vmod.y_mod.evaluate(input) * zoom);
		}
	}
	bool needs_beat();
	bool needs_second();
	bool needs_y_offset();

	ModifiableVector3 pos_mod;
	ModifiableVector3 rot_mod;
	ModifiableVector3 zoom_vmod;
	// Often all three elements of zoom need to change together.
	ModifiableValue zoom_mod;
};

#endif
