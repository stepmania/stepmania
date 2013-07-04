return Def.ActorFrame {
	Def.Quad{
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("1,0,0,0"));
			self:blend(Blend.Multiply);
		end;
		OnCommand=function(self)
			self:smooth(1);
			self:diffuse(color("0.75,0,0,0.75"));
			self:decelerate(2);
			self:diffuse(color("0,0,0,1"));
		end;
	};
};