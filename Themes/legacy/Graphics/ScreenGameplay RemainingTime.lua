local PlayerNumber = ...;
assert( PlayerNumber );

local t = LoadFont("ScreenGameplay","RemainingTime") .. {
	Name="SurvivalTime";
	Text="";
};

return t