return Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("#fffdf2FF"));
		end;
		OnCommand=function(self)
			self:decelerate(0.75);
			self:diffusealpha(0);
		end;
	};
};