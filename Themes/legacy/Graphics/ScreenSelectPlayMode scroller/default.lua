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

local icon_color = ModeIconColors[gc:GetName()];

local string_name = gc:GetText();
local string_expl = THEME:GetString(Var "LoadingScreen", gc:GetName() .. "Explanation");
local t = Def.ActorFrame {};

-- Background!
t[#t+1] = Def.ActorFrame {
	LoadActor(THEME:GetPathG("ScreenSelectPlayMode","BackgroundFrame")) .. {
		InitCommand=cmd(diffuse,Color("Black");diffusealpha,0.45);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
	};
 	LoadActor("_HighlightFrame") .. {
		InitCommand=cmd(diffuse,icon_color;diffusealpha,0);
		GainFocusCommand=cmd(finishtweening;diffuse,ColorLightTone(icon_color);linear,1;diffuse,icon_color);
		LoseFocusCommand=cmd(finishtweening;diffusealpha,0);
		OffFocusedCommand=cmd(finishtweening;glow,Color("White");decelerate,1;glow,Color("Invisible"));
	};
};

-- Emblem Frame
t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=cmd(x,-192;zoom,0.9);
	-- Main Emblem
	LoadActor( gc:GetName() ) .. {
		InitCommand=cmd(diffusealpha,0;zoom,0.75);
		GainFocusCommand=cmd(finishtweening;stopeffect;diffusealpha,1;zoom,1;glow,Color("White");decelerate,0.5;glow,Color("Invisible");pulse;effecttiming,0.75,0.125,0.125,0.75;effectmagnitude,0.95,1,1;);
		LoseFocusCommand=cmd(finishtweening;stopeffect;smooth,0.4;diffusealpha,0;zoom,0.75;glow,Color("Invisible"));
		OffFocusedCommand=cmd(finishtweening;stopeffect;glow,icon_color;decelerate,0.5;rotationy,360;glow,Color("Invisible"));
	};
};

-- Text Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,-192/2;y,-10);
	LoadFont("Common Large") .. {
		Text=string_name;
		InitCommand=cmd(y,-2;horizalign,left;diffuse,icon_color;strokecolor,ColorDarkTone(icon_color);shadowlength,2;diffusealpha,0;skewx,-0.125);
		OnCommand=cmd(glowshift;textglowmode,'TextGlowMode_Inner';
				effectcolor1,color("1,1,1,0.5");effectcolor2,color("1,1,1,0")
		);
		GainFocusCommand=cmd(finishtweening;x,-16;diffuse,ColorLightTone(icon_color);decelerate,0.45;diffusealpha,1;x,0;diffuse,icon_color);
		LoseFocusCommand=cmd(finishtweening;x,0;accelerate,0.4;diffusealpha,0;x,32;diffusealpha,0);
	};
	LoadFont("Common Normal") .. {
		Text=string_expl;
		InitCommand=cmd(horizalign,right;x,320;y,30;shadowlength,1;diffusealpha,0;skewx,-0.125;zoom,0.75);
		GainFocusCommand=cmd(finishtweening;x,320+16;decelerate,0.45;diffusealpha,1;x,320);
		LoseFocusCommand=cmd(finishtweening;x,320;accelerate,0.4;diffusealpha,0;x,320-32;diffusealpha,0);
	};
	Def.Quad {
		InitCommand=cmd(horizalign,left;y,20;zoomto,320,2;diffuse,icon_color;diffusealpha,0;fadeleft,0.35;faderight,0.35);
		GainFocusCommand=cmd(stoptweening;linear,0.5;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;linear,0.1;diffusealpha,0);
	};
};

t.GainFocusCommand=cmd(finishtweening;visible,true;zoom,1.1;decelerate,0.25;zoom,1);
t.LoseFocusCommand=cmd(finishtweening;visible,false;zoom,1);

return t