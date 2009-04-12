local t = LoadFallbackB();

t[#t+1] = StandardDecorationFromFile( "StageFrame", "StageFrame" );

t[#t+1] = LoadActor("_warning") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;
		vertalign,top;
		wag;effectmagnitude,0,0,10;effectperiod,2;
	);
	OnCommand=cmd(diffusealpha,0);
	ShowDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,1);
	HideDangerAllMessageCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,0);
};
	
t[#t+1] = StandardDecorationFromFile( "LifeFrame", "LifeFrame" );
t[#t+1] = StandardDecorationFromFile( "ScoreFrame", "ScoreFrame" );
t[#t+1] = StandardDecorationFromFile( "LeftFrame", "LeftFrame" );
t[#t+1] = StandardDecorationFromFile( "RightFrame", "RightFrame" );


if ShowStandardDecoration("ModIconRows") then
	for pn in ivalues(PlayerNumber) do
		local t2 = Def.ModIconRow {
				InitCommand=cmd(Load,"ModIconRowGameplay"..ToEnumShortString(pn),pn);
			};	
		t[#t+1] = StandardDecorationFromTable( "ModIconRow" .. ToEnumShortString(pn), t2 );
	end
end

return t;
