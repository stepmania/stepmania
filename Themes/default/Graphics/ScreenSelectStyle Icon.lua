local gc = Var("GameCommand");

local string_name = gc:GetText();
local string_expl = THEME:GetString("StyleType", gc:GetStyle():GetStyleType());
local icon_color = color("#FFCB05");
local icon_color2 = color("#F0BA00");

local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame { 
	GainFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconGainFocusCommand");
	LoseFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconLoseFocusCommand");

	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_background base"))..{
		GainFocusCommand=cmd(diffuse,color("#981F41"));
		LoseFocusCommand=cmd(diffuse,color("#740A27"));
	};
	LoadFont("_overpass 36px")..{
		Text=string.upper(string_name);
		InitCommand=cmd(y,-12;maxwidth,232);
		OnCommand=cmd(diffuse,icon_color);
	};
	LoadFont("Common Italic Condensed")..{
		Text=string.upper(string_expl);
		InitCommand=cmd(y,29.5;maxwidth,128);
	};

	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_background base"))..{
		DisabledCommand=cmd(diffuse,color("0,0,0,0.5"));
		EnabledCommand=cmd(diffuse,color("1,1,1,0"));
	};
};
return t