return Def.ActorFrame {
	Def.Quad{
		InitCommand=function(self)
			self:scaletocover(-SCREEN_WIDTH * 2, SCREEN_TOP, SCREEN_WIDTH * 2, SCREEN_BOTTOM);
			self:diffuse(color("0,0,0,0.5"));
		end;
	};
};