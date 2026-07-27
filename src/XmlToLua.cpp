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

#include <cstddef>
#include <map>
#include <set>
#include <vector>

#define TWEEN_QUEUE_MAX 50

RString unique_name();
void convert_xmls_in_dir(RString const& dirname);
void convert_xml_file(RString const& fname, RString const& dirname);
RString maybe_conv_pos(RString pos, RString (*conv_func)(float p));
RString add_extension_to_relative_path_from_found_file(
	RString const& relative_path, RString const& found_file);

RString unique_name(RString const& type)
{
	static char const* name_chars= "abcdefghijklmnopqrstuvwxyz";
	static int name_count= 0;
	int curr_name= name_count;
	RString ret= "xtl_" + type + "_"; // Minimize the chance of a name collision.
	ret= ret + name_chars[curr_name%26];
	while(curr_name / 26 > 0)
	{
		curr_name= curr_name / 26;
		ret= ret + name_chars[curr_name%26];
	}
	++name_count;
	return ret;
}

void convert_xmls_in_dir(RString const& dirname)
{
	std::vector<RString> listing;
	FILEMAN->GetDirListing(dirname, listing, false, true);
	for(std::vector<RString>::iterator curr_file= listing.begin();
		curr_file != listing.end(); ++curr_file)
	{
		switch(ActorUtil::GetFileType(*curr_file))
		{
			case FT_Xml:
				convert_xml_file(*curr_file, dirname);
				break;
			case FT_Directory:
				convert_xmls_in_dir((*curr_file) + "/");
				break;
			default: // Ignore anything not xml or directory.
				break;
		}
	}
}

RString convert_xpos(float x)
{
	return "SCREEN_CENTER_X + " + std::to_string(x - 320.0f);
}

RString convert_ypos(float y)
{
	return "SCREEN_CENTER_Y + " + std::to_string(y - 240.0f);
}

RString maybe_conv_pos(RString pos, RString (*conv_func)(float p))
{
	float f;
	if(pos >> f)
	{
		return conv_func(f);
	}
	return pos;
}

std::size_t after_slash_or_zero(RString const& s)
{
	std::size_t ret= s.rfind('/');
	if(ret != std::string::npos)
	{
		return ret+1;
	}
	return 0;
}

RString add_extension_to_relative_path_from_found_file(
	RString const& relative_path, RString const& found_file)
{
	std::size_t rel_last_slash= after_slash_or_zero(relative_path);
	std::size_t found_last_slash= after_slash_or_zero(found_file);
	return relative_path.Left(rel_last_slash) +
		found_file.substr(found_last_slash, std::string::npos);
}

bool verify_arg_count(RString cmd, std::vector<RString>& args, std::size_t req)
{
	if(args.size() < req)
	{
		LuaHelpers::ReportScriptError("Not enough args to " + cmd + " command.");
		return false;
	}
	return true;
}

typedef void (*arg_converter_t)(std::vector<RString>& args);

std::map<RString, arg_converter_t> arg_converters;
std::map<RString, std::size_t> tween_counters;
std::set<RString> fields_that_are_strings;
std::map<RString, RString> chunks_to_replace;

#define COMMON_ARG_VERIFY(count) if(!verify_arg_count(args[0], args, count)) return;
void x_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	float pos;
	if(args[1] >> pos)
	{
		args[1]= convert_xpos(pos);
	}
}
void y_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	float pos;
	if(args[1] >> pos)
	{
		args[1]= convert_ypos(pos);
	}
}
void string_arg_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	args[1]= "\"" + args[1] + "\"";
}
void lower_string_conv(std::vector<RString>& args)
{
	args[0].MakeLower();
}
void hidden_conv(std::vector<RString>& args)
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
void diffuse_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	RString retarg;
	for(std::size_t i= 1; i < args.size(); ++i)
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
const RString& BlendModeToString(BlendMode);
const RString& CullModeToString(CullMode);
void blend_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	for(int i= 0; i < NUM_BlendMode; ++i)
	{
		RString blend_str= BlendModeToString(static_cast<BlendMode>(i));
		blend_str.MakeLower();
		if(args[1] == blend_str)
		{
			args[1]= "\"BlendMode_" + BlendModeToString(static_cast<BlendMode>(i)) + "\"";
			break;
		}
	}
}
void cull_conv(std::vector<RString>& args)
{
	COMMON_ARG_VERIFY(2);
	for(int i= 0; i < NUM_CullMode; ++i)
	{
		RString cull_str= CullModeToString(static_cast<CullMode>(i));
		cull_str.MakeLower();
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

void convert_lua_chunk(RString& chunk_text)
{
	for(std::map<RString, RString>::iterator chunk= chunks_to_replace.begin();
			chunk != chunks_to_replace.end(); ++chunk)
	{
		chunk_text.Replace(chunk->first, chunk->second);
	}
}

// Conditions are mapped by condition string.
// So condition_set_t::iterator->first is the lua to execute for the
// condition, and condition_set_t::iterator->second is the name of the
// condition.
typedef std::map<RString, RString> condition_set_t;
typedef std::map<RString, RString> field_cont_t;
struct frame_t
{
	int frame;
	float delay;
	frame_t() :frame(0), delay(0.0f) {}
};

struct actor_template_t
{
	RString type;
	field_cont_t fields;
	RString condition;
	RString name;
	std::vector<frame_t> frames;
	std::vector<actor_template_t> children;
	RString x;
	RString y;
	void make_space_for_frame(int id);
	void store_cmd(RString const& cmd_name, RString const& full_cmd);
	void store_field(RString const& field_name, RString const& value, bool cmd_convert, RString const& pref= "", RString const& suf= "");
	void store_field(RString const& field_name, XNodeValue const* value, bool cmd_convert, RString const& pref= "", RString const& suf= "");
	void rename_field(RString const& old_name, RString const& new_name);
	RString get_field(RString const& field_name);
	void load_frames_from_file(RString const& fname, RString const& rel_path);
	void load_model_from_file(RString const& fname, RString const& rel_path);
	void load_node(XNode const& node, RString const& dirname, condition_set_t& conditions);
	void output_to_file(RageFile* file, RString const& indent);
};

void actor_template_t::make_space_for_frame(int id)
{
	if(id >= static_cast<int>(frames.size()))
	{
		frames.resize(id+1);
	}
}

void actor_template_t::store_cmd(RString const& cmd_name, RString const& full_cmd)
{
	if(full_cmd.Left(1) == "%")
	{
		RString cmd_text= full_cmd.Right(full_cmd.size()-1);
		convert_lua_chunk(cmd_text);
		fields[cmd_name]= cmd_text;
		return;
	}
	std::vector<RString> cmds;
	split(full_cmd, ";", cmds, true);
	std::size_t queue_size= 0;
	// If someone has a simfile that uses a playcommand that pushes tween
	// states onto the queue, queue size counting will have to be made much
	// more complex to prevent that from causing an overflow.
	for(std::vector<RString>::iterator cmd= cmds.begin(); cmd != cmds.end(); ++cmd)
	{
		std::vector<RString> args;
		split(*cmd, ",", args, true);
		if(!args.empty())
		{
			for(std::vector<RString>::iterator arg= args.begin(); arg != args.end(); ++arg)
			{
				std::size_t first_nonspace= 0;
				std::size_t last_nonspace= arg->size();
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
			std::map<RString, arg_converter_t>::iterator conv= arg_converters.find(args[0]);
			if(conv != arg_converters.end())
			{
				conv->second(args);
			}
			std::map<RString, std::size_t>::iterator counter= tween_counters.find(args[0]);
			if(counter != tween_counters.end())
			{
				queue_size+= counter->second;
			}
		}
		*cmd= join(",", args);
	}
	// This code is probably actually useless, OITG has the same tween queue size
	// and the real reason I saw overflows in converted files was a bug in
	// foreground loading that ran InitCommand twice. -Kyz
	if(queue_size >= TWEEN_QUEUE_MAX)
	{
		std::size_t num_to_make= (queue_size / TWEEN_QUEUE_MAX) + 1;
		std::size_t states_per= (queue_size / num_to_make) + 1;
		std::size_t states_in_curr= 0;
		RString this_name= cmd_name;
		std::vector<RString> curr_cmd;
		for(std::vector<RString>::iterator cmd= cmds.begin(); cmd != cmds.end(); ++cmd)
		{
			curr_cmd.push_back(*cmd);
			std::vector<RString> args;
			split(*cmd, ",", args, true);
			if(!args.empty())
			{
				std::map<RString, std::size_t>::iterator counter= tween_counters.find(args[0]);
				if(counter != tween_counters.end())
				{
					states_in_curr+= counter->second;
					if(states_in_curr >= states_per - 1)
					{
						RString next_name= unique_name("cmd");
						curr_cmd.push_back("queuecommand,\"" + next_name + "\"");
						fields[this_name]= "cmd(" + join(";", curr_cmd) + ")";
						curr_cmd.clear();
						this_name= next_name;
						states_in_curr= 0;
					}
				}
			}
		}
		if(!curr_cmd.empty())
		{
			fields[this_name]= "cmd(" + join(";", curr_cmd) + ")";
		}
	}
	else
	{
		fields[cmd_name]= "cmd(" + join(";", cmds) + ")";
	}
}

void actor_template_t::store_field(RString const& field_name, RString const& value, bool cmd_convert, RString const& pref, RString const& suf)
{
	// OITG apparently allowed "Oncommand" as valid.
	if(field_name.Right(7).MakeLower() != "command")
	{
		cmd_convert= false;
	}
	if(cmd_convert)
	{
		RString real_field_name= field_name.Left(field_name.size()-7) + "Command";
		store_cmd(real_field_name, value);
	}
	else
	{
		fields[field_name]= pref + value + suf;
	}

}
void actor_template_t::store_field(RString const& field_name, XNodeValue const* value, bool cmd_convert, RString const& pref, RString const& suf)
{
	RString val;
	value->GetValue(val);
	store_field(field_name, val, cmd_convert, pref, suf);
}

void actor_template_t::rename_field(RString const& old_name, RString const& new_name)
{
	field_cont_t::iterator old_field= fields.find(old_name);
	if(old_field == fields.end())
	{
		return;
	}
	fields[new_name]= old_field->second;
	fields.erase(old_field);
}

RString actor_template_t::get_field(RString const& field_name)
{
	field_cont_t::iterator field= fields.find(field_name);
	if(field == fields.end())
	{
		return "";
	}
	return field->second;
}

void actor_template_t::load_frames_from_file(RString const& fname, RString const& rel_path)
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
		FOREACH_CONST_Attr(sprite_node, attr)
		{
			// Frame and Delay fields have names of the form "Frame0000" where the
			// "0000" part is the id of the frame.
			RString field_type= attr->first.Left(5);
			if(field_type == "Frame")
			{
				int id= StringToInt(attr->first.Right(attr->first.size()-5));
				make_space_for_frame(id);
				attr->second->GetValue(frames[id].frame);
			}
			else if(field_type == "Delay")
			{
				int id= StringToInt(attr->first.Right(attr->first.size()-5));
				make_space_for_frame(id);
				attr->second->GetValue(frames[id].delay);
			}
			else if(field_type == "Textu")
			{
				store_field("Texture", attr->second, false, rel_path, "");
			}
			else
			{
				// Unrecognized, ignore.
			}
		}
	}
}

void actor_template_t::load_model_from_file(RString const& fname, RString const& rel_path)
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
		FOREACH_CONST_Attr(model_node, attr)
		{
			store_field(attr->first, attr->second, false, rel_path, "");
		}
	}
}

void actor_template_t::load_node(XNode const& node, RString const& dirname, condition_set_t& conditions)
{
	type= node.GetName();
	bool type_set_by_automagic= false;
#define set_type(auto_type) type_set_by_automagic= true; type= auto_type;
	FOREACH_CONST_Attr(&node, attr)
	{
		if(attr->first == "Name")
		{
			attr->second->GetValue(name);
		}
		else if(attr->first == "Condition")
		{
			RString cond_str;
			attr->second->GetValue(cond_str);
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
		else if(attr->first == "Type" || attr->first == "Class")
		{
			if(!type_set_by_automagic)
			{
				attr->second->GetValue(type);
			}
		}
		else if(attr->first == "__TEXT__")
		{
			// Ignore.  This attribute seems to be put in by the xml parser, and
			// not part of the actual xml code.
		}
		else if(attr->first == "Text")
		{
			set_type("BitmapText");
			store_field(attr->first, attr->second, true);
		}
		else if(attr->first == "File")
		{
			RString relative_path;
			attr->second->GetValue(relative_path);
			RString sfname= dirname + relative_path;
			if(FILEMAN->IsADirectory(sfname))
			{
				set_type("LoadActor");
				store_field("File", attr->second, false);
			}
			else
			{
				std::vector<RString> files_in_dir;
				FILEMAN->GetDirListing(sfname + "*", files_in_dir, false, true);
				int handled_level= 0;
				RString found_file= "";
				for(std::vector<RString>::iterator file= files_in_dir.begin();
						file != files_in_dir.end() && handled_level < 2; ++file)
				{
					RString extension= GetExtension(*file);
					FileType file_type= ActorUtil::GetFileType(*file);
					RString this_relative=
						add_extension_to_relative_path_from_found_file(relative_path, *file);
					switch(file_type)
					{
						case FT_Xml:
							this_relative= SetExtension(this_relative, "lua");
							[[fallthrough]];
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
							load_frames_from_file(dirname + this_relative, Dirname(relative_path));
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
							load_model_from_file(dirname + this_relative, Dirname(relative_path));
							handled_level= 2;
						}
					}
				}
				if(!handled_level)
				{
					if(!files_in_dir.empty())
					{
						RString this_relative=
							add_extension_to_relative_path_from_found_file(relative_path, files_in_dir[0]);
						store_field("File", this_relative, false);
					}
					else
					{
						store_field("File", attr->second, false);
					}
				}
			}
		}
		else
		{
			store_field(attr->first, attr->second, true);
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
		FOREACH_CONST_Child(xren, child)
		{
			actor_template_t chill_plate;
			chill_plate.load_node(*child, dirname, conditions);
			children.push_back(chill_plate);
		}
	}
	if(!x.empty() || !y.empty())
	{
		RString pos_init= "xy," + x + "," + y;
		field_cont_t::iterator init= fields.find("InitCommand");
		if(init != fields.end())
		{
			pos_init= pos_init + ";queuecommand,xtl_passed_initCommand";
			fields["xtl_passed_initCommand"]= init->second;
		}
		fields["InitCommand"]= "cmd(" + pos_init + ")";
	}
}

void actor_template_t::output_to_file(RageFile* file, RString const& indent)
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
	RString subindent= indent + "  ";
	if(name.empty())
	{
		name= unique_name("actor");
	}
	file->Write(subindent + "Name= \"" + name + "\",\n");
	if(!frames.empty())
	{
		file->Write(subindent + "Frames= {\n");
		RString frameindent= subindent + "  ";
		for(std::vector<frame_t>::iterator frame= frames.begin();
			frame != frames.end(); ++frame)
		{
			file->Write(frameindent + "{Frame= " + std::to_string(frame->frame) +
				", Delay= " + std::to_string(frame->delay) + "},\n");
		}
		file->Write(indent + "},\n");
	}
	for(field_cont_t::iterator field= fields.begin();
		field != fields.end(); ++field)
	{
		std::set<RString>::iterator is_string= fields_that_are_strings.find(field->first);
		if(is_string != fields_that_are_strings.end())
		{
			file->Write(subindent + field->first + "= \"" + field->second + "\",\n");
		}
		else
		{
			file->Write(subindent + field->first + "= " + field->second + ",\n");
		}
	}
	for(std::vector<actor_template_t>::iterator child= children.begin();
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

void convert_xml_file(RString const& fname, RString const& dirname)
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
	RString out_name= fname.Left(fname.size()-4) + ".lua";
	if(!file->Open(out_name, RageFile::WRITE))
	{
		LOG->Trace("Could not open %s: %s", out_name.c_str(), file->GetError().c_str());
		return;
	}
	LOG->Trace("Saving conversion to: %s", out_name.c_str());
	for(condition_set_t::iterator cond= conditions.begin();
		cond != conditions.end(); ++cond)
	{
		RString cond_text= cond->first;
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
	RString dir= SArg(1);
	std::vector<RString> xml_list;
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
