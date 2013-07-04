local gc = Var("GameCommand");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame { 
  GainFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconGainFocusCommand");
  LoseFocusCommand=THEME:GetMetric(Var "LoadingScreen","IconLoseFocusCommand");
	LoadActor(THEME:GetPathG("_SelectIcon",gc:GetName() )) .. {
		DisabledCommand=function(self)
			self:diffuse(color("0.5,0.5,0.5,1"));
		end;
		EnabledCommand=function(self)
			self:diffuse(color("1,1,1,1"));
		end;
	};
};
return t