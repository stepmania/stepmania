local PlayerNumber = ...;
assert( PlayerNumber );

local t = LoadFont("ScreenGameplay","RemainingTime") .. {
	Name="RemainingTime";
	Text="";
	JudgmentMessageCommand=function(self,params)
		if params.Player == PlayerNumber then
			if params.TapNoteScore then
				local tns = ToEnumShortString(params.TapNoteScore)
				local prefname= ("TimeMeterSecondsChange%s"):format(tns)
				if PREFSMAN:PreferenceExists(prefname) then
					self:playcommand( "GainSeconds" );
					self:playcommand( tns );
					self:settextf( "%+1.1fs", PREFSMAN:GetPreference(prefname) );
				end
			else
				return
			end
		else
			return
		end
	end;
};
return t