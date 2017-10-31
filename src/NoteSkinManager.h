#ifndef NEW_SKIN_MANAGER_H
#define NEW_SKIN_MANAGER_H

#include "NoteSkin.h"

struct NoteSkinManager
{
	NoteSkinManager();
	~NoteSkinManager();

	void load_skins();
	void get_skins_for_stepstype(StepsType type, std::vector<NoteSkinLoader const*>& ret);
	void get_all_skin_names(std::vector<std::string>& ret);
	void get_skin_names_for_stepstype(StepsType type, std::vector<std::string>& ret, bool disable_supports_all= false);
	std::string get_first_skin_name_for_stepstype(StepsType type);
	std::vector<StepsType> const& get_supported_stepstypes();
	bool skin_supports_stepstype(std::string const& skin, StepsType type);
	NoteSkinLoader const* get_loader_for_skin(std::string const& skin_name);
	std::string get_path(NoteSkinLoader const* skin,
		std::string file);
	bool named_skin_exists(std::string const& skin_name);

	void PushSelf(lua_State* L);

private:
	std::vector<NoteSkinLoader> m_skins;
	std::vector<StepsType> m_supported_types;
};

extern NoteSkinManager* NOTESKIN; // global and accessible from anywhere in our program

#endif
