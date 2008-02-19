return LoadFont( "common normal" ) .. {
	Text="XXXX";
	SetCommand=function(self,param)
			local s = DifficultyAndStepsTypeToLocalizedString( param.Difficulty, param.StepsType );
			s = string.upper(s);	-- TODO: string.upper doesn't work correctly for French
			self:settext( s );
			local c = DifficultyAndStepsTypeToColor( param.Difficulty, param.StepsType );
			self:diffuse( c );
		end;
}
