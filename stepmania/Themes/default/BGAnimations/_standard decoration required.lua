local MetricsName, FileName = ...;
local t = LoadActor( THEME:GetPathG(Var "LoadingScreen",FileName) );
if type(t) == "table" then
	t = t .. {
		InitCommand=function(self) self:name(MetricsName); ActorUtil.LoadAllCommandsAndSetXYAndOnCommand(self,Var "LoadingScreen"); end;
	};
end
return t;
