local MetricsName, FileName = ...;
return LoadActor( THEME:GetPathG(Var "LoadingScreen",FileName) ) .. {
	InitCommand=cmd(x,THEME:GetMetric(Var "LoadingScreen",MetricsName .. "X");y,THEME:GetMetric(Var "LoadingScreen",MetricsName .. "Y"););
	OnCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OnCommand");
	OffCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OffCommand");
};