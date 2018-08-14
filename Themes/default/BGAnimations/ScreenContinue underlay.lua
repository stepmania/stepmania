local timer_seconds = THEME:GetMetric(Var "LoadingScreen","TimerSeconds");
local t = Def.ActorFrame {};

-- Fade
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center);	
	Def.Quad {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
		OnCommand=function(self)
			self:diffuse(color("#000000"):diffusealpha(0):linear(0.5):diffusealpha(0.25):
			self:sleep(timer_seconds/2):linear(timer_seconds/2-0.5):diffusealpha(1);
		end;
		OffCommand=cmd(stoptweening;sleep,0.4;decelerate,0.5;diffusealpha,1);
	};
	-- Warning Fade
	Def.Quad {
		InitCommand=cmd(y,16;scaletoclipped,SCREEN_WIDTH,250);
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.5;linear,timer_seconds;zoomtoheight,180);
		OffCommand=cmd(stoptweening);
	}
};
--
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(Center;y,SCREEN_CENTER_Y-56;zoom,1.5);
	LoadFont("Common Fallback Font") .. {
		Text="Continue?";
		OnCommand=cmd(diffuse,color("#FF8312");diffusebottomedge,color("#FFD75B");shadowlength,1;strokecolor,color("#472211"));
	};
};
--
return t