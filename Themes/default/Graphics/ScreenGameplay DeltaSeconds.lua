local PlayerNumber = ...;
assert( PlayerNumber );

local t = LoadFont("ScreenGameplay","RemainingTime") .. {
	Name="RemainingTime";
	Text="";
	JudgmentMessageCommand=function(self,params)
		if params.Player == PlayerNumber then
			if params.TapNoteScore then 
				self:playcommand( "GainSeconds" );
				self:playcommand( ToEnumShortString( params.TapNoteScore ) );
				self:settextf( "%+1.1fs", PREFSMAN:GetPreference( string.format("TimeMeterSecondsChange%s", ToEnumShortString( params.TapNoteScore ) ) ) );
			else
				return
			end
		else
			return
		end
	end;
};
return t