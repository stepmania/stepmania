-- Pester Kyzentun for an explanation if you need to customize this screen.
-- Also, this might be rewritten to us a proper customizable lua menu system
-- in the future.

local profile= GAMESTATE:GetEditLocalProfile()

local cursor_width_padding = 16
local cursor_spacing_value = 30

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
local menu_start= SCREEN_TOP + 80
local menu_x= SCREEN_CENTER_X * 0.25
local value_x= ( SCREEN_CENTER_X * 0.25 ) + 256
local fader
local cursor_on_menu= true
local menu_item_actors= {}
local menu_values= {}

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
				--fade_actor_to(fader, .8)
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
			--fade_actor_to(fader, 0)
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
