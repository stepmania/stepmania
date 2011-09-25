local t = Def.ActorFrame {};

if( not GAMESTATE:IsCourseMode() ) then
t[#t+1] = Def.Actor{
	JudgmentMessageCommand = function(self, params)
		if params.TapNoteScore and
		   params.TapNoteScore ~= 'TapNoteScore_Invalid' and
		   params.TapNoteScore ~= 'TapNoteScore_None'
		then
			Scoring[GetUserPref("UserPrefScoringMode")](params, 
				STATSMAN:GetCurStageStats():GetPlayerStageStats(params.Player))
		end
	end;
};
end;
return t;