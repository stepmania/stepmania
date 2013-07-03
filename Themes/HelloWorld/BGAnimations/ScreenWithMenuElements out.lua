return Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("#fffdf200"));
		end;
		OnCommand=function(self)
			self:decelerate(0.875);
			self:diffusealpha(1);
		end;
	};
}