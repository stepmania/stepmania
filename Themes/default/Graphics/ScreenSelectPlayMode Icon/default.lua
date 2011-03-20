local gc = Var("GameCommand");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame { 
  GainFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconGainFocusCommand");
  LoseFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconLoseFocusCommand");
--	IconGainFocusCommand=cmd(stoptweening;glowshift;decelerate,0.125;zoom,1);
--	IconLoseFocusCommand=cmd(stoptweening;stopeffect;decelerate,0.125;zoom,fZoom);

	LoadActor("_background base")..{
		InitCommand=cmd(diffuse,ModeIconColors[gc:GetName()]);
	};
	LoadActor("_background effect");
	LoadActor("_gloss");
	LoadActor("_stroke");
	LoadActor("_cutout");

	-- todo: generate a better font for these.
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=cmd(y,-12;zoom,1.1;diffuse,color("#000000");uppercase,true;settext,gc:GetName(););
	};
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=cmd(y,27.5;zoom,0.45;maxwidth,320*1.6;uppercase,true;settext,THEME:GetString(Var "LoadingScreen", gc:GetName().."Explanation"));
	};
	LoadActor("_background base") .. {
		DisabledCommand=cmd(diffuse,color("0,0,0,0.5"));
		EnabledCommand=cmd(diffuse,color("1,1,1,0"));
	};
	--[[
	LoadActor(THEME:GetPathG("_SelectIcon",gc:GetName() )) .. {
		DisabledCommand=cmd(diffuse,color("0.5,0.5,0.5,1"));
		EnabledCommand=cmd(diffuse,color("1,1,1,1"));
	};
	--]]
};
return t