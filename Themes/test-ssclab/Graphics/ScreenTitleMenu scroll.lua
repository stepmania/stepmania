local gc = Var("GameCommand");

return Def.ActorFrame { 
	LoadFont("Common Normal") .. {
		Text=gc:GetText();
		OnCommand=cmd(strokecolor,color("0,0,0,0.5"));
		GainFocusCommand=cmd(zoom,1;diffuse,color("1,1,1,1"));
		LoseFocusCommand=cmd(zoom,0.75;diffuse,color("0.5,0.5,0.5,1"));
	};
};