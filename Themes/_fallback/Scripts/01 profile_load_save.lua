-- Having single LoadProfileCustom and SaveProfileCustom functions doesn't
-- extend very well when you have a bunch of custom things to save to the
-- profile and you just want to create callback functions for handling
-- loading/saving.
-- So this is a system for adding to a list of custom load/save functions.
-- -Kyz

local profile_load_callbacks= {}
local profile_save_callbacks= {}

function add_profile_load_callback(func)
	table.insert(profile_load_callbacks, func)
end
function add_profile_save_callback(func)
	table.insert(profile_save_callbacks, func)
end

function LoadProfileCustom(profile, dir)
	for i, callback in ipairs(profile_load_callbacks) do
		callback(profile, dir)
	end
end
function SaveProfileCustom(profile, dir)
	for i, callback in ipairs(profile_save_callbacks) do
		callback(profile, dir)
	end
end
