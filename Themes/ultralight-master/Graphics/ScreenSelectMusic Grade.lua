local Player = ...
assert(Player,"Must pass in a player, dingus");

local profile;
if PROFILEMAN:IsPersistentProfile(Player) then
	-- player profile
	profile = PROFILEMAN:GetProfile(Player);
else
	-- machine profile
	profile = PROFILEMAN:GetMachineProfile();
end;

return LoadFont("Common normal")..{
	InitCommand=cmd(shadowlength,1;zoom,0.65;strokecolor,color("0.2,0.2,0.2,0.05");halign,Player==PLAYER_1 and 1 or 0);
	BeginCommand=function(self)
		self:visible( GAMESTATE:IsHumanPlayer(Player) );
	end;
	SetCommand=function(self)
		local text = "";
		local SongOrCourse, StepsOrTrail;
		if GAMESTATE:IsCourseMode() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
			StepsOrTrail = GAMESTATE:GetCurrentTrail(Player);
		else
			SongOrCourse = GAMESTATE:GetCurrentSong();
			StepsOrTrail = GAMESTATE:GetCurrentSteps(Player);
		end;
		if SongOrCourse then
			if StepsOrTrail then
				local scorelist = profile:GetHighScoreList(SongOrCourse,StepsOrTrail);
				assert(scorelist);
				local scores = scorelist:GetHighScores();
				local topscore = scores[1];
				text = topscore and THEME:GetString( "Grade",ToEnumShortString(topscore:GetGrade()) ) or "";
			else
				text = "";
			end;
		else
			text = "";
		end;
		self:settext(text);
	end;
	PlayerJoinedMessageCommand=function(self,param)
		if param.Player == Player then
			self:visible(true);
		end;
	end;
	PlayerUnjoinedMessageCommand=function(self,param)
		if param.Player == Player then
			self:visible(false);
		end;
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	CurrentStepsP1ChangedMessageCommand=function(self)
		if Player == PLAYER_1 then self:playcommand("Set"); end;
	end;
	CurrentTrailP1ChangedMessageCommand=function(self)
		if Player == PLAYER_1 then self:playcommand("Set"); end;
	end;
	CurrentStepsP2ChangedMessageCommand=function(self)
		if Player == PLAYER_2 then self:playcommand("Set"); end;
	end;
	CurrentTrailP2ChangedMessageCommand=function(self)
		if Player == PLAYER_2 then self:playcommand("Set"); end;
	end;
};