return Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:zoomto(THEME:GetMetric(Var "LoadingScreen","ChatOutputBoxWidth"), THEME:GetMetric(Var "LoadingScreen","ChatOutputBoxHeight"));
			self:diffuse(color("0,0,0,0.25"));
		end;
	};
};