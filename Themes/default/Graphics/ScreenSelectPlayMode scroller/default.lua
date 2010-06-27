local gc = Var "GameCommand";
local colors = {
	Easy		= color("#00ff00"),
	Normal		= color("#feee00"),
	Hard		= color("#feee00"),
	Extended	= color("#00ffff"),
	Oni			= color("#d70b8c"),
	Rave		= color("#c44dff"),
};
local t = Def.ActorFrame {};
-- Background!
t[#t+1] = Def.ActorFrame {
-- 	GainFocusCommand=cmd(visible,true);
-- 	LoseFocusCommand=cmd(visible,false);
 	LoadActor("_HighlightFrame") .. {
		InitCommand=cmd(diffuse,colors[gc:GetName()];diffusealpha,0);
		GainFocusCommand=cmd(stoptweening;linear,0.125;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;linear,0.125;diffusealpha,0);
		OffFocusedCommand=cmd(finishtweening;glow,Color("White");decelerate,1.5;glow,Color("Invisible"));
	};
};
-- Emblem Frame
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=cmd(x,-192;zoom,0.9);
	LoadActor( gc:GetName() ) .. {
		InitCommand=cmd(diffusealpha,0;zoom,0.75);
		GainFocusCommand=cmd(stoptweening;stopeffect;smooth,0.125;diffusealpha,1;zoom,1;glow,Color("White");decelerate,0.25;glow,Color("Invisible");pulse;effecttiming,0.75,0.125,0.125,0.75;effectmagnitude,0.95,1,1;);
		LoseFocusCommand=cmd(stoptweening;stopeffect;smooth,0.25;diffusealpha,0;zoom,0.75;glow,Color("Invisible"));
		OffFocusedCommand=cmd(finishtweening;stopeffect;glow,colors[gc:GetName()];decelerate,1.75;rotationy,360*1;glow,Color("Invisible"));
	};
};
-- Text Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,-192/2;y,-10);
	Def.Quad {
		InitCommand=cmd(horizalign,left;y,20;zoomto,320,2;diffuse,colors[gc:GetName()];diffusealpha,0;fadeleft,0.35;faderight,0.35);
		GainFocusCommand=cmd(stoptweening;linear,0.2;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;linear,0.2;diffusealpha,0);
	};
	LoadFont("_helveticaneuelt std extblk cn 42px") .. {
		Text=gc:GetName();
		InitCommand=cmd(horizalign,left;diffuse,colors[gc:GetName()];shadowcolor,ColorDarkTone(colors[gc:GetName()]);shadowlength,2;diffusealpha,0;skewx,-0.125);
		GainFocusCommand=cmd(stoptweening;x,-16;decelerate,0.25;diffusealpha,1;x,0);
		LoseFocusCommand=cmd(stoptweening;x,0;accelerate,0.25;diffusealpha,0;x,16;diffusealpha,0);
	};
	LoadFont("_helveticaneuelt std extblk cn 42px") .. {
		Text=THEME:GetString(Var "LoadingScreen", gc:GetName() .. "Explanation");
		InitCommand=cmd(horizalign,right;x,320;y,30;shadowlength,1;diffusealpha,0;skewx,-0.125;zoom,0.5);
		GainFocusCommand=cmd(stoptweening;x,320-16;decelerate,0.25;diffusealpha,1;x,320);
		LoseFocusCommand=cmd(stoptweening;x,320;accelerate,0.25;diffusealpha,0;x,320+16;diffusealpha,0);
	};
};
-- t.GainFocusCommand=cmd(visible,true);
-- t.LoseFocusCommand=cmd(visible,false);
return t