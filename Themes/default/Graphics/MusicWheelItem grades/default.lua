-- This actor is duplicated.  Upvalues will not be duplicated.
local grades = {
	Grade_Tier01 = 0,
	Grade_Tier02 = 1,
	Grade_Tier03 = 2,
	Grade_Tier04 = 3,
	Grade_Tier05 = 4,
	Grade_Tier06 = 5,
	Grade_Tier07 = 6,
	Grade_Tier08 = 7,
	Grade_Tier09 = 8,
	Grade_Tier10 = 9,
	Grade_Tier11 = 10,
	Grade_Tier12 = 11,
	Grade_Tier13 = 12,
	Grade_Tier14 = 13,
	Grade_Tier15 = 14,
	Grade_Tier16 = 15,
	Grade_Tier17 = 16,
	Grade_Failed = 17,
	Grade_None = nil
};

return LoadActor("grades")..{
	InitCommand=cmd(pause);
	SetGradeCommand=function(self, params)
		if GAMESTATE:IsCourseMode() then
			self:visible(false);
			return;
		end;
		local state = grades[params.Grade] or grades.Grade_None;
		if state == nil then
			self:visible(false)
		else
			self:visible(true)
			state = state*2
			if params.PlayerNumber == PLAYER_2 then state = state+1 end
			self:setstate(state)
		end
	end;
};