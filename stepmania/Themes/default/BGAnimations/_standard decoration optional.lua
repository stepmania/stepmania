local MetricsName, FileName = ...;
if THEME:GetMetric(Var "LoadingScreen","Show"..MetricsName) then
	return LoadActor( THEME:GetPathB("","_standard decoration required"), MetricsName, FileName );
else
	return Def.Actor {};
end
