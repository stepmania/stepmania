local gc = Var("GameCommand");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	GainFocusCommand=cmd(stoptweening;bob;effectmagnitude,0,6,0;decelerate,0.05;zoom,1);
	LoseFocusCommand=cmd(stoptweening;stopeffect;decelerate,0.1;zoom,0.6);

	LoadActor("_background base")..{
		InitCommand=cmd(diffuse,ModeIconColors[gc:GetName()]);
	};
	LoadActor("_background effect");
	LoadActor("_gloss");
	LoadActor("_stroke");
	LoadActor("_cutout");

	-- todo: generate a better font for these.
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=cmd(y,-12;zoom,1.1;diffuse,color("#000000");uppercase,true;settext,gc:GetText(););
		GainFocusCommand=cmd(diffuse,Color.Black;stopeffect);
		LoseFocusCommand=cmd(diffuse,Color.Black;stopeffect);
	};
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=cmd(y,27.5;zoom,0.45;maxwidth,320*1.6;uppercase,true;settext,THEME:GetString(Var "LoadingScreen", gc:GetName().."Explanation"));
		GainFocusCommand=cmd(diffuse,Color.White;stopeffect);
		LoseFocusCommand=cmd(diffuse,Color.White;stopeffect);
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