local gc = Var("GameCommand");

local string_name = gc:GetText()
local string_expl = THEME:GetString(Var "LoadingScreen", gc:GetName().."Explanation")
local icon_color = ModeIconColors[gc:GetName()];

local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	GainFocusCommand=cmd(stoptweening;bob;effectmagnitude,0,6,0;decelerate,0.05;zoom,1);
	LoseFocusCommand=cmd(stoptweening;stopeffect;decelerate,0.1;zoom,0.6);

	LoadActor("_background base")..{
		InitCommand=cmd(diffuse,icon_color);
	};
	LoadActor("_background effect");
	LoadActor("_gloss");
	LoadActor("_stroke");
	LoadActor("_cutout");

	-- todo: generate a better font for these.
	LoadFont("Common Large")..{
		Text=string.upper(string_name);
		InitCommand=cmd(y,-12;maxwidth,232);
		OnCommand=cmd(diffuse,Color.Black;shadowlength,1;shadowcolor,color("#ffffff77");skewx,-0.125);
	};
	LoadFont("Common Normal")..{
		Text=string.upper(string_expl);
		InitCommand=cmd(y,27.5;maxwidth,232);
	};
	LoadActor("_background base") .. {
		DisabledCommand=cmd(diffuse,color("0,0,0,0.5"));
		EnabledCommand=cmd(diffuse,color("1,1,1,0"));
	};
};
return t