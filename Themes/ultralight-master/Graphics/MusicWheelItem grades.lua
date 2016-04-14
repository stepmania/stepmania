return Def.ActorFrame{
	Font("mentone","24px")..{
		InitCommand=cmd(NoStroke;zoom,0.65;halign,1);
		SetGradeCommand=function(self,params)
			local sGrade = params.Grade or 'Grade_None';
			local gradeString = THEME:GetString("Grade",ToEnumShortString(sGrade))
			self:settext(gradeString)
			self:diffuse(PlayerColor(params.PlayerNumber))
		end;
	};
}