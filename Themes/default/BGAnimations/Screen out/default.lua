local fSleepTime = THEME:GetMetric( Var "LoadingScreen","ScreenOutDelay");

return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:Center();
			self:zoomto(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0"));
			self:sleep(fSleepTime);
			self:linear(0.01);
			self:diffusealpha(1);
		end;
	};
};