local t = LoadFallbackB();

t[#t+1] = LoadActor( "settings pane" ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X-200;y,SCREEN_TOP;vertalign,top;);
};
t[#t+1] = LoadFont("Common", "normal") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X-202;y,SCREEN_TOP+40;shadowlengthx,0;shadowlengthy,2;playcommand,"Set");
	SetCommand=function(self)
		local s = "";

		local fmt = THEME:GetString( Var "LoadingScreen", "%d songs in %d groups" );
		local text = string.format( fmt, SONGMAN:GetNumSongs(), SONGMAN:GetNumSongGroups() )

		s = s .. text;

		local fmt = THEME:GetString( Var "LoadingScreen", "%d courses in %d groups" );
		local text = string.format( fmt,  SONGMAN:GetNumCourses(), SONGMAN:GetNumCourseGroups() )

		s = s .. "\n" .. text;

		if PREFSMAN:GetPreference("UseUnlockSystem") then
			local fmt = THEME:GetString( Var "LoadingScreen", "%d unlocks" );
			local text = string.format( fmt, UNLOCKMAN:GetNumUnlocks() )
			s = s .. "\n" .. text;
		end

		local fmt = THEME:GetString( Var "LoadingScreen", "Gametype:" );
		local text = fmt .. " " .. GAMESTATE:GetCurrentGame():GetName();
		s = s .. "\n" .. text;
		
		local fmt = THEME:GetString( Var "LoadingScreen", "Difficulty:" );
		local text = fmt .. " " .. GetTimingDifficulty();
		s = s .. "\n" .. text;

		self:settext( s )
	end;
};
t[#t+1] = LoadFont("Common", "normal") .. {
	InitCommand=cmd(x,SCREEN_LEFT+20;y,SCREEN_TOP+36;horizalign,left;diffuse,0.6,0.6,0.6,1;shadowlength,2);
};
t[#t+1] = Def.ActorFrame {
       InitCommand=cmd(x,SCREEN_CENTER_X+210;y,SCREEN_CENTER_Y+200;);
       LoadActor( "stepmania logo" ) .. {
       };
       LoadFont( "common normal" ) .. {
               InitCommand=cmd(y,-25;
               --settext,ProductVersion();
               settext,"April 1st edition";
               diffuse,color("#000000");shadowlength,0;);
       };
};
return t;
