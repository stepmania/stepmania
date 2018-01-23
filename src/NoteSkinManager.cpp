#include "global.h"

#include "ActorUtil.h"
#include "NoteSkinManager.h"
#include "RageFileManager.h"
#include "SpecialFiles.h"

using std::vector;

NoteSkinManager* NOTESKIN= nullptr; // global and accessible from anywhere in our program

NoteSkinManager::NoteSkinManager()
{
	// Register with Lua.
	Lua *L = LUA->Get();
	lua_pushstring(L, "NOTESKIN");
	PushSelf(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	LUA->Release(L);

	load_skins();
}

NoteSkinManager::~NoteSkinManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal("NOTESKIN");
}

void NoteSkinManager::load_skins()
{
	vector<std::string> dirs;
	FILEMAN->GetDirListing(SpecialFiles::NOTESKINS_DIR + "*", dirs, true, true);
	m_skins.clear();
	m_supported_types.clear();
	m_skins.reserve(dirs.size());
	for(auto&& dir : dirs)
	{
		std::string skin_file= dir + "/noteskin.lua";
		// If noteskin.lua doesn't exist, maybe the folder is for something else.
		// Ignore it.
		if(FILEMAN->DoesFileExist(skin_file))
		{
			NoteSkinLoader loader;
			if(loader.load_from_file(skin_file))
			{
				m_skins.push_back(loader);
			}
		}
	}
	for(int st= 0; st < NUM_StepsType; ++st)
	{
		bool supported= false;
		for(auto&& skin : m_skins)
		{
			if(skin.supports_needed_buttons(static_cast<StepsType>(st)))
			{
				supported= true;
				break;
			}
		}
		if(supported)
		{
			m_supported_types.push_back(static_cast<StepsType>(st));
		}
	}
}

void NoteSkinManager::get_skins_for_stepstype(StepsType type, std::vector<NoteSkinLoader const*>& ret)
{
	for(auto&& skin : m_skins)
	{
		if(skin.supports_needed_buttons(type))
		{
			ret.push_back(&skin);
		}
	}
}

void NoteSkinManager::get_all_skin_names(std::vector<std::string>& ret)
{
	for(auto&& skin : m_skins)
	{
		ret.push_back(skin.get_name());
	}
}

void NoteSkinManager::get_skin_names_for_stepstype(StepsType type, std::vector<std::string>& ret, bool disable_supports_all)
{
	for(auto&& skin : m_skins)
	{
		if(skin.supports_needed_buttons(type, disable_supports_all))
		{
			ret.push_back(skin.get_name());
		}
	}
}

std::string NoteSkinManager::get_first_skin_name_for_stepstype(StepsType type)
{
	for(auto&& skin : m_skins)
	{
		if(skin.supports_needed_buttons(type))
		{
			return skin.get_name();
		}
	}
	std::string stype_name= StepsTypeToString(type);
	LuaHelpers::ReportScriptError("No noteskin supports the stepstype " + stype_name);
	return "default";
}

std::vector<StepsType> const& NoteSkinManager::get_supported_stepstypes()
{
	return m_supported_types;
}

bool NoteSkinManager::skin_supports_stepstype(std::string const& skin, StepsType type)
{
	NoteSkinLoader const* loader= get_loader_for_skin(skin);
	// This does not report an error when the skin is not found because it is
	// used by the profile to pick a skin to use, and the profile might have the
	// names of unknown skins in it.
	if(loader == nullptr)
	{
		return false;
	}
	return loader->supports_needed_buttons(type);
}

NoteSkinLoader const* NoteSkinManager::get_loader_for_skin(std::string const& skin_name)
{
	for(auto&& skin : m_skins)
	{
		if(skin.get_name() == skin_name)
		{
			return &skin;
		}
	}
	return nullptr;
}

std::string NoteSkinManager::get_path(
	NoteSkinLoader const* skin, std::string file)
{
	if(skin == nullptr)
	{
		return "";
	}
	// Check to see if the filename is already a valid path.
	std::string resolved= file;
	if(ActorUtil::ResolvePath(resolved, skin->get_name(), true))
	{
		return resolved;
	}
	// Fallback loop cases are detected and silently ignored by storing each
	// fallback in used_fallbacks.  This allows skins to mutually fall back on
	// each other if someone really needs to do that.
	std::unordered_set<std::string> used_fallbacks;
	std::string next_path= skin->get_load_path();
	std::string next_fallback= skin->get_fallback_name();
	std::string found_path;
	while(!next_path.empty())
	{
		resolved= next_path + file;
		next_path.clear();
		if(ActorUtil::ResolvePath(resolved, skin->get_name(), true))
		{
			return resolved;
		}
		else if(!next_fallback.empty() &&
			used_fallbacks.find(next_fallback) == used_fallbacks.end())
		{
			used_fallbacks.insert(next_fallback);
			NoteSkinLoader const* fallback= get_loader_for_skin(next_fallback);
			if(fallback != nullptr)
			{
				used_fallbacks.insert(next_fallback);
				next_path= fallback->get_load_path();
				next_fallback= fallback->get_fallback_name();
			}
		}
	}
	return "";
}

bool NoteSkinManager::named_skin_exists(std::string const& skin_name)
{
	for(auto&& skin : m_skins)
	{
		if(skin_name == skin.get_name())
		{
			return true;
		}
	}
	return false;
}


#include "LuaBinding.h"

struct LunaNoteSkinManager: Luna<NoteSkinManager>
{
	static int get_all_skin_names(T* p, lua_State* L)
	{
		vector<std::string> names;
		p->get_all_skin_names(names);
		LuaHelpers::CreateTableFromArray(names, L);
		return 1;
	}
	static int get_skin_names_for_stepstype(T* p, lua_State* L)
	{
		StepsType stype= Enum::Check<StepsType>(L, 1);
		vector<std::string> names;
		bool without_all= static_cast<bool>(lua_toboolean(L, 2));
		p->get_skin_names_for_stepstype(stype, names, without_all);
		LuaHelpers::CreateTableFromArray(names, L);
		return 1;
	}
	static int get_path(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		std::string file_name= SArg(2);
		NoteSkinLoader const* loader= p->get_loader_for_skin(skin_name);
		if(loader == nullptr)
		{
			luaL_error(L, "No such noteskin.");
		}
		std::string path= p->get_path(loader, file_name);
		if(path.empty())
		{
			lua_pushnil(L);
		}
		else
		{
			lua_pushstring(L, path.c_str());
		}
		return 1;
	}
	static int get_skin_parameter_defaults(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		NoteSkinLoader const* loader= p->get_loader_for_skin(skin_name);
		if(loader == nullptr)
		{
			luaL_error(L, "No such noteskin.");
		}
		else
		{
			loader->push_skin_parameter_defaults(L);
		}
		return 1;
	}
	static int get_skin_parameter_info(T* p, lua_State* L)
	{
		std::string skin_name= SArg(1);
		NoteSkinLoader const* loader= p->get_loader_for_skin(skin_name);
		if(loader == nullptr)
		{
			luaL_error(L, "No such noteskin.");
		}
		else
		{
			loader->push_skin_parameter_info(L);
		}
		return 1;
	}
	static int reload_skins(T* p, lua_State* L)
	{
		p->load_skins();
		COMMON_RETURN_SELF;
	}

	LunaNoteSkinManager()
	{
		ADD_METHOD(get_all_skin_names);
		ADD_METHOD(get_skin_names_for_stepstype);
		ADD_METHOD(get_path);
		ADD_METHOD(get_skin_parameter_defaults);
		ADD_METHOD(get_skin_parameter_info);
		ADD_METHOD(reload_skins);
	}
};

LUA_REGISTER_CLASS(NoteSkinManager);
