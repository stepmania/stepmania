return Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("0,0,0,0"));
		end;
		OnCommand=function(self)
			self:accelerate(0.5);
			self:diffusealpha(1);
		end;
	};
};