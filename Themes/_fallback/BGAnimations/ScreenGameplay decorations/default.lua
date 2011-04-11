local t = Def.ActorFrame {};

t[#t+1] = Def.Actor{
	JudgmentMessageCommand = function(self, params)
		SpecialScoring[GetUserPref("UserPrefSpecialScoringMode")](params, 
			STATSMAN:GetCurStageStats():GetPlayerStageStats(params.Player))
	end;
};

return t;