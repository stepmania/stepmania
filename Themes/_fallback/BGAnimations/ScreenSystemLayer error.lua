-- If you are a common themer, DO NOT INCLUDE THIS FILE IN YOUR THEME.
-- This layer is purely for error reporting, so that you can see errors on screen easily while running stepmania.
-- If you include this file in your theme, you will not benefit from any improvements in error reporting.
-- If you want a different background behind the errors, then include "ScreenSystemLayer errorbg.lua" in your theme.
-- If you want to adjust how long errors stay on screen for, call the "SetErrorMessageTime" function. (see usage notes in comments above that function)

-- Minor text formatting functions from Kyzentun.
-- TODO:  Figure out why BitmapText:maxwidth doesn't do what I want.
local function width_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or 1
	if text:GetWidth() * natural_zoom > limit then
		text:zoomx(limit / text:GetWidth())
	else
		text:zoomx(natural_zoom)
	end
end

local function width_clip_text(text, limit)
	local full_text= text:GetText()
	local fits= text:GetZoomedWidth() <= limit
	local prev_max= #full_text - 1
	local prev_min= 0
	if not fits then
		while prev_max - prev_min > 1 do
			local new_max= math.round((prev_max + prev_min) / 2)
			text:settext(full_text:sub(1, 1+new_max))
			if text:GetZoomedWidth() <= limit then
				prev_min= new_max
			else
				prev_max= new_max
			end
		end
		text:settext(full_text:sub(1, 1+prev_min))
	end
end

local function width_clip_limit_text(text, limit, natural_zoom)
	natural_zoom= natural_zoom or text:GetZoomY()
	local text_width= text:GetWidth() * natural_zoom
	if text_width > limit * 2 then
		text:zoomx(natural_zoom * .5)
		width_clip_text(text, limit)
	else
		width_limit_text(text, limit, natural_zoom)
	end
end

local text_actors= {}
local line_height= 12 -- A good line height for Common Normal at .5 zoom.
local line_width= SCREEN_WIDTH - 20

local next_message_actor= 1
local show_requested= false

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
	Trace("Setting error message " .. which .. " time to " .. t)
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

local frame_args= {
	Name="Error frame",
	InitCommand= function(self)
		self:y(-SCREEN_HEIGHT)
	end,
	ScriptErrorMessageCommand = function(self, params)
		show_requested= false
		self:stoptweening()
		self:visible(true)
		local covered_height= math.min(line_height * next_message_actor, SCREEN_HEIGHT)
		self:y(-SCREEN_HEIGHT + covered_height)
		self:sleep(message_time.show)
		self:queuecommand("DecNextActor")
		-- Shift the text on all the actors being shown up by one.
		for i= next_message_actor, 1, -1 do
			if text_actors[i] then
				text_actors[i]:visible(true)
				if i > 1 and text_actors[i-1] then
					text_actors[i]:settext(text_actors[i-1]:GetText())
				else
					text_actors[i]:settext(params.Message)
				end
				width_clip_limit_text(text_actors[i], line_width)
			end
		end
		if next_message_actor <= #text_actors then
			next_message_actor= next_message_actor + 1
		end
	end,
	ToggleErrorsMessageCommand= function(self, params)
		if show_requested then
			self:playcommand("HideErrors")
		else
			self:playcommand("ShowErrors")
		end
	end,
	ShowErrorsMessageCommand= function(self, params)
		show_requested= true
		for i, mactor in ipairs(text_actors) do
			if mactor:GetText() ~= "" then
				mactor:visible(true)
				next_message_actor= next_message_actor + 1
			end
		end
		next_message_actor= next_message_actor - 1
		local covered_height= math.min(line_height * next_message_actor, SCREEN_HEIGHT)
		self:stoptweening()
		self:visible(true)
		self:linear(message_time.hide)
		self:y(-SCREEN_HEIGHT + covered_height)
	end,
	HideErrorsMessageCommand= function(self, params)
		if not show_requested then return end
		show_requested= false
		self:stoptweening()
		next_message_actor= 1
		self:linear(message_time.hide)
		self:y(-SCREEN_HEIGHT)
		self:queuecommand("HideText")
	end,
	ClearErrorsMessageCommand= function(self, params)
		-- This is so that someone using ShowErrorsMessageCommand can clear ones
		-- they've dealt with.
		-- Only allow clearing errors that we have scrolled off screen.
		for i, mactor in ipairs(text_actors) do
			if not mactor:GetVisible() then
				mactor:settext("")
			end
		end
	end,
	DecNextActorCommand= function(self)
		self:linear(message_time.hide)
		self:y(self:GetY()-line_height)
		if text_actors[next_message_actor] then
			text_actors[next_message_actor]:visible(false)
		end
		next_message_actor= next_message_actor - 1
		if next_message_actor > 1 then
			self:queuecommand("DecNextActor")
		else
			self:queuecommand("Off")
		end
	end,
	HideTextCommand= function(self)
		for i, mactor in ipairs(text_actors) do
			mactor:visible(false)
		end
	end,
	OffCommand= cmd(visible,false),
	Def.Quad {
		Name= "errorbg",
		InitCommand= function(self)
			self:setsize(SCREEN_WIDTH, SCREEN_HEIGHT)
			self:horizalign(left)
			self:vertalign(top)
			self:diffuse(color("0,0,0,0"))
			self:diffusealpha(.85)
		end,
	}
}
-- Create enough text actors that we can fill the screen.
local num_text= SCREEN_HEIGHT / line_height
for i= 1, num_text do
	frame_args[#frame_args+1]= 	LoadFont("Common","Normal") .. {
		Name="Text" .. i,
		InitCommand= function(self)
			-- Put them in the list in reverse order so the ones at the bottom of the screen are used first.
			text_actors[num_text-i+1]= self
			self:horizalign(left)
			self:vertalign(top)
			self:x(SCREEN_LEFT + 10)
			self:y(SCREEN_TOP + (line_height * (i-1)) + 2)
			self:shadowlength(1)
			self:zoom(.5)
			self:visible(false)
		end,
		OffCommand= cmd(visible,false),
	}
end

Trace("Loaded error layer.")

return Def.ActorFrame(frame_args)
