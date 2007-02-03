local children = {};
for idx, diff in pairs(Difficulty) do -- 0, Difficulty_Beginner
	children[#children+1] = LoadActor( "_DifficultyDisplay 6x1" ) .. {
		InitCommand=cmd(pause;setstate,Difficulty:Reverse()[diff]);
		ShowCommand=cmd(stoptweening;linear,0.1;diffusealpha,1);
		HideCommand=cmd(stoptweening;linear,0.1;diffusealpha,0);
		BeginCommand=cmd(y,16*idx);

		SetCommand=function(self)
			local song = GAMESTATE:GetCurrentSong()
			local st = GAMESTATE:GetCurrentStyle():GetStepsType()
			local bHasStepsTypeAndDifficulty =
				song and song:HasStepsTypeAndDifficulty( st, diff );

			self:playcommand( bHasStepsTypeAndDifficulty and "Show" or "Hide" );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	};
end

return Def.ActorFrame {
	children = children
};
