local t = Def.ActorFrame {};

t[#t+1] = Def.Actor{
	JudgmentMessageCommand = function(self, params)
		Scoring[GetUserPref("UserPrefScoringMode")](params, 
			STATSMAN:GetCurStageStats():GetPlayerStageStats(params.Player))
	end;
};

return t;