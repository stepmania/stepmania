local gc = Var("GameCommand");

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
    Text=gc:GetText();
	InitCommand=THEME:GetMetric(Var "LoadingScreen","ScrollerItemInitCommand");
	GainFocusCommand=THEME:GetMetric(Var "LoadingScreen","ScrollerItemGainFocusCommand");
	LoseFocusCommand=THEME:GetMetric(Var "LoadingScreen","ScrollerItemGainFocusCommand");
};


return t;