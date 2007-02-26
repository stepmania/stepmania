Trace( "huh " .. PREFSMAN:GetPreference( "ShowSongOptions" ) );
if PREFSMAN:GetPreference( "ShowSongOptions" ) ~= "Maybe_Ask" then
	return LoadActor( THEME:GetPathB("", "_options to options") );
end

local t = Def.ActorFrame {
	children = {
		LoadActor( THEME:GetPathB("", "_fade out with sound") );

		LoadActor( THEME:GetPathG(Var "LoadingScreen", "ask options") ) .. {
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+100;visible,false);
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
		LoadActor( THEME:GetPathG(Var "LoadingScreen", "entering options") ) .. {
			InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+100;visible,false);
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
};

return t;

