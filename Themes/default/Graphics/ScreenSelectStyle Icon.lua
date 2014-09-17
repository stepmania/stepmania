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
		InitCommand=cmd(diffuse,icon_color;diffusebottomedge,icon_color2);
	};
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_background effect"));
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_gloss"));
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_stroke"));
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_cutout"));

	LoadFont("Common Large")..{
		Text=string.upper(string_name);
		InitCommand=cmd(y,-12;maxwidth,232);
		OnCommand=cmd(diffuse,Color.Black;shadowlength,1;shadowcolor,color("#ffffff77");skewx,-0.125);
	};
	LoadFont("Common Normal")..{
		Text=string.upper(string_expl);
		InitCommand=cmd(y,27.5;maxwidth,232);
	};

	LoadActor(THEME:GetPathG("ScreenSelectPlayMode", "icon/_background base"))..{
		DisabledCommand=cmd(diffuse,color("0,0,0,0.5"));
		EnabledCommand=cmd(diffuse,color("1,1,1,0"));
	};
};
return t