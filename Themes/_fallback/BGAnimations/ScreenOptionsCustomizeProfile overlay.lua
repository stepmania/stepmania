-- Pester Kyzentun for an explanation if you need to customize this screen.
-- Also, this might be rewritten to us a proper customizable lua menu system
-- in the future.

local profile= GAMESTATE:GetEditLocalProfile()

local number_entry= new_numpad_entry{
	Name= "number_entry",
	InitCommand= cmd(diffusealpha, 0; xy, _screen.cx*1.5, _screen.cy),
	value_color= PlayerColor(PLAYER_1),
	cursor_draw= "first",
	cursor_color= PlayerDarkColor(PLAYER_1),
}

local function item_value_to_text(item, value)
	if item.item_type == "bool" then
		if value then
			value= THEME:GetString("ScreenOptionsCustomizeProfile", item.true_text)
		else
			value= THEME:GetString("ScreenOptionsCustomizeProfile", item.false_text)
		end
	end
	return value
end

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
	{name= "exit", item_type= "exit"}
}

local menu_cursor
local menu_pos= 1
local menu_start= 72
local menu_x= 32
local value_x= 300
local fader
local cursor_on_menu= true
local menu_item_actors= {}
local menu_values= {}

local function fade_actor_to(actor, alf)
	actor:stoptweening()
	actor:linear(.2)
	actor:diffusealpha(alf)
end

local function update_menu_cursor()
	menu_cursor:stoptweening()
	menu_cursor:linear(.1)
	menu_cursor:y(menu_item_actors[menu_pos]:GetY())
	menu_cursor:SetWidth(menu_item_actors[menu_pos]:GetWidth() + 20)
end

local function input(event)
	local pn= event.PlayerNumber
	if not pn then return false end
	if event.type == "InputEventType_Release" then return false end
	local button= event.GameButton
	if cursor_on_menu then
		if button == "Start" then
			local item= menu_items[menu_pos]
			if item.item_type == "bool" then
				local value= not Profile[item.get](profile)
				menu_values[menu_pos]:settext(item_value_to_text(item, value))
				Profile[item.set](profile, value)
			elseif item.item_type == "number" then
				fade_actor_to(fader, .8)
				fade_actor_to(number_entry.container, 1)
				number_entry.value= Profile[item.get](profile)
				number_entry.value_actor:playcommand("Set", {number_entry.value})
				number_entry.auto_done_value= item.auto_done
				number_entry.max_value= item.max
				number_entry:update_cursor(number_entry.cursor_start)
				number_entry.prompt_actor:playcommand(
					"Set", {THEME:GetString("ScreenOptionsCustomizeProfile", item.name)})
				cursor_on_menu= false
			elseif item.item_type == "exit" then
				SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
				SOUND:PlayOnce(THEME:GetPathS("Common", "Start"))
			end
		else
			if button == "MenuLeft" or button == "MenuUp" then
				if menu_pos > 1 then menu_pos= menu_pos - 1 end
				update_menu_cursor()
			elseif button == "MenuRight" or button == "MenuDown" then
				if menu_pos < #menu_items then menu_pos= menu_pos + 1 end
				update_menu_cursor()
			end
		end
	else
		local done= number_entry:handle_input(button)
		if done then
			local item= menu_items[menu_pos]
			Profile[item.set](profile, number_entry.value)
			menu_values[menu_pos]:settext(item_value_to_text(item, number_entry.value))
			fade_actor_to(fader, 0)
			fade_actor_to(number_entry.container, 0)
			cursor_on_menu= true
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
	Def.Quad{
		Name= "menu_cursor", InitCommand= function(self)
			menu_cursor= self
			self:horizalign(left)
			self:setsize(0, 24)
			self:diffuse(PlayerColor(PLAYER_1))
			self:xy(menu_x - 10, menu_start)
		end,
	},
}

for i, item in ipairs(menu_items) do
	local item_y= menu_start + ((i-1) * 24)
	args[#args+1]= Def.BitmapText{
		Name= "menu_" .. item.name, Font= "Common Normal",
		Text= THEME:GetString("ScreenOptionsCustomizeProfile", item.name),
		InitCommand= function(self)
			menu_item_actors[i]= self
			self:xy(menu_x, item_y)
			self:diffuse(Color.White)
			self:horizalign(left)
		end
	}
	if item.get then
		local value_text= item_value_to_text(item, Profile[item.get](profile))
		args[#args+1]= Def.BitmapText{
			Name= "value_" .. item.name, Font= "Common Normal",
			Text= value_text,
			InitCommand= function(self)
				menu_values[i]= self
				self:xy(value_x, menu_start + ((i-1) * 24))
				self:diffuse(Color.White)
				self:horizalign(left)
			end
		}
	end
end

args[#args+1]= Def.Quad{
	Name= "fader", InitCommand= function(self)
		fader= self
		self:setsize(270, #menu_items * 24)
		self:horizalign(left)
		self:vertalign(top)
		self:xy(menu_x-10, menu_start-12)
		self:diffuse(Color.Black)
		self:diffusealpha(0)
	end
}

args[#args+1]= number_entry:create_actors()

return Def.ActorFrame(args)
