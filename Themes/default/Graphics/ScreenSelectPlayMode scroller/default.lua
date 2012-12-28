local gc = Var "GameCommand";
local colors = {
	Easy		= color("#00ff00"),
	Normal		= color("#feee00"),
	Hard		= color("#feee00"),
	Rave		= color("#db93ff"),
	Nonstop		= color("#00ffff"),
	Oni			= color("#d70b8c"),
	Endless		= color("#b4c3d2"),
};
local t = Def.ActorFrame {};
-- Background!
t[#t+1] = Def.ActorFrame {
-- 	GainFocusCommand=cmd(visible,true);
-- 	LoseFocusCommand=cmd(visible,false);
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode","BackgroundFrame")) .. {
		InitCommand=cmd(diffuse,Color("Black");diffusealpha,0.7);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
	};
 	LoadActor("_HighlightFrame") .. {
		InitCommand=cmd(diffuse,ModeIconColors[gc:GetName()];diffusealpha,0);
		GainFocusCommand=cmd(finishtweening;diffusealpha,1;glow,Color.Alpha(Color.White,1);linear,0.25;glow,Color.Invisible);
		LoseFocusCommand=cmd(finishtweening;diffusealpha,0;glow,Color.Invisible);
		OffFocusedCommand=cmd(finishtweening;glow,Color("White");decelerate,1;glow,Color("Invisible"));
	};
};
-- Emblem Frame
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=cmd(x,-192;zoom,0.9);
	-- Main Shadow
	LoadActor( gc:GetName() ) .. {
		InitCommand=cmd(x,2;y,2;diffuse,Color("Black");diffusealpha,0;zoom,0.75);
		GainFocusCommand=cmd(stoptweening;stopeffect;smooth,0.125;diffusealpha,0;zoom,1;decelerate,0.25;diffusealpha,0.5;pulse;effecttiming,0.75,0.125,0.125,0.75;effectmagnitude,0.95,1,1;);
		LoseFocusCommand=cmd(stoptweening;stopeffect;smooth,0.25;diffusealpha,0;zoom,0.75;);
		OffFocusedCommand=cmd(finishtweening;stopeffect;glow,ModeIconColors[gc:GetName()];decelerate,1;rotationy,360*2;);
	};
	-- Main Emblem
	LoadActor( gc:GetName() ) .. {
		InitCommand=cmd(diffusealpha,0;zoom,0.75);
		GainFocusCommand=cmd(stoptweening;stopeffect;smooth,0.125;diffusealpha,1;zoom,1;glow,Color("White");decelerate,0.25;glow,Color("Invisible");pulse;effecttiming,0.75,0.125,0.125,0.75;effectmagnitude,0.95,1,1;);
		LoseFocusCommand=cmd(stoptweening;stopeffect;smooth,0.25;diffusealpha,0;zoom,0.75;glow,Color("Invisible"));
		OffFocusedCommand=cmd(finishtweening;stopeffect;glow,ModeIconColors[gc:GetName()];decelerate,1;rotationy,360*2;glow,Color("Invisible"));
	};
};
-- Text Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,-192/2;y,-10);
	Def.Quad {
		InitCommand=cmd(horizalign,left;y,20;zoomto,320,2;diffuse,ModeIconColors[gc:GetName()];diffusealpha,0;fadeleft,0.35;faderight,0.35);
		GainFocusCommand=cmd(stoptweening;linear,0.2;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;linear,0.2;diffusealpha,0);
	};
	LoadFont("_helveticaneuelt std extblk cn 42px") .. {
		Text=gc:GetText();
		InitCommand=cmd(horizalign,left;diffuse,ModeIconColors[gc:GetName()];shadowcolor,ColorDarkTone(ModeIconColors[gc:GetName()]);shadowlength,2;diffusealpha,0;skewx,-0.125);
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
t.GainFocusCommand=cmd(finishtweening;visible,true;zoom,1.1;bounceend,0.25;zoom,1);
t.LoseFocusCommand=cmd(finishtweening;visible,false;zoom,1);
return t