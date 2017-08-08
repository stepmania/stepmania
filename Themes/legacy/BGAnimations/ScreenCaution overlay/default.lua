local t = Def.ActorFrame {};

-- Fade
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);	
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0;linear,0.5;diffusealpha,0.75);
		OffCommand=cmd(linear,0.25;diffusealpha,0);
	};
};
-- Emblem
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);
	OnCommand=cmd(diffusealpha,0.5);
	LoadActor("_warning bg") .. {
		OnCommand=cmd(diffuse,Color.Yellow);
	};
	Def.ActorFrame {
		LoadActor("_exclamation") .. {
			OnCommand=cmd(diffuse,Color.Yellow);
		};
	};
};

-- Text
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center;y,SCREEN_CENTER_Y);
	OnCommand=cmd(addy,-96);
	-- Underline
	Def.Quad {
		InitCommand=cmd(y,24;zoomto,256,2);
		OnCommand=cmd(diffuse,color("#ffd400");shadowcolor,BoostColor(color("#ffd40077"),0.25);linear,0.25;zoomtowidth,256;fadeleft,0.25;faderight,0.25);
	};
	LoadFont("Common Large") .. {
		Text=Screen.String("Caution");
		OnCommand=cmd(skewx,-0.125;diffuse,color("#ffd400");strokecolor,ColorDarkTone(color("#ffd400")));
	};
	LoadFont("Common Normal") .. {
		Text=Screen.String("CautionText");
		InitCommand=cmd(y,128);
		OnCommand=cmd(strokecolor,color("0,0,0,0.5");shadowlength,1;wrapwidthpixels,SCREEN_WIDTH-80);
	};
};
--
return t
