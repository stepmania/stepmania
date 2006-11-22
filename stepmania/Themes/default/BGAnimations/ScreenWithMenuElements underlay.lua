if not THEME:GetMetric(Var "LoadingScreen", "StyleIcon") then
	return Def.Actor {};
end

local t = LoadActor( THEME:GetPathG(Var "LoadingScreen", "StyleIcon") )() .. {
	BeginCommand = cmd(x,SCREEN_CENTER_X+817;y,SCREEN_TOP+26);
	OnCommand = cmd(draworder,97;spring,0.5;addx,-SCREEN_WIDTH);
	OffCommand = cmd(bouncebegin,0.5;addx,SCREEN_WIDTH);
};
return t;
