return Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("0,0,0,1"));
			self:cropbottom(1);
			self:fadebottom(1);
		end;
		OnCommand=function(self)
			self:decelerate(0.5);
			self:cropbottom(0);
			self:fadebottom(0);
		end;
	};
};