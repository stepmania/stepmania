if not THEME:GetMetric(Var "LoadingScreen", "StyleIcon") then
	return Def.Actor {};
end

local t = LoadActor( THEME:GetPathG(Var "LoadingScreen", "StyleIcon") ) .. {
	BeginCommand = cmd(x,SCREEN_CENTER_X+8;y,SCREEN_TOP-11);
	OnCommand = cmd(draworder,200;linear,0.25;addy,33);
	OffCommand = cmd(linear,0.25;addy,-44);
};
return t;
