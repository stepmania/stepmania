if PREFSMAN:GetPreference( "ShowSongOptions" ) ~= "Maybe_Ask" then
	return LoadActor( THEME:GetPathB("Screen", "out") );
end

local t = Def.ActorFrame {
	LoadActor( THEME:GetPathB("Screen", "out") );

	LoadFont( "common normal" ) .. {
		InitCommand=cmd(settext,"Press &START; for more options";x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+100;visible,false);
		AskForGoToOptionsCommand=cmd(
			visible,true;
			diffusealpha,0;
			linear,0.15;
			zoomy,1;
			diffusealpha,1;
			sleep,1;
			linear,0.15;
			diffusealpha,0;
			zoomy,0;
		);
		GoToOptionsCommand=cmd(visible,false);
	};
	LoadFont( "common normal" ) .. {
		InitCommand=cmd(settext,"entering options...";x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+100;visible,false);
		AskForGoToOptionsCommand=cmd(
			visible,false;
			linear,0.15;
			zoomy,1;
			diffusealpha,1;
			sleep,1;
			linear,0.15;
			diffusealpha,0;
			zoomy,0;
		);
		GoToOptionsCommand=cmd(visible,true);
	};
};

return t;