-- If you are a common themer, DO NOT INCLUDE THIS FILE IN YOUR THEME.
-- This layer is purely for error reporting, so that you can see errors on screen easily while running stepmania.
-- If you include this file in your theme, you will not benefit from any improvements in error reporting.
-- If you want to adjust how long errors stay on screen for, call the "SetErrorMessageTime" function. (see usage notes in comments above that function)

local line_height= 12 -- A good line height for Common Normal at .5 zoom.

local min_message_time= {show= 1, hide= .03125}
local default_message_time= {show= 4, hide= .125}
local message_time= {}
for k, v in pairs(default_message_time) do
	message_time[k]= v
end

-- Example usage:
-- "SetErrorMessageTime('show' 5)" sets errors to show for 5 seconds before beginning to hide.
-- "SetErrorMessageTime('hide' .5)" sets errors to hide one error every .5 seconds after the show time has passed.
function SetErrorMessageTime(which, t)
	if not min_message_time[which] then
		MESSAGEMAN:Broadcast(
			"ScriptError", {
				Message= "Attempted to set invalid overlay message time field: " ..
					tostring(which)})
		return
	end
	if t < min_message_time[which] then
		MESSAGEMAN:Broadcast(
			"ScriptError", {
				Message= "Attempted to set overlay message " .. which ..
					" time to below minimum of " .. min_message_time[which] .. "."})
		return
	end
	message_time[which]= t
end

function GetErrorMessageTime(which)
	return message_time[which]
end

function GetErrorMessageTimeMin(which)
	return min_message_time[which]
end

function GetErrorMessageTimeDefault(which)
	return default_message_time[which]
end

local log_args= {
	Name= "ScriptError",
	ReplaceLinesWhenHidden= true,
	IgnoreIdentical= true,
	Times= message_time,
	Font= "Common Error",
}

return Def.LogDisplay(log_args)
