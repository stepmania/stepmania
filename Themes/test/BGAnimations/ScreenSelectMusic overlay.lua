local t = Def.ActorFrame{};

t[#t+1] = LoadFont("Common Normal")..{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
	--CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	--CurrentStepsChangedP1MessageCommand=cmd(playcommand,"Set");
	OnCommand=cmd(playcommand,"Set");
	SetCommand=function(self,params)
		-- song test cases
		--[[
		local song = GAMESTATE:GetCurrentSong();
		if not song then self:settext(""); return; end;

		-- song file path test case
		self:settext( song:GetSongFilePath() );

		-- get bpm from beat test case
		self:settext( "BPM at beat 45 = "..song:GetBPMAtBeat(45) );
		
		-- lol
		--Trace( "MD5 of ".. song:GetDisplayFullTitle() .." = "..CRYPTMAN:MD5File(song:GetSongFilePath()) );

		-- writing to profile dir!
		local profDir = PROFILEMAN:GetProfileDir('ProfileSlot_Player1') or nil;
		if profDir then
			self:settext(profDir);
			WriteFile(profDir.."hello.txt","hello world! eat my dick, from all your friends at SSC, but aj wrote it so really just him.");
		end;
		]]
	end;
};

return t;