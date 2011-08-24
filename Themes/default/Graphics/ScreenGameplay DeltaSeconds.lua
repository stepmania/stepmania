local PlayerNumber = ...;
assert( PlayerNumber );

local t = LoadFont("ScreenGameplay","RemainingTime") .. {
	Name="RemainingTime";
	Text="";
	JudgmentMessageCommand=function(self,params)
		if params.Player == PlayerNumber then
			if params.TapNoteScore then
				local tns = ToEnumShortString(params.TapNoteScore)
				self:playcommand( "GainSeconds" );
				self:playcommand( tns );
				self:settextf( "%+1.1fs", PREFSMAN:GetPreference( string.format("TimeMeterSecondsChange%s", tns) ) );
			else
				return
			end
		else
			return
		end
	end;
};
return t