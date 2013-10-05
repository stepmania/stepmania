local gc = Var("GameCommand");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame { 
	LoadFont("Common Normal") .. {
		Text=gc:GetName();
		InitCommand=cmd(shadowlength,1.5);
	};
	GainFocusCommand=cmd(visible,true);
	LoseFocusCommand=cmd(visible,false);
};
return t
