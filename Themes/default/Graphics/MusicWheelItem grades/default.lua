-- This actor is duplicated.  Upvalues will not be duplicated.

local grades = {
	Grade_Tier01 = 0;
	Grade_Tier02 = 1;
	Grade_Tier03 = 2;
	Grade_Tier04 = 3;
	Grade_Tier05 = 4;
	Grade_Tier06 = 5;
	Grade_Tier07 = 6;
	Grade_Failed = 7;
	Grade_None = 8;
};

--[[ local t = LoadActor( "grades" ) .. {
	InitCommand=cmd(pause);
	SetGradeCommand=function(self, params)
		local state = grades[params.Grade] or grades.Grade_None;
		state = state*2;

		if params.PlayerNumber == PLAYER_2 then
			state = state+1;
		end

		self:setstate(state);
	end;
}; --]]
local t = LoadFont("Common Normal") .. {
	InitCommand=cmd(zoom,0.75;shadowlength,1;strokecolor,Color("Black"));
	ShowCommand=cmd(stoptweening;bounceend,0.15;zoomy,0.75);
	HideCommand=cmd(stoptweening;bouncebegin,0.15;zoomy,0);
	SetGradeCommand=function(self,params)
		local pnPlayer = params.PlayerNumber;
		local sGrade = params.Grade or 'Grade_None';
		local gradeString = THEME:GetString("Grade",string.sub(sGrade,7));

		self:settext(gradeString);
		self:diffuse(PlayerColor(pnPlayer));
		self:diffusetopedge(BoostColor(PlayerColor(pnPlayer),1.5));
		self:strokecolor(BoostColor(PlayerColor(pnPlayer),0.25));
		
--[[ 		if sGrade == "Grade_NoTier" then
			self:playcommand("Hide");
		else
			self:playcommand("Show");
		end; --]]
	end;
};

return t;

-- (c) 2007 Glenn Maynard
-- All rights reserved.
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.
