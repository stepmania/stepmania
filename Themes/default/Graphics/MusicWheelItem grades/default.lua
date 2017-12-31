-- This actor is duplicated.  Upvalues will not be duplicated.
local grades = {
	Grade_Tier01 = 0,
	Grade_Tier02 = 1,
	Grade_Tier03 = 2,
	Grade_Tier04 = 3,
	Grade_Tier05 = 4,
	Grade_Tier06 = 5,
	Grade_Tier07 = 6,
	Grade_Failed = 7,
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