local gc = Var("GameCommand");

return Def.ActorFrame {
	LoadFont("_roboto condensed Bold 48px") .. {
		Text=THEME:GetString("ScreenTitleMenu",gc:GetText());
		OnCommand=cmd(shadowlength,1);
		GainFocusCommand=cmd(stoptweening;linear,0.1;zoom,1;diffuse,color("#A3375C"));
		LoseFocusCommand=cmd(stoptweening;linear,0.1;zoom,0.75;diffuse,color("#512232"));
	};
};