return Def.ActorFrame {
	GainFocusCommand=cmd();
	LoseFocusCommand=cmd();
	LoadActor( PROFILEMAN:GetLocalProfile( Var("GameCommand"):GetProfileID() ):GetCharacter():GetIconPath() ) .. {
		InitCommand=cmd(x,20);
	};
	LoadFont( "Common normal" ) .. {
		Text=PROFILEMAN:GetLocalProfile( Var("GameCommand"):GetProfileID() ):GetDisplayName();
		OnCommand=cmd(diffusealpha,0;linear,0.3;diffusealpha,1);
		OffCommand=cmd(linear,0.3;diffusealpha,0);
		InitCommand=cmd(halign,0;shadowlength,2;x,50);
		GainFocusCommand=cmd(stoptweening;diffuseshift;effectperiod,0.5;effectcolor1,1,0.5,0.5,1;effectcolor2,0.5,0.25,0.25,1;linear,0;zoom,1.0);
		LoseFocusCommand="stoptweening;stopeffect;linear,0;zoom,0.9";
		EnabledCommand="";
		DisabledCommand="diffuse,0.5,0.5,0.5,1";
	};
	LoadFont( "Common normal" ) .. {
		Text=PROFILEMAN:GetLocalProfile( Var("GameCommand"):GetProfileID() ):GetNumTotalSongsPlayed() .. " song(s) played";
		OnCommand=cmd(diffusealpha,0;linear,0.3;diffusealpha,1);
		OffCommand=cmd(linear,0.3;diffusealpha,0);
		InitCommand=cmd(halign,0;shadowlength,2;x,280);
		GainFocusCommand=cmd(stoptweening;diffuseshift;effectperiod,0.5;effectcolor1,1,0.5,0.5,1;effectcolor2,0.5,0.25,0.25,1;linear,0;zoom,1.0);
		LoseFocusCommand=cmd(stoptweening;stopeffect;linear,0;zoom,0.9);
		EnabledCommand=cmd();
		DisabledCommand=cmd(diffuse,0.5,0.5,0.5,1);
	};
};
