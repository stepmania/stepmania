#include "global.h"
#include "ActorUtil.h"
#include "IniFile.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageTypes.h"
#include "LuaManager.h"
#include "LuaBinding.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"

#include <vector>
#include <map>
#include <set>

using std::vector;
using std::string;

#define TWEEN_QUEUE_MAX 50

void convert_xmls_in_dir(std::string const& dirname);
void convert_xml_file(std::string const& fname, std::string const& dirname);
std::string maybe_conv_pos(std::string pos, std::string (*conv_func)(float p));
std::string add_extension_to_relative_path_from_found_file(
	std::string const& relative_path, std::string const& found_file);

void convert_xmls_in_dir(std::string const& dirname)
{
	vector<std::string> listing;
	FILEMAN->GetDirListing(dirname, listing, false, true);
	for (auto &file: listing)
	{
		switch(ActorUtil::GetFileType(file))
		{
			case FT_Xml:
				convert_xml_file(file, dirname);
				break;
			case FT_Directory:
				convert_xmls_in_dir(file + "/");
				break;
			default: // Ignore anything not xml or directory.
				break;
		}
	}
}

std::string convert_xpos(float x)
{
	return "SCREEN_CENTER_X + " + FloatToString(x - 320.0f);
}

std::string convert_ypos(float y)
{
	return "SCREEN_CENTER_Y + " + FloatToString(y - 240.0f);
}

std::string maybe_conv_pos(std::string pos, std::string (*conv_func)(float p))
{
	float f;
	if(pos >> f)
	{
		return conv_func(f);
	}
	return pos;
}

size_t after_slash_or_zero(std::string const& s)
{
	size_t ret= s.rfind('/');
	if(ret != string::npos)
	{
		return ret+1;
	}
	return 0;
}

std::string add_extension_to_relative_path_from_found_file(
	std::string const& relative_path, std::string const& found_file)
{
	size_t rel_last_slash= after_slash_or_zero(relative_path);
	size_t found_last_slash= after_slash_or_zero(found_file);
	return Rage::head( relative_path, rel_last_slash) +
		found_file.substr(found_last_slash, string::npos);
}

bool verify_arg_count(std::string cmd, vector<std::string>& args, size_t req)
{
	if(args.size() < req)
	{
		LuaHelpers::ReportScriptError("Not enough args to " + cmd + " command.");
		return false;
	}
	return true;
}

typedef void (*arg_converter_t)(vector<std::string>& args);

std::map<std::string, arg_converter_t> arg_converters;
std::map<std::string, size_t> tween_counters;
std::set<std::string> fields_that_are_strings;
std::map<std::string, std::string> chunks_to_replace;

#define COMMON_ARG_VERIFY(count) if(!verify_arg_count(args[0], args, count)) return;
void x_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	float pos;
	if(args[1] >> pos)
	{
		args[1]= convert_xpos(pos);
	}
}
void y_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	float pos;
	if(args[1] >> pos)
	{
		args[1]= convert_ypos(pos);
	}
}
void string_arg_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	args[1]= "\"" + args[1] + "\"";
}
void lower_string_conv(vector<std::string>& args)
{
	args[0] = Rage::make_lower(args[0]);
}
void hidden_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	args[0]= "visible";
	if(args[1] == "1")
	{
		args[1]= "false";
	}
	else
	{
		args[1]= "true";
	}
}
void diffuse_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	// TODO: Utilize Rage::join.
	std::string retarg;
	for(size_t i= 1; i < args.size(); ++i)
	{
		retarg+= args[i];
		if(i < args.size()-1)
		{
			retarg+= ",";
		}
	}
	args[1]= "color('" + retarg + "')";
	args.resize(2);
}

// Prototype for a function that is created by a macro in another translation unit and has no visible prototype, don't do this unless you have a good reason.
std::string const BlendModeToString(BlendMode);
std::string const CullModeToString(CullMode);
void blend_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	for(int i= 0; i < NUM_BlendMode; ++i)
	{
		std::string blend_str = Rage::make_lower(BlendModeToString(static_cast<BlendMode>(i)));
		if(args[1] == blend_str)
		{
			args[1]= "\"BlendMode_" + BlendModeToString(static_cast<BlendMode>(i)) + "\"";
			break;
		}
	}
}
void cull_conv(vector<std::string>& args)
{
	COMMON_ARG_VERIFY(2);
	for(int i= 0; i < NUM_CullMode; ++i)
	{
		std::string cull_str = Rage::make_lower(CullModeToString(static_cast<CullMode>(i)));
		if(args[1] == cull_str)
		{
			args[1]= "\"CullMode_" + CullModeToString(static_cast<CullMode>(i)) + "\"";
			break;
		}
	}
}
#undef COMMON_ARG_VERIFY

void init_parser_helpers()
{
	arg_converters["x"]= &x_conv;
	arg_converters["y"]= &y_conv;
	arg_converters["queuecommand"]= &string_arg_conv;
	arg_converters["playcommand"]= &string_arg_conv;
	arg_converters["effectclock"]= &string_arg_conv;
	arg_converters["EffectMagnitude"]= &lower_string_conv;
	arg_converters["ZoomToWidth"]= &lower_string_conv;
	arg_converters["ZoomToHeight"]= &lower_string_conv;
	arg_converters["blend"]= &blend_conv;
	arg_converters["cullmode"]= &cull_conv;
	arg_converters["hidden"]= &hidden_conv;
	arg_converters["diffuse"]= &diffuse_conv;
	arg_converters["effectcolor1"]= &diffuse_conv;
	arg_converters["effectcolor2"]= &diffuse_conv;
	tween_counters["sleep"]= 2;
	tween_counters["linear"]= 1;
	tween_counters["accelerate"]= 1;
	tween_counters["decelerate"]= 1;
	tween_counters["spring"]= 1;
	tween_counters["tween"]= 1;
	tween_counters["queuecommand"]= 1;
	fields_that_are_strings.insert("Texture");
	fields_that_are_strings.insert("Text");
	fields_that_are_strings.insert("AltText");
	fields_that_are_strings.insert("File");
	fields_that_are_strings.insert("Font");
	fields_that_are_strings.insert("Meshes");
	fields_that_are_strings.insert("Materials");
	fields_that_are_strings.insert("Bones");
	chunks_to_replace["hidden(0)"]= "visible(true)";
	chunks_to_replace["hidden(1)"]= "visible(false)";
	chunks_to_replace["effectdelay"]= "effect_hold_at_full";
	chunks_to_replace["IsPlayerEnabled(0)"]= "IsPlayerEnabled(PLAYER_1)";
	chunks_to_replace["IsPlayerEnabled(1)"]= "IsPlayerEnabled(PLAYER_2)";
}

void convert_lua_chunk(std::string& chunk_text)
{
	for (auto &chunk: chunks_to_replace)
	{
		Rage::replace(chunk_text, chunk.first, chunk.second);
	}
}

// Conditions are mapped by condition string.
// So condition_set_t::iterator->first is the lua to execute for the
// condition, and condition_set_t::iterator->second is the name of the
// condition.
typedef std::map<std::string, std::string> condition_set_t;
typedef std::map<std::string, std::string> field_cont_t;
struct frame_t
{
	int frame;
	float delay;
	frame_t() :frame(0), delay(0.0f) {}
};

struct actor_template_t
{
	std::string type;
	field_cont_t fields;
	std::string condition;
	std::string name;
	vector<frame_t> frames;
	vector<actor_template_t> children;
	std::string x;
	std::string y;
	void make_space_for_frame(int id);
	void store_cmd(std::string const& cmd_name, std::string const& full_cmd);
	void store_field(std::string const& field_name, std::string const& value, bool cmd_convert, std::string const& pref= "", std::string const& suf= "");
	void store_field(std::string const& field_name, XNodeValue const* value, bool cmd_convert, std::string const& pref= "", std::string const& suf= "");
	void rename_field(std::string const& old_name, std::string const& new_name);
	std::string get_field(std::string const& field_name);
	void load_frames_from_file(std::string const& fname, std::string const& rel_path);
	void load_model_from_file(std::string const& fname, std::string const& rel_path);
	void load_node(XNode const& node, std::string const& dirname, condition_set_t& conditions);
	void output_to_file(RageFile* file, std::string const& indent);
};

void actor_template_t::make_space_for_frame(int id)
{
	if(id >= static_cast<int>(frames.size()))
	{
		frames.resize(id+1);
	}
}

void actor_template_t::store_cmd(std::string const& cmd_name, std::string const& full_cmd)
{
	if (Rage::starts_with(full_cmd, "%"))
	{
		std::string cmd_text = Rage::tail(full_cmd, -1);
		convert_lua_chunk(cmd_text);
		fields[cmd_name]= cmd_text;
		return;
	}
	auto cmds = Rage::split(full_cmd, ";", Rage::EmptyEntries::skip);
	size_t queue_size= 0;
	// If someone has a simfile that uses a playcommand that pushes tween
	// states onto the queue, queue size counting will have to be made much
	// more complex to prevent that from causing an overflow.
	for(auto cmd= cmds.begin(); cmd != cmds.end(); ++cmd)
	{
		vector<std::string> args = Rage::split(*cmd, ",", Rage::EmptyEntries::skip);
		if(!args.empty())
		{
			for(auto arg = args.begin(); arg != args.end(); ++arg)
			{
				size_t first_nonspace= 0;
				size_t last_nonspace= arg->size();
				while((*arg)[first_nonspace] == ' ')
				{
					++first_nonspace;
				}
				while((*arg)[last_nonspace] == ' ')
				{
					--last_nonspace;
				}
				*arg= arg->substr(first_nonspace, last_nonspace - first_nonspace);
			}
			auto conv = arg_converters.find(args[0]);
			if(conv != arg_converters.end())
			{
				conv->second(args);
			}
			auto counter= tween_counters.find(args[0]);
			if(counter != tween_counters.end())
			{
				queue_size+= counter->second;
			}
		}
		*cmd= Rage::join(",", args);
	}
	// This code is probably actually useless, OITG has the same tween queue size
	// and the real reason I saw overflows in converted files was a bug in
	// foreground loading that ran InitCommand twice. -Kyz
	if(queue_size >= TWEEN_QUEUE_MAX)
	{
		size_t num_to_make= (queue_size / TWEEN_QUEUE_MAX) + 1;
		size_t states_per= (queue_size / num_to_make) + 1;
		size_t states_in_curr= 0;
		std::string this_name= cmd_name;
		vector<std::string> curr_cmd;
		for(auto cmd = cmds.begin(); cmd != cmds.end(); ++cmd)
		{
			curr_cmd.push_back(*cmd);
			auto args = Rage::split(*cmd, ",", Rage::EmptyEntries::skip);
			if(!args.empty())
			{
				auto counter= tween_counters.find(args[0]);
				if(counter != tween_counters.end())
				{
					states_in_curr+= counter->second;
					if(states_in_curr >= states_per - 1)
					{
						std::string next_name= unique_name("cmd");
						curr_cmd.push_back("queuecommand,\"" + next_name + "\"");
						fields[this_name]= "cmd(" + Rage::join(";", curr_cmd) + ")";
						curr_cmd.clear();
						this_name= next_name;
						states_in_curr= 0;
					}
				}
			}
		}
		if(!curr_cmd.empty())
		{
			fields[this_name]= "cmd(" + Rage::join(";", curr_cmd) + ")";
		}
	}
	else
	{
		fields[cmd_name]= "cmd(" + Rage::join(";", cmds) + ")";
	}
}

void actor_template_t::store_field(std::string const& field_name, std::string const& value, bool cmd_convert, std::string const& pref, std::string const& suf)
{
	// OITG apparently allowed "Oncommand" as valid.
	Rage::ci_ascii_string command{ "command" };
	if (command != Rage::tail( field_name, 7))
	{
		cmd_convert= false;
	}
	if(cmd_convert)
	{
		std::string real_field_name= Rage::head(field_name, -7) + "Command";
		store_cmd(real_field_name, value);
	}
	else
	{
		fields[field_name]= pref + value + suf;
	}

}
void actor_template_t::store_field(std::string const& field_name, XNodeValue const* value, bool cmd_convert, std::string const& pref, std::string const& suf)
{
	std::string val;
	value->GetValue(val);
	store_field(field_name, val, cmd_convert, pref, suf);
}

void actor_template_t::rename_field(std::string const& old_name, std::string const& new_name)
{
	field_cont_t::iterator old_field= fields.find(old_name);
	if(old_field == fields.end())
	{
		return;
	}
	fields[new_name]= old_field->second;
	fields.erase(old_field);
}

std::string actor_template_t::get_field(std::string const& field_name)
{
	field_cont_t::iterator field= fields.find(field_name);
	if(field == fields.end())
	{
		return "";
	}
	return field->second;
}

void actor_template_t::load_frames_from_file(std::string const& fname, std::string const& rel_path)
{
	IniFile ini;
	if(!ini.ReadFile(fname))
	{
		LOG->Trace("Failed to read sprite file %s: %s", fname.c_str(), ini.GetError().c_str());
		return;
	}
	XNode const* sprite_node= ini.GetChild("Sprite");
	if(sprite_node != nullptr)
	{
		for (auto const &attr: sprite_node->m_attrs)
		{
			// Frame and Delay fields have names of the form "Frame0000" where the
			// "0000" part is the id of the frame.
			std::string field_type{ Rage::head(attr.first, 5) };
			if(field_type == "Frame")
			{
				int id= StringToInt( Rage::tail(attr.first, -5) );
				make_space_for_frame(id);
				attr.second->GetValue(frames[id].frame);
			}
			else if(field_type == "Delay")
			{
				int id= StringToInt( Rage::tail(attr.first, -5) );
				make_space_for_frame(id);
				attr.second->GetValue(frames[id].delay);
			}
			else if(field_type == "Textu")
			{
				store_field("Texture", attr.second, false, rel_path, "");
			}
			else
			{
				// Unrecognized, ignore.
			}
		}
	}
}

void actor_template_t::load_model_from_file(std::string const& fname, std::string const& rel_path)
{
	IniFile ini;
	if(!ini.ReadFile(fname))
	{
		LOG->Trace("Failed to read model file %s: %s", fname.c_str(), ini.GetError().c_str());
		return;
	}
	XNode const* model_node= ini.GetChild("Model");
	if(model_node != nullptr)
	{
		for (auto const &attr: model_node->m_attrs)
		{
			store_field(attr.first, attr.second, false, rel_path, "");
		}
	}
}

void actor_template_t::load_node(XNode const& node, std::string const& dirname, condition_set_t& conditions)
{
	type= node.GetName();
	bool type_set_by_automagic= false;
#define set_type(auto_type) type_set_by_automagic= true; type= auto_type;
	for (auto const &attr: node.m_attrs)
	{
		if(attr.first == "Name")
		{
			attr.second->GetValue(name);
		}
		else if(attr.first == "Condition")
		{
			std::string cond_str;
			attr.second->GetValue(cond_str);
			condition_set_t::iterator cond= conditions.find(cond_str);
			if(cond == conditions.end())
			{
				condition= unique_name("cond");
				conditions[cond_str]= condition;
			}
			else
			{
				condition= cond->second;
			}
		}
		else if(attr.first == "Type" || attr.first == "Class")
		{
			if(!type_set_by_automagic)
			{
				attr.second->GetValue(type);
			}
		}
		else if(attr.first == "__TEXT__")
		{
			// Ignore.  This attribute seems to be put in by the xml parser, and
			// not part of the actual xml code.
		}
		else if(attr.first == "Text")
		{
			set_type("BitmapText");
			store_field(attr.first, attr.second, true);
		}
		else if(attr.first == "File")
		{
			std::string relative_path;
			attr.second->GetValue(relative_path);
			std::string sfname= dirname + relative_path;
			if(FILEMAN->IsADirectory(sfname))
			{
				set_type("LoadActor");
				store_field("File", attr.second, false);
			}
			else
			{
				vector<std::string> files_in_dir;
				FILEMAN->GetDirListing(sfname + "*", files_in_dir, false, true);
				int handled_level= 0;
				std::string found_file= "";
				for(auto file= files_in_dir.begin();
						file != files_in_dir.end() && handled_level < 2; ++file)
				{
					std::string extension= GetExtension(*file);
					FileType file_type= ActorUtil::GetFileType(*file);
					std::string this_relative=
						add_extension_to_relative_path_from_found_file(relative_path, *file);
					switch(file_type)
					{
						case FT_Xml:
							this_relative= SetExtension(this_relative, "lua");
						case FT_Lua:
							set_type("LoadActor");
							store_field("File", this_relative, false);
							handled_level= 2;
							break;
						case FT_Ini:
							break;
						case FT_Bitmap:
						case FT_Movie:
							if(handled_level < 2)
							{
								set_type("Sprite");
								store_field("File", this_relative, false);
								handled_level= 1;
							}
							break;
						case FT_Sound:
							set_type("ActorSound");
							store_field("File", this_relative, false);
							handled_level= 1;
							break;
						case FT_Sprite:
							set_type("Sprite");
							load_frames_from_file(dirname + this_relative, Rage::dir_name(relative_path));
							handled_level= 2;
							break;
						case FT_Model:
							set_type("Model");
							store_field("Meshes", this_relative, false);
							store_field("Materials", this_relative, false);
							store_field("Bones", this_relative, false);
							handled_level= 2;
							break;
						default:
							break;
					}
					if(!handled_level)
					{
						if(extension == "model")
						{
							set_type("Model");
							load_model_from_file(dirname + this_relative, Rage::dir_name(relative_path));
							handled_level= 2;
						}
					}
				}
				if(!handled_level)
				{
					if(!files_in_dir.empty())
					{
						std::string this_relative=
							add_extension_to_relative_path_from_found_file(relative_path, files_in_dir[0]);
						store_field("File", this_relative, false);
					}
					else
					{
						store_field("File", attr.second, false);
					}
				}
			}
		}
		else
		{
			store_field(attr.first, attr.second, true);
		}
	}
	if(type == "BitmapText")
	{
		rename_field("Texture", "Font");
		rename_field("File", "Font");
	}
	if(type == "Sprite")
	{
		rename_field("File", "Texture");
	}
	XNode const* xren= node.GetChild("children");
	if(xren != nullptr)
	{
		for (auto const *child: *xren)
		{
			actor_template_t chill_plate;
			chill_plate.load_node(*child, dirname, conditions);
			children.push_back(chill_plate);
		}
	}
	if(!x.empty() || !y.empty())
	{
		std::string pos_init= "xy," + x + "," + y;
		field_cont_t::iterator init= fields.find("InitCommand");
		if(init != fields.end())
		{
			pos_init= pos_init + ";queuecommand,xtl_passed_initCommand";
			fields["xtl_passed_initCommand"]= init->second;
		}
		fields["InitCommand"]= "cmd(" + pos_init + ")";
	}
}

void actor_template_t::output_to_file(RageFile* file, std::string const& indent)
{
	if(!condition.empty())
	{
		file->Write(indent + "optional_actor(" + condition + "_result,\n");
	}
	if(type == "LoadActor")
	{
		file->Write(indent + "LoadActor('" + get_field("File") + "') .. {\n");
	}
	else
	{
		file->Write(indent + "Def." + type + "{\n");
	}
	std::string subindent= indent + "  ";
	if(name.empty())
	{
		name= unique_name("actor");
	}
	file->Write(subindent + "Name= \"" + name + "\",\n");
	if(!frames.empty())
	{
		file->Write(subindent + "Frames= {\n");
		std::string frameindent= subindent + "  ";
		for(vector<frame_t>::iterator frame= frames.begin();
			frame != frames.end(); ++frame)
		{
			file->Write(frameindent + "{Frame= " + std::to_string(frame->frame) +
				", Delay= " + FloatToString(frame->delay) + "},\n");
		}
		file->Write(indent + "},\n");
	}
	for(auto field= fields.begin(); field != fields.end(); ++field)
	{
		auto is_string = fields_that_are_strings.find(field->first);
		if(is_string != fields_that_are_strings.end())
		{
			file->Write(subindent + field->first + "= \"" + field->second + "\",\n");
		}
		else
		{
			file->Write(subindent + field->first + "= " + field->second + ",\n");
		}
	}
	for(vector<actor_template_t>::iterator child= children.begin();
		child != children.end(); ++child)
	{
		child->output_to_file(file, subindent);
		file->Write(",\n");
	}
	file->Write(indent + "}");
	if(!condition.empty())
	{
		file->Write(")");
	}
}

void convert_xml_file(std::string const& fname, std::string const& dirname)
{
	if(arg_converters.empty())
	{
		init_parser_helpers();
	}
	LOG->Trace("Beginning conversion of entry: %s", fname.c_str());
	XNode xml;
	if(!XmlFileUtil::LoadFromFileShowErrors(xml, fname))
	{
		LOG->Trace("Error when loading xml.");
		return;
	}
	actor_template_t plate;
	condition_set_t conditions;
	plate.load_node(xml, dirname, conditions);
	RageFile* file= new RageFile;
	std::string out_name{ Rage::head(fname, -4) + ".lua" };
	if(!file->Open(out_name, RageFile::WRITE))
	{
		LOG->Trace("Could not open %s: %s", out_name.c_str(), file->GetError().c_str());
		return;
	}
	LOG->Trace("Saving conversion to: %s", out_name.c_str());
	for(condition_set_t::iterator cond= conditions.begin();
		cond != conditions.end(); ++cond)
	{
		std::string cond_text= cond->first;
		convert_lua_chunk(cond_text);
		file->Write("local " + cond->second + "_result= " + cond_text + "\n\n");
	}
	if(!conditions.empty())
	{
		file->Write("local function optional_actor(cond, actor)\n"
			"	if cond then return actor end\n"
			"	return Def.Actor{}\n"
			"end\n\n");
	}
	file->Write("return ");
	plate.output_to_file(file, "");
	file->Write("\n");
	file->Close();
	delete file;
}

int LuaFunc_convert_xml_bgs(lua_State* L);
int LuaFunc_convert_xml_bgs(lua_State* L)
{
	std::string dir= SArg(1);
	vector<std::string> xml_list;
	convert_xmls_in_dir(dir + "/");
	return 0;
}

LUAFUNC_REGISTER_COMMON(convert_xml_bgs);

/*
 * (c) 2014 Eric Reese
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
