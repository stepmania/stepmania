local MetricsName, FileName = ...;
local t = LoadActor( THEME:GetPathG(Var "LoadingScreen",FileName) );
if type(t) == "table" then
	t = t .. {
		InitCommand=function(self)
				local x1 = THEME:GetMetric(Var "LoadingScreen",MetricsName .. "X");
				if x1 then self:x(x1); end
				local y1 = THEME:GetMetric(Var "LoadingScreen",MetricsName .. "Y");
				if y1 then self:y(y1); end
			end;
		OnCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OnCommand");
		OffCommand=THEME:GetMetric(Var "LoadingScreen",MetricsName .. "OffCommand");
	};
end
return t;
