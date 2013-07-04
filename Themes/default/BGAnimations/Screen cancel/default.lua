return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:Center();
			self:zoomto(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
			self:sleep(5 / 60);
			self:diffusealpha(1);
			self:sleep(5 / 60);
		end;
	};
	LoadActor(THEME:GetPathS("_Screen","cancel")) .. {
		StartTransitioningCommand=function(self)
			self:play();
		end;
	};
};