local t = LoadFallbackB();

t[#t+1] = LoadFont("Common Normal") .. {
	Name="Explanation";
	Text=THEME:GetString(Var "LoadingScreen","Explanation");
	InitCommand=function(self)
		ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen");
	end;
};

return t;