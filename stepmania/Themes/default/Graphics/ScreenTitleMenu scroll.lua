local t = Def.ActorFrame {
	LoadFont("", "blaster") ..{
		Text=THEME:GetString( 'ScreenTitleMenu', ThisGameCommand:GetText() );
		InitCommand=cmd(horizalign,center;shadowlength,0);
		GainFocusCommand=cmd(stoptweening;diffuseshift;effectperiod,0.5;effectcolor1,0.5,1,0.5,1;effectcolor2,0.25,0.5,0.25,1;);
		LoseFocusCommand=cmd(stoptweening;stopeffect);
		DisabledCommand=cmd(diffuse,0.5,0.5,0.5,1);
	};
};

return t;
