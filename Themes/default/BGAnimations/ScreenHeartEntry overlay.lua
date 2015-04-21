local cursor_width_padding = 16
local cursor_spacing_value = 30

local heart_xs= {
	[PLAYER_1]= SCREEN_CENTER_X * 0.625,
	[PLAYER_2]= SCREEN_CENTER_X * 1.375,
}

local heart_entries= {}
for i, pn in ipairs(GAMESTATE:GetEnabledPlayers()) do
	local profile= PROFILEMAN:GetProfile(pn)
	if profile and profile:GetIgnoreStepCountCalories() then
		heart_entries[pn]= new_numpad_entry{
			Name= pn .. "_heart_entry",
			InitCommand= cmd(xy, heart_xs[pn], SCREEN_CENTER_Y+48),
			value = LoadFont("Common Large") .. {
				InitCommand=cmd(xy,0,-62),
				OnCommand=cmd(zoom,0.75;diffuse,PlayerColor(pn);strokecolor,ColorDarkTone(PlayerColor(pn)));
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
					OnCommand=cmd(diffuse,PlayerDarkColor(pn)),
					FitCommand=function(self, param)
						self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(stoptweening;decelerate,0.15)})
					end,
				},
				LoadActor( THEME:GetPathG("_frame", "1D"),
					{ 2/18, 14/18, 2/18 },
					LoadActor(THEME:GetPathB("_frame", "cursors/rounded gloss"))
				) .. {
					OnCommand=cmd(diffuse,PlayerColor(pn)),
					FitCommand=function(self, param)
						self:playcommand("SetSize",{ Width=param:GetWidth()+cursor_width_padding, tween=cmd(stoptweening;decelerate,0.15)})
					end,
				}
			},
			cursor_draw= "first",
			prompt = LoadFont("Common Bold") .. {
				Name="prompt",
				Text=THEME:GetString("ScreenHeartEntry", "Heart Rate"),
				InitCommand=cmd(xy,0,-96);
				OnCommand=cmd(shadowlength,1;skewx,-0.125;diffusebottomedge,color("#DDDDDD");strokecolor,Color.Outline);
			},
			max_value= 300,
			auto_done_value= 100,
		}
	end
end

local function input(event)
	local pn= event.PlayerNumber
	if not pn then return end
	if event.type == "InputEventType_Release" then return end
	if not heart_entries[pn] then return end
	local done= heart_entries[pn]:handle_input(event.GameButton)
	if done then
		SOUND:PlayOnce(THEME:GetPathS("Common", "Start"), true)
		local all_done= true
		for pn, entry in pairs(heart_entries) do
			if not entry.done then all_done= false break end
		end
		if all_done then
			for pn, entry in pairs(heart_entries) do
				local profile= PROFILEMAN:GetProfile(pn)
				if profile and profile:GetIgnoreStepCountCalories() then
					local calories= profile:CalculateCaloriesFromHeartRate(
						entry.value, GAMESTATE:GetLastGameplayDuration())
					profile:AddCaloriesToDailyTotal(calories)
				end
			end
			SCREENMAN:GetTopScreen():StartTransitioningScreen("SM_GoToNextScreen")
		end
	end
end

local timer_text
local function timer_update(self)
	local time= math.floor((self:GetSecsIntoEffect() % 60) * 10) / 10
	if time < 10 then
		timer_text:settext(("0%.1f"):format(time))
	else
		timer_text:settext(("%.1f"):format(time))
	end
end

local args= {
	--
	Def.ActorFrame{
		Name= "timer",
		InitCommand= function(self)
			self:effectperiod(2^16)
			timer_text= self:GetChild("timer_text")
			self:SetUpdateFunction(timer_update)
		end,
		OnCommand= function(self)
			SCREENMAN:GetTopScreen():AddInputCallback(input)
		end,
		Def.BitmapText{
			Name= "timer_text", Font= "Common Normal", Text= "00.0",
			InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-80; diffuse, Color.White),
			OnCommand= cmd(strokecolor,Color.Outline),
		}
	},
	Def.Quad {
		InitCommand=cmd(xy, SCREEN_CENTER_X+1, SCREEN_CENTER_Y-100+1;zoomto,2,2);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;linear,0.25;zoomtowidth,420;fadeleft,0.25;faderight,0.25);
	};
	Def.Quad {
		InitCommand=cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-100;zoomto,2,2);
		OnCommand=cmd(diffuse,color("#ffd400");shadowcolor,BoostColor(color("#ffd40077"),0.25);linear,0.25;zoomtowidth,420;fadeleft,0.25;faderight,0.25);
	};
	Def.BitmapText {
		Name= "explanation", Font= "Common Large",
		Text= string.upper(THEME:GetString("ScreenHeartEntry", "Enter Heart Rate")),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y-128; diffuse, Color.White),
		OnCommand=cmd(skewx,-0.125;diffuse,color("#ffd400");strokecolor,ColorDarkTone(color("#ffd400")))}
	,

	Def.BitmapText{
		Name= "song_len_label", Font= "Common Normal",
		Text= THEME:GetString("ScreenHeartEntry", "Song Length"),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y+192-32; diffuse, Color.White),
		OnCommand= cmd(shadowlength,1)},
	Def.BitmapText{
		Name= "song_len", Font= "Common Normal",
		Text= SecondsToMMSS(GAMESTATE:GetLastGameplayDuration()),
		InitCommand= cmd(xy, SCREEN_CENTER_X, SCREEN_CENTER_Y+192-8; diffuse, Color.White),
		OnCommand= cmd(shadowlength,1;zoom,0.75),
	}

}

for pn in ivalues(GAMESTATE:GetEnabledPlayers()) do
	args[#args+1] = LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black", 128, 192) .. {
		InitCommand=cmd(x,heart_xs[pn];y,SCREEN_CENTER_Y+28),
	}
end

for pn, entry in pairs(heart_entries) do
	args[#args+1]= entry:create_actors()
end

return Def.ActorFrame(args)
