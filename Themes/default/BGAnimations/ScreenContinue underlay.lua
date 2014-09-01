local timer_seconds = THEME:GetMetric(Var "LoadingScreen","TimerSeconds");
local t = Def.ActorFrame {};

-- Fade
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);	
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0;linear,0.5;diffusealpha,0.25;
					sleep,timer_seconds/2;  
					linear,timer_seconds/2-0.5;diffusealpha,1);
	};
	-- Warning Fade
	Def.Quad {
		InitCommand=cmd(y,16;scaletoclipped,SCREEN_WIDTH,148);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;
					  linear,timer_seconds;zoomtoheight,148*0.75);
	}
};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center;y,SCREEN_CENTER_Y-24);
	-- Underline
	Def.Quad {
		InitCommand=cmd(y,16;zoomto,256,1);
		OnCommand=cmd(diffuse,color("#ffd400");shadowlength,1;shadowcolor,BoostColor(color("#ffd40077"),0.25);linear,0.25;zoomtowidth,256;fadeleft,0.25;faderight,0.25);
	};
	LoadFont("Common Bold") .. {
		Text="Continue?";
		OnCommand=cmd(skewx,-0.125;diffuse,color("#ffd400");shadowlength,2;shadowcolor,BoostColor(color("#ffd40077"),0.25));
	};
};
--
return t
