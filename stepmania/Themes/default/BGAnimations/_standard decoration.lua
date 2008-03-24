local MetricsName, FileName = ...;
return LoadActor( THEME:GetPathG(Var "LoadingScreen",FileName) ) .. {
	InitCommand=function(self)
			local x1 = THEME:GetMetric(Var "LoadingScreen",MetricsName .. "X");
			if x1 then self:x(x1); end
			local y1 = THEME:GetMetric(Var "LoadingScreen",MetricsName .. "Y");
			if y1 then self:y(y1); end
		end;
	OnCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OnCommand");
	OffCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OffCommand");
};