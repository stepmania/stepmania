-- Pester Kyzentun for an explanation if you need to customize this screen.
-- Also, this might be rewritten to us a proper customizable lua menu system
-- in the future.

-- Copy this file into your theme, then modify as needed to suit your theme.
-- Each of the things on this list has a comment marking it, so you can
-- quickly find it by searching.
-- Things you will want to change:
-- 1.  The Numpad
-- 2.  The Cursor
-- 3.  The Menu Items
-- 4.  The Menu Values
-- 4.1  The L/R indicators
-- 5.  The Menu Fader

local profile= GAMESTATE:GetEditLocalProfile()

local cursor_width_padding = 16
local cursor_spacing_value = 30

-- 1.  The Numpad
-- This is what sets up how the numpad looks.  See Scripts/04 NumPadEntry.lua
-- for a full description of how to customize a NumPad.
-- Note that if you provide a custom prompt actor for the NumPad, it must
-- have a SetCommand because the NumPad is used any time the player needs to
-- enter a number, and the prompt is updated by running its SetCommand.
local number_entry= new_numpad_entry{
	Name= "number_entry",
	InitCommand= cmd(diffusealpha, 0; xy, _screen.cx, _screen.cy * 1.5),
	value = LoadFont("Common Large") .. {
		InitCommand=cmd(xy,0,-62),
		OnCommand=cmd(zoom,0.75;diffuse,PlayerColor(PLAYER_1);strokecolor,ColorDarkTone(PlayerColor(PLAYER_1)));
		SetCommand=function(self, param)
			self:settext(param[1])
		end,
	},
	button = LoadFont("Common Normal") ..{
		InitCommand=cmd(shadowlength,1),
		SetCommand=function(self, param)
			self:settext(param[1])
		end,
		OnCommand=cmd(diffuse,color("0.8,0.8,0.8,1");zoom,0.875),
		GainFocusCommand=cmd(finishtweening;decelerate,0.125;zoom,1;diffuse,Color.White),
		LoseFocusCommand=cmd(finishtweening;smooth,0.1;zoom,0.875;diffuse,color("0.8,0.8,0.8,1"))
	},
	button_positions = {{-cursor_spacing_value, -cursor_spacing_value}, {0, -cursor_spacing_value}, {cursor_spacing_value, -cursor_spacing_value},
		{-cursor_spacing_value, 0},   {0, 0},   {cursor_spacing_value, 0},
		{-cursor_spacing_value, cursor_spacing_value}, {0, cursor_spacing_value},   {cursor_spacing_value, cursor_spacing_value},
		{-cursor_spacing_value, cursor_spacing_value*2}, {0, cursor_spacing_value*2},   {cursor_spacing_value, cursor_spacing_value*2}},
	cursor = Def.ActorFrame {
		-- Move whole container
		MoveCommand=function(self, param)
			self:stoptweening()
			self:decelerate(0.15)
			self:xy(param[1], param[2])
			if param[3] then
				self:z(param[3])
			end
		end,
		--
		LoadActor( THEME:GetPathG("_frame", "1D"),
							 { 2/18, 14/18, 2/18 },
							 LoadActor(THEME:GetPathB("_frame", "cursors/rounded fill"))
		) .. {
			OnCommand=cmd(diffuse,PlayerDarkColor(PLAYER_1)),
			FitCommand=function(self, param)
				self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(decelerate,0.125)})
			end,
				 },
		LoadActor( THEME:GetPathG("_frame", "1D"),
							 { 2/18, 14/18, 2/18 },
							 LoadActor(THEME:GetPathB("_frame", "cursors/rounded gloss"))
		) .. {
			OnCommand=cmd(diffuse,PlayerColor(PLAYER_1)),
			FitCommand=function(self, param)
				self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(decelerate,0.125)})
			end,
				 }
	},
	cursor_draw= "first",
	prompt = LoadFont("Common Bold") .. {
		Name="prompt",
		InitCommand=cmd(xy,0,-96);
		OnCommand=cmd(shadowlength,1;skewx,-0.125;diffusebottomedge,color("#DDDDDD");strokecolor,Color.Outline);
		SetCommand= function(self, params)
			self:settext(params[1])
		end
	},
	LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black", 128, 192) .. {
		InitCommand=cmd(xy, 0, -20)
	}
}

local function calc_list_pos(value, list)
	for i, entry in ipairs(list) do
		if entry.setting == value then
			return i
		end
	end
	return 1
end

local function item_value_to_text(item, value)
	if item.item_type == "bool" then
		if value then
			value= THEME:GetString("ScreenOptionsCustomizeProfile", item.true_text)
		else
			value= THEME:GetString("ScreenOptionsCustomizeProfile", item.false_text)
		end
	elseif item.item_type == "list" then
		local pos= calc_list_pos(value, item.list)
		return item.list[pos].display_name
	end
	return value
end

local char_list= {}
do
	local all_chars= CHARMAN:GetAllCharacters()
	for i, char in ipairs(char_list) do
		char_list[#char_list+1]= {
			setting= char:GetCharacterID(), display_name= char:GetDisplayName()}
	end
end

-- Uncomment this section if you need to test the behavior of actors in the
-- character list but don't have any duncing characters to test with.
--[=[

local fake_profile_mt= {__index= {}}
local val_list= {}
local function get_set_pair(name, default)
	fake_profile_mt.__index["Get"..name]= function(self)
		return self[name]
	end
	fake_profile_mt.__index["Set"..name]= function(self, val)
		self[name]= val
	end
	val_list[#val_list+1]= {name, default}
end
get_set_pair("WeightPounds", 0)
get_set_pair("Voomax", 0)
get_set_pair("BirthYear", 0)
get_set_pair("IgnoreStepCountCalories", false)
get_set_pair("IsMale", true)
get_set_pair("Character", "dietlinde")
fake_profile_mt.__index.init= function(self)
	for i, vald in ipairs(val_list) do
		self[vald[1]]= vald[2]
	end
end

profile= setmetatable({}, fake_profile_mt)
profile:init()

char_list= {
	{setting= "shake", display_name= "soda"},
	{setting= "freem", display_name= "inc"},
	{setting= "midi", display_name= "man"},
	{setting= "kyz", display_name= "zentun"},
	{setting= "mad", display_name= "matt"},
	{setting= "db", display_name= "k2"},
}
]=]

local menu_items= {
	{name= "weight", get= "GetWeightPounds", set= "SetWeightPounds",
	 item_type= "number", auto_done= 100},
	{name= "voomax", get= "GetVoomax", set= "SetVoomax", item_type= "number",
	 auto_done= 10},
	{name= "birth_year", get= "GetBirthYear", set= "SetBirthYear",
	 item_type= "number", auto_done= 1000},
	{name= "calorie_calc", get= "GetIgnoreStepCountCalories",
	 set= "SetIgnoreStepCountCalories", item_type= "bool",
	 true_text= "use_heart", false_text= "use_steps"},
	{name= "gender", get= "GetIsMale", set= "SetIsMale", item_type= "bool",
	 true_text= "male", false_text= "female"},
}
if #char_list > 0 then
	menu_items[#menu_items+1]= {
		name= "character", get= "GetCharacter", set= "SetCharacter",
		item_type= "list", list= char_list}
end
menu_items[#menu_items+1]= {name= "exit", item_type= "exit"}

local menu_cursor
local menu_pos= 1
local menu_start= SCREEN_TOP + 80
if #menu_items > 6 then
	menu_start= SCREEN_TOP + 68
end
local menu_x= SCREEN_CENTER_X * 0.25
local value_x= ( SCREEN_CENTER_X * 0.25 ) + 256
local fader
local cursor_on_menu= "main"
local menu_item_actors= {}
local menu_values= {}
local list_pos= 0
local active_list= {}
local left_showing= false
local right_showing= false

local function fade_actor_to(actor, alf)
	actor:stoptweening()
	actor:smooth(.15)
	actor:diffusealpha(alf)
end

local function update_menu_cursor()
	local item= menu_item_actors[menu_pos]
	menu_cursor:playcommand("Move", {item:GetX(), item:GetY()})
	menu_cursor:playcommand("Fit", item)
end

local function update_list_cursor()
	local valactor= menu_values[menu_pos]
	valactor:playcommand("Set", {active_list[list_pos].display_name})
	if list_pos > 1 then
		if not left_showing then
			valactor:playcommand("ShowLeft")
			left_showing= true
		end
	else
		if left_showing then
			valactor:playcommand("HideLeft")
			left_showing= false
		end
	end
	if list_pos < #active_list then
		if not right_showing then
			valactor:playcommand("ShowRight")
			right_showing= true
		end
	else
		if right_showing then
			valactor:playcommand("HideRight")
			right_showing= false
		end
	end
end

local function exit_screen()
	local profile_id= GAMESTATE:GetEditLocalProfileID()
	PROFILEMAN:SaveLocalProfile(profile_id)
	SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
	SOUND:PlayOnce(THEME:GetPathS("Common", "Start"), true)
end

local function input(event)
	local pn= event.PlayerNumber
	if not pn then return false end
	if event.type == "InputEventType_Release" then return false end
	local button= event.GameButton
	if cursor_on_menu == "main" then
		if button == "Start" then
			local item= menu_items[menu_pos]
			if item.item_type == "bool" then
				local value= not profile[item.get](profile)
				menu_values[menu_pos]:playcommand(
					"Set", {item_value_to_text(item, value)})
				profile[item.set](profile, value)
			elseif item.item_type == "number" then
				--fade_actor_to(fader, .8)
				fade_actor_to(number_entry.container, 1)
				number_entry.value= profile[item.get](profile)
				number_entry.value_actor:playcommand("Set", {number_entry.value})
				number_entry.auto_done_value= item.auto_done
				number_entry.max_value= item.max
				number_entry:update_cursor(number_entry.cursor_start)
				number_entry.prompt_actor:playcommand(
					"Set", {THEME:GetString("ScreenOptionsCustomizeProfile", item.name)})
				cursor_on_menu= "numpad"
			elseif item.item_type == "list" then
				cursor_on_menu= "list"
				active_list= menu_items[menu_pos].list
				list_pos= calc_list_pos(
					profile[menu_items[menu_pos].get](profile), active_list)
				update_list_cursor()
			elseif item.item_type == "exit" then
				exit_screen()
			end
		elseif button == "Back" then
			exit_screen()
		else
			if button == "MenuLeft" or button == "MenuUp" then
				if menu_pos > 1 then menu_pos= menu_pos - 1 end
				update_menu_cursor()
			elseif button == "MenuRight" or button == "MenuDown" then
				if menu_pos < #menu_items then menu_pos= menu_pos + 1 end
				update_menu_cursor()
			end
		end
	elseif cursor_on_menu == "numpad" then
		local done= number_entry:handle_input(button)
		if done or button == "Back" then
			local item= menu_items[menu_pos]
			if button ~= "Back" then
				profile[item.set](profile, number_entry.value)
				menu_values[menu_pos]:playcommand(
					"Set", {item_value_to_text(item, number_entry.value)})
			end
			--fade_actor_to(fader, 0)
			fade_actor_to(number_entry.container, 0)
			cursor_on_menu= "main"
		end
	elseif cursor_on_menu == "list" then
		if button == "MenuLeft" or button == "MenuUp" then
			if list_pos > 1 then list_pos= list_pos - 1 end
			update_list_cursor()
			menu_values[menu_pos]:playcommand("PressLeft")
		elseif button == "MenuRight" or button == "MenuDown" then
			if list_pos < #active_list then list_pos= list_pos + 1 end
			update_list_cursor()
			menu_values[menu_pos]:playcommand("PressRight")
		elseif button == "Start" or button == "Back" then
			if button ~= "Back" then
				profile[menu_items[menu_pos].set](
					profile, active_list[list_pos].setting)
			end
			local valactor= menu_values[menu_pos]
			left_showing= false
			right_showing= false
			valactor:playcommand("HideLeft")
			valactor:playcommand("HideRight")
			cursor_on_menu= "main"
		end
	end
end

local args= {
	Def.Actor{
		OnCommand= function(self)
			update_menu_cursor()
			SCREENMAN:GetTopScreen():AddInputCallback(input)
		end
	},
	-- 2.  The Cursor
	-- This is the cursor on the main portion of the menu.
	-- It needs to have Move and Fit commands for when it's moved to a new
	-- item on the list.
	Def.ActorFrame {
		Name= "menu_cursor", InitCommand= function(self)
			menu_cursor= self
		end,
		-- Move whole container
		MoveCommand=function(self, param)
			self:stoptweening()
			self:decelerate(0.15)
			self:xy(param[1], param[2])
			if param[3] then
				self:z(param[3])
			end
		end,
		FitCommand= function(self, param)
			self:addx(param:GetWidth()/2)
		end,
		--
		LoadActor( THEME:GetPathG("_frame", "1D"),
							 { 2/18, 14/18, 2/18 },
							 LoadActor(THEME:GetPathB("_frame", "cursors/rounded fill"))
		) .. {
			OnCommand=cmd(diffuse,PlayerDarkColor(PLAYER_1)),
			FitCommand=function(self, param)
				self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(stoptweening;decelerate,0.15)})
			end,
				 },
		LoadActor( THEME:GetPathG("_frame", "1D"),
							 { 2/18, 14/18, 2/18 },
							 LoadActor(THEME:GetPathB("_frame", "cursors/rounded gloss"))
		) .. {
			OnCommand=cmd(diffuse,PlayerColor(PLAYER_1)),
			FitCommand=function(self, param)
				self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(stoptweening;decelerate,0.15)})
			end,
				 }
	},
}

-- Note that the "character" item in the menu only shows up if there are
-- characters to choose from.  You might want to adjust positioning for that.
for i, item in ipairs(menu_items) do
	local item_y= menu_start + ((i-1) * 24)
	-- 3.  The Menu Items
	-- This creates the actor that will be used to show each item on the menu.
	args[#args+1]= Def.BitmapText{
		Name= "menu_" .. item.name, Font= "Common Normal",
		Text= THEME:GetString("ScreenOptionsCustomizeProfile", item.name),
		InitCommand= function(self)
			-- Note that the item adds itself to the list menu_item_actors.  This
			-- is so that when the cursor is moved, the appropriate item can be
			-- easily fetched for positioning and sizing the cursor.
			-- Note the ActorFrames have a width of 1 unless you set it, so when
			-- you change this from an BitmapText to a ActorFrame, you will have
			-- to make the FitCommand of your cursor look at the children.
			menu_item_actors[i]= self
			self:xy(menu_x, item_y)
			self:diffuse(Color.White)
			self:horizalign(left)
		end
	}
	if item.get then
		local value_text= item_value_to_text(item, profile[item.get](profile))
		-- 4.  The Menu Values
		-- Each of the values needs to have a SetCommand so it can be updated
		-- when the player changes it.
		-- And ActorFrame is used because values for list items need to have
		-- left/right indicators for when the player is making a choice.
		local value_args= {
			Name= "value_" .. item.name,
			InitCommand= function(self)
				-- Note that the ActorFrame is being added to the list menu_values
				-- so it can be easily fetched and updated when the value changes.
				menu_values[i]= self
				self:xy(value_x, menu_start + ((i-1) * 24))
			end,
			Def.BitmapText{
				Name= "val", Font= "Common Normal", Text= value_text,
				InitCommand= function(self)
					self:diffuse(Color.White)
					self:horizalign(left)
				end,
				SetCommand= function(self, param)
					self:settext(param[1])
				end,
			}
		}
		if item.item_type == "list" then
			-- 4.1  The L/R indicators
			-- The L/R indicators are there to tell the player when there is a
			-- choice to the left or right of the choice they are on.
			-- Note that they are placed inside the ActorFrame for the value, so
			-- when commands are played on the ActorFrame, they are played for the
			-- indicators too.
			-- The commands are ShowLeft, HideLeft, PressLeft, and the same for
			-- Right.
			-- Note that the right indicator has a SetCommand so it sees when the
			-- value changes and checks the new width to position itself.
			-- Show/Hide is only played when the indicator changes state.
			-- Command execution order: Set, Show/Hide (if change occurred), Press
			value_args[#value_args+1]= LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand= function(self)
					self:rotationy(-180)
					self:x(-8)
					self:visible(false)
					self:playcommand("Set", {value_text})
				end,
				ShowLeftCommand= cmd(visible, true),
				HideLeftCommand= cmd(visible, false),
				PressLeftCommand= cmd(finishtweening;zoom,1.5;smooth,0.25;zoom,1),
			}
			value_args[#value_args+1]= LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand= function(self)
					self:visible(false)
					self:playcommand("Set", {value_text})
				end,
				SetCommand= function(self)
					local valw= self:GetParent():GetChild("val"):GetWidth()
					self:x(valw+8)
				end,
				ShowRightCommand= cmd(visible, true),
				HideRightCommand= cmd(visible, false),
				PressRightCommand= cmd(finishtweening;zoom,1.5;smooth,0.25;zoom,1),
			}
		end
		args[#args+1]= Def.ActorFrame(value_args)
	end
end

local _height = (#menu_items) * 24
args[#args+1]= LoadActor(THEME:GetPathB("_frame", "3x3"),"rounded black",474,_height) .. {
	Name= "fader", InitCommand= function(self)
		fader= self
		self:draworder(-20)
		self:xy(menu_x + 474/2, menu_start + _height/2 - 12)
		self:diffuse(Color.Black)
		self:diffusealpha(0.75)
	end
}

args[#args+1]= number_entry:create_actors()

return Def.ActorFrame(args)
